# ZK-ARCHE Privacy Considerations

Status: **normative-privacy consolidation draft**. This document records privacy properties, observable metadata, assumptions, residual risks, and unresolved privacy semantics already implied by the current specification, roadmap, assurance artifacts, and implementation evidence. It does **not** allocate new wire behavior, claim anonymous authentication beyond retained evidence, promote optional anonymous-credential research, clear TD-001 through TD-004, or imply IETF/RFC status.

Normative keywords are used only where current repository behavior is already precise enough to test. Where privacy behavior remains unresolved, this document names the gap rather than selecting policy here.

## 1. Privacy objective and claim boundary

ZK-ARCHE aims to minimize unnecessary disclosure of stable identity, authorization state, role membership, trust topology, and application context while preserving explicit authentication, authorization, replay, and lifecycle security properties.

Privacy is not equivalent to authentication secrecy. A cryptographically authenticated exchange can still leak identity through stable pseudonyms, transport addresses, packet sizes, timing, retries, lookup behavior, channel bindings, deployment topology, or repeated protocol state. Likewise, a role-membership proof can hide one attribute while other fields remain linkable.

The current repository therefore treats the following as separate evidence questions:

- whether a protected field is absent from cleartext wire encoding;
- whether the same peer/session can be linked across exchanges;
- whether an observer can infer a role, authorization decision, trust relation, or lifecycle event;
- whether an authorized peer learns more than is required for the operation;
- whether a transport or channel binding introduces an external stable identifier;
- whether implementation timing, allocation, error, retry, or lookup behavior creates an oracle;
- whether a formal model proves an observational-equivalence privacy property for the exact modeled behavior.

No stronger privacy state is inferred from a weaker one.

## 2. Threat model for privacy analysis

Privacy analysis considers at least these observers separately:

- a passive network observer able to record packet contents, sizes, timing, direction, retransmissions, endpoints, and long-term traffic patterns;
- an active network attacker able to inject, delay, replay, reorder, reflect, fragment, duplicate, and selectively drop traffic to probe observable behavior;
- an authenticated but unauthorized or over-curious peer attempting to infer roles, grants, trust edges, credential references, or local policy;
- an authorized peer attempting to correlate multiple sessions or lifecycle transitions beyond what its operation requires;
- a transport or infrastructure operator able to observe addresses, connection identifiers, routing metadata, TLS certificates, DNS or rendezvous information, or gateway topology;
- a local implementation observer able to measure timing, memory access, persistence, logging, crash behavior, or debug output.

The current repository does not claim protection against global traffic analysis, physical device tracking, radio-frequency fingerprinting, host compromise, privileged local inspection, or hardware side channels unless a separate artifact explicitly qualifies such a claim.

## 3. Pseudonyms, peer identifiers, and unlinkability

A pseudonymous identifier is not automatically unlinkable. If a PID, credential reference, lookup key, key fingerprint, session-independent handle, or other stable value repeats across authentications, a passive or active observer may correlate those exchanges even when a human-readable identity is absent.

An implementation MUST NOT describe a field as "anonymous" merely because it is not a legal name or globally registered identity. Privacy claims must state the correlation scope under which the value is stable, including whether stability is per credential, peer pair, role, deployment, authorization generation, session, transport binding, or longer-lived trust lineage.

The current mandatory core does not yet have retained evidence sufficient to claim general cross-session or cross-transport unlinkability. Any stronger unlinkability statement requires an explicit equivalence definition, adversary model, exact wire/runtime behavior, and retained tests or formal analysis.

## 4. Role and authorization privacy

The custom role-membership construction is intended to prove authorization-related membership properties without disclosing unnecessary role information, but TD-001 independent cryptographic review remains open. Deterministic vectors, Rust/C agreement, symbolic abstractions, and successful protocol authentication do not establish computational role privacy by themselves.

