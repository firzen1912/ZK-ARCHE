# ZK-ARCHE Registry and Allocation Specification

Status: **draft normative work**. This document records current wire-stable allocations, draft-only AUTH-v3 allocations, registry ownership, unknown-value behavior, and change-control rules. It does not advertise AUTH v3, promote a draft profile, or claim IETF/IANA registration.

ZK-ARCHE registries are repository-managed. Their structure follows the change-control discipline expected by RFC 8126-style registries: values have explicit ownership, status, allocation rules, and compatibility behavior; assigned values are not silently reused.

## 1. Registry authority and status vocabulary

A registry entry has one of these states:

| State | Meaning |
|---|---|
| `stable` | implemented wire behavior or a reviewed compatibility allocation that MUST NOT be renumbered |
| `draft` | reserved for a candidate specification or conformance vector; MUST NOT be advertised/selectable unless its promotion gate is satisfied |
| `reserved` | intentionally unavailable for ordinary allocation |
| `experimental` | isolated testing/research value with no interoperability claim outside the experiment |
| `deprecated` | retained for decoding/compatibility where specified but MUST NOT be selected for new sessions |
| `private-use` | deployment-local value; interoperability outside the private deployment is not implied |
| `unassigned` | no semantics are registered |

Registry changes that alter security semantics, wire interpretation, transcript inputs, downgrade behavior, privacy behavior, or cross-language compatibility require checkpoint-style review. A new entry is not complete merely because a number was added to this file.

For a security- or wire-relevant allocation to become `stable`, retain at minimum:

```text
normative semantics
+ compatibility / downgrade rule
+ Rust/C constant or parser support where applicable
+ positive vector/test
+ negative unknown/incompatible-value test
+ versioned change record
```

Cryptographic suite changes additionally require the review/evidence appropriate to the primitive or proof behavior being introduced.

Assigned values MUST NOT be recycled for unrelated semantics. A removed feature is marked `deprecated` or `reserved`; its number remains unavailable for reassignment.

## 2. Production, draft, experimental, and private ranges

Unless a registry below defines a narrower policy:

- value zero is reserved for `invalid` / `not selected` where the field permits such a sentinel;
- values explicitly listed as `stable` or `draft` are repository-controlled;
- unlisted values have no ZK-ARCHE semantics;
- private-use values MUST NOT be emitted by a Common Contract conformance test unless that test explicitly declares the private deployment contract;
- experimental values MUST NOT silently become production allocations;
- deterministic anti-ossification/GREASE fixtures MAY exercise deliberately unassigned values, but those values MUST remain unassigned and MUST NOT acquire semantics because a test used them.

The exact experimental/private/GREASE subranges for a registry are defined only when that registry needs them. Do not infer one registry's range policy for another registry.

## 3. Protocol Version Registry (`u8`)

The packet-header `version` field and negotiated protocol version use this registry.

| Value | Name | State | Semantics |
|---:|---|---|---|
| `0x00` | INVALID | reserved | MUST NOT be emitted as a negotiated protocol version |
| `0x01` | LEGACY-RESERVED | reserved | below the current minimum supported version; no current ZK-ARCHE semantics |
| `0x02` | ZK-ARCHE-v2 | stable | current deployed compatibility behavior |
| `0x03` | ZK-ARCHE-AUTH-v3 | draft | candidate AUTH-v3 semantics from ADR 0001; non-advertised/non-selectable until its promotion gate passes |
| `0x04`–`0xEF` | — | unassigned | require reviewed allocation before use |
| `0xF0`–`0xFE` | — | reserved | held for deterministic unknown-version / anti-ossification testing; MUST NOT be assigned normal semantics |
| `0xFF` | INVALID | reserved | MUST NOT be negotiated |

Current Rust/C production implementations advertise `0x02` and use `0x02` as the minimum supported version. Merely receiving a syntactically valid `0x03` header does not authorize AUTH-v3 processing.

If negotiation selects version `V`, every security-relevant state machine for that session MUST execute semantics defined for `V`. A peer MUST fail closed rather than reinterpret a selected v3 session as v2.

An unknown or unimplemented selected version results in the registered unsupported-version error. Implementations MAY parse enough header state to return an error safely, but parsing is not acceptance.

## 4. Cryptographic Suite Registry (`u16`)

Suite IDs identify the cryptographic bundle used by the selected protocol/profile. A suite number is security-significant and, where the protocol version requires it, MUST be transcript/context bound.

