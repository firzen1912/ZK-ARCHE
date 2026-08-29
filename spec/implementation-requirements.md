# ZK-ARCHE Implementation Requirements

Status: **draft normative implementation contract**. This document translates existing roadmap, specification, and assurance requirements into testable implementation obligations. It does not make AUTH v3 selectable, clear TD-001 through TD-004, establish Common Contract conformance, or claim deployment qualification.

Normative keywords **MUST**, **MUST NOT**, **SHOULD**, and **MAY** are used in the BCP 14 sense when the stated behavior is testable. These requirements apply to every implementation claiming the corresponding ZK-ARCHE profile or protocol behavior.

## 1. Evidence and conformance boundary

An implementation claim MUST identify the exact protocol version, suite, profile, implementation revision, build configuration, and validation evidence supporting that claim.

Passing unit tests alone MUST NOT be reported as proof of cryptographic soundness, formal implementation verification, Common Contract conformance, external review, or deployment qualification.

Where Rust and C claim the same protocol behavior, both implementations MUST preserve the same normative wire bytes, transcript inputs, proof inputs, KDF/MAC inputs, profile semantics, state-transition decisions, and accept/reject classifications.

A platform or profile for which required evidence is unavailable MUST retain the missing evidence as an explicit gap rather than inheriting evidence from another platform or profile.

## 2. Parsing, framing, and bounded resource behavior

Implementations MUST validate framing and structural plausibility before allocating, indexing, copying, or iterating according to attacker-controlled length/count fields.

Length and count arithmetic MUST be checked for overflow, underflow, truncation, and impossible structural relationships before memory access or allocation.

Selected constrained profiles MUST enforce their declared maximum message size, entry count, per-entry value length, aggregate canonical length, and state-capacity bounds before materializing attacker-controlled collections.

Malformed input MUST fail deterministically according to the applicable protocol error class. Implementations MUST NOT reinterpret malformed input as a different valid message merely to preserve forward compatibility.

Unknown values MUST follow the applicable registry/profile criticality rule: explicitly ignorable non-critical values MAY be ignored where specified; unsupported critical selections MUST fail closed.

Duplicate, non-canonical, reserved, forbidden, or out-of-order fields MUST be rejected wherever the relevant wire/profile specification declares them invalid.

C implementations SHOULD use caller-owned or otherwise statically bounded storage in constrained hot paths where practical. Rust implementations MAY use dynamic storage, but attacker-controlled fields MUST NOT directly trigger unbounded or structurally implausible allocation.

Parser/resource evidence is profile- and path-specific. A bounded result for one malformed fixture MUST NOT be generalized into a zero-allocation or denial-of-service-resistance claim for all parser inputs.

## 3. Cryptographic execution requirements

Secret-dependent cryptographic operations MUST use implementations intended to avoid secret-dependent timing and memory-access behavior for the declared target and library configuration.

RNG/CSPRNG failure MUST fail closed. Implementations MUST NOT substitute deterministic, zero, repeated, predictable, or uninitialized values for required fresh cryptographic randomness.

Long-term secrets, ephemeral secrets, session keys, resumption secrets, and derived traffic/association keys MUST be separated by the protocol-defined derivation context and MUST NOT be reused across purposes contrary to the applicable key schedule.

Domain-separation labels, transcript/context inputs, suite/profile identifiers, and direction labels MUST match the normative specification exactly. An implementation MUST NOT silently omit a security-relevant input because it is unavailable from a local API.

Secret zeroization, secure storage, rollback resistance, secure boot/debug posture, entropy provenance, DRBG/reseed behavior, accelerator behavior, and side-channel posture are target evidence requirements where a deployment or constrained-target claim depends on them. They MUST NOT be inferred from symbolic formal results.

The custom role-membership proof remains subject to TD-001 independent cryptographic review. No implementation requirement in this document changes that review status.

## 4. Authentication, authorization, and trust mutation

Authentication, authorization, and trust mutation MUST remain distinct implementation decisions.

Normal AUTH MUST be NO-LEARNING: successful authentication MUST NOT create, expand, or rewrite trusted peer/credential state unless the active protocol state is an explicitly authorized enrollment, commissioner, grant, re-registration, rekey, or equivalent trust-mutation flow.

Possession of a valid authentication credential MUST NOT by itself authorize an operation outside the locally accepted audience, deployment/domain, role, scope, policy, validity, generation, or revocation context required by the selected profile.

A constrained peer MUST locally verify the mandatory authentication decision. A gateway, cloud service, CA, DNS service, central registry, or controller MAY assist discovery, synchronization, policy distribution, or audit, but MUST NOT replace the peer's mandatory local authentication verification for an infrastructure-independent profile.

Trust MUST be treated as local and non-transitive by default. Delegated authority MUST NOT be inferred merely from a trust relationship unless an applicable future delegation specification explicitly authorizes and bounds that transition.

### 4.1 Identity-attribution resolver requirements

When an implementation accepts a credential, credential reference, opaque local handle, registry entry, key identifier, commitment, or equivalent lookup result for AUTH, it MUST resolve that object through one authoritative local attribution relation before treating possession/proof verification as authentication of a peer identity or policy subject.