Authorization processing should reveal only what is necessary to decide the requested operation. A peer MUST NOT expose the complete local role set, trust store, policy graph, unrelated grants, or authorization database solely to answer a narrower authorization question when current protocol semantics do not require that disclosure.

Authorization acceptance/rejection can itself leak information. Distinct error codes, response sizes, retry behavior, processing latency, lookup depth, or branch-specific network behavior may let an attacker test hypotheses about whether a credential, role, grant, predecessor, audience, or authorization generation exists. General privacy-preserving authorization-error semantics remain incomplete TD-004 work.

## 5. NO-LEARNING AUTH and privacy

Normal AUTH is NO-LEARNING with respect to trust mutation: successful authentication must not create new trusted state. That property reduces one class of privacy risk because an unauthenticated or merely authenticated exchange does not automatically cause a peer to persist a new trust relationship.

NO-LEARNING does **not** mean NO-OBSERVATION. A peer may still learn that a known credential is active, that a particular authorization context is accepted, that a session completed, or that a transport endpoint is currently reachable. Such observations must not be promoted into broader trust or identity claims unless a reviewed lifecycle operation authorizes that mutation.

## 6. Lookup behavior and private discovery

Server-side credential or authorization lookup can leak privacy even when wire identifiers are compact. Lookup-table access patterns, variable search depth, cache hits, negative-result timing, memory allocation, storage access, or remote lookup traffic may reveal whether a candidate identity or role is present.

The current repository does not claim oblivious or private lookup. Any encrypted lookup hint, PIR-like mechanism, private-set-membership technique, HPKE envelope, large-registry index, or anonymous-credential extension remains optional research until explicitly promoted with target, parser, interoperability, and review evidence.

Mandatory constrained peers must not be forced to adopt gateway-class private-lookup machinery merely to satisfy the Common Contract.

## 7. Transport metadata and channel binding

Transport independence does not imply metadata privacy. Depending on the adapter, an observer may learn IP addresses, ports, MAC addresses, BLE identities, radio channels, serial endpoints, CAN identifiers, robotics middleware names, gateway routes, connection lifetimes, or packet direction.

Transport addresses are not ZK-ARCHE protocol identity, but they can still be powerful correlators. NAT rebinding or address mobility may reduce or alter linkability without providing cryptographic anonymity.

Where AUTH is bound to an existing secure channel, the channel-binding value may intentionally connect the ZK-ARCHE exchange to that transport-security context. That binding can improve security while reducing privacy across otherwise separable layers. Privacy claims must therefore identify whether the binding is externally observable, stable across sessions, or derived from a channel whose peer identity is independently linkable.

## 8. TLS/mTLS binding and certificate linkability

TLS or mTLS is an optional binding, not a mandatory ZK-ARCHE trust root. When mTLS is used, client certificates, certificate chains, raw public keys, resumption identifiers, server names, or associated deployment metadata may create strong cross-session linkability independent of ZK-ARCHE's own pseudonym or role-proof design.

A deployment MUST NOT claim ZK-ARCHE-level unlinkability while ignoring a stable lower-layer certificate or transport identity that trivially correlates the same sessions. Privacy analysis for a bound profile must cover the composed stack, not only the ZK-ARCHE message fields.

## 9. Timing, packet size, retry, and state-machine leakage

Even when sensitive fields are encrypted or omitted, an observer may distinguish protocol branches through message count, packet size, fragmentation, ordering, retransmission, retry-cookie behavior, timeout, or completion latency.

Current repository evidence does not establish that all rejection paths are observationally equivalent. Implementations should avoid unnecessary branch-specific diagnostics before authentication and should not add response distinctions merely for convenience when those distinctions reveal protected state.

Constant-time cryptographic implementation and constant-observable protocol behavior are separate requirements. Symbolic secrecy or correspondence results do not prove either property.

## 10. Replay, restart, and privacy state

Replay caches, session identifiers, generations, epochs, tickets, predecessor references, or freshness anchors can be security-critical while also serving as correlators. Retaining a stable anti-replay or lifecycle identifier may therefore create a deliberate privacy/security tradeoff.

