# ZK-ARCHE RFC-Style Protocol Evolution Plan

This document defines how ZK-ARCHE should evolve from a working Rust/C research implementation into a protocol package that can be specified, implemented, tested, reviewed, and compared with the discipline expected of mature Internet security protocols.

The goal is **not** to claim IETF adoption or RFC status. The goal is to remove ambiguity between implementation behavior, research hypotheses, security claims, and normative protocol requirements while preserving constrained-device feasibility.

## 1. Specification authority and promotion boundary

The RFC-style package sits downstream of research and roadmap decisions:

```text
research source
  ↓
research backlog
  ↓
weekly findings
  ↓ explicit human promotion
roadmap / ADR
  ↓
spec/ normative text
  ↓
Rust + C + vectors/tests
  ↓
assurance evidence
```

A research paper, RFC, Internet-Draft, implementation, or benchmark is a design comparator until ZK-ARCHE explicitly adopts a behavior. Likewise, source code does not become normative merely because it exists. Security- or wire-relevant behavior belongs in `spec/` and must be tied to executable evidence or an explicit unresolved gap.

## 2. Current specification baseline — 2026-08-25

The unified repository has two active implementation lanes, Rust and C, but the normative package is still incomplete. The controlling specification debt remains TD-004, with adjacent dependencies on:

- TD-001: independent cryptographic review of the custom role-membership proof;
- TD-002: target-class constrained-device evidence;
- TD-003: formal model expansion and model-to-code traceability.

The strongest current findings that must now shape the RFC-style work are:

1. authentication, authorization, and trust mutation require separate semantics;
2. normal AUTH is NO-LEARNING and cannot silently enroll a peer;
3. profile selection, capabilities, methods, suites, and extensions require compatibility rules and fail-closed semantics;
4. revocation is a convergence/lifecycle protocol, not merely a local record deletion;
5. replay, key usage, rekey, resumption, invalidation, and channel binding are connected lifecycle state;
6. resumption must revalidate authorization context and bound credential reuse;
7. anonymity and unlinkability are distinct properties and observable failures are part of privacy behavior;
8. protocol conformance and deployment/field readiness are separate claims;
9. target benchmarks require pinned cryptographic execution context;
10. advanced credentials and PQ hybrids remain optional profile research until measured and reviewed.

## 3. Scope and non-goals

ZK-ARCHE is a privacy-preserving device authentication, scoped authorization, trust/lifecycle, and secure-association framework for heterogeneous peers. It may define native, constrained, P2P, transport-bound, and data-sovereignty profiles, but it must not silently become a replacement for TLS, DTLS, EDHOC, OSCORE, ACE, or a general-purpose credential ecosystem.

The specification effort should define:

- native ZK-ARCHE wire semantics;
- explicit negotiated profiles for constrained and edge deployments;
- optional CoAP/OSCORE-oriented mappings when measured;
- optional TLS/mTLS exporter-bound operation;
- native datagram robustness informed by DTLS practice;
- explicit ENROLL/TRUST lifecycle behavior separate from AUTH;
- a shared Rust/C conformance and interoperability corpus;
- exact claim boundaries for protocol conformance, target support, external review, and deployment readiness.

Non-goals:

- no claim of IETF standardization without the IETF process;
- no mandatory TLS, DTLS, CBOR, COSE, OAuth/ACE, BBS, PQ, or general-purpose ZK dependency for the constrained core without explicit reviewed adoption;
- no trust-on-first-use side effect hidden inside normal AUTH;
- no stable identity or authorization shortcut introduced merely for lookup convenience;
- no maturity claim inferred from RFC-style prose alone.

## 4. Standards and research used as engineering comparators

