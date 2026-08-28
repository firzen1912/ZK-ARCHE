# AUTH-v3 Formal Traceability Record

Status: **scoped assurance evidence**. This record reconciles the retained draft AUTH-v3 ProVerif execution with the current repository surfaces that implement, specify, or test the modeled security context. It advances TD-003 traceability only. It does not make AUTH v3 selectable, prove the custom role-membership proof, establish replay persistence, close TD-003, or establish RFC-class status.

## 1. Exact evidence identity

Current repository anchor for this traceability record:

```text
repository_commit = d700b5e201d288dac5fac162dc2d70937178eab9
branch            = dev
ci_run            = #56
ci_result         = success
```

Retained formal result:

```text
docs/assurance/formal-runs/2026-08-27-d4994da-proverif-auth-v3.md
formal_run_commit = d4994daaa3dd771cfa87b42485d9032302af82f0
tool              = ProVerif 2.05
retained_results  = 6 TRUE query results
```

Current synchronized draft-v3 model copies:

```text
rust/models/proverif/zk_arche_auth_v3_draft.pv
c/models/proverif/zk_arche_auth_v3_draft.pv
Git blob SHA      = 4740b0a16a6148382f17fbc756f104222fc6cbd3
```

The Rust and C model paths have the same Git blob SHA at the repository anchor above. They are synchronized copies, not independent formal implementations.

The retained formal run records model SHA-256 `a49b316029d834a84e98aea6cde13371f229c595e75400d3b40207e1fb22deda` at its exact execution commit. A future model edit MUST produce a new retained run before a prior result is described as evidence for the edited model text.

## 2. Attacker and abstraction scope

The retained draft-v3 run is interpreted under the active-network attacker class corresponding to A0 in `formal-model-contract.md`: the attacker can read, drop, delay, replay, reorder, inject, modify, interleave, and initiate concurrent protocol runs but does not initially know uncompromised long-term secrets.

The model additionally makes these important abstractions explicit:

- `schnorr_proof` is idealized; the model does not establish computational Schnorr security;
- `role_proof` is idealized; TD-001 independent cryptographic review remains required;
- canonical hashing and concrete byte encoding are abstracted as symbolic bitstrings;
- the replay table is persistent and unbounded;
- restart, rollback, bounded eviction, fresh replay epochs, resumption, revocation convergence, transport failure observability, and implementation memory behavior are not modeled;
- constant-time behavior, RNG/DRBG quality, key storage, zeroization, memory safety, side channels, and target resource behavior are outside the symbolic model.

Accordingly, every TRUE result below is scoped to the modeled transition system and assumptions rather than promoted to a whole-protocol or deployment claim.

## 3. Property-to-query-to-repository map