Restart must not silently reset a privacy guarantee. If a peer rotates pseudonyms, tickets, replay state, or lookup identifiers, the rotation and recovery behavior must be defined consistently with replay and authorization requirements. Conversely, deleting correlation state to improve privacy must not reopen replay or stale-authorization acceptance.

No current repository evidence justifies a general claim that restart improves unlinkability or that retained replay state is privacy-neutral.

## 11. Resumption and linkability

Resumption is a distinct privacy mode. A resumption ticket, PSK identity, cache key, authorization-generation handle, or reuse policy can allow repeated sessions to be correlated even when the full authentication exchange would otherwise expose less stable material.

Authorization-aware resumption semantics remain incomplete. Before a profile can claim privacy-preserving resumption, it must define at least ticket/PSK scope, lifetime, audience, authorization-context binding, revocation interaction, single-use or bounded-reuse behavior, compromise consequences, and observable rejection behavior.

TLS, QUIC, EDHOC, or another comparator may inform this design but does not automatically provide the ZK-ARCHE privacy property.

## 12. Enrollment, commissioner grants, and first-contact privacy

Enrollment and commissioner/grant flows intentionally mutate trust and therefore may disclose more context than normal AUTH. A commissioner may need to learn the device, requested role, provenance, authorization scope, or deployment context in order to make an explicit trust decision.

The repository does not currently claim anonymous enrollment, anonymous commissioning, or privacy-preserving provenance. Late-enrollment or disconnected-enrollment designs must state which party learns the joining peer, credential, role request, grant issuer, and deployment scope, and what durable audit state is retained.

Normal AUTH privacy claims must not be generalized to enrollment simply because both flows reuse cryptographic primitives or credential formats.

## 13. Rekey, revocation, and lineage privacy

LINEAGE_REPLACE and related lifecycle operations can expose that a key or credential is being replaced, the timing of replacement, predecessor/successor relationships, authorization generation changes, or recovery after interruption. Those facts may be operationally sensitive even if key material remains secret.

Current lineage contracts prioritize fail-closed authorization, freshness, and durable replacement ordering. They do not establish that predecessor/successor relationships are hidden from an observing authorized peer, local storage observer, or transport observer.

Revocation propagation can similarly leak membership, failure, or policy events. General revocation convergence and stale-authorization bounds remain unresolved, so no privacy theorem is inferred for disconnected revocation behavior.

## 14. Logging, telemetry, and retained evidence

Operational logs, traces, benchmark manifests, crash dumps, packet captures, and assurance evidence can contain identifiers, role decisions, transport endpoints, timing, storage generations, or credential references that are not necessary in ordinary production logs.

Test and assurance tooling should prefer synthetic or explicitly authorized identifiers where possible and should document when retained evidence contains privacy-sensitive material. Production logging should avoid persisting secrets, raw long-term private keys, complete trust stores, or unnecessarily stable identifiers.

A privacy property at the wire layer does not survive if equivalent identifying material is emitted to logs, telemetry, analytics, crash reporting, or external infrastructure.

## 15. Optional anonymous credentials and stronger privacy profiles

BBS-style credentials, anonymous credentials, selective disclosure, private lookup, PQ/hybrid privacy mechanisms, and large trust-graph privacy remain optional research unless explicitly promoted. They MUST NOT become hidden prerequisites for `iot-core` or `p2p-iot-core` without roadmap promotion, Rust/C interoperability, parser/state-machine definition, resource evidence, and appropriate cryptographic review.

A higher-capability peer may support a stronger optional privacy profile, but absence of that feature on a constrained peer must not silently weaken the mandatory authentication security floor or make the constrained peer non-interoperable with the mandatory core.

## 16. Formal privacy-analysis boundary

Current formal work primarily establishes scoped authentication, replay, secrecy, and lifecycle-ordering properties. It does not establish a general anonymity, unlinkability, role-indistinguishability, or failure-observability theorem for complete runtime behavior.

