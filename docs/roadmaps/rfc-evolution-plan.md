# ZK-ARCHE RFC-Class Protocol Evolution Plan

This document defines how ZK-ARCHE should evolve from a working Rust/C research implementation into a protocol package that can be specified, implemented, tested, reviewed, and interoperated with the discipline expected of mature Internet security protocols.

The objective is **RFC-class engineering quality**, not a claim of IETF adoption. ZK-ARCHE is not an Internet Standard or RFC unless it actually completes the applicable IETF process. The purpose of this plan is to hold ZK-ARCHE to comparable standards of specification precision, interoperability, security analysis, change control, implementation independence, and reproducible evidence.

ZK-ARCHE must remain its own protocol. RFCs and external protocols are engineering references and comparators. They do not become dependencies merely because ZK-ARCHE borrows a successful design discipline.

---

## 1. Specification authority and promotion boundary

The RFC-style package sits downstream of research and roadmap decisions:

```text
external RFC / paper / implementation / benchmark
        ↓
docs/research/
        ↓
docs/findings/
        ↓ explicit human promotion
docs/roadmaps/ + docs/adr/
        ↓
spec/ normative text
        ↓
Rust + C + vectors/tests
        ↓
formal / fuzz / benchmark / review evidence
        ↓
conformance + release claims
```

An RFC, Internet-Draft, academic protocol, or successful implementation is a comparator until ZK-ARCHE explicitly adopts a behavior. Source code is not normative merely because it exists. Security- or wire-relevant behavior belongs in `spec/` and must be tied to executable evidence or an explicit unresolved gap.

---

## 2. Current specification baseline — 2026-08-25

The repository has two active implementation lanes:

- Rust reference implementation and canonical deterministic vectors;
- independent C implementation and constrained-device portability anchor.

The controlling specification debt remains TD-004, with adjacent dependencies on:

- TD-001: independent cryptographic review of the custom role-membership proof;
- TD-002: target-class constrained-device evidence;
- TD-003: formal model expansion and model-to-code traceability.

The Common Contract and bottom-up doctrine in `improvement-roadmap.md` are normative roadmap intent for this evolution plan:

- less-capable and more-capable peers must share the same mandatory authentication assurance floor;
- basic P2P authentication must not require a CA or always-online external infrastructure;
- transport/platform differences must be normalized below the common security contract;
- optional capabilities may scale upward without weakening or replacing the constrained interoperability floor;
- authentication, authorization, and trust mutation remain separate semantics;
- normal AUTH is NO-LEARNING.

---

## 3. RFC-class quality target

A ZK-ARCHE specification-grade release should be reviewable as if an independent standards body were evaluating it. At minimum, the protocol package must provide:

```text
scope and applicability
precise terminology
threat model
security goals and non-goals
wire grammar / canonical encoding
protocol state machines
cryptographic computations
key schedule / domain separation
mandatory-to-implement floor
negotiation and downgrade behavior
extension rules
registries and allocation policy
error/alert behavior
replay / retry / retransmission behavior
rekey and resumption lifecycle
trust / enrollment / revocation semantics
privacy considerations
security considerations
implementation requirements
resource / MTU limits for constrained profiles
positive vectors
negative vectors
annotated traces
independent implementation interoperability
formal-analysis scope and results
external review status
change-control / versioning policy
known limitations
claim boundaries
```

The protocol is not “RFC-class” merely because the prose resembles an RFC. The package must be independently implementable from the specification and produce the same wire bytes and accept/reject decisions across conformant implementations.

---

## 4. Standards-process references

These references define the **quality discipline** around the protocol, not the ZK-ARCHE wire design itself.

| Reference | Discipline to adopt | ZK-ARCHE requirement |
|---|---|---|
| BCP 14 — RFC 2119 + RFC 8174 | precise normative keywords | use MUST/SHOULD/MAY only for behavior that is sufficiently specified and testable |
| BCP 72 — RFC 3552 | rigorous Security Considerations | explicitly enumerate in-scope, out-of-scope, mitigated, and residual threats |
| BCP 26 — RFC 8126 | registry/change-control discipline | define internal registries with ranges, allocation policy, reserved values, deprecation, and change control |
| RFC 7322 and RFC-series style guidance | readable standards structure | keep terminology, references, examples, appendices, and normative/informative material clearly separated |

Reference links:

- https://www.rfc-editor.org/info/rfc2119
- https://www.rfc-editor.org/info/rfc8174
- https://www.rfc-editor.org/info/rfc3552
- https://www.rfc-editor.org/info/rfc8126
- https://www.rfc-editor.org/info/rfc7322