The resolved attribution relation MUST bind, as applicable to the selected profile, the exact credential or security object to the exact authentication key or commitment, peer identity, role/policy identity, audience or deployment/domain, authorization provenance, permitted key operations, and profile/version context on which the local decision depends.

A key identifier, transport address, registry slot, database row number, cache key, opaque hint, or lookup accelerator MUST NOT by itself define protocol identity or authorization. Such values MAY select candidate local state, but the implementation MUST validate the complete locally required attribution relation before authenticated completion or authorization admission.

If the same credential or security object is supplied or referenced through more than one accepted path in one decision, all authoritative paths MUST resolve to the same security object and the same locally required attribution tuple. Conflicting, stale, ambiguous, or multiply bound results MUST fail closed.

An implementation MUST reject a mapping substitution in which cryptographic possession or proof verification succeeds for one key/commitment while the selected local record attributes that proof to a different peer identity, role/policy subject, audience/domain, authorization lineage, or incompatible profile/key-operation context.

Reusing the same authentication key or commitment across multiple locally authorized identities or policy records MUST NOT cause implicit identity equivalence. If a deployment intentionally permits such reuse, each accepted identity/policy binding MUST remain explicit and the active AUTH decision MUST select exactly one locally authorized attribution tuple.

Resolver success during normal AUTH MUST NOT create a new alias, repair a missing mapping, merge records, learn a new credential, or mutate trust. Missing authoritative attribution state MUST cause AUTH to fail closed unless the active state is a separately specified and authorized trust-mutation flow.

Cached resolver results MAY be used only while the implementation can prove they remain valid under the applicable credential generation, authorization generation, policy epoch, revocation epoch, profile/version context, and local invalidation rules. A stale cache entry MUST NOT override newer local authoritative state.

Where Rust and C claim the same resolver behavior, shared negative evidence SHOULD cover at least: same key under two identity records without an explicit selected binding; stale alias after reprovisioning; credential/reference mismatch across accepted input paths; correct key with wrong role/policy or audience/domain record; incompatible key-operation/profile binding; and consistent references that legitimately resolve to the same credential/security object.

This subsection defines local attribution semantics only. It does not define a new wire field, credential format, global identifier namespace, enrollment authority, delegation model, certificate dependency, cloud registry, or online lookup requirement. A future wire-visible credential/reference mechanism requires its own TD-004 specification, registry, vectors, and interoperability review.

## 5. Negotiation, profiles, and downgrade behavior

Implementations MUST distinguish protocol version, cryptographic suite/method, selected profile, optional capabilities, and critical extensions according to their normative registries and compatibility rules.

A selected profile MUST determine one immutable semantic contract for that profile identifier. Local resource configuration MUST NOT silently redefine the security meaning of a registered profile.

Capability negotiation MAY remove optional functionality, but it MUST NOT negotiate away mandatory security properties of the selected profile or mandatory Common Contract floor.

If peers cannot identify a mutually supported mandatory security floor, the exchange MUST fail closed rather than select an undocumented weaker mode.

Higher-capability peers claiming constrained-profile interoperability MUST implement the constrained mandatory floor without reducing authentication assurance. Optional high-end behavior MUST remain isolated from the mandatory constrained path.

Production downgrade-resistance and complete negotiation semantics remain TD-004 work until the applicable normative state/selection rules and negative vectors are complete.

## 6. State machine and transcript behavior

Implementations MUST reject messages that are invalid for the current protocol state, wrong direction, wrong sequence, stale, cross-session, reflected, or bound to a different authenticated context unless the applicable state machine explicitly permits the case.

Every security-relevant input owned by the selected protocol/profile MUST be bound into the transcript, security context, key-confirmation context, authorization decision, or another explicitly specified authenticated structure before the implementation relies on it.

Transport addresses, socket identifiers, BLE handles, CAN identifiers, MAC addresses, or other adapter metadata MUST NOT become protocol identity merely because the transport exposes them.

Transport/channel metadata used as an authenticated binding MUST have an explicit binding method and canonical input contract. Otherwise it MUST be treated as untrusted metadata.

Error response, no-response behavior, retry behavior, externally visible size class, and timing are security/privacy-relevant implementation surfaces and MUST follow the applicable state/error specification when one exists. Current privacy-equivalence coverage remains incomplete and MUST NOT be overstated.

## 7. Replay, restart, rekey, and resumption

Replay acceptance state MUST be updated according to the applicable protocol/profile ordering rule before an implementation reports authenticated completion where the specification requires prior replay recording.

Loss, staleness, rollback suspicion, or unverifiable replay-continuity state MUST fail closed according to `spec/replay-continuity.md`. Process restart, transport reconnection, address change, or a new outer session identifier MUST NOT alone be treated as an authenticated fresh replay epoch.

Persistent writes for replay continuity, credentials, authorization lineage, policy/revocation state, enrollment grants, tickets, or resumption state MUST be atomic or use an explicitly recoverable state transition when loss or rollback could weaken a security guarantee.