| Value | Name | State | Current meaning |
|---:|---|---|---|
| `0x0000` | INVALID | reserved | no suite selected |
| `0x0001` | RISTRETTO255-SHA256 | stable | current implemented suite: Ristretto255-based operations with the repository's current SHA-256/HKDF-SHA256/HMAC-SHA256 key-confirmation bundle and versioned proof/transcript rules |
| `0x0002` | RISTRETTO255-SHA512-CANDIDATE | reserved | documented placeholder only; not implemented and MUST NOT be advertised |
| `0x0003`–`0xEFFF` | — | unassigned | require reviewed suite definition and interoperability evidence |
| `0xF000`–`0xFEFF` | — | reserved | unknown-suite / anti-ossification test space; MUST NOT acquire stable semantics |
| `0xFF00`–`0xFFFE` | — | private-use | deployment-local experiments only |
| `0xFFFF` | INVALID | reserved | MUST NOT be selected |

Suite `0x0001` is the only production-implemented suite at this revision.

A peer MUST NOT select a suite it did not advertise as supported. A known suite that is incompatible with the selected protocol version/profile MUST be rejected just as an unsupported suite is rejected; “known” does not mean “allowed here.”

A future suite registration MUST state at least:

```text
key-agreement / shared-secret mechanism
proof / authentication method compatibility
hash functions and exact purpose
KDF and labels
authenticator / MAC / AEAD use
canonical key and group-element encoding
transcript/domain-separation rules
key-usage / lifecycle limits
method/profile compatibility
constrained resource evidence where claimed
external-review status where custom cryptography is involved
```

## 5. Protocol/Security Profile Registry (`u16`)

`profile_id` is a **protocol/security profile identifier**. It is not a local memory/timing preset and is not the same namespace as the legacy capability bits named `PROFILE_MINIMAL`, `PROFILE_STANDARD`, or `PROFILE_GATEWAY`.

| Value | Name | State | Intended scope |
|---:|---|---|---|
| `0x0000` | NO-PROFILE | reserved | invalid for AUTH v3; a v3 security context MUST identify a selected profile |
| `0x0001` | `iot-core` | draft | constrained Common Contract candidate for MCU-class peers |
| `0x0002` | `iot-edge` | draft | capable edge/gateway candidate with optional richer services |
| `0x0003` | `p2p-iot-core` | draft | infrastructure-independent, bidirectional constrained P2P Common Contract candidate |
| `0x0004`–`0x7EFF` | — | unassigned | require normative profile semantics and evidence |
| `0x7F00`–`0x7FFF` | — | experimental | repository research profiles only; no Common Contract claim |
| `0x8000`–`0xFFFE` | — | private-use | deployment-local profiles |
| `0xFFFF` | INVALID | reserved | MUST NOT be selected |

All currently named protocol/security profiles remain `draft`; none of the numeric profile IDs above is production-selectable solely because it appears in this registry.

The draft AUTH-v3 reference vector uses `profile_id = 0x0001` to exercise the candidate `iot-core` encoding path. That vector is still `draft-reference-only`; it is not proof that `iot-core` requirements are complete or that v3 may be advertised.

A profile promotion MUST define:

- mandatory protocol version(s) and suite(s);
- mandatory and forbidden capability semantics;
- allowed/required critical extensions;
- resource ceilings or evidence requirements;
- replay lifetime/epoch behavior;
- authorization-context schema;
- channel-binding policy;
- transport assumptions that affect protocol behavior;
- error/privacy behavior;
- Rust/C conformance vectors and negative cases.

Local implementation presets MAY be named similarly for operator convenience, but they MUST NOT be serialized into `profile_id` unless an explicit mapping is normatively defined.

## 6. Capability Bit Registry (`u64`)

HELLO capability advertisements currently use a 64-bit bitmap. Bits `0..31` are protocol-managed; bits `32..63` remain vendor/private space in the existing implementation contract.

### 6.1 Existing protocol-managed bits