ZK-ARCHE registries are repository-managed until and unless an external standards process adopts them. The design should nevertheless be structured so that migration to formal registries would not require redesigning the protocol.

---

## 5. Primary secure-channel and authenticated-key-exchange references

### 5.1 TLS 1.3 — RFC 9846

Current reference: https://www.rfc-editor.org/info/rfc9846

RFC 9846 supersedes RFC 8446 while retaining TLS 1.3 compatibility. ZK-ARCHE should study and emulate:

- transcript-bound negotiation;
- downgrade resistance;
- explicit handshake state;
- separation of handshake keys and application/traffic keys;
- structured key schedule and domain-separated derivation;
- Finished/key-confirmation semantics;
- authenticated parameter negotiation;
- resumption treated as a distinct security mode with explicit replay/privacy consequences;
- clear security properties and limitations;
- algorithm registry discipline;
- removal/deprecation of legacy behavior rather than indefinite accumulation.

ZK-ARCHE does **not** inherit TLS's PKI assumptions. TLS is a secure-channel reference, not a requirement that ZK-ARCHE use X.509 or a CA.

### 5.2 Mutual TLS / certificate-authenticated TLS

“mTLS” is not a separate TLS protocol version or standalone base RFC. It is the deployment pattern in which TLS authenticates both endpoints, normally using certificate authentication. TLS 1.3 already defines optional client authentication.

Useful reference:

- RFC 9846 — TLS 1.3: https://www.rfc-editor.org/info/rfc9846
- RFC 8705 — OAuth 2.0 Mutual-TLS Client Authentication and Certificate-Bound Access Tokens: https://www.rfc-editor.org/info/rfc8705

ZK-ARCHE should borrow the **mutual-proof and channel-bound authorization discipline**, not the requirement for a certificate authority.

### 5.3 Raw Public Keys for TLS/DTLS — RFC 7250

Reference: https://www.rfc-editor.org/info/rfc7250

RFC 7250 is especially relevant to the ZK-ARCHE bottom-up doctrine because it demonstrates a standards-track pattern for authenticating with raw public keys while avoiding full certificate-path processing. Its important lesson is that removing certificates does **not** remove the need for a trustworthy binding between the key and the entity.

ZK-ARCHE implication:

- CA-less does not mean trustless;
- locally provisioned/enrolled key or credential bindings must be explicit;
- a short key representation is not sufficient without possession proof and trusted binding semantics.

### 5.4 DTLS 1.3 — RFC 9147 + RFC 9853

References:

- https://www.rfc-editor.org/info/rfc9147
- https://www.rfc-editor.org/info/rfc9853

Adopt as design references for:

- datagram loss and reordering;
- explicit epochs and sequence numbers;
- replay windows;
- retransmission;
- fragmentation/reassembly constraints;
- stateless return-routability/source validation;
- anti-amplification and pre-authentication DoS controls;
- connection/address mobility without treating address as identity;
- bounded state before peer validation.

The ZK-ARCHE transport contract should generalize these disciplines beyond UDP rather than importing DTLS itself as a mandatory dependency.

### 5.5 EDHOC — RFC 9528 + RFC 9529

References:

- https://www.rfc-editor.org/info/rfc9528
- https://www.rfc-editor.org/info/rfc9529

EDHOC is a principal constrained-AKE comparator because it targets highly constrained peers while providing mutual authentication, forward secrecy, identity protection, cipher-suite negotiation, exporters, key update, and compact messages.

ZK-ARCHE should emulate:

- constrained-first protocol budgeting;
- a small mandatory exchange;
- explicit initiator/responder state;
- transcript hashes;
- exporter/context interfaces;
- secure suite negotiation;
- explicit error behavior;
- test traces with intermediate values;
- interoperability verified by independent implementations.

RFC 9529 establishes an especially valuable bar: ZK-ARCHE should eventually publish equivalent annotated deterministic traces for each mandatory method/profile and major negative path.

### 5.6 IKEv2 / IPsec — RFC 7296 + RFC 4303

References:

- https://www.rfc-editor.org/info/rfc7296
- https://www.rfc-editor.org/info/rfc4303

Use IKEv2/IPsec as references for:

- mutual authentication and Security Association lifecycle;
- explicit rekey/replacement semantics;
- traffic/authorization selectors;
- liveness and restart behavior;
- replay protection;
- anti-DoS cookies;
- distinction between control-plane key establishment and protected data-plane state.