A future fresh replay-epoch transition MUST be authenticated and bound to a new security context before predecessor replay state can be discarded. That mechanism is currently unresolved and MUST NOT be synthesized by implementation convention.

Resumption MUST NOT preserve stale authorization merely because a resumption secret is valid. When the required authorization context cannot be safely revalidated, implementations MUST fall back to the applicable full AUTH path or fail closed according to the future resumption specification.

Rekey, key replacement, and resumption MUST NOT silently preserve dependent state that the applicable credential/policy/revocation change invalidates.

## 8. Failure handling and memory safety

Security-relevant failures MUST propagate to the protocol state machine. Implementations MUST NOT ignore a parser, cryptographic verification, authorization, replay, storage, or RNG failure and continue as if authentication succeeded.

C code MUST check allocation, length, pointer, and buffer-capacity conditions before use on attacker-controlled paths. Rust code MUST avoid introducing `unsafe` for protocol parsing or cryptographic state handling unless the safety contract and tests are explicitly documented and reviewed.

Assertions MAY protect programmer invariants, but malformed network input MUST be handled as a recoverable protocol failure rather than relying on undefined behavior, memory corruption, or process abort as the security mechanism.

Sanitizer/static-analysis success is implementation evidence only; it MUST NOT be reported as proof of memory safety for all executions.

## 9. Interoperability and vector governance

Canonical deterministic vectors and negative vectors MUST remain versioned whenever wire, transcript, proof, KDF, MAC/key-confirmation, negotiation, or state-machine semantics change.

Rust remains the canonical checked-in vector source under the current roadmap. C MUST reproduce the same bytes and decisions wherever it claims the same behavior.

A semantic wire/vector change MUST NOT overwrite prior vector meaning under the same version/profile identity. Versioning, replacement, or deprecation MUST follow the applicable registry/change-control rules.

Interop qualification MUST include negative behavior, not only successful handshakes. Unsupported critical values, malformed encodings, wrong profile/suite combinations, reflection/replay cases, context mismatches, incompatible mandatory floors, and identity-attribution/mapping-substitution cases MUST be represented where applicable.

Transport adapters MUST NOT change the protocol-level authentication, authorization, trust, transcript, or identity semantics of the Common Contract merely to fit a transport API.

## 10. Constrained-profile implementation evidence

Before a constrained-profile implementation is reported as MEASURED, the evidence package MUST identify at least:

- target and board revision;
- compiler/toolchain/build profile;
- implementation and cryptographic-library versions;
- cryptographic execution boundary and accelerator/software path;
- entropy source, DRBG/reseed posture, and key-generation mode;
- root-seed/private-key representation and storage location;
- secure-boot/debug/rollback assumptions where relevant;
- wire bytes and MTU/fragmentation context;
- stack, heap, static RAM, flash, and CPU/latency measurements actually obtainable on that target;
- persistent replay/authorization state and restart behavior;
- registry/trust scaling limits where they affect the profile.

Unavailable physical measurements MUST remain explicit blockers. Host measurements, simulation, static size estimates, or symbolic models MUST NOT be relabeled as physical MCU measurements.

## 11. Required evidence classes

A normative behavior should be linked to at least one of the following as appropriate:

```text
positive deterministic vector
negative deterministic vector
Rust test
C test
cross-language decision/byte parity test
fuzz/sanitizer/static-analysis evidence
scoped formal result
retained counterexample
constrained-target measurement
independent external review
explicit unresolved evidence gap
```

The strongest applicable evidence class MUST be stated accurately. `IMPLEMENTED`, `TESTED`, `INTEROPERABLE`, `MEASURED`, `FORMALLY ANALYZED`, `EXTERNALLY REVIEWED`, `COMMON-CONFORMANT`, `RFC-CLASS DOCUMENTED`, and `DEPLOYMENT-QUALIFIED` remain distinct states.

## 12. Current qualification boundary

Current repository evidence materially supports bounded parsing, deterministic Rust/C decision parity on existing shared corpora, synchronized fail-closed formal gates, replay-continuity fail-closed behavior, and Rust/C release qualification for the implemented surfaces.

The following remain open and prevent stronger blanket claims:

- TD-001 independent cryptographic review;
- TD-002 physical STM32/ESP32-S3-class measurements and execution-context evidence;
- TD-003 complete model/runtime traceability and properties blocked by missing normative semantics;
- TD-004 complete RFC-class normative grammar, production state machines, downgrade behavior, privacy/error contract, annotated traces, and specification-grade conformance package;
- executable Rust/C identity-attribution resolver implementations and shared mapping-substitution negative fixtures;
- authorization authority/provenance and enrollment/delegation semantics beyond the local attribution contract above;
- revocation convergence and bounded stale-authorization policy;
- authorization-aware resumption;
- authenticated fresh replay-epoch transition;
- full P2P Common Contract and cross-class constrained interoperability evidence.

Therefore this document is an implementation contract and qualification checklist, not an RFC-class, Common-Conformant, externally reviewed, or deployment-qualified claim.