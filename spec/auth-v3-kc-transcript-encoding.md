# AUTH-v3 KC Transcript Encoding

Status: **draft normative work**. This document specifies the byte-level encoding currently implemented by the non-advertised AUTH-v3 Rust and C reference primitives. It does not promote AUTH v3, make `0x03` production-selectable, or supersede the promotion gates in `zk-arche-protocol.md` and `registries.md`.

The purpose of this document is to remove source-code dependence from one security-critical TD-004 surface: an independent implementation should be able to construct the candidate `KC-TRANSCRIPT-v3` bytes and `TH_v3` from this text and the referenced field semantics.

## 1. Scope and authority

This document defines only:

- the transcript domain encoding;
- the per-field framing grammar;
- the exact field labels and order;
- the fixed value widths used by the current suite/profile reference path;
- the byte representation requirements for integer, point, scalar, nonce, identifier, and hash inputs;
- the transcript length and digest operation for the current deterministic reference vector;
- fail-closed requirements for non-canonical or structurally incompatible inputs.

It does not define the canonical inner encoding of `authz_context_hash`, `critical_extensions_hash`, or `channel_binding_hash`. Those hashes are transcript inputs whose underlying canonical context encodings remain governed by their own specifications and promotion gates.

## 2. BCP 14 keyword policy

The key words **MUST**, **MUST NOT**, **REQUIRED**, **SHOULD**, **SHOULD NOT**, and **MAY** in this document are normative only where the behavior is precise and testable. This follows the repository's BCP 14 / RFC-class specification discipline.

## 3. Transcript domain

The AUTH-v3 key-confirmation transcript begins with the ASCII domain string:

```text
zk-arche/kc/v3
```

The domain contains exactly 14 bytes and is encoded as:

```text
u8(domain_length) || domain_bytes
```

Therefore the transcript prefix is:

```text
0x0e || ASCII("zk-arche/kc/v3")
```

No NUL terminator, text encoding marker, or additional length field is present.

An implementation MUST NOT reuse the AUTH-v2 transcript domain for AUTH v3.

## 4. Field framing grammar

Every transcript field after the domain uses this exact framing:

```text
field =
    u8(label_length)
    || label_bytes
    || u32_le(value_length)
    || value_bytes
```

Requirements:

1. `label_bytes` are the exact lowercase ASCII bytes listed in Section 5.
2. `label_length` is the number of bytes in `label_bytes`; it MUST fit in one octet.
3. `value_length` is an unsigned 32-bit little-endian integer containing the number of bytes in `value_bytes`.
4. Fields MUST appear exactly once and in the order listed in Section 5.
5. Fields MUST NOT be reordered, omitted, duplicated, renamed, case-folded, Unicode-normalized, NUL-terminated, or padded.
6. Fixed-width fields MUST use the width listed in Section 5. An implementation MUST fail closed rather than truncate, extend, or reinterpret a value of another width.
7. The transcript contains no enclosing count, no trailing checksum, and no trailing padding.

The grammar is intentionally self-delimiting even though the current fields have fixed widths. The labels and lengths are authenticated transcript bytes and therefore are security-significant.

## 5. Canonical field order and widths

The fields are encoded in the following order.