ZK-ARCHE should learn from these lifecycle semantics while avoiding the configuration and implementation complexity that conflicts with the constrained Common Contract.

---

## 6. Constrained-IoT and object-security references

### 6.1 CoAP — RFC 7252

Reference: https://www.rfc-editor.org/info/rfc7252

Use as a reference for:

- low header overhead;
- bounded parser complexity;
- constrained-device applicability;
- message IDs/tokens and duplicate handling;
- asynchronous and lossy-network behavior;
- explicit transport/security binding boundaries.

ZK-ARCHE should be usable above, below, or beside CoAP depending on profile; CoAP is not mandatory.

### 6.2 OSCORE — RFC 8613

Reference: https://www.rfc-editor.org/info/rfc8613

Use as a reference for:

- application/object-layer end-to-end protection independent of trusted proxies;
- explicit security-context state;
- sequence/replay-window handling;
- derivation of sender/recipient keys and IVs;
- protected vs unprotected metadata analysis;
- deterministic test vectors;
- proxy/intermediary threat boundaries.

This is highly relevant to ZK-ARCHE-DATA and to deployments where intermediate gateways must not terminate the end-to-end trust relationship.

### 6.3 ACE-OAuth — RFC 9200

Reference: https://www.rfc-editor.org/info/rfc9200

Use as a semantic reference for:

- authentication vs authorization separation;
- resource/audience scope;
- expiring authorization;
- proof-of-possession binding;
- authorization-server/resource-server/client role separation;
- constrained authorization representation.

ZK-ARCHE must not inherit a mandatory centralized authorization-server dependency. ACE is a semantic comparator for scoped authorization and lineage.

### 6.4 BRSKI — RFC 8995

Reference: https://www.rfc-editor.org/info/rfc8995

Use as a lifecycle comparator for:

- explicit bootstrap state;
- separation of unconfigured vs configured device behavior;
- authorization of enrollment;
- voucher/grant concepts;
- disconnected or limited-connectivity provisioning cases;
- explicit trust-anchor installation.

BRSKI's manufacturer/PKI/MASA model is **not** the ZK-ARCHE constrained P2P baseline. The transferable lesson is that enrollment and trust mutation are explicit authorized acts, never side effects of normal AUTH.

---

## 7. Encoding, schema, and cryptographic-object references

### 7.1 CBOR — RFC 8949

Reference: https://www.rfc-editor.org/info/rfc8949

CBOR is relevant because it targets small code size, compact messages, and extensibility. ZK-ARCHE should benchmark CBOR against its current encoding before adopting it as a mandatory profile dependency.

Required lesson regardless of encoding choice:

- one unambiguous wire representation for security-sensitive fields;
- explicit canonicalization rules where bytes enter transcripts/signatures/MACs;
- deterministic invalid/unexpected-input behavior.

### 7.2 CDDL — RFC 8610 + RFC 9682

References:

- https://www.rfc-editor.org/info/rfc8610
- https://www.rfc-editor.org/info/rfc9682

If ZK-ARCHE adopts CBOR for a profile, CDDL should be evaluated as the machine-readable normative schema language. Even if the core keeps another encoding, the protocol should provide an equivalent machine-checkable grammar.

### 7.3 COSE — RFC 9052 + RFC 9053

References:

- https://www.rfc-editor.org/info/rfc9052
- https://www.rfc-editor.org/info/rfc9053

Use as references for:

- compact cryptographic object structure;
- protected/unprotected header separation;
- critical parameter processing;
- algorithm/key registries;
- CBOR-native key representation;
- security-object test examples.

COSE is optional unless a measured ZK-ARCHE profile explicitly adopts it.

---

## 8. Cryptographic construction references

### 8.1 HKDF — RFC 5869

Reference: https://www.rfc-editor.org/info/rfc5869

Use as a reference for extract/expand key schedules and domain-separated derivation. ZK-ARCHE must document exact labels, context, input ordering, output lengths, and key-erasure lifecycle.

### 8.2 X25519 / Curve25519 — RFC 7748

Reference: https://www.rfc-editor.org/info/rfc7748

Use as a candidate portable ECDH reference where it matches the reviewed mandatory-suite requirements and target evidence.

### 8.3 EdDSA — RFC 8032

Reference: https://www.rfc-editor.org/info/rfc8032

Use as a signature reference where a signature-based profile is justified. This does not imply that every constrained profile must use Ed25519.

### 8.4 ChaCha20-Poly1305 — RFC 8439

Reference: https://www.rfc-editor.org/info/rfc8439