| Reference family | Useful discipline | ZK-ARCHE implication |
|---|---|---|
| EDHOC / LAKE | compact authenticated exchange, profile negotiation, exporter/context discipline, retransmission considerations | benchmark and adopt explicit profile/context semantics where appropriate |
| OSCORE / KUDOS | replay/key-usage/rekey lifecycle and constrained state handling | define lifecycle transitions, exhaustion, restart, and rekey behavior |
| ACE | separation of authentication and authorization, scoped/expiring rights, revocation convergence | keep authn/authz distinct and model authorization lineage/epochs |
| TLS 1.3 / RFC 9266 / exporters | transcript binding and upper-layer channel binding | bind a unique ZK-ARCHE AUTH instance, not only a transport connection |
| DTLS 1.3 | datagram replay/retransmit/source validation | define native UDP robustness explicitly |
| BRSKI/FDO/Matter-style onboarding | commissioning/enrollment as an authorized lifecycle act | keep enrollment/trust mutation separate from AUTH |
| CFRG Sigma / Fiat-Shamir work | proof interface, canonical serialization, composition/domain-separation review | make the custom proof review contract explicit |
| SAPIC+/ProVerif/Tamarin work | formal traceability and privacy properties | model one canonical state machine with explicit attacker/property matrix |
| NIST constrained/module guidance | execution boundary, entropy/key-storage/self-test context | require benchmark manifests without implying certification |

Comparators provide evidence and design vocabulary; they do not override ZK-ARCHE’s own threat model, wire format, or profile constraints.

## 5. RFC-style deliverables

Before specification-grade claims, maintain and cross-link at least:

| Artifact | Path | Required content |
|---|---|---|
| Protocol specification | `spec/zk-arche-protocol.md` | normative flows, state transitions, fields, cryptographic computations, errors |
| Registries | `spec/registries.md` | versions, methods, suites, profiles, extensions, message/error/binding identifiers, compatibility rules |
| IoT profiles | `spec/iot-profiles.md` | negotiated profile semantics, resource ceilings, evidence requirements |
| Security considerations | `spec/security-considerations.md` | replay, downgrade, UKS/reflection, DoS, RNG/storage, lifecycle, compromise assumptions |
| Privacy considerations | `spec/privacy-considerations.md` | anonymity, unlinkability, role privacy, failure/timing/metadata surfaces |
| Test-vector specification | `spec/test-vectors.md` | canonical positive/negative vectors, versioning/regeneration rules |
| Implementation requirements | `spec/implementation-requirements.md` | bounded parsing/state, RNG/DRBG, storage, constant-time, error behavior |
| Interop evidence | `evidence/interop-matrix.md` when curated | feature/vector/profile parity and known gaps |
| Formal traceability | retained assurance artifact | model state/event ↔ spec section ↔ Rust/C implementation mapping |
| Target evidence | retained benchmark artifacts | execution-context manifest + wire/RAM/CPU/flash/storage measurements |

Every normative security behavior must have at least one of:

```text
positive vector
negative vector
executable test
formal-model property/result
reviewed proof argument
explicit unresolved evidence gap
```

## 6. Terminology that must become normative

The specification should distinguish at minimum:

### Authentication

Cryptographic evidence that a peer possesses or satisfies the credential/proof requirements associated with existing trusted state.

### Authorization

The decision that an authenticated peer may perform a particular operation under explicit scope, audience/deployment, role/policy, validity, epoch, and revocation state.

### Trust mutation

A state-changing act that creates, replaces, delegates, revokes, or otherwise modifies trusted credentials, roles, authorities, or lineage. Trust mutation requires an explicit authorized lifecycle operation and cannot be a side effect of normal AUTH.

### Profile

A negotiated set of protocol/security semantics and limits. It is not the same as a local runtime resource preset or a bag of capability bits.

### Capability / extension

Optional behavior advertised or carried within profile-defined compatibility rules. Critical and ignorable unknown values must be distinguished.

### Authorization lineage / generation

The versioned relationship by which current authorization replaces or descends from earlier authorization state and can be invalidated consistently across sessions, resumption, derived keys, and DATA release state.

### Revocation view