| Bit | Mask | Name | State | Scope |
|---:|---:|---|---|---|
| 0 | `0x0000000000000001` | `AUTH_V2` | stable | supports the current v2 online AUTH flow |
| 1 | `0x0000000000000002` | `ROLE_RERAND` | stable | role-commitment re-randomization proof support |
| 2 | `0x0000000000000004` | `ROLE_SET_MEMBERSHIP` | stable | role set-membership proof support |
| 3 | `0x0000000000000008` | `PAIRING_TOKEN` | stable | pairing-token-gated setup support |
| 4 | `0x0000000000000010` | `TOFU_SETUP` | stable | lab-mode TOFU setup support; not a production trust guarantee |
| 5–7 | — | — | unassigned | no semantics |
| 8 | `0x0000000000000100` | `PROFILE_MINIMAL` | stable legacy advertisement | implementation/runtime preset marker; **not** a protocol `profile_id` |
| 9 | `0x0000000000000200` | `PROFILE_STANDARD` | stable legacy advertisement | implementation/runtime preset marker; **not** a protocol `profile_id` |
| 10 | `0x0000000000000400` | `PROFILE_GATEWAY` | stable legacy advertisement | implementation/runtime preset marker; **not** a protocol `profile_id` |
| 11–15 | — | — | unassigned | no semantics |
| 16 | `0x0000000000010000` | `CBOR_FRAMING` | stable advertisement | framing capability; not automatically an AUTH security-semantic selection |
| 17–31 | — | — | unassigned | no semantics |

Bits `32..63` are vendor/private advertisement bits. They MUST NOT change Common Contract security semantics unless a reviewed, interoperable extension promotes equivalent semantics into a protocol-managed registry.

### 6.2 Raw advertisement vs selected security capabilities

Raw HELLO advertisements and `AUTH-v3.selected_capabilities` are different objects.

For v2, the current implementation intersects capability advertisements and requires its baseline bit set. This historical behavior is preserved for v2 compatibility.

For draft v3:

- `selected_capabilities` MUST contain only protocol-managed, registry-known capabilities whose semantics are explicitly active for the selected v3 profile;
- runtime-profile markers (`PROFILE_MINIMAL`, `PROFILE_STANDARD`, `PROFILE_GATEWAY`) MUST NOT appear in the v3 selected security-capability set;
- vendor/private bits MUST NOT appear in a Common Contract selected set;
- unknown bits MUST NOT be silently promoted into the selected set merely because both peers advertised them;
- a security-critical behavior that cannot be represented by a registered selected capability MUST use a registered critical extension or cause negotiation to fail closed.

No dedicated `AUTH_V3` capability bit is allocated by this revision. Protocol version `0x03`, a promoted profile, and its normative mandatory-capability rules will determine v3 eligibility unless a later reviewed change explicitly allocates such a bit.

The current draft AUTH-v3 primitive vector uses `selected_capabilities = 0x0000000000000005` only as a deterministic context-encoding input. That value includes legacy `AUTH_V2` plus `ROLE_SET_MEMBERSHIP`; it is **not** a production-valid AUTH-v3 selected-capability set and MUST be regenerated before production promotion if the final v3 selection rules differ.

## 7. Packet / Message Type Registry (`u8`)

These current type values are wire-stable and MUST NOT be reassigned:

| Value | Name | State | Direction / purpose |
|---:|---|---|---|
| `0x01` | `HELLO` | stable | initiator/client capability/version probe |
| `0x02` | `HELLO_REPLY` | stable | responder/server negotiation reply |
| `0x11` | `SETUP_1` | stable | setup initiation |
| `0x12` | `SETUP_2` | stable | setup challenge/server proof |
| `0x13` | `SETUP_3` | stable | client setup proof |
| `0x14` | `SETUP_ACK` | stable | setup completion |
| `0x21` | `AUTH_1` | stable | AUTH initiation |
| `0x22` | `AUTH_2` | stable | server AUTH response / server Finished |
| `0x23` | `AUTH_3` | stable | client Finished |
| `0x24` | `AUTH_ACK` | stable type | v2 payload is one-byte ACK; draft v3 reuses the type with version-disambiguated 32-byte authenticated completion semantics |
| `0x7F` | `ERROR` | stable | structured wire error |

All other packet-type values are unassigned unless a future registry update says otherwise.

An implementation receiving an unassigned packet type MUST reject it with the registered unknown-packet-type behavior unless the selected protocol version/profile explicitly defines an extension envelope that permits otherwise.

A packet-type number MAY have version-specific payload semantics only when the header version unambiguously selects those semantics and downgrade/cross-version tests exist. AUTH_ACK is the current draft example: the type remains `0x24`, while v2 and draft-v3 payloads are intentionally different and version separated.

## 8. Header Flag Registry (`u16`)