Use as a portable software-efficient AEAD comparator, particularly for targets without strong AES acceleration. Hardware-aligned AEAD profiles may coexist when benchmarked and negotiated without weakening the mandatory floor.

### 8.5 HPKE — RFC 9180

Reference: https://www.rfc-editor.org/info/rfc9180

Use as a reference for:

- standard-primitive hybrid encryption composition;
- KEM/KDF/AEAD suite separation;
- explicit modes;
- `info`/AAD context binding;
- exporter behavior;
- deterministic test vectors.

The optional encrypted registry lookup-hint work should prefer a bounded HPKE-style standard construction over inventing a new public-key encryption envelope unless evidence demands otherwise.

---

## 9. Transport/component separation references

### 9.1 QUIC — RFC 9000 + RFC 9001

References:

- https://www.rfc-editor.org/info/rfc9000
- https://www.rfc-editor.org/info/rfc9001

QUIC is important not because ZK-ARCHE should become QUIC, but because it demonstrates a disciplined interface between a cryptographic handshake and a transport that owns different responsibilities.

Use as a reference for:

- authenticated transport parameters;
- explicit interface between key-establishment and transport state;
- packet-number spaces / lifecycle separation;
- key-update limits;
- anti-amplification behavior;
- version negotiation;
- 0-RTT replay caveats;
- maintaining protocol security while changing the underlying transport machinery.

This maps directly to the ZK-ARCHE Common Contract + transport-adapter architecture.

### 9.2 TLS channel binding — RFC 9266

Reference: https://www.rfc-editor.org/info/rfc9266

Use as a reference for binding upper-layer ZK-ARCHE authentication to an already-established TLS channel. The ZK-ARCHE binding must identify a unique AUTH instance and must not assume that one transport connection implies one authorization context.

TLS/DTLS bindings remain optional. Native P2P must remain functional without PKI/TLS when the selected ZK-ARCHE profile does not require them.

---

## 10. Trustworthiness, group, and future-system references

### 10.1 RATS architecture — RFC 9334

Reference: https://www.rfc-editor.org/info/rfc9334

Use as an optional-attestation reference for:

- distinction between trust and trustworthiness;
- Evidence / Verifier / Relying Party roles;
- freshness of evidence;
- appraisal-policy boundaries;
- platform-neutral attestation terminology.

Remote attestation must remain an optional extension unless a future profile explicitly requires it. The constrained P2P authentication floor cannot depend on an online verifier.

### 10.2 Messaging Layer Security — RFC 9420

Reference: https://www.rfc-editor.org/info/rfc9420

MLS is not a direct P2P-authentication dependency, but is a useful future reference for:

- epoch-based group state;
- forward secrecy;
- post-compromise security;
- asynchronous membership changes;
- large group key-state evolution;
- explicit exporter labels and registries.

Any future ZK-ARCHE swarm/group-trust work may compare against MLS while preserving ZK-ARCHE's own infrastructure-independence requirements. MLS's Authentication Service assumption must not silently become a ZK-ARCHE core dependency.

---

## 11. WireGuard simplicity and lightweight implementation reference

WireGuard is a **non-RFC engineering reference**, not a normative ZK-ARCHE dependency.

Primary references:

- WireGuard organization: https://github.com/WireGuard
- Go implementation: https://github.com/WireGuard/wireguard-go
- tools: https://github.com/WireGuard/wireguard-tools
- protocol/cryptography overview: https://www.wireguard.com/protocol/
- technical whitepaper: https://www.wireguard.com/papers/wireguard.pdf
- cross-platform contract: https://www.wireguard.com/xplatform/

ZK-ARCHE should borrow the following WireGuard design disciplines.

### 11.1 Small attack surface

Prefer a small mandatory core that can be audited by an individual or small team rather than a framework where every deployment enables a different collection of large subsystems.

For ZK-ARCHE this means:

- keep `p2p-iot-core` intentionally narrow;
- minimize mandatory parser surface;
- minimize mandatory cryptographic choices;
- avoid hidden framework dependencies;
- keep state transitions explicit;
- favor direct, bounded interfaces over deeply abstract plug-in stacks in constrained hot paths.

### 11.2 Simple peer contract

WireGuard's useful conceptual lesson is that a peer can be represented by a small stable cryptographic configuration while endpoint/network location remains operational state.

ZK-ARCHE analogue:

```text
cryptographic peer/trust identity != transport address
```

A changing UDP endpoint, BLE address, radio path, interface, NAT mapping, or gateway route must not silently change ZK-ARCHE identity.

### 11.3 Automatic lifecycle where safe