The durable issuer/authority/epoch-scoped state by which a peer knows which credential/authorization state is no longer valid and how fresh that knowledge is.

## 7. Candidate protocol profiles

### ZK-ARCHE-Core / `iot-core`

Target: STM32-class and ESP32-S3-class constrained devices interoperating with capable MCU or Linux peers.

Requirements:

- fixed/bounded message and state buffers;
- no mandatory heap-heavy parser in constrained C hot paths;
- no mandatory anonymous-credential, PQ-hybrid, certificate-chain, large trust-graph, or general-purpose ZK dependency;
- small-datagram design target unless the profile explicitly selects fragmentation/stream transport;
- strict sequence/replay/transcript/profile/extension negative tests;
- deterministic Rust vectors with C semantic reproduction;
- NO-LEARNING normal AUTH;
- exact target evidence before field-readiness language.

### `p2p-iot-core`

Adds initiator/responder mutual authentication and local trust evaluation while retaining the same constrained floor. Trust is not implicitly transitive and delegation remains bounded and revocable.

### ZK-ARCHE-Edge / `iot-edge`

Target: Raspberry Pi/Jetson-class peers, gateways, commissioners, and servers.

May support larger registries, encrypted lookup hints, richer policy evaluation, more extensive evidence collection, transport bindings, optional advanced credentials, or PQ experiments when profile-negotiated and measured.

### ZK-ARCHE-Research

Target: alternative proofs/credentials, PQ/hybrid suites, formal exploration, encoding experiments, and external review. Research features must remain isolated from mandatory constrained behavior until explicitly promoted.

## 8. Native specification shape

The main specification should evolve toward the following sections:

1. Introduction and applicability
2. Terminology and trust roles
3. Threat model and protocol goals
4. Protocol overview and suite decomposition
5. Constants, identifiers, and registries
6. Cryptographic suites and method/suite compatibility
7. Canonical encoding and transcript construction
8. HELLO / capability advertisement / profile selection
9. SETUP / bootstrap / initial trust establishment
10. AUTH
11. Authorization-context evaluation
12. Late enrollment and commissioner grants
13. Rekey / re-registration / lineage replacement
14. Revocation convergence and invalidation propagation
15. Session/key lifecycle and usage limits
16. Session resumption
17. Secure-channel and transport bindings
18. Native datagram behavior
19. Error handling and observable failure classes
20. State machines
21. Test vectors and negative vectors
22. Security considerations
23. Privacy considerations
24. Constrained implementation requirements
25. Registry and extension policy
26. Conformance and claim language

Normative MUST/SHOULD language should be added only when behavior is explicit enough to implement and test independently.

## 9. HELLO, profile selection, registries, and extensibility

The current protocol concepts must be normalized into separate layers:

```text
local runtime/resource configuration
≠ negotiated protocol/security profile
≠ optional capabilities/extensions
```

The spec should define:

- stable profile identifiers;
- which parameters are prescriptive for each profile;
- how profiles are advertised/selected;
- whether capabilities can further specialize an already selected profile;
- critical vs ignorable extension semantics;
- suite/method/profile compatibility constraints;
- behavior for known-but-incompatible combinations;
- fail-closed behavior when a peer violates a prescriptive profile parameter;
- transcript binding for selected profile and security-relevant negotiation;
- reserved values usable in deterministic GREASE-style conformance tests.

The conformance suite should continuously exercise unknown extension paths so extensibility does not ossify.

## 10. AUTH and proof semantics

Normal AUTH must remain a prior-trust operation. A proof-valid but unenrolled peer remains unauthorized unless a distinct ENROLL/TRUST flow creates the necessary state.

The specification must clearly separate:

```text
credential/proof possession
role-membership statement
peer identity/commitment binding
authorization scope/audience/policy
session/channel binding
trust-store mutation
```

The custom role-membership proof cannot be normatively described only by source code. The RFC-style package must eventually document enough of the statement/witness relation, serialization, challenge construction, role ordering, protocol/instance identifiers, invalid-input handling, and implementation boundaries for independent review and interop testing.