A formal privacy claim requires an observational-equivalence style property (or another explicitly justified privacy formulation) whose two worlds differ only in the protected fact while all permitted public context remains controlled. The exact model text, tool/version, queries, attacker assumptions, and result must be retained.

Even a successful symbolic equivalence result would not prove transport-layer anonymity, implementation constant-time behavior, traffic-analysis resistance, physical unlinkability, secure logging, or computational privacy of the custom proof.

Privacy-model work must remain synchronized with normative runtime behavior. Where the specification has not defined error observability, lookup semantics, resumption reuse, or credential/reference privacy, the model must mark the property blocked rather than inventing a convenient abstraction.

## 17. Privacy evidence and regression testing

Privacy regressions should be tested at the smallest observable boundary that can falsify a claim. Useful evidence classes include:

- repeated-authentication captures showing which identifiers and lengths remain stable;
- positive/negative tests demonstrating that unrelated role/trust records are not serialized or returned;
- cross-session and cross-transport correlation tests for pseudonyms, lookup handles, channel bindings, tickets, and credential references;
- timing/response-shape differential tests for selected accepted and rejected paths where a bounded constant-observable claim is intended;
- restart/resumption tests showing whether identifiers rotate or persist exactly as specified;
- composed-stack traces when TLS/mTLS or another binding is used;
- formal observational-equivalence results for precisely defined privacy properties.

The absence of a detected difference in a finite test corpus is not proof of unlinkability. Testing supplies falsification evidence; stronger privacy claims require an explicit claim definition and appropriate analysis.

## 18. Data minimization and data-sovereignty relationship

Privacy and data sovereignty overlap but are not the same property. Data sovereignty concerns who controls protected data, release policy, encryption, audit, provenance, and lifecycle; privacy additionally concerns what observers and authorized parties can infer from metadata and protocol behavior.

Future ZK-ARCHE-DATA work should minimize disclosed metadata, bind release decisions to authenticated/authorized context, and avoid making a gateway or cloud service the mandatory point where protected plaintext becomes visible unless the promoted profile explicitly requires that trust relationship.

Data-sovereignty implementation must not be used to claim anonymity or unlinkability without separate privacy evidence.

## 19. Privacy evidence states that must not be conflated

The repository's general evidence states remain independent:

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

For privacy specifically:

- a field being encrypted does not imply unlinkability;
- a pseudonym does not imply anonymity;
- a role proof does not imply computational role privacy without review;
- a symbolic secrecy query does not imply observational equivalence;
- a host timing test does not imply MCU side-channel resistance;
- a ZK-ARCHE message property does not override linkability introduced by TLS/mTLS or another transport;
- one privacy-preserving optional profile does not redefine the mandatory Common Contract;
- RFC-style prose does not imply RFC-class completion or IETF status.

## 20. Residual open privacy work

Material open privacy work includes at least:

- explicit cross-session PID/credential-reference correlation semantics;
- complete role/privacy claim language after TD-001 independent review;
- privacy-preserving or explicitly bounded credential/authorization lookup behavior;
- observable error/alert/retry semantics and differential testing;
- authorization-aware resumption and ticket/PSK reuse bounds;
- revocation and lineage-transition observability;
- privacy semantics for enrollment, commissioner grants, delegation, and provenance;
- composed-stack privacy analysis for TLS/mTLS and other channel bindings;
- privacy behavior under restart, replay-state persistence, and transport migration;
- formal observational-equivalence properties for anonymity/unlinkability/role-indistinguishability where normative behavior exists;
- constrained-target timing/resource evidence where a target-specific privacy claim is intended;
- logging/telemetry guidance for deployment-qualified profiles.

These are retained gaps, not implied future wire behavior. This document consolidates exact-current privacy boundaries and advances TD-004 documentation quality without closing TD-001, TD-002, TD-003, TD-004, or any deployment/privacy claim whose required evidence does not yet exist.