| # | Label | Width | Value representation |
|---:|---|---:|---|
| 1 | `protocol_version` | 1 | unsigned protocol version octet; AUTH v3 uses `0x03` |
| 2 | `suite_id` | 2 | `u16` little-endian |
| 3 | `profile_id` | 2 | `u16` little-endian |
| 4 | `selected_capabilities` | 8 | `u64` little-endian |
| 5 | `session_id` | 16 | exact session identifier bytes |
| 6 | `authz_context_hash` | 32 | SHA-256 digest bytes of the canonical authorization context selected by the profile |
| 7 | `critical_extensions_hash` | 32 | SHA-256 digest bytes of the canonical selected critical-extension set |
| 8 | `channel_binding_hash` | 32 | SHA-256 digest bytes of the canonical channel-binding context |
| 9 | `pid` | 32 | exact protocol device/pseudonymous identifier bytes used by the current suite |
| 10 | `a_c` | 32 | canonical compressed Ristretto255 point encoding |
| 11 | `s_c` | 32 | canonical scalar encoding for the current Ristretto255 suite |
| 12 | `nonce_c` | 32 | exact client nonce bytes |
| 13 | `eph_c` | 32 | canonical compressed Ristretto255 point encoding |
| 14 | `server_pub` | 32 | canonical compressed Ristretto255 point encoding |
| 15 | `a_s` | 32 | canonical compressed Ristretto255 point encoding |
| 16 | `s_s` | 32 | canonical scalar encoding for the current Ristretto255 suite |
| 17 | `nonce_s` | 32 | exact server nonce bytes |
| 18 | `eph_s` | 32 | canonical compressed Ristretto255 point encoding |

The order above is normative. A parser or builder MUST NOT derive field order from map iteration, struct layout, language reflection, ABI layout, source declaration order, or implementation-specific serialization.

## 6. Integer encoding

`suite_id` and `profile_id` are encoded as unsigned 16-bit little-endian values.

For value `x`:

```text
u16_le(x) = [x & 0xff, (x >> 8) & 0xff]
```

`selected_capabilities` is encoded as an unsigned 64-bit little-endian value:

```text
u64_le(x)[i] = (x >> (8 * i)) & 0xff, for i = 0..7
```

`protocol_version` is a single octet and therefore has no additional endianness rule.

Host byte order, C struct layout, Rust memory layout, and platform ABI MUST NOT affect the transcript bytes.

## 7. Point and scalar encoding

For suite `0x0001`, every Ristretto255 point field in this transcript MUST use the canonical 32-byte compressed Ristretto encoding accepted by the suite's point decoder.

The point fields are:

```text
a_c
eph_c
server_pub
a_s
eph_s
```

A non-canonical, invalid, or identity value that is forbidden by the owning protocol operation MUST be rejected before the value can be treated as an authenticated valid protocol element. Constructing a transcript around malformed bytes MUST NOT convert those bytes into a valid point.

The scalar fields `s_c` and `s_s` MUST use the suite's canonical 32-byte scalar representation. A scalar rejected by the suite's canonical scalar decoder MUST NOT be normalized modulo the group order merely to make it transcript-compatible.

This document fixes how accepted point/scalar values are serialized into the transcript; it does not weaken the independent point/scalar validation requirements in the protocol and implementation specifications.

## 8. Hash, nonce, and identifier inputs

The three context-hash fields are inserted as their raw 32-byte SHA-256 digest outputs. They MUST NOT be hex encoded, base64 encoded, prefixed with an algorithm identifier, or rehashed solely for transcript insertion.

`nonce_c` and `nonce_s` are inserted as their exact 32-byte protocol nonce values.

`session_id` is inserted as its exact 16-byte AUTH-instance identifier.

`pid` is inserted as the exact 32-byte protocol identifier value produced/validated by the current suite path. This field MUST NOT be substituted with a transport address, hostname, socket tuple, device model, or other locator.

## 9. Complete transcript construction

Let:

```text
F(label, value) =
    u8(len(label))
    || ASCII(label)
    || u32_le(len(value))
    || value
```

Then:

```text
KC-TRANSCRIPT-v3 =
    0x0e
    || ASCII("zk-arche/kc/v3")
    || F("protocol_version", protocol_version)
    || F("suite_id", u16_le(suite_id))
    || F("profile_id", u16_le(profile_id))
    || F("selected_capabilities", u64_le(selected_capabilities))
    || F("session_id", session_id)
    || F("authz_context_hash", authz_context_hash)
    || F("critical_extensions_hash", critical_extensions_hash)
    || F("channel_binding_hash", channel_binding_hash)
    || F("pid", pid)
    || F("a_c", canonical_compress(a_c))
    || F("s_c", canonical_scalar(s_c))
    || F("nonce_c", nonce_c)
    || F("eph_c", canonical_compress(eph_c))
    || F("server_pub", canonical_compress(server_pub))
    || F("a_s", canonical_compress(a_s))
    || F("s_s", canonical_scalar(s_s))
    || F("nonce_s", nonce_s)
    || F("eph_s", canonical_compress(eph_s))
```