WireGuard automates rekeying and session lifecycle behind a simple interface. ZK-ARCHE should aim for similar operational simplicity while retaining explicit authorization, revocation, and evidence semantics.

The operator/application should not have to manually micromanage routine key rotation, replay-window maintenance, retry handling, or session refresh when the protocol can safely automate them.

### 11.4 Conservative primitive selection

WireGuard intentionally uses a small fixed set of modern primitives to reduce complexity. ZK-ARCHE should borrow the **conservative-floor** principle, but not necessarily WireGuard's exact primitive set.

The mandatory ZK-ARCHE floor should be:

- small;
- reviewed;
- portable;
- efficient in software;
- measurable on constrained targets;
- implemented consistently in Rust/C;
- resistant to downgrade;
- stable enough for long-lived interoperability.

### 11.5 Cross-platform semantic identity

WireGuard's cross-platform implementations are expected to preserve the same protocol behavior. ZK-ARCHE should impose the same rule across Rust, C, MCU ports, Linux-edge implementations, and future language/platform ports.

Platform adaptation may change APIs, scheduling, storage, acceleration, and transport glue. It must not change wire semantics or the mandatory security contract.

### 11.6 What ZK-ARCHE must **not** copy blindly

WireGuard intentionally avoids broad negotiation/extensibility/cryptographic agility. RFC 8922 notes this tradeoff explicitly. ZK-ARCHE has a different requirement: heterogeneous IoT systems may remain deployed for many years and may require controlled suite/profile evolution.

Therefore ZK-ARCHE should combine:

```text
WireGuard-like mandatory-core simplicity
              +
TLS/EDHOC-like versioned negotiation discipline
              +
RFC-style registries and change control
```

The goal is **bounded agility**, not unlimited plug-ins and not a permanently frozen protocol.

---

## 12. Reticulum common-contract reference

Reticulum remains a non-RFC architectural comparator for heterogeneous interface normalization. The transferable idea is to keep the upper protocol contract stable while different media/platform adapters provide the environment-specific integration.

ZK-ARCHE applies this concept to:

```text
identity
authentication
authorization
trust
secure association
lifecycle state
```

rather than routing itself.

Reticulum and WireGuard together provide useful implementation-architecture lessons:

- Reticulum: heterogeneous-interface/common-contract thinking;
- WireGuard: minimal, auditable, cross-platform secure-core thinking;
- RFC-class protocols: specification, negotiation, lifecycle, interoperability, and change-control discipline.

---

## 13. Standards reference matrix

| Reference | Class | Primary lesson for ZK-ARCHE | Must become dependency? |
|---|---|---|---|
| RFC 9846 TLS 1.3 | Standards Track | transcript/key schedule/downgrade/resumption/security model | No |
| TLS mutual authentication / RFC 8705 example | Standards Track profile | bilateral authentication + credential-bound authorization | No |
| RFC 7250 Raw Public Keys | Standards Track | CA-less key authentication with explicit trusted key binding | No |
| RFC 9147 DTLS 1.3 | Standards Track | datagram replay/reorder/retransmit/DoS | No |
| RFC 9853 DTLS return routability | Standards Track | address-change validation without identity conflation | No |
| RFC 9528 EDHOC | Standards Track | lightweight constrained AKE | No |
| RFC 9529 EDHOC traces | Informational | reproducible annotated interop traces | No |
| RFC 7296 IKEv2 | Internet Standard | SA lifecycle, mutual auth, rekey, DoS | No |
| RFC 4303 ESP | Standards Track | anti-replay and protected association semantics | No |
| RFC 7252 CoAP | Standards Track | constrained messaging simplicity | No |
| RFC 8613 OSCORE | Standards Track | object security + replay context | No |
| RFC 9200 ACE | Standards Track | authn/authz split and scoped authorization | No |
| RFC 8995 BRSKI | Standards Track | explicit bootstrapping/trust mutation | No |
| RFC 8949 CBOR | Internet Standard | compact extensible encoding | Profile decision |
| RFC 8610/9682 CDDL | Standards Track | machine-readable grammar | Profile decision |
| RFC 9052/9053 COSE | Internet Standard / Standards Track | compact crypto objects/registries | Profile decision |
| RFC 5869 HKDF | Informational | extract/expand key derivation | Suite decision |
| RFC 7748 X25519 | Informational | portable ECDH | Suite decision |
| RFC 8032 EdDSA | Informational | signature construction | Suite decision |
| RFC 8439 ChaCha20-Poly1305 | Informational | software-efficient AEAD | Suite decision |
| RFC 9180 HPKE | CFRG Informational | standard hybrid encryption composition | Optional feature |
| RFC 9000/9001 QUIC | Standards Track | handshake/transport component boundary | No |
| RFC 9266 TLS channel binding | Standards Track | exporter-based upper-layer binding | Optional binding |
| RFC 9334 RATS | Informational | trustworthiness/attestation roles | Optional extension |
| RFC 9420 MLS | Standards Track | epoch/group lifecycle and PCS | Future comparator |
| RFC 3552 | BCP | security-considerations rigor | Yes, as documentation discipline |
| RFC 8126 | BCP | registry/change-control rigor | Yes, as documentation discipline |
| RFC 2119/8174 | BCP | normative language | Yes, once spec behavior is stable |
| WireGuard | non-RFC implementation/protocol reference | minimal attack surface, simple peer model, fixed core | No |
| Reticulum | non-RFC architecture reference | heterogeneous common interface contract | No |