| Value | Name | State | Semantics |
|---:|---|---|---|
| `0x0000` | `NONE` | stable | no flag |
| `0x0001` | `RETRANSMIT` | stable | sender marks retransmission; receiver applies idempotent/session-specific retransmission rules |
| all other bits | — | unassigned | MUST be zero unless selected version/profile explicitly registers them |

Unknown header flag bits MUST NOT silently change security semantics. Before a future flag is allowed in Common Contract traffic, define whether unknown peers reject it or can safely ignore it and add negative interoperability tests.

## 9. HELLO TLV Tag Registry (`u16`)

The current v2 HELLO/TLV codec uses these stable tags:

| Value | Name | State | Semantics |
|---:|---|---|---|
| `0x0001` | `MIN_VERSION` | stable | minimum protocol version advertisement |
| `0x0002` | `SUITE_LIST` | stable | supported suite list |
| `0x0003` | `CAPS` | stable | 64-bit capability advertisement |
| `0x0004` | `MTU_HINT` | stable | transport/message-size hint |
| `0x0100` | `VENDOR_ID` | stable legacy tag | vendor metadata |
| `0x0101` | `DEVICE_MODEL` | stable legacy tag | device-model metadata |

The existing v2 codec skips unknown TLV tags. That behavior is preserved for v2 compatibility and MUST NOT be interpreted as a generic rule that unknown **security-critical** negotiation data is safe to ignore.

Security-critical v3 extensibility uses the Critical Extension Registry below rather than relying on v2's ignorable-unknown TLV behavior.

## 10. Critical Extension Registry (`u16` occurrence identifier)

AUTH-v3 and future Common Contract work need explicit critical-vs-ignorable semantics. This registry is separate from HELLO TLV tags.

An extension occurrence identifier is a `u16`:

```text
bit 15      critical flag
bits 0..14  extension base id
```

Define:

```text
critical = (wire_id & 0x8000) != 0
base_id  = wire_id & 0x7FFF
```

`base_id = 0` is reserved and invalid.

No extension base ID is stable in this revision. The roadmap names candidate behaviors such as `AUTH_RETRY`, encrypted lookup hints, resumption, channel binding, remote attestation, PQ hybrids, and DATA, but names in a roadmap are not numeric allocations.

Unknown extension behavior is normative:

- unknown **critical** extension: the peer MUST fail closed before AUTH completion;
- unknown non-critical extension: the peer MAY ignore it only when the selected version/profile permits ignorable extensions in that location;
- an extension that changes authentication, authorization, trust, transcript, downgrade, key schedule, replay, or channel-binding semantics MUST be critical unless a future proof/spec explicitly establishes safe ignorance;
- duplicate critical extension occurrences MUST be rejected unless that extension's specification explicitly defines multiplicity and canonical ordering;
- a known extension that is forbidden by the selected profile MUST be rejected even if its syntax is valid;
- critical extension selection and security-relevant values MUST be covered by the AUTH-v3 `critical_extensions_hash` once its canonical set encoding is specified.

Allocation policy for extension base IDs:

| Base-ID range | Policy |
|---|---|
| `0x0001`–`0x5FFF` | repository specification required + checkpoint review + Rust/C negative/positive conformance evidence |
| `0x6000`–`0x6FFF` | private-use extensions; no Common Contract claim |
| `0x7000`–`0x77FF` | experimental/research-only; MUST NOT become stable without explicit reallocation/promotion |
| `0x7800`–`0x7FFF` | reserved for deterministic unknown-extension / anti-ossification fixtures; MUST remain unassigned |

The exact canonical byte encoding of the selected critical-extension set remains a TD-004 item. Until that encoding and its empty representation have normative vectors, production AUTH v3 remains disabled.

## 11. Transport / Channel-Binding Registry (`u16`)

This registry identifies the **semantic binding type**, not the transport adapter itself. ZK-ARCHE identity MUST NOT be derived merely from a UDP address, TCP connection, BLE address, CAN identifier, serial port, or other transport locator.

| Value | Name | State | Semantics |
|---:|---|---|---|
| `0x0000` | `NO-CHANNEL-BINDING` | draft | may be selected only by a profile that explicitly permits no channel binding and defines the canonical empty binding context |
| `0x0001`–`0x7EFF` | — | unassigned | future reviewed binding definitions, e.g. exporter/context-based bindings |
| `0x7F00`–`0x7FFF` | — | experimental | research bindings only |
| `0x8000`–`0xFFFE` | — | private-use | deployment-local binding semantics |
| `0xFFFF` | INVALID | reserved | MUST NOT be selected |