For the current fixed-width AUTH-v3 reference fields, the resulting transcript length is exactly:

```text
726 bytes
```

A conforming implementation of this candidate encoding MUST produce 726 bytes for the current reference vector and current field set. A different length indicates different labels, framing, order, widths, or inputs and MUST NOT be silently accepted as equivalent AUTH-v3 transcript semantics.

## 10. Transcript hash

The candidate transcript hash is:

```text
TH_v3 = SHA-256(KC-TRANSCRIPT-v3)
```

No additional domain prefix, prehash marker, hexadecimal conversion, or length field is added around the 726-byte transcript before SHA-256.

For the current deterministic reference vector, the retained Rust primitive expects:

```text
TH_v3 = e2b85befd4f3f58b5e880673ce1b27e81de875bf4443d7af6971d811e10439d2
```

This digest is a conformance anchor for the candidate encoding, not evidence that AUTH v3 is production-qualified.

## 11. Mutation and anti-aliasing requirements

Any byte-level mutation to a security-significant field, its label, its encoded length, or its position MUST change the transcript bytes. Except for a SHA-256 collision, such mutation is expected to change `TH_v3`.

Implementations MUST NOT accept any of the following as aliases for the normative representation:

- big-endian integer encodings;
- omitted zero-valued fields;
- reordered fields;
- duplicate fields where one occurrence is ignored;
- shortened fixed-width values with implicit zero extension;
- labels with alternate case or spelling;
- NUL-terminated labels or values when the NUL is not part of the normative value;
- JSON, CBOR, language-native, packed-struct, or ABI serialization substituted for this framing;
- decompressed or alternate point encodings;
- scalar values reduced or normalized from non-canonical encodings;
- textual encodings of binary hashes, nonces, identifiers, points, or scalars.

Negative conformance tests SHOULD mutate each independently representable dimension above. Rust and C MUST agree on the resulting transcript bytes and accept/reject decisions wherever both claim AUTH-v3 reference support.

## 12. Relationship to negotiation and lifecycle state

Correct transcript encoding is necessary but not sufficient for AUTH acceptance.

A peer MUST still independently enforce:

- negotiated version/suite/profile compatibility;
- critical-extension compatibility;
- authorization-context validity and freshness;
- replay/restart/key-usage continuity;
- local trust and NO-LEARNING rules;
- channel-binding requirements;
- AUTH message ordering and state transitions;
- point/scalar/proof validation;
- Finished and authenticated-completion verification.

A byte-correct `KC-TRANSCRIPT-v3` MUST NOT be treated as proof that any of these lifecycle predicates succeeded.

## 13. Promotion and evidence boundary

This document converts existing Rust/C transcript-builder behavior into independently implementable normative text. It does not complete TD-004 by itself.

AUTH v3 remains non-advertised and non-selectable until the wider promotion gate has evidence for, at minimum:

- canonical authorization-context encoding;
- canonical critical-extension-set encoding and empty representation;
- canonical channel-binding context(s) and empty representation where permitted;
- complete AUTH-v3 packet/wire grammar;
- state-machine and error/privacy behavior;
- positive and negative Rust/C vectors covering this transcript grammar;
- negotiation/downgrade qualification;
- lifecycle replay/restart/rekey/resumption requirements;
- formal-analysis scope appropriate to promoted claims;
- required external cryptographic review and dispositioned findings;
- constrained-target evidence for any constrained-profile maturity claim.

The authoritative implementation anchors for this candidate encoding are currently the non-advertised Rust and C AUTH-v3 reference primitives. If those implementations and this specification disagree, production promotion MUST remain blocked until the discrepancy is explicitly reviewed and reconciled; source code does not silently override the normative package.