“Must become dependency?” means runtime/protocol dependency. Documentation/process references may still be mandatory engineering discipline.

---

## 14. Native ZK-ARCHE specification package

The RFC-class package should maintain at least:

| Artifact | Path | Required content |
|---|---|---|
| Core protocol | `spec/zk-arche-protocol.md` | flows, state transitions, computations, errors |
| Registries | `spec/registries.md` | versions, methods, suites, profiles, extensions, messages, alerts, bindings |
| Common Contract | `spec/common-contract.md` or integrated core section | mandatory cross-platform semantic floor |
| IoT profiles | `spec/iot-profiles.md` | resource ceilings, mandatory/optional behavior, target evidence |
| P2P profile | specification section/artifact | local trust, mutual AUTH, delegation, infrastructure independence |
| Security considerations | `spec/security-considerations.md` | RFC 3552-style threat inventory and residual risks |
| Privacy considerations | `spec/privacy-considerations.md` | anonymity, unlinkability, metadata/timing/failure surfaces |
| Test vectors | `spec/test-vectors.md` + checked-in corpus | canonical positive/negative vectors and regeneration rules |
| Traces | checked-in trace corpus | EDHOC-style annotated intermediates for mandatory flows |
| Implementation requirements | `spec/implementation-requirements.md` | parser/state/RNG/storage/constant-time/zeroization rules |
| Interop evidence | `evidence/interop-matrix.md` when curated | Rust/C/target/profile parity and known gaps |
| Formal traceability | assurance artifact | model event ↔ spec section ↔ code/test mapping |
| Target evidence | benchmark artifacts | execution-context + wire/RAM/CPU/flash/storage data |

---

## 15. Mandatory normative structure

The main protocol specification should eventually contain sections equivalent to:

1. Introduction and applicability
2. Conventions and BCP 14 terminology
3. Terminology and trust roles
4. Threat model, security goals, and non-goals
5. Common Contract architecture
6. Protocol overview and module decomposition
7. Constants, versions, identifiers, and registries
8. Cryptographic suites and mandatory-to-implement floor
9. Canonical wire encoding and grammar
10. HELLO / capability advertisement / profile selection
11. SETUP / bootstrap / initial trust establishment
12. AUTH
13. Authorization-context evaluation
14. Late enrollment and commissioner grants
15. Rekey / re-registration / lineage replacement
16. Revocation convergence
17. Replay, sequence, and key-usage limits
18. Session resumption
19. Secure association / key export
20. Transport adapters and channel bindings
21. Native unreliable-datagram behavior
22. Error/alert behavior and observable failure classes
23. State machines
24. Positive vectors
25. Negative vectors and invalid-message corpus
26. Annotated protocol traces
27. Security Considerations
28. Privacy Considerations
29. Constrained implementation requirements
30. Registry / extension / deprecation policy
31. Conformance requirements
32. Implementation and deployment claim language
33. Known limitations
34. IANA-style considerations / future external registry mapping

Normative MUST/SHOULD language should only be introduced when the behavior is explicit enough for two independent implementations to agree without reading each other's source.

---

## 16. Common Contract conformance model

ZK-ARCHE should define **conformance to a contract**, not conformance to a hardware family.

The minimum matrix must eventually include:

```text
Rust reference ↔ C reference
MCU-core ↔ MCU-core
MCU-core ↔ MCU-plus
MCU-core ↔ Linux-edge
MCU-core ↔ accelerated-edge
reverse initiator/responder directions
multiple transport adapters
infrastructure-present and infrastructure-absent cases
```

