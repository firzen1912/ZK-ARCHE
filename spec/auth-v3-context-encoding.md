# AUTH-v3 Canonical Context Encoding v1

Status: **draft normative work**. This encoding is non-advertised and exists to remove ambiguity beneath the three AUTH-v3 context hashes. It does not promote AUTH v3, allocate new authorization fields or channel-binding methods, or enable production negotiation.

## 1. Purpose

AUTH v3 authenticates three 32-byte digests:

- `authz_context_hash`;
- `critical_extensions_hash`;
- `channel_binding_hash`.

Each digest MUST be computed as SHA-256 over one byte-exact canonical context encoding defined here. Implementations MUST NOT hash implementation-native structs, maps, JSON, CBOR, pointer layouts, unordered collections, or decoder-normalized objects unless a future versioned specification explicitly replaces this encoding.

This design follows the strict deterministic-encoding and duplicate-rejection discipline highlighted by RFC 8949 and RFC 9052 while retaining a small custom fixed-width envelope suitable for bounded C implementations. CBOR/COSE are references, not dependencies.

## 2. Canonical envelope

Every context is encoded as:

```text
magic             5 bytes   ASCII "ZKCTX"
encoding_version  u8        = 0x01
context_kind      u8
entry_count       u16 LE
entries           repeated entry_count times
```

Total empty-context length is 9 bytes.

`context_kind` values for encoding v1 are:

| Value | Name | Hash destination |
|---:|---|---|
| `0x01` | `AUTHORIZATION` | `authz_context_hash` |
| `0x02` | `CRITICAL_EXTENSIONS` | `critical_extensions_hash` |
| `0x03` | `CHANNEL_BINDING` | `channel_binding_hash` |

All other kind values are invalid for encoding v1.

Each entry is:

```text
id        u16 LE
flags     u8       = 0x00 in encoding v1
value_len u16 LE
value     value_len bytes
```

The v1 `flags` octet is reserved. An encoder MUST emit zero. A parser MUST reject a non-zero value instead of silently masking it.

## 3. Canonical ordering and uniqueness

Entries MUST appear in strictly increasing numeric `id` order.

Consequences:

- duplicate IDs are invalid;
- descending or unsorted IDs are invalid;
- an encoder MUST NOT silently sort malformed caller input unless its public API clearly performs a separate semantic normalization step before invoking this canonical encoder;
- a decoder MUST NOT use first-one-wins or last-one-wins duplicate handling;
- a parser MUST reject trailing bytes after the declared final entry.

Encoding v1 permits one entry per ID. If an extension needs multiplicity, its specification MUST encode that multiplicity canonically inside the single entry value or define a future versioned encoding. This avoids duplicate-ID ambiguity on constrained parsers.

`id = 0x0000` is invalid in every v1 context.

## 4. Authorization-context entries

For `context_kind = AUTHORIZATION`, entry IDs and value schemas are defined by the selected protocol/security profile or a future shared Authorization Field Registry.

This document intentionally does not assign authorization semantics to IDs. The fixture IDs in `context-encoding-v1.json` are test-only values and MUST NOT be interpreted as registry allocations.

A selected profile MUST define, before production promotion:

- which authorization entry IDs are mandatory, optional, or forbidden;
- exact value encoding for each ID;
- holder/session confirmation semantics;
- audience/deployment/domain semantics where applicable;
- role/policy/scope semantics;
- validity, generation, policy epoch, or revocation epoch semantics where applicable;
- whether the empty authorization context is legal. If authorization is required, the empty encoding MUST fail policy validation even though it has a deterministic hash.

Canonical encoding and authorization validity are separate checks: a byte string can be canonically encoded yet still be unauthorized under the selected profile.

## 5. Critical-extension entries

For `context_kind = CRITICAL_EXTENSIONS`:

- each entry `id` is the full 16-bit critical-extension occurrence identifier from `spec/registries.md`;
- bit 15 MUST be set;
- the base ID (`id & 0x7FFF`) MUST be non-zero;
- the value MUST be the extension specification's canonical security-relevant value encoding;
- all negotiated critical extensions active for the AUTH instance MUST be represented exactly once;
- unknown critical IDs MUST be rejected before AUTH completion;
- an extension whose value cannot be canonicalized MUST cause failure rather than omission from the hash.