Any review result that changes semantics requires versioned vectors and checkpoint review.

## 11. Transcript and context binding

AUTH transcript v3 should bind every field that can change security semantics. Candidate mandatory categories include:

```text
protocol/version
method + suite + selected profile
critical capabilities/extensions
session and sequence identity
ephemeral keys
peer/server identity or commitments
role/policy/authorization context
deployment/domain/audience
transport/channel-binding label
fresh upper-layer AUTH-instance identity where applicable
canonical payload bytes
```

The transcript specification must define exact field order, length/canonical encoding, domain separators, and omission rules. A field that affects semantics but is not bound needs an explicit justification and threat analysis.

## 12. Authentication versus authorization

The normative package should introduce an authorization-context object or equivalent semantics distinct from the authentication proof.

At minimum, future authorization state should be capable of binding:

- holder or authenticated-session confirmation;
- audience/resource/group/deployment scope;
- requested vs granted role/policy;
- issuer/authority;
- validity/expiration;
- authorization lineage/generation;
- relevant policy/registry/revocation epoch;
- allowed operations/profile where required.

A valid authentication proof must not be specified as indefinite authorization.

## 13. Enrollment and trust mutation

### NO-LEARNING baseline

Normal AUTH and P2P AUTH do not create trusted records. Explicit trust mutation is confined to reviewed lifecycle operations.

### Late enrollment

A future one-time grant should be scoped and replay protected. Candidate binding includes suite/profile compatibility, server/domain/audience, intended holder/device commitment where possible, allowed role/policy, issuer/commissioner authority, validity, nonce/replay state, and authorization lineage.

### Commissioner enrollment

Commissioner power must be bounded by role/scope/depth/validity/epoch and auditable. A commissioner cannot transfer or expose an existing peer’s secret merely by delegating enrollment.

## 14. Rekey, key usage, replay continuity, and invalidation

Rekey, replay protection, and authorization lifecycle must be specified together rather than as independent utilities.

The package should eventually define:

- per-suite encryption/authentication usage accounting where applicable;
- warning and hard-stop thresholds;
- rekey triggers;
- replay window/counter persistence;
- behavior after replay-state loss or rollback;
- authenticated continuity restoration or mandatory rekey/full AUTH;
- atomic credential/key lineage replacement;
- invalidation propagation from credential/policy/authorization change into sessions, resumption, derived keys, and DATA release state.

Loss of durable replay/lineage state must not silently become a cache miss that reopens acceptance.

## 15. Revocation convergence

Revocation semantics must acknowledge disconnected and intermittently connected deployments.

A future profile should define a durable revocation view or equivalent, including:

- issuer/authority and epoch/cursor identity;
- full snapshot and bounded differential reconciliation where appropriate;
- duplicate/out-of-order handling;
- reconnect behavior after missed updates;
- rollback/stale-view detection;
- maximum stale-authorization window or explicit freshness objective per profile;
- behavior when the peer cannot establish sufficiently fresh authorization state;
- privacy consequences of revocation identifiers or global feeds.

The constrained baseline must not silently require a continuously reachable central authority.

## 16. Session resumption

Resumption must preserve current authorization semantics, not only cryptographic possession of a ticket/PSK.

A future resumption record should bind enough state to evaluate whether privilege remains valid, including as appropriate:

```text
holder/peer
suite/profile
deployment/domain/audience
original security context
authorization lineage/generation
policy/registry/revocation epoch
issue + expiry time
reuse/usage counter or policy
privacy-relevant identifier state
```

Required failure cases include:

- revoked credential;
- stale authorization lineage;
- changed role/policy/audience/deployment;
- missing cached authorization state;
- rollback to older resumption state;
- excessive reuse;
- repeated identifier linkability;
- inability to safely reevaluate context.

When a safe decision cannot be made, the normative behavior should be full AUTH rather than silent privilege carry-forward.