Conformance requires the same mandatory:

- wire interpretation;
- transcript bytes;
- possession/authentication result;
- authorization decision inputs;
- replay/freshness behavior;
- profile negotiation behavior;
- fail-closed decisions;
- key derivation/key confirmation;
- invalid-input handling where normative.

Equal security semantics do not require equal CPU cost, memory size, registry capacity, logging depth, or optional functionality.

---

## 17. Negotiation and bounded agility

ZK-ARCHE should avoid both extremes:

```text
unbounded plug-in negotiation  ← too complex / hard to audit
permanently frozen protocol    ← hard to maintain over long IoT lifetimes
```

The preferred model is:

```text
small mandatory floor
+ versioned optional suites/profiles
+ explicit compatibility matrix
+ transcript-bound negotiation
+ fail-closed critical extensions
+ deterministic anti-ossification tests
+ controlled registry/change policy
```

A high-end peer may negotiate an advanced suite with another high-end peer. It must retain the mandatory constrained floor if it claims `p2p-iot-core` compatibility.

---

## 18. Authentication, authorization, and trust mutation

The normative package must preserve three different concepts:

1. **Authentication** — cryptographic evidence that the peer satisfies a credential/possession/role-proof statement relative to existing trusted state.
2. **Authorization** — a decision that the authenticated peer may perform an operation under explicit audience, scope, policy, validity, lineage, and revocation state.
3. **Trust mutation** — an explicit authorized state transition that creates, replaces, delegates, or revokes trusted state.

Normal AUTH and P2P AUTH are NO-LEARNING. A cryptographically valid unknown peer does not become trusted merely because its message verifies mathematically.

ACE, BRSKI, TLS/mTLS, Raw Public Keys, WireGuard, and RATS should be compared specifically to keep these semantic boundaries explicit.

---

## 19. Lifecycle: replay, rekey, revocation, resumption

ZK-ARCHE should treat lifecycle as one connected state machine rather than separate utilities.

The normative package must eventually define:

- sequence/replay state;
- replay-state persistence and loss behavior;
- cryptographic usage limits;
- rekey trigger and completion;
- atomic credential lineage replacement;
- revocation view/epoch convergence;
- maximum stale-authorization window per profile;
- session/resumption invalidation after policy/credential change;
- resumption authorization revalidation;
- rollback/restart/reclone behavior;
- offline behavior when freshness cannot be established.

TLS, DTLS, IKEv2/IPsec, OSCORE, QUIC, WireGuard, and MLS provide complementary lifecycle lessons. No single one should be copied wholesale.

---

## 20. Error, retry, DoS, and privacy behavior

Externally observable failure is part of protocol behavior.

The specification should inventory:

```text
response vs silence
error/alert code
packet size bucket
retry behavior
source-validation token behavior
timing/dummy-work policy
unknown vs known peer behavior
unauthorized vs authorized role behavior
capability/profile ordering
connection/session identifiers
resumption identifiers
transport-address changes
```

DTLS and QUIC provide useful anti-amplification/source-validation references. TLS provides structured alert/state behavior. EDHOC provides constrained error handling. WireGuard provides a useful simplicity target for minimizing exposed state and control surface.

---

## 21. Test-vector and trace standard

ZK-ARCHE should maintain two complementary evidence layers.

### Deterministic vectors

For each mandatory suite/profile:

- inputs/seeds;
- encoded messages;
- transcript hashes;
- proof values;
- KDF intermediate/output material where safe for test fixtures;
- key-confirmation values;
- expected accept/reject state;
- Rust/C parity.

### Annotated traces

Following the EDHOC RFC 9529 model, retain human-readable traces showing the major intermediate states and bytes for complete flows.

Negative traces should include:

- transcript mutation;
- suite/profile downgrade;
- replay;
- wrong role/policy;
- unknown peer;
- wrong key/reference mapping;
- invalid proof;
- stale authorization;
- revoked lineage;
- retry-token misuse;
- resumption misuse;
- transport/address change;
- unsupported critical extension.

At least two independent implementations should verify the mandatory trace corpus before a strong interoperability claim.

---

## 22. Security-review standard

Before strong cryptographic-security claims:

- document the custom proof statement/witness relation;
- document Fiat-Shamir/domain-separation construction;
- document serialization and role ordering;
- publish deterministic positive/negative vectors;
- define constant-time boundaries;
- define RNG/entropy/key-storage requirements;
- expand formal models and model-to-code traceability;
- retain independent cryptographic review results;
- disposition review findings with regression evidence.