| Property | ProVerif event/query evidence | Normative/design owner | Rust implementation/evidence anchor | C implementation/evidence anchor | Current evidence state |
|---|---|---|---|---|---|
| FM-02 client-to-server agreement | injective `ServerCompleteV3 ==> ClientAuth3SentV3` | ADR 0001 + `spec/zk-arche-protocol.md` draft AUTH-v3 state/completion semantics | `rust/crates/proto/src/auth_v3.rs`: `build_kc_transcript_v3`, `derive_kc_keys_v3`, `finished_tag_v3`, `completion_hash_v3`, `completion_tag_v3`; deterministic reference test/vector | `c/include/auth/auth_v3.h`, `c/src/proto/auth_v3.c`, `c/tests/test_auth_v3_reference.c` | scoped TRUE in retained model; production state-machine interoperability not established |
| FM-03 server-to-client / mutual completion | injective `ClientCompleteV3 ==> ServerCompleteV3`; `ClientCompleteV3 ==> ServerAuth2SentV3` | ADR 0001 authenticated AUTH_ACK-v3 design | same Rust v3 reference primitives and vector | same C v3 reference primitives and vector | scoped TRUE in retained model; production dispatch remains disabled |
| FM-04 accepted-message replay ordering | `ServerAuth1AcceptedV3 ==> ReplayRecordedV3` | `spec/iot-profiles.md` + `spec/replay-continuity.md` | bounded runtime replay implementation plus shared FIFO/replay-edge fixtures | bounded runtime replay implementation plus shared FIFO/replay-edge fixtures | scoped TRUE only for persistent/unbounded model replay memory; runtime restart/rollback/epoch equivalence not established |
| FM-05 transcript/security-context integrity | completion/agreement queries carry identical `secctx` and `kcctx` | AUTH-v3 security context + `spec/iot-profiles.md` + `spec/auth-v3-context-encoding.md` + `spec/iot-core-authorization-context.md` | `AuthV3Context` binds version, suite, profile, selected capabilities, session ID, authz hash, critical-extension hash, channel-binding hash; raw `iot-core` authorization receive path validates a fixed 148-byte/7-entry schema and hashes exact accepted bytes | `auth_v3_context_t` exposes the same draft fields; C raw `iot-core` receive path validates the same fixed schema in caller-owned bounded storage and hashes exact accepted bytes | scoped TRUE for equality of modeled symbolic fields; concrete `iot-core` authz canonicalization/profile validation is now separately executable and CI-tested, but remains an assumption at the ProVerif boundary |
| FM-09 NO-LEARNING AUTH | `ServerCompleteV3 ==> TrustedRecordPresent(client)` | roadmap NO-LEARNING invariant and future TRUST/AUTH normative text | production trust-store mutation remains separate from draft v3 reference helpers | production trust-store mutation remains separate from draft v3 reference helpers | scoped TRUE in model relative to pre-existing trusted record; not a complete enrollment/trust-state proof |

## 4. Security-context field traceability

The model constructor is:

```text
security_context_v3(
  protocol_version,
  suite,
  profile,
  selected_capabilities,
  session_id,
  authz_context,
  critical_extensions,
  channel_binding)
```

The Rust draft context exposes the corresponding fields as:

```text
AuthV3Context::protocol_version
AuthV3Context::suite_id
AuthV3Context::profile_id
AuthV3Context::selected_capabilities
AuthV3Context::session_id
AuthV3Context::authz_context_hash
AuthV3Context::critical_extensions_hash
AuthV3Context::channel_binding_hash
```

The C draft context exposes the same semantic set through `auth_v3_context_t`.

The model still treats the three hashed subcontexts as already-formed bitstrings. It does not parse `ZKCTX`, count entries, validate IDs/lengths, detect duplicates, enforce reserved flags, evaluate profile-specific limits, or compute SHA-256 over received bytes. Those concrete operations therefore remain outside the six retained ProVerif results.

### 4.1 Concrete `iot-core` authorization boundary now covered by executable evidence

At the current repository anchor, the `iot-core` authorization subcontext has a materially stronger concrete contract than the abstraction present when the retained AUTH-v3 model was executed.

Normative draft owner:

```text
spec/iot-core-authorization-context.md
profile_id                 = 0x0001
context kind               = AUTHORIZATION
canonical byte length      = 148
entry count                = 7
entry IDs                  = 0x0001 .. 0x0007 in ascending order
entry value lengths        = 32,32,8,8,8,8,8
scope_bits                 = 1 only
lifecycle integers         = non-zero
unknown/additional fields  = forbidden in schema v1
```

Rust receive/hash anchors:

```text
rust/crates/proto/src/auth_v3_context_parser.rs
  parse_canonical_context_bounded(...)

rust/crates/proto/src/auth_v3_iot_core_authz.rs
  decode_iot_core_authorization_context_bytes(...)
  hash_iot_core_authorization_context_bytes(...)
```

The Rust profile receive path enforces the exact 148-byte profile length before generic parsing, supplies the seven-entry ceiling before the parser allocates its entry vector, validates the context kind plus exact ID/value-length schema, applies semantic zero/scope checks, and hashes the exact received bytes only after successful validation.

C receive/hash anchors:

```text
c/src/proto/auth_v3_context_parser.c
c/src/proto/auth_v3_iot_core_authz.c
  auth_v3_iot_core_authz_decode_bytes(...)
  auth_v3_iot_core_authz_hash_bytes(...)
```

