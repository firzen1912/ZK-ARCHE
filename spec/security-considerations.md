# ZK-ARCHE Security Considerations

Status: **normative-security consolidation draft**. This document records security requirements, threat boundaries, and residual risks already owned by the current specification, roadmap, ADRs, and executable evidence. It does **not** allocate new wire behavior, promote AUTH-v3 to production negotiation, clear TD-001 through TD-004, claim RFC/IETF status, or broaden existing cryptographic/formal/deployment claims.

Normative keywords are used only where the referenced repository behavior is already precise enough to test. Where protocol behavior remains unresolved, this document names the gap instead of choosing policy here.

## 1. Security objective and assurance boundary

ZK-ARCHE is intended to let previously authorized peers authenticate each other, bind authentication to an exact protocol/security context, derive or confirm an association, and evaluate scoped authorization without making transport addresses or external infrastructure the root identity decision.

The security objective is deliberately narrower than a claim of complete system security. Protocol authentication does not establish physical platform integrity, secure boot, constant-time execution, correct entropy, rollback-resistant storage, compromise recovery, safe application authorization, or deployment readiness unless those properties have separate retained evidence.

The mandatory constrained floor follows these repository invariants:

- normal `AUTH` is **NO-LEARNING**: successful authentication proves against pre-existing trusted state and does not create or expand trust;
- authentication, authorization, and trust mutation are distinct decisions;
- capability negotiation cannot remove mandatory security requirements;
- transport addresses and connection identifiers are not protocol identity;
- already-authorized peers must not require a CA, cloud service, DNS, central registry, blockchain, manufacturer cloud, or gateway approval for the root core-AUTH decision when sufficient local state exists;
- trust is local and non-transitive by default; delegation, where later supported, must be explicit and bounded;
- the least-capable conformant peer defines the resource envelope, not a weaker authentication assurance level.

## 2. Threat model

The baseline network threat is an active Dolev-Yao-style attacker able to observe, inject, modify, delay, drop, replay, reorder, reflect, duplicate, and interleave messages and to operate malicious peers. The attacker may also exploit malformed inputs, retry behavior, state exhaustion, stale durable state, and transport-address changes.

Additional threat classes are treated separately because the current repository does not prove one universal attacker model:

- **authorized-but-malicious peer:** owns legitimate credential/trust state but attempts privilege, audience, scope, lineage, or context expansion;
- **stale/offline state attacker:** delays synchronization, restores older valid local records, or exploits restart/eviction boundaries;
- **selective compromise attacker:** obtains long-term, session, replay, authorization, or storage state; guarantees after compromise depend on explicit recovery semantics that are not yet complete;
- **infrastructure-loss condition:** optional infrastructure becomes unavailable; this is primarily an availability/dependency condition, not a substitute for freshness/revocation analysis;
- **local implementation attacker:** targets parser bounds, memory safety, timing, RNG, key handling, persistence, or hardware/debug configuration. Symbolic protocol results do not cover this class automatically.

## 3. Prior trust and NO-LEARNING AUTH

A normal AUTH exchange MUST NOT install a new credential, trust anchor, role grant, commissioner authority, delegation edge, or authorization expansion merely because the peer completed cryptographic authentication.

Trust mutation belongs to an explicit enrollment, grant, commissioner, rekey/re-registration, or equivalent reviewed lifecycle transition. Discovery information, transport reachability, socket addresses, radio identifiers, or unauthenticated peer metadata MUST NOT be promoted to trusted identity as a side effect of AUTH.

This requirement limits unknown-peer and first-contact risk but does not solve bootstrap. Provisioning and enrollment remain separate security ceremonies with their own authority and replay requirements.

## 4. Identity, transcript, and security-context binding

Security-relevant decisions must be bound to the same authenticated protocol instance. Current AUTH-v3 work binds explicit session/security context material and separates directional Finished/key-confirmation semantics. A consumer MUST NOT infer peer identity solely from a transport endpoint, connection identifier, packet source address, or lookup handle.

Where a credential/reference, peer identity, role/policy record, authorization context, channel binding, profile, suite, or session identifier participates in an authentication or lifecycle decision, implementations must use the canonical repository-defined representation for that surface. Substitution across a different session, authorization-context hash, channel-binding hash, predecessor reference, or successor reference must fail closed where the corresponding current contract defines the check.

The formal model provides scoped evidence for modeled transcript/context fields and directional Finished separation. It does not establish computational collision resistance, parser/model equivalence, or complete unknown-key-share resistance for credential/reference relationships whose normative resolver semantics are still open.

## 5. Authentication is not authorization