RFC-class formatting cannot substitute for independent cryptographic analysis.

---

## 23. Simplicity budget

Every proposed mandatory feature should answer:

```text
How many wire bytes does this add?
How much code/parser surface does this add?
How much RAM/stack/flash does this add?
How much persistent state does this add?
Does this add a new cryptographic primitive?
Does this add a new trust dependency?
Does this add a new online dependency?
Can the least-capable conformant peer implement it?
Can one reviewer understand its security role in isolation?
Can it remain optional instead?
```

The burden of proof is on complexity.

This is the principal WireGuard-inspired discipline ZK-ARCHE should keep: **a feature is not automatically good merely because it is cryptographically sophisticated or standards-aligned.** The mandatory protocol should contain only what the common security contract requires.

---

## 24. Reference freshness and supersession policy

RFC references must be checked for current status before a roadmap/spec update.

Examples as of 2026-08-25:

- use RFC 9846 as the current TLS 1.3 base reference; RFC 8446 is historical/superseded;
- use RFC 9147 for DTLS 1.3 and account for RFC 9853 where return-routability/CID behavior is relevant;
- use RFC 9528 for EDHOC and RFC 9529 for traces;
- check “Updated by”, “Obsoletes”, errata, and IANA registry state before adopting behavior.

A coding or research agent must not hard-code an obsolete RFC as the primary normative comparator merely because older papers or source comments cite it.

---

## 25. Claim language

Use these states distinctly:

```text
DESIGNED                architecture/spec intent exists
SPECIFIED               normative behavior is complete enough for independent implementation
IMPLEMENTED             code path exists
TESTED                  deterministic tests pass
INTEROPERABLE           independent implementations agree
COMMON-CONFORMANT        mandatory Common Contract corpus passes
MEASURED                target resource evidence exists
FORMALLY ANALYZED        scoped model result exists
EXTERNALLY REVIEWED      independent review exists
RFC-CLASS DOCUMENTED     complete standards-style package exists
DEPLOYMENT-QUALIFIED     required platform/field context exists
STANDARDIZED             only if an actual standards body has adopted it
```

`RFC-CLASS DOCUMENTED` must never be described as “an RFC”, “IETF standardized”, or “Internet Standard”.

---

## 26. Exit gate for the RFC-evolution program

The RFC-evolution program is successful only when all of the following are true for the claimed mandatory baseline:

```text
NORMATIVE SPEC COMPLETE
+ MACHINE-CHECKABLE WIRE GRAMMAR
+ VERSIONED REGISTRIES
+ MANDATORY-TO-IMPLEMENT FLOOR DEFINED
+ THREAT / SECURITY / PRIVACY CONSIDERATIONS COMPLETE
+ STATE MACHINES COMPLETE
+ POSITIVE + NEGATIVE VECTORS
+ EDHOC-STYLE ANNOTATED TRACES
+ RUST/C INDEPENDENT INTEROPERABILITY
+ CROSS-CLASS COMMON-CONTRACT INTEROPERABILITY
+ CONSTRAINED TARGET MEASUREMENTS
+ FORMAL TRACEABILITY
+ CUSTOM CRYPTO REVIEW STATUS EXPLICIT
+ CHANGE / DEPRECATION POLICY
+ KNOWN LIMITATIONS
+ CLAIM LANGUAGE MATCHES EVIDENCE
= RFC-CLASS ENGINEERING PACKAGE
```

This gate is intentionally stronger than “the code works”.

---

## 27. Agent editing contract

Future automated edits to the RFC-style package must preserve:

- current RFC references rather than obsolete primary references;
- RFCs as comparators unless explicitly adopted;
- WireGuard and Reticulum as non-RFC architectural/implementation references;
- no implication that ZK-ARCHE is an IETF RFC or Internet Standard;
- BCP 14 normative-language discipline;
- RFC 3552-style security considerations;
- RFC 8126-style registry/change-control discipline;
- deterministic vectors and independent implementation traceability;
- the Common Contract and bottom-up P2P doctrine;
- CA/cloud/gateway independence of the mandatory P2P authentication path;
- same-assurance cross-class interoperability;
- bounded agility rather than uncontrolled plug-in complexity;
- WireGuard-inspired simplicity/minimal attack surface without blindly copying its lack of negotiation;
- explicit separation of authentication, authorization, and trust mutation;
- NO-LEARNING normal AUTH;
- checkpoint-style review for cryptographic, wire, transcript, negotiation, replay, resumption, authorization, revocation, parser, RNG, memory-safety, formal-model, and interoperability changes.