The C profile receive path enforces the same 148-byte profile length, parses into a fixed seven-entry caller-owned array, maps an excessive declared count to a profile entry-limit failure, validates the same context kind and ID/value-length schema, applies the same semantic checks, and hashes the exact accepted bytes.

Existing Rust/C tests cover the shared canonical vector and profile-bound failures including 147/149-byte inputs, an encoded entry count of eight, wrong context kind, and wrong schema ID. The broader shared `ZKCTX` negative corpus separately covers malformed envelope/version/kind/ID/order/flags/length/trailing-byte conditions. Exact-head CI #56 completed successfully with the Rust lane, C lane, formal gates, and release qualification green.

This advances the traceability statement from:

> canonicalization/parser semantics of the authorization subcontext are only an unresolved implementation assumption

to:

> the concrete `iot-core` authorization parse/validate/hash boundary is independently executable and tested in Rust and C, while the symbolic AUTH-v3 model still assumes the resulting authorization digest as an opaque already-validated bitstring.

That distinction is important. CI-tested parser/profile behavior does **not** turn FM-05 into a proof of parser correctness, SHA-256 collision resistance, authorization-policy correctness, resource sufficiency on an MCU, or equivalence between concrete bytes and the ProVerif abstraction.

### 4.2 Current model/runtime assumption boundary

For FM-05, the model-to-runtime handoff is currently:

```text
attacker-controlled raw authorization bytes
        ↓
strict ZKCTX parse + profile bounds             executable Rust/C evidence
        ↓
exact seven-field iot-core schema validation    executable Rust/C evidence
        ↓
semantic zero/scope validation                  executable Rust/C evidence
        ↓
SHA-256(exact accepted canonical bytes)         executable Rust/C evidence
        ↓
authz_context_hash in AuthV3Context             implementation mapping
        ↓
authz_core symbolic bitstring                    ProVerif abstraction boundary
        ↓
security_context_v3(...) / kc_context_v3(...)   retained symbolic analysis
```

A malformed or profile-nonconformant raw authorization byte string is therefore rejected before it can become a valid `iot-core` authorization hash in the current Rust/C receive helpers. The retained ProVerif model does not itself establish that fact; it begins downstream of that fact.

Critical-extension and channel-binding subcontext schemas remain less concrete than this `iot-core` authorization schema. FM-05 must therefore continue to state the canonicalization assumption separately for those inputs until equivalent normative and executable boundaries exist.

## 5. Replay traceability after the current replay work

The current repository has materially advanced runtime replay semantics beyond the state present when the retained v3 run was first produced:

```text
minimum iot-core replay window = 64 accepted AUTH_1 identifiers
retention                      = deterministic FIFO
restart rule                   = restore trusted state or CONTINUITY_BROKEN
replay epoch rule              = unresolved
profile selectable             = 0
```

The runtime replay-continuity state machine and shared Rust/C decision corpus therefore provide executable evidence for fail-closed restart behavior, but they do not change what the retained ProVerif AUTH-v3 run proved. A separate replay-continuity ProVerif lane now analyzes the promoted fail-closed continuity states; that result is owned by `docs/assurance/replay-continuity-formal-traceability.md` and must not be conflated with this AUTH-v3 model.

The following claim is the strongest allowed reconciliation:

> The retained draft-v3 model establishes accepted-message replay-record ordering under persistent/unbounded symbolic replay memory. Current Rust/C evidence separately establishes bounded FIFO decisions and fail-closed replay-continuity state transitions, and the dedicated replay-continuity model provides scoped symbolic analysis of those states. These evidence lanes are complementary and are not yet equivalent to one complete implementation model.

The authenticated fresh replay-epoch transition remains unresolved and must not be invented by this traceability record.

## 6. Retained counterexample lineage

The legacy AUTH-v2 formal run remains part of the evidence chain. Its two material correspondence failures were not erased:

1. outer `session_id` was not authenticated by the v2 key-confirmation context;
2. the public one-byte AUTH_ACK could be synthesized by an active attacker, so client completion did not establish server acceptance of AUTH_3.

The draft-v3 model addresses those specific modeled failures by binding `session_id` and the wider security context into `KC-TRANSCRIPT-v3` and by authenticating server completion with a dedicated completion key/MAC. The six retained TRUE draft-v3 results are therefore regression evidence for those modeled fixes, not proof that all AUTH-v3 protocol properties are complete.