A future binding allocation MUST define:

- exact context bytes and canonical encoding;
- endpoint/application/deployment identity inputs where relevant;
- freshness / AUTH-instance inputs;
- proxy/termination/resumption assumptions where relevant;
- cross-session and cross-protocol negative tests;
- whether absence of the binding is permitted by each profile.

`channel_binding_hash` in AUTH v3 is the SHA-256 digest of the selected binding's canonical context, not a hash of arbitrary transport metadata.

The current draft AUTH-v3 reference vector uses a synthetic `0x33...33` channel-binding hash solely to exercise transcript binding. It is not a registered channel-binding context.

## 12. Alert / Error Code Registry (`u16`)

Wire errors use `u16` little-endian codes in the ERROR payload. The high byte groups the category.

### 12.1 Wire-stable categories

| Range | Category |
|---|---|
| `0x0100`–`0x01FF` | version / capability |
| `0x0200`–`0x02FF` | framing / parsing |
| `0x0300`–`0x03FF` | cryptographic validation |
| `0x0400`–`0x04FF` | session / replay |
| `0x0500`–`0x05FF` | authorization |
| `0x0600`–`0x06FF` | rate limiting / resource |
| `0x0700`–`0x07FF` | storage / backend |
| `0x7FFF` | unspecified / internal wire error |

### 12.2 Current wire-stable codes

| Value | Name |
|---:|---|
| `0x0101` | `UNSUPPORTED_VERSION` |
| `0x0102` | `UNSUPPORTED_SUITE` |
| `0x0103` | `CAPABILITY_MISMATCH` |
| `0x0201` | `MALFORMED_PACKET` |
| `0x0202` | `UNKNOWN_PACKET_TYPE` |
| `0x0203` | `PAYLOAD_TOO_LARGE` |
| `0x0204` | `PAYLOAD_TOO_SHORT` |
| `0x0205` | `INVALID_ENCODING` |
| `0x0301` | `INVALID_POINT` |
| `0x0302` | `NON_CANONICAL_SCALAR` |
| `0x0303` | `IDENTITY_POINT` |
| `0x0304` | `PROOF_VERIFY_FAILED` |
| `0x0305` | `KEY_CONFIRM_FAILED` |
| `0x0306` | `PEER_KEY_MISMATCH` |
| `0x0401` | `UNKNOWN_SESSION` |
| `0x0402` | `SESSION_EXPIRED` |
| `0x0403` | `REPLAY_DETECTED` |
| `0x0404` | `SEQUENCE_OUT_OF_ORDER` |
| `0x0501` | `UNKNOWN_DEVICE` |
| `0x0502` | `DEVICE_NOT_ENROLLED` |
| `0x0503` | `ROLE_NOT_PERMITTED` |
| `0x0504` | `PAIRING_TOKEN_INVALID` |
| `0x0601` | `RATE_LIMITED` |
| `0x0602` | `SERVER_BUSY` |
| `0x0603` | `TOO_MANY_ACTIVE` |
| `0x0701` | `STORAGE_FAILURE` |
| `0x0702` | `CREDENTIAL_MISSING` |
| `0x0703` | `REGISTRY_CORRUPT` |
| `0x7FFF` | `UNSPECIFIED` |

C also defines local process/API errors `0x0001`–`0x0005`. Those are **not wire-registry allocations** and MUST NOT be serialized as protocol errors solely because they share the `auth_err_t` type.

Unknown received wire codes are interpreted as `UNSPECIFIED` by the current implementations. New code allocations MUST preserve privacy/error-normalization requirements; a more specific code MUST NOT create a credential/role/existence oracle without explicit security/privacy review.

## 13. Authorization Context Registry Boundary

AUTH-v3 binds `authz_context_hash`, but this registry revision does not fabricate a completed authorization schema.

The selected profile MUST eventually define a canonical authorization context covering, as applicable:

```text
holder / authenticated-session confirmation
audience / deployment / group / domain
requested and granted role/policy scope
issuer / authority
validity / expiry
policy / authorization lineage or generation
registry / revocation epoch
allowed operation / profile constraints
```

Until that encoding, empty representation, and negative mutation vectors are normative, production AUTH v3 remains disabled. A peer MUST NOT substitute an implementation-specific struct serialization or language-native encoding for the canonical authorization context.

## 14. Registry Compatibility and Downgrade Rules

The following rules apply across registries:

1. **Known does not imply compatible.** A known suite/profile/capability/extension can still be invalid for the selected protocol version or peer policy.
2. **Unknown critical semantics fail closed.** A peer MUST NOT ignore a value that could change authentication, authorization, trust, replay, key schedule, or channel-binding semantics.
3. **Draft is not production.** A `draft` allocation can be used by deterministic vectors, formal models, or isolated experiments, but MUST NOT be advertised as production support until its promotion gate passes.
4. **No silent downgrade.** Negotiated/selected security values that are authenticated by a transcript/context MUST match the values actually executed.
5. **No cross-namespace substitution.** Runtime presets, capability bits, transport addresses, TLV tags, extension IDs, and profile IDs are different namespaces.
6. **No number reuse.** Deprecated/reserved values remain unavailable for unrelated future semantics.
7. **Unknown-value tests are mandatory for promoted extensibility.** Rust and C MUST agree on accept/ignore/reject decisions for representative unknown, reserved, incompatible-known, and critical-unknown values.
8. **Security-relevant selection is transcript bound.** For AUTH v3, selected version, suite, protocol profile, selected security capabilities, authorization-context hash, critical-extension hash, session ID, and channel-binding hash are authenticated context.

## 15. Allocation Procedure

A proposed registry change should include:

```text
registry name
requested value or allocation range
name and semantic definition
stable | draft | experimental | private-use status
owner/spec section
version/profile compatibility
critical-vs-ignorable behavior where applicable
wire/parser impact
security/privacy implications
downgrade behavior
positive vector/test
negative unknown/incompatible test
Rust/C implementation status
formal/review impact where relevant
migration/deprecation rule
```

Allocation review should prefer the smallest surface that solves a demonstrated interoperability problem. Do not allocate identifiers merely to reserve feature ideas from the roadmap.

For a draft-to-stable transition, the same commit/review series should update all normative and executable owners that can otherwise drift, including constants, parser/negotiation tests, registries, profile text, and canonical vectors where applicable.

## 16. Current AUTH-v3 Registry Gate

This registry work advances but does not complete the AUTH-v3 promotion gate.

Now explicit:

```text
protocol version 0x03                draft / non-advertised
suite 0x0001                         stable existing suite
profile_id 0x0001                    draft iot-core candidate
legacy capability-bit meanings       recorded
v3 selected-capability restrictions  defined
packet/error/TLV stable allocations  recorded
critical-vs-ignorable extension rule defined
critical extension numeric IDs       not yet allocated
channel-binding type namespace       defined; only draft NO-BINDING sentinel
```

Still required before production v3 selection:

- accepted/superseding decision for ADR 0001;
- complete `iot-core` / `p2p-iot-core` profile semantics and evidence;
- final production-valid AUTH-v3 selected-capability set and regenerated vectors if needed;
- canonical authorization-context encoding and vectors;
- canonical critical-extension-set encoding and empty representation;
- canonical channel-binding context encoding and empty representation;
- replay lifetime/epoch policy aligned with bounded runtime state;
- Rust/C production negotiation and full AUTH-v3 state machines;
- negative registry/downgrade/unknown-critical corpus;
- production-corresponding formal rerun and constrained measurements.

The existing `rust/test-vectors/auth-v3/reference-primitives-v1.json` remains a primitive/context-binding reference vector, not a negotiation-conformance vector.

## 17. Source-of-truth implementation anchors

At this revision, stable existing values are mirrored in:

- Rust protocol version, suite, and capabilities: `rust/crates/proto/src/caps.rs`;
- Rust packet types, flags, and HELLO TLVs: `rust/crates/proto/src/wire.rs`;
- Rust wire errors: `rust/crates/proto/src/error.rs`;
- C protocol version, suite, capabilities, and error codes: `c/include/auth/auth.h`;
- draft AUTH-v3 context/reference primitive: `rust/crates/proto/src/auth_v3.rs`, `c/include/auth/auth_v3.h`, and `c/src/proto/auth_v3.c`;
- profile design direction: `spec/iot-profiles.md`, `docs/roadmaps/improvement-roadmap.md`;
- detailed standards/reference policy: `docs/roadmaps/rfc-evolution-plan.md`;
- draft AUTH-v3 normative contract: `spec/zk-arche-protocol.md` and ADR 0001.

If source and this registry disagree, do not silently choose one. Treat the discrepancy as specification/interoperability debt, fail the affected promotion claim, and reconcile it through a reviewed versioned change.