Successful possession proof or session completion MUST NOT be interpreted as permission to perform an arbitrary operation. Authorization decisions must be evaluated against the current locally accepted authorization state and the exact authenticated holder/session context.

The current `iot-core` authorization work separates authentication completion, authorization-context admission, attribution, predecessor binding, and privilege-preservation checks. A valid credential presented for the wrong audience, scope, operation, lineage, or authorization generation must not gain broader authority merely because the cryptographic proof verifies.

Authority/provenance namespaces, generalized delegation, revocation convergence, and authorization-aware resumption are not yet complete enough for this document to invent their policy. Those surfaces remain explicit normative gaps.

## 6. Replay, restart, and freshness

Replay protection is a lifecycle property, not only a packet-cache check. Implementations must preserve the semantics of the repository replay and replay-continuity contracts across duplicate delivery, reordering, bounded cache behavior, restart, and epoch transitions for the profiles that claim those properties.

A successful integrity check on durable storage does not prove freshness. Old but authentic state may still be dangerous. Where a lifecycle decision requires a newest-authorized generation or high-water value, the implementation must obtain that fact from a freshness mechanism whose security domain and rollback assumptions are explicitly qualified. Absence or uncertainty of required freshness must fail closed rather than being treated as proof that the stored record is current.

The current AUTH symbolic replay table is persistent/unbounded and therefore stronger than bounded runtime FIFO storage in important ways. Formal replay correspondence must not be cited as proof of runtime eviction, power-loss, or rollback behavior.

## 7. Rekey, re-registration, and lineage replacement

Credential/key replacement is an authorization and persistence transaction, not merely acceptance of a new public key. Current LINEAGE_REPLACE contracts require the existing authorization/possession/session/context gates before storage mutation and define the logical durable order:

```text
PERSIST_PENDING
ACTIVATE_SUCCESSOR
RETIRE_PREDECESSOR
INVALIDATE_DEPENDENT_STATE
CLEAR_PENDING
```

Pre-storage rejection must not mutate durable lineage state. After `PERSIST_PENDING`, an intermediate storage failure must not be converted into optimistic success or silently compensated by clearing the pending marker. Recovery must reconcile the durable evidence that actually survived.

This ordering does not by itself establish filesystem/flash atomicity, malicious rollback resistance, secure erasure, or power-cut survival. Those are adapter/target evidence obligations. A backend described generically as “secure storage” must not be assumed to provide replay protection or trusted freshness without explicit capability and target evidence.

## 8. Negotiation, downgrade, and extensions

A capability or profile negotiation mechanism MUST NOT permit peers to negotiate away mandatory security properties while retaining the same conformance claim. Unknown critical behavior must fail closed once the corresponding extension grammar is promoted.

Production AUTH-v3 negotiation, complete mandatory-floor downgrade semantics, and generalized critical-extension processing remain incomplete. Therefore ZK-ARCHE does not currently claim a complete RFC-class downgrade-resistance story. Optional high-end features must remain isolated from the constrained mandatory floor until their negotiation and failure semantics are independently specified and tested.

## 9. Malformed input, parser, and state-exhaustion safety

Unauthenticated input is attacker-controlled. Parsers should reject malformed length/count/encoding combinations before expensive work or attacker-sized allocation where the applicable grammar permits a structural bound. C implementations must preserve explicit capacity checks and memory-safety invariants; Rust memory safety does not remove CPU, allocation, panic, or state-exhaustion risks.

The retained AUTH-v3 context-parser evidence demonstrates specific bounded hostile-count behavior and shared Rust/C accept/reject decisions. It is not proof of zero allocation for all inputs, complete pre-authentication DoS resistance, parser-to-symbolic-model equivalence, or physical MCU resource fitness.

Retry cookies, source validation, anti-amplification, retransmission, fragmentation, and general pre-authentication resource budgeting are not yet complete across every transport/profile. Implementations must not claim those protections based only on analogous behavior in TLS, DTLS, EDHOC, QUIC, or another comparator.

## 10. RNG, key generation, and key lifecycle

Security depends on correct entropy, CSPRNG/DRBG behavior, key generation, domain separation, and key lifecycle. A protocol test vector or symbolic proof cannot establish entropy quality on a deployed platform.

Any constrained-target maturity claim must identify the entropy source, health-test posture, DRBG/reseed behavior, key-generation mode, stored key/seed representation, accelerator path, zeroization assumptions, secure-storage location, debug/boot posture, and relevant rollback/clone/reprovision assumptions. Missing physical evidence remains TD-002 and must not be inferred from host tests.

## 11. Custom proof and cryptographic-review boundary