## 7. Explicitly open formal properties

This record does not upgrade the following FM rows to analyzed:

```text
FM-06 unknown-key-share resistance beyond current bound identities/context
FM-07 explicit reflection resistance
FM-08 downgrade resistance and production negotiation
FM-10 full authorization-scope semantics
FM-11 non-transitive trust
FM-12 bounded delegation
FM-13 revocation freshness/convergence
FM-14/FM-15 resumption authorization and reuse bounds
FM-16 P2P role symmetry
FM-17 infrastructure independence as an explicit modeled property
FM-18 credential/reference binding
FM-19..FM-21 privacy and observable-failure properties
FM-22 compromise/recovery boundaries
```

The new `iot-core` authorization receive evidence does not upgrade FM-10. It validates syntax, fixed schema, basic semantic invariants, and exact-byte hashing; it does not prove that a peer's local holder/audience/role-policy/lineage/revocation state authorizes the requested operation. The August 28 research also identifies authority/provenance namespace semantics for authorization generations/epochs as unresolved, so no cross-authority lineage claim is made here.

## 8. Evidence-state update

At the repository anchor for this record:

```text
DRAFT AUTH-v3 MODEL COPIES SYNCHRONIZED       YES
DRAFT AUTH-v3 RETAINED PROVERIF RUN           YES
FM-02/FM-03 DRAFT-V3 AGREEMENT                SCOPED FORMALLY ANALYZED
FM-04 REPLAY-RECORD ORDERING                   SCOPED FORMALLY ANALYZED UNDER PERSISTENT/UNBOUNDED MODEL STATE
FM-05 MODELED SECURITY-CONTEXT INTEGRITY       SCOPED FORMALLY ANALYZED FOR MODELED FIELDS
IOT-CORE AUTHZ RAW PARSE/PROFILE BOUNDS         TESTED IN RUST + C
IOT-CORE AUTHZ EXACT-BYTE HASH PATH             TESTED IN RUST + C
IOT-CORE AUTHZ PARSER ↔ PROVERIF EQUIVALENCE   NOT ESTABLISHED; EXPLICIT ABSTRACTION BOUNDARY
CRITICAL-EXTENSION CANONICAL SCHEMA             NOT YET EQUIVALENTLY TRACED
CHANNEL-BINDING CANONICAL SCHEMA                NOT YET EQUIVALENTLY TRACED
FM-09 PRE-EXISTING-TRUST RELATION              SCOPED FORMALLY ANALYZED
RUST/C DRAFT-V3 REFERENCE PRIMITIVE PARITY     TESTED BY EXISTING CI/VECTORS
PRODUCTION AUTH-V3                             NOT IMPLEMENTED / NOT SELECTABLE
REPLAY EPOCH RECOVERY                          UNRESOLVED
TD-003                                         OPEN
TD-004                                         OPEN
TD-002                                         OPEN
TD-001 EXTERNAL CRYPTO REVIEW                  OPEN
FORMALLY VERIFIED                              NOT CLAIMED
RFC-CLASS DOCUMENTED                           NOT CLAIMED
DEPLOYMENT-QUALIFIED                           NOT CLAIMED
```

## 9. Next dependency-ready TD-003 packet

The highest-value dependency-ready formal packet is now to tighten the **authorization-context abstraction handoff** without pretending to prove the byte parser in ProVerif.

A suitable packet should make the model/spec/code mapping mechanically harder to drift by introducing an explicit modeled event or constructor boundary for "validated authorization context admitted to the AUTH-v3 security context" and tracing it to the Rust/C receive/hash APIs and the canonical vector. The property should remain narrow: AUTH-v3 completion may depend only on an authorization context that has crossed the modeled validation boundary. The concrete parser remains established by executable Rust/C evidence, not by symbolic parsing.

That packet should also retain negative evidence showing that malformed/profile-excessive raw contexts never reach the concrete hash-admission API. It should not invent authority-namespace semantics, critical-extension schemas, channel-binding policy, replay-epoch recovery, resumption, or production negotiation.