The empty critical-extension context is canonical and means no critical extension is active. It is not permission to ignore a critical extension observed elsewhere in negotiation.

Unknown non-critical advertisements are not encoded in this critical-only context unless a future profile explicitly promotes them into authenticated security semantics.

## 6. Channel-binding entries

For `context_kind = CHANNEL_BINDING`, entry IDs come from the Channel-Binding Registry in `spec/registries.md`.

Encoding v1 allows at most one channel-binding entry for a profile unless a later profile explicitly defines composition inside one canonical value. The entry value is the exact canonical binding context defined by that binding specification.

The empty channel-binding context is the sole v1 representation of **no channel binding**. Implementations MUST hash the empty encoding below; they MUST NOT substitute 32 zero bytes, an empty byte string, a null pointer, or an implementation-specific sentinel for `channel_binding_hash`.

## 7. Canonical empty encodings

These are distinct because `context_kind` is authenticated inside the bytes:

```text
AUTHORIZATION empty
5a4b43545801010000
SHA-256 = 505121c6096720d111eab443818cc974bb66f3339e06de742f69e4692dd2717a

CRITICAL_EXTENSIONS empty
5a4b43545801020000
SHA-256 = ef8116870a7dc594749827eae3c9a5346057612b0d93ed3d1f0cea3d6ff0f3ed

CHANNEL_BINDING empty
5a4b43545801030000
SHA-256 = 7f724afa7e3e7a6c13e0fe167fc48a034888d10c523abd7864671c68aaea5fa8
```

A hash for one context kind MUST NOT be reused in another kind.

## 8. Parse/validate/encode/hash pipeline

A conformant implementation applies this order:

```text
input semantic/context entries
        ↓
profile/registry validation
        ↓
ID validity + strict-order + duplicate validation
        ↓
kind-specific validation
        ↓
canonical encode
        ↓
SHA-256(canonical bytes)
        ↓
AUTH-v3 security-context field
```

A receiver that obtains raw encoded context bytes performs strict parse and validation before using their hash in an authorization decision. Hash equality does not excuse malformed, duplicate, unknown-critical, non-canonical, or profile-forbidden semantics.

## 9. Bounded implementation contract

Encoding v1 is designed so constrained implementations can operate with caller-owned fixed buffers.

A profile MUST define practical limits for:

- maximum entry count;
- maximum value length per registered entry;
- maximum total canonical context bytes.

The generic encoding permits lengths representable by `u16`, but a constrained profile MAY define substantially lower limits. Exceeding a profile limit MUST fail closed; silently truncating an entry or context is forbidden.

An implementation may stream the canonical encoding directly into SHA-256 if it also provides byte-exact conformance evidence that the streamed bytes are identical to the retained vectors.

## 10. Deterministic conformance corpus

The canonical positive/negative corpus is:

```text
rust/test-vectors/auth-v3/context-encoding-v1.json
```

At minimum both Rust and C MUST reproduce:

- all three canonical empty encodings and hashes;
- a multi-entry authorization fixture;
- a multi-entry critical-extension fixture;
- a channel-binding fixture;
- rejection of ID zero;
- rejection of duplicate IDs;
- rejection of descending IDs;
- rejection of a critical-extension ID without bit 15 set.

Future negative coverage before production AUTH-v3 promotion MUST additionally cover malformed lengths, non-zero reserved flags, unknown context kinds, trailing bytes, unknown critical extensions at negotiation validation, profile-forbidden authorization fields, and channel-binding multiplicity violations.

## 11. Formal-analysis boundary

The current draft AUTH-v3 ProVerif model treats authorization context, critical-extension context, and channel-binding context as symbolic bitstrings. Its retained TRUE correspondence results therefore assume collision-resistant and semantically unambiguous context construction; they do not prove this parser/canonicalization contract.

Until canonical parsers/encoders and cross-language negative tests are retained, formal results MUST be reported with canonicalization as an external assumption.

## 12. Promotion boundary

This specification closes the byte-envelope and canonical-empty ambiguity, but it does not complete AUTH-v3 promotion. Production v3 remains disabled until selected profiles define their actual authorization field schemas, critical-extension value schemas, channel-binding methods, replay epoch/lifetime semantics, production state machines, constrained measurements, and the remaining promotion gates in `spec/zk-arche-protocol.md`.
