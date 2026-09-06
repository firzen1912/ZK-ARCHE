# ZK-ARCHE Critical Extension Set Encoding

Status: **draft normative work**. This document defines the canonical byte representation hashed into `AUTH-v3.security_context.critical_extensions_hash`. It does not allocate any extension identifier, advertise AUTH v3, or make a draft profile selectable.

## 1. Purpose and boundary

AUTH v3 authenticates `critical_extensions_hash = SHA-256(critical_extension_set)`. A conformant implementation MUST derive that hash from the canonical encoding below and MUST NOT hash a language-native structure, transport metadata, parser order, or an implementation-specific serialization.

The extension registry and critical/ignorable semantics remain owned by `spec/registries.md`. This document only defines the authenticated set representation.

## 2. Canonical occurrence

Each negotiated extension occurrence is encoded as:

```text
wire_id       u16 little-endian
value_len     u16 little-endian
value         value_len octets
```

`wire_id` is the complete occurrence identifier, including bit 15 as the critical flag and bits 0..14 as the registered base identifier.

`base_id = wire_id & 0x7fff` MUST NOT be zero. `value_len` MUST equal the exact number of following value octets and MUST NOT exceed 65535.

The value bytes are defined by the extension's own normative specification. If an extension has no semantic value bytes, its registered specification MUST explicitly define the zero-length value as canonical; implementations MUST NOT invent padding or sentinel bytes.

## 3. Canonical set

The complete set is encoded as:

```text
count         u16 little-endian
occurrence_0
occurrence_1
...
occurrence_(count-1)
```

Occurrences MUST be sorted by unsigned numeric `wire_id` in strictly increasing order before hashing. Duplicate `wire_id` values are therefore non-canonical and MUST be rejected unless a future extension registration explicitly replaces this set format with a versioned multiplicity rule. This revision defines no such exception.

The canonical empty set is exactly two octets:

```text
00 00
```

Therefore the no-extension AUTH-v3 context uses:

```text
critical_extensions_hash = SHA-256(0x0000)
```

and not SHA-256 of an empty byte string, an all-zero digest, a null pointer representation, or an omitted field.

## 4. Selection and unknown-value rules

Canonical encoding does not make an extension acceptable. Before an occurrence enters the selected set, the implementation MUST apply the selected version/profile registry policy.

An unknown critical occurrence MUST fail closed before AUTH completion and MUST NOT be included as though it were understood. An unknown non-critical occurrence MAY be ignored only where the selected version/profile permits ignorance in that location; an ignored occurrence is not part of the selected authenticated set unless its specification explicitly defines otherwise.

Known-but-incompatible, reserved, draft-not-promoted, malformed, duplicate, or profile-forbidden occurrences MUST NOT be normalized into an acceptable set merely because they can be encoded.

## 5. Transcript and downgrade requirements

The canonical bytes are hashed once with SHA-256 and the resulting 32-byte digest occupies `critical_extensions_hash` in the AUTH-v3 security context. Consequently, changing any selected `wire_id`, critical bit, canonical value byte, value length, occurrence count, or selected-set membership MUST change the authenticated context except for a cryptographic hash collision.

Peers MUST NOT negotiate one extension set and execute another. Parser arrival order MUST NOT affect the canonical digest. Reordering the same valid set before canonical sorting MUST produce the same digest; changing membership or semantics MUST not.

An implementation MUST fail closed if it cannot canonicalize the selected set uniquely.

## 6. Bounded implementation requirements

A constrained implementation MAY canonicalize using a fixed-capacity array or streaming preparation, but it MUST enforce a profile-defined maximum occurrence count and total encoded length before allocation or copying. Resource exhaustion MUST fail closed without weakening critical-extension processing.

Profiles claiming constrained Common Contract support MUST publish their maximum selected extension count and encoded-byte budget. A high-capability peer MUST respect the constrained peer's negotiated bound rather than requiring an unbounded extension parser.

## 7. Required conformance evidence

Before AUTH v3 or a profile using this encoding is promoted, Rust and C MUST share positive/negative evidence covering at least:

1. canonical empty set `00 00` and its SHA-256 digest;
2. one known critical occurrence;
3. two known occurrences supplied in opposite parser order but producing identical canonical bytes/hash;
4. changed critical bit changes the digest or fails compatibility policy;
5. changed value changes the digest;
6. changed membership changes the digest;
7. duplicate `wire_id` rejection;
8. `base_id = 0` rejection;
9. truncated `value_len` rejection;
10. trailing-byte/non-canonical representation rejection;
11. unknown critical occurrence fail-closed behavior;
12. known-but-profile-incompatible occurrence rejection;
13. profile maximum-count/maximum-length overflow rejection;
14. v3 context mutation proving a changed extension-set digest invalidates Finished/completion verification.

Positive encoding vectors MUST contain intermediate canonical bytes and the resulting SHA-256 value so an independent implementation can reproduce the result without reading Rust or C source.

## 8. Claim boundary

This specification closes only the ambiguity of the critical-extension-set byte representation and canonical empty value. It does **not** establish production AUTH-v3 support, allocate extension IDs, prove parser safety, supply Rust/C vectors, provide formal analysis, or satisfy the complete TD-004/RFC-class gate. Those claims require their own retained evidence.