General-purpose state-changing 0-RTT remains outside the constrained baseline.

## 17. Error handling and privacy observability

The specification must treat externally observable failure behavior as part of the privacy surface.

Privacy analysis should inventory:

- response vs no response;
- alert/error class;
- packet/response size bucket;
- retry behavior;
- response timing/dummy work where applicable;
- known vs unknown credential/reference behavior;
- allowed vs denied role/policy behavior;
- capability/suite/profile ordering and fingerprinting;
- session/connection identifiers;
- retry tokens;
- resumption ticket/PSK identifiers;
- lower-layer identifiers outside ZK-ARCHE’s control but relevant to claims.

Anonymity and unlinkability must be specified as separate properties under explicit attacker models. Changing PID values is not, by itself, proof of unlinkability.

## 18. `AUTH_RETRY`, pre-auth cost, and denial of service

Native datagram deployments may use a stateless or bounded MAC-only retry/source-validation step before expensive registry lookup or proof verification.

Normative work must define:

- retry token binding inputs;
- lifetime/replay rules;
- address/endpoint assumptions;
- maximum pre-validation response/amplification;
- maximum unauthenticated state;
- interaction with registry lookup hints;
- privacy impact of different failure classes;
- malformed/oversized input behavior.

The objective is bounded asymmetry: unauthenticated traffic should not cheaply force O(n) registry work or expensive proof verification.

## 19. Optional encrypted registry lookup hints

The first concrete optional design may use a randomized encrypted opaque registry key as a non-authoritative prefilter.

Normative invariants, if promoted, should include:

- hint success never establishes identity or authorization;
- full PID/possession/proof/transcript/state verification remains required;
- hint key/epoch rotation and revocation are defined;
- wrong/malformed/decryption-failed hints cannot bypass retry or throttling;
- passive-linkability and length leakage are measured;
- Rust/C vectors cover correct and incorrect hints;
- the privacy-preserving O(n) scan remains an allowed fallback where required.

VOPRF/POPRF remains a comparator unless a stronger server-blindness threat model justifies its added interaction/state.

## 20. TLS/mTLS exporter-bound profile

ZK-ARCHE may bind application-layer AUTH to TLS exporter material when TLS is already present.

Candidate construction remains conceptually:

```text
zk_arche_tls_channel_binding =
    TLS-Exporter("EXPORTER-ZK-ARCHE-v1", auth_instance_context, 32)
```

The `auth_instance_context` must be normative and unique to the upper-layer ZK-ARCHE authentication instance. Candidate inputs include:

- application/ALPN context;
- deployment/domain;
- endpoint identities or protocol-safe commitments;
- method/suite/profile;
- a fresh AUTH-instance/session identifier;
- relevant transcript hash/state.

Required negative fixtures include wrong application, wrong deployment, wrong endpoint, wrong suite/profile, stale/cross-instance AUTH identity, TLS resumption mismatch, and proxy/termination assumptions.

mTLS certificates must not be described as unlinkable merely because ZK-ARCHE runs over the connection.

## 21. Native datagram / DTLS-style robustness

Native UDP behavior should explicitly specify:

- session/epoch identity;
- source validation/retry;
- replay window and duplicate suppression;
- retransmission and response-cache rules;
- reorder tolerance;
- wrong-address behavior;
- maximum pending state;
- amplification limits;
- stale/cross-session rejection;
- privacy-aware alerts/failures.

Every accepted retransmission behavior must be deterministic enough for Rust/C interoperability tests.

## 22. EDHOC/CoAP/OSCORE-inspired constrained profile work

ZK-ARCHE may borrow constrained-protocol discipline without copying dependencies or claiming equivalence.

Measured comparison should cover:

- total handshake bytes and flights;
- base authenticated key-exchange overhead vs ZK-ARCHE-specific authorization/privacy overhead;
- exporter/context semantics;
- parser/encoding cost;
- retransmission behavior;
- CPU/RAM/flash on at least one MCU-class target;
- fragmentation/MTU consequences.