The custom Schnorr/role-membership proof remains subject to TD-001 independent cryptographic review. Existing deterministic vectors, Rust/C agreement, negative tests, and symbolic abstractions do not substitute for independent cryptographic analysis.

In particular, symbolic `schnorr_proof` or `role_proof` interfaces assume properties that must be justified computationally and at the implementation level. Until independent review exists, repository claims must continue to state that boundary and must not describe the proof as externally reviewed, standardized, or cryptographically proven.

## 12. Side channels, memory safety, and implementation behavior

Protocol correctness does not prove constant-time behavior. Secret-dependent branches, table lookups, compiler transformations, allocator behavior, cache effects, fault injection, debug exposure, or hardware accelerator behavior require separate analysis where relevant.

Rust memory safety does not make unsafe dependencies, FFI, secret lifetime, side channels, persistence, or logic errors impossible. C qualification must retain warnings, static analysis, sanitizers, malformed-input tests, bounds checks, and cross-language decision/vector comparisons. Neither sanitizer success nor static analysis is a proof of absence of memory-safety defects.

## 13. Formal-analysis boundary

Current ProVerif work provides scoped symbolic evidence for selected AUTH-v3 and replay-continuity properties and models LINEAGE_REPLACE commit ordering. Formal evidence is valid only for the exact retained model text, tool/version, queries, and attacker assumptions recorded with the run.

Formal analysis does not establish constant-time behavior, RNG quality, C/Rust memory safety, storage atomicity, secure erasure, computational soundness of the custom proof, or field readiness. A model edit requires a new exact-model retained run before the edited model can inherit a `FORMALLY ANALYZED` state.

Several property families remain intentionally blocked until normative/runtime behavior exists: generalized downgrade behavior, authorization authority/provenance, non-transitive delegation, revocation freshness, resumption authorization/reuse bounds, reverse-role P2P symmetry, credential/reference binding, privacy equivalence, and compromise/recovery transitions.

## 14. Infrastructure and transport boundaries

Optional infrastructure may assist discovery, synchronization, revocation propagation, fleet administration, audit, backup, or large-registry indexing. It must not silently become the root decision authority for core authentication between already-authorized peers under a profile that claims infrastructure-independent operation.

Loss of infrastructure may still reduce availability or freshness. A peer with stale state must not treat “offline” as permission to ignore a profile’s required revocation, epoch, or freshness rule.

Transport metadata is untrusted unless explicitly authenticated or channel-bound. Address mobility, NAT rebinding, link-layer identifiers, TLS connections, serial ports, robotics middleware identities, or gateway routing information must not be equated with ZK-ARCHE protocol identity without a defined binding.

## 15. Error behavior and observable failure

Fail-closed security decisions are required where current contracts define mandatory evidence. However, different rejection causes may themselves create observable timing, packet-size, response, or retry behavior. The current repository does not yet establish a general failure-observability privacy theorem.

Implementations should avoid adding unnecessary distinctions to unauthenticated errors and must not expose protected role/trust state solely to make diagnostics easier. Exact alert/error grammar and privacy-preserving failure behavior remain TD-004 work where not already owned by a concrete contract.

## 16. Security evidence that must not be conflated

The following claim classes are independent:

```text
IMPLEMENTED
TESTED
INTEROPERABLE
COMMON-CONFORMANT
MEASURED
FORMALLY ANALYZED
EXTERNALLY REVIEWED
RFC-CLASS DOCUMENTED
DEPLOYMENT-QUALIFIED
```

Evidence for a weaker class does not imply a stronger one. In particular:

- Rust/C code does not imply interoperability until byte/decision compatibility is actually exercised;
- deterministic vectors do not imply cryptographic review;
- symbolic proofs do not imply computational proof or implementation verification;
- host resource observations do not imply constrained-target measurements;
- protocol conformance does not imply secure provisioning, hardware posture, safe operations, or field readiness;
- RFC-like prose does not imply RFC-class completion or IETF status.

## 17. Residual open security work

At the time of this draft, material open work includes at least:

- TD-001 independent review of the custom proof;
- TD-002 reproducible constrained-target execution/storage/RNG evidence;
- TD-003 complete property coverage and model/runtime traceability;
- TD-004 complete normative grammar/state/error/privacy/registry/conformance package;
- production negotiation and downgrade semantics;
- authorization authority/provenance and generalized credential/reference binding;
- revocation convergence and stale-state bounds;
- authorization-aware resumption and reuse limits;
- complete P2P role symmetry/common-contract qualification;
- target-specific durable and rollback-resistant lineage storage evidence;
- complete observable-error/privacy analysis.

These gaps are retained blockers, not implied future behavior. This document is a security-analysis consolidation of exact-current repository semantics and does not close them.