CBOR/CDDL/COSE remain optional specification/encoding research until target evidence and migration cost justify adoption.

## 23. Formal specification and model traceability

The formal-assurance package should not consist of independent hand-maintained models that drift semantically.

Preferred direction:

- one canonical model source or mechanically synchronized representation;
- pinned reproducible tooling;
- a property/attacker matrix before proof work;
- explicit state for registry/credential mappings, replay continuity, authorization lineage, revocation, retry/error behavior, and repeated sessions where relevant;
- separate anonymity and unlinkability properties;
- retained counterexamples;
- mappings from model states/events to normative spec sections and concrete Rust/C functions/tests.

Formal claims must remain scoped to modeled properties and assumptions.

## 24. Constrained implementation requirements

The implementation-requirements specification should eventually mandate or bound:

- parser/message/state sizes;
- dynamic-allocation policy for constrained C paths;
- maximum pending sessions and caches;
- registry/trust scaling assumptions;
- RNG/entropy/DRBG failure behavior;
- secure-storage and rollback assumptions;
- key/seed zeroization requirements where applicable;
- constant-time boundaries;
- restart/crash consistency;
- versioned `crypto_execution_context` evidence for benchmarked targets.

A field-readiness claim must additionally identify deployment context: firmware/update posture, provisioning/revocation authority, secure boot/debug configuration, external controls, and residual assumptions. Protocol conformance alone cannot satisfy this requirement.

## 25. Conformance and interoperability

Conformance should be defined by declared profile and evidence, not by vague “supports ZK-ARCHE” language.

Candidate labels:

```text
ZK-ARCHE-Core protocol-conformant
ZK-ARCHE-Core Rust/C interoperable
ZK-ARCHE-Core measured on <target>
ZK-ARCHE P2P profile-conformant
ZK-ARCHE transport-binding profile-conformant
```

Labels must not imply external review, formal verification, field readiness, or certification unless those independent evidence gates are also satisfied.

The conformance corpus should include:

- positive vectors;
- malformed/oversized input;
- wrong type/sequence/session;
- replay/reorder/reflection;
- negotiation downgrade;
- unsupported critical value;
- ignorable unknown extension;
- incompatible method/suite/profile combination;
- transcript mutation;
- authorization expiry/revocation/lineage mismatch;
- resumption stale-context/reuse cases;
- channel-binding mismatch;
- privacy/failure-class fixtures where deterministic testing is possible.

## 26. Migration sequence

Recommended sequence:

1. Preserve Rust canonical vector generation and C independent reproduction.
2. Stabilize clean-checkout CI and evidence generation.
3. Write exact current wire/header/TLV/AUTH behavior into the spec without changing semantics merely for cleanliness.
4. Normalize profile/capability/registry terminology and compatibility semantics.
5. Specify authentication/authorization/trust-mutation boundaries.
6. Specify transcript v3 and strict AUTH state/error behavior with negative vectors.
7. Specify lifecycle state: replay continuity, key usage, rekey, revocation convergence, resumption, invalidation propagation.
8. Expand formal traceability against the same state machine.
9. Add target evidence manifests and constrained measurements.
10. Add optional TLS exporter, native datagram, and constrained protocol mappings only after the base semantics are stable.
11. Add conformance labels only when tests/vectors/evidence support the exact label.
12. Keep BBS/PQ/alternative proof/encoding work isolated until a future explicit promotion decision.

## 27. Specification completion rule

An RFC-style section is not complete because prose exists. It is complete when:

```text
normative behavior is unambiguous
+ identifiers/encodings are registered or explicitly fixed
+ state transitions and failure behavior are defined
+ Rust and C can implement the same semantics independently
+ positive/negative evidence exists
+ security/privacy assumptions are stated
+ unresolved evidence gaps are explicit
```

Where those conditions are not yet met, keep the section visibly draft and avoid premature normative claims.
