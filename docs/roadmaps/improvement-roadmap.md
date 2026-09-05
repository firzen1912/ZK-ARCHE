# ZK-ARCHE Unified Rust/C Improvement Roadmap

This roadmap defines the evidence-gated improvement path for the unified ZK-ARCHE Rust and C repository. It is the canonical long-horizon engineering plan for protocol maturity, constrained-device viability, heterogeneous interoperability, privacy, lifecycle security, data sovereignty, and infrastructure-independent P2P zero-trust operation.

It is deliberately **evidence-gated rather than calendar-gated**. A phase exists to define what must be implemented and what evidence must exist before the associated capability or maturity claim can advance. Research findings, standards drafts, papers, or prototypes do not become roadmap requirements automatically. Promotion occurs only through explicit human-reviewed roadmap updates.

The long-term interoperability north star is a **ZK-ARCHE Common Contract Architecture**: any conformant peer, from the least-capable supported MCU to a high-performance edge computer, should be able to mutually authenticate and establish appropriately scoped trust using the same mandatory security floor without requiring a CA, cloud service, central registry, gateway, DNS, Internet connectivity, blockchain, or other always-online external infrastructure. More capable peers may provide additional functionality, but those extensions must remain optional and must not weaken or become prerequisites for the constrained interoperability core.

The architectural inspiration is similar to the way Reticulum handles heterogeneous communication media through a common interface contract: ZK-ARCHE should normalize **identity, authentication, authorization, trust, and secure-association semantics** across heterogeneous IoT and edge platforms instead of binding protocol correctness to one hardware family, operating system, transport, or deployment topology. This is an architectural analogy only; Reticulum is not a dependency and does not define ZK-ARCHE's cryptographic or trust model.

The organization of this roadmap also takes inspiration from the [Reticulum development roadmap](https://github.com/markqvist/Reticulum/blob/master/Roadmap.md): enduring development efforts are kept distinct from individual milestones so that portability, comprehensibility, functionality, interfaceability, usability, and assurance can progress together without forcing every useful ecosystem feature into the protocol core.

## 1. Roadmap authority and evidence flow

Use the documentation layers as follows:

```text
external source / hypothesis
        ↓
docs/research/daily/
        ↓
docs/research/backlog.md
        ↓ human-reviewed synthesis
docs/findings/week-of-*-findings.md
        ↓ explicit human promotion
docs/roadmaps/                         ← this document
        ↓ bounded implementation intent
docs/requests/week-of-*-request.md
        ↓ consequential decision?
docs/adr/
        ↓ normative behavior
spec/
        ↓
rust/ + c/ + tests/vectors
        ↓
docs/assurance/ + retained evidence
        ↓
docs/release/
```

The arrows are gates, not automatic transitions. Weekly findings can strengthen or weaken a roadmap hypothesis without changing implementation scope. Weekly requests define bounded execution work; they must not silently rewrite protocol policy.

## 2. Current evidence baseline — 2026-08-25

The repository currently has two active implementation lanes:

| Lane | Path | Role |
|---|---|---|
| Rust reference | `rust/` | canonical deterministic vectors, protocol implementation, fuzz/formal tooling, higher-level validation |
| C implementation | `c/` | independent C11/libsodium implementation, constrained-device anchor, cross-language validation |

Canonical byte-level interoperability remains anchored by the Rust vectors under `rust/test-vectors/0x0001/`, with C expected to reproduce the same wire, transcript, proof, KDF, MAC/key-confirmation, and state-machine semantics wherever both lanes implement the same feature.

The active debt ceiling remains:

| Debt | Blocking gap | Roadmap ownership |
|---|---|---|
| TD-001 | independent cryptographic review of the custom role-membership proof | zk207 / zk209 / zk217 / zk225 / zk226 / zk241 |
| TD-002 | reproducible STM32/ESP32-S3-class target measurements | zk208 / zk216 / zk222 / zk236 / zk240 / zk241 |
| TD-003 | formal models remain skeletons without full model-to-code traceability | zk207 / zk209 / zk217 / zk218 / zk239 |
| TD-004 | RFC-style normative specification remains incomplete | zk217 / zk218 / zk225 / zk226 / zk230 / zk240 |
| TD-005 | exact-head executable qualification is not enforced on every path into `dev` | zk202 / zk210 / zk222 |

The current research backlog contains R-001 through R-015. The strongest human-reviewed findings now affecting roadmap acceptance criteria are:

- custom proof review must be property-driven rather than a generic “get review” task;
- MCU evidence must pin cryptographic execution context, entropy/key-storage posture, accelerator path, restart/rollback assumptions, and raw resource measurements;
- authentication, authorization, and trust mutation must be distinct semantics;
- profile, registry, and extension behavior need compatibility rules and anti-ossification tests;
- replay, key usage, rekey, resumption, revocation, and channel binding must be treated as one lifecycle problem;
- formal assurance needs one canonical or mechanically synchronized state model plus explicit privacy properties;
- resumption must revalidate authorization context and bound ticket/PSK reuse;
- revocation must converge across disconnected peers with an explicit stale-authorization bound;
- protocol conformance and whole-product/field readiness must remain separate claims;
- BBS and PQ/hybrid work remain optional research profiles until measured and reviewed;
- P2P operation must not depend on a CA or external infrastructure for the root authentication decision;
- less-capable peers must be able to authenticate higher-capability peers, and vice versa, with the same mandatory assurance floor;
- resource asymmetry may reduce optional functionality but must not reduce mandatory authentication security;
- transport, platform, and deployment differences should be normalized behind a small common contract rather than fragmenting ZK-ARCHE into device-specific protocol variants.

These findings and explicit architectural decisions sharpen existing phases; they do not claim those phases are implemented.

## 3. ZK-ARCHE Common Contract Architecture

### 3.1 North-star contract

The common contract is the minimum protocol/security surface that every conformant implementation must share regardless of hardware, operating system, transport, deployment topology, or local resource budget.

A conformant peer must be able to participate in the mandatory core without needing gateway-class services. The common contract should eventually cover at least:

```text
canonical protocol framing and encoding
version / suite / profile negotiation
mandatory reviewed cryptographic floor
mutual peer authentication
proof-of-possession / role-proof verification where in profile
transcript and context binding
local trust evaluation
scoped authorization evaluation
replay / sequence / freshness handling
secure association and key establishment
revocation / policy epoch checks available to the peer
fail-closed extension handling
bounded failure behavior
transport adapter interface
```

The implementation may expose only a subset of optional services, but the mandatory contract must remain semantically identical across conformant targets.

### 3.2 Bottom-up doctrine

ZK-ARCHE follows a bottom-up interoperability doctrine:

> **Systems scale functionality upward from the least-capable conformant peer rather than scaling security downward from the most-capable peer.**

This means:

- a high-end peer must adapt to the constrained common profile when authenticating a lower-capability peer;
- the constrained peer must still verify the complete mandatory authentication decision locally;
- a constrained peer must not be reduced to an unauthenticated sensor that forwards a security decision to a gateway;
- resource asymmetry may change optional functionality, performance, registry size, caching, audit depth, discovery, policy richness, or acceleration;
- resource asymmetry must **not** silently weaken the authentication, transcript-binding, replay, possession, authorization-scope, or fail-closed properties of the mandatory profile;
- hardware accelerators may improve execution but must not be required for semantic correctness;
- a platform unable to implement the mandatory floor is not conformant to that floor rather than being assigned a weaker authentication mode under the same profile name.

### 3.3 Infrastructure-independent trust root

Basic P2P authentication between already-authorized peers must not require:

```text
certificate authority
online certificate-status service
cloud identity provider
central registry lookup
gateway/controller approval
DNS
Internet connectivity
blockchain/distributed ledger
manufacturer cloud
always-online policy service
```

External infrastructure may assist discovery, synchronization, revocation propagation, large-registry indexing, auditing, backup, policy administration, fleet management, or optional attestation. Those services are **infrastructure-assisted**, not **infrastructure-required**, for the core P2P trust decision.

Loss of optional infrastructure may affect availability, freshness, discovery, synchronization speed, or optional features. It must not retroactively invalidate the ability of two already-authorized peers with sufficient local state to execute the mandatory common-contract authentication flow.

### 3.4 Symmetric assurance with asymmetric computation

P2P security assurance is symmetric even when computational work is asymmetric.

A Jetson, Raspberry Pi, server, or large MCU may perform extra indexing, policy evaluation, logging, attestation processing, privacy extensions, or trust-graph management. A smaller MCU may hold only the bounded state required for its local decision. Both peers must still authenticate each other under the same mandatory security properties.

The roadmap therefore adopts the rule:

> **Asymmetric computation is acceptable; asymmetric authentication assurance is not.**

### 3.5 Platform independence

Compatibility is based on the common contract, not device branding or device class. Representative targets include but are not limited to:

```text
STM32-class bare-metal / RTOS
ESP32 / ESP32-S3-class RTOS
Nordic-class MCU platforms
other constrained C-capable MCUs
Raspberry Pi-class Linux
Jetson-class Linux edge nodes
x86 / ARM64 servers and gateways
industrial controllers
robotics companion computers
sensors, actuators, UAVs, UGVs, and other embedded peers
```

A new target should normally require a platform/transport adaptation layer, build profile, crypto-backend integration, storage/RNG integration, and conformance evidence—not a redesign of the ZK-ARCHE protocol.

### 3.6 RFC-class reference doctrine

ZK-ARCHE targets **RFC-class engineering quality** without claiming IETF adoption or RFC status. The detailed standards/reference matrix, current RFC numbers, protocol-by-protocol lessons, WireGuard comparison, standards-process guidance, and RFC-class completion gate are owned by [`rfc-evolution-plan.md`](./rfc-evolution-plan.md). This roadmap intentionally does **not** duplicate that matrix.

The main roadmap instead enforces the following stable doctrine across all phases:

- security- or wire-relevant behavior must be independently implementable from normative specification text rather than source code alone;
- normative requirements use BCP 14 discipline only when behavior is precise and testable;
- threat and Security Considerations work follows the rigor expected by RFC 3552-class analysis;
- versions, suites, methods, profiles, extensions, alerts, and bindings use explicit registry/change-control discipline comparable to RFC 8126;
- protocol state machines, transcript/key-schedule behavior, replay/retry/rekey/resumption lifecycle, and error semantics must be explicit;
- promoted behavior requires positive and negative vectors, executable tests, or an explicit retained evidence gap;
- important flows should have annotated/reproducible traces and independent implementation interoperability evidence comparable in spirit to mature TLS/DTLS/EDHOC ecosystems;
- security, privacy, constrained-resource, formal-analysis, and external-review claims remain independently scoped;
- reference protocols are comparators, not automatic dependencies or proof that an analogous ZK-ARCHE design is secure.

The current reference families include TLS 1.3 and mutual-authentication/channel-binding patterns, DTLS 1.3, EDHOC, OSCORE/CoAP, ACE, IKEv2/IPsec, QUIC, CBOR/CDDL/COSE, HKDF, X25519/EdDSA, ChaCha20-Poly1305, HPKE, RATS, MLS, and related standards tracked in `rfc-evolution-plan.md`. WireGuard is retained there as a non-RFC reference for small attack surface, simple peer semantics, automatic lifecycle management, and implementation consistency; Reticulum remains an architectural reference for universality and interfaceability.

The intended synthesis is:

```text
Reticulum-like universality / interfaceability
              +
WireGuard-like mandatory-core simplicity
              +
TLS / DTLS / EDHOC-class protocol discipline
              +
RFC / BCP specification and change-control rigor
              ↓
        ZK-ARCHE Common Contract
```

When an external RFC or comparator is revised or superseded, update `rfc-evolution-plan.md` first. The main roadmap should change only when the resulting lesson materially changes ZK-ARCHE architecture, sequencing, evidence gates, or acceptance criteria.

## 4. Non-negotiable boundaries

- Do not change cryptographic primitives, domain separators, packet formats, suite identifiers, test-vector meanings, replay semantics, credential/trust semantics, or wire compatibility without checkpoint-style review and retained evidence.
- Rust and C must remain byte- and decision-compatible wherever both lanes claim the same protocol behavior.
- Normal `AUTH` is **NO-LEARNING**: it proves possession/authorization relative to already trusted state and must not create or expand trust as a side effect.
- Trust-store mutation belongs only to explicit ENROLL, commissioner, grant, rekey/re-registration, or equivalent reviewed state-transition flows.
- Authentication does not by itself imply authorization. Authorization must be scoped, time/epoch bounded, revocable, audience/deployment bound, and tied to the authenticated holder or session context.
- `iot-core` and `p2p-iot-core` remain constrained interoperability floors, not weaker-security profiles.
- Capability negotiation may reduce optional functionality but must not negotiate away mandatory security properties.
- A stronger peer must adapt downward to the constrained interoperability floor; a weaker peer must not be forced upward into gateway-class dependencies.
- Basic P2P authentication must remain possible without a CA or always-online infrastructure when peers possess sufficient locally authorized trust state.
- Trust is local and non-transitive by default. `A trusts B` and `B trusts C` must not imply `A trusts C` without explicit bounded delegation evidence accepted by A.
- Heavy anonymous credentials, post-quantum hybrids, large certificate-chain parsers, full trust-graph engines, remote-attestation stacks, and general-purpose SNARK/STARK provers cannot become mandatory MCU dependencies without explicit future charter change and target evidence.
- Optional extensions must not redefine the semantics of the common core or create hidden interoperability dependencies.
- Hardware acceleration may optimize implementation behavior but must never define protocol correctness.
- Fuzzing, formal models, benchmarks, and simulation are evidence producers; none alone establishes complete security.
- Protocol conformance is not field readiness. Product/platform lifecycle, provisioning authority, secure storage, update posture, debug/boot posture, external controls, and residual assumptions must be identified separately before deployment claims.

## 5. IoT capability and evidence contract

| Class | Representative targets | Expected role |
|---|---|---|
| MCU-core | STM32-class bare-metal/RTOS and comparable constrained MCUs | constrained P2P peer, tiny trust store, bounded/fixed-buffer common-contract authentication |
| MCU-plus | ESP32/ESP32-S3/Nordic-class RTOS | constrained peer, optional commissioner-lite or small registry when measured |
| Linux-edge | Raspberry Pi-class Linux | peer/server, commissioner, local gateway, benchmark/interop harness |
| Accelerated-edge | Jetson-class Linux | peer/gateway/server, large-registry work, formal/fuzz/review artifact generation |

Device class determines resource envelope and optional capabilities, not the mandatory authentication assurance level.

Before any target/profile maturity claim, retain a versioned benchmark manifest containing at minimum:

```text
target + board revision
build/toolchain profile
implementation/library versions
crypto boundary
accelerator family/microarchitecture + software fallback
entropy source + health-test posture
DRBG/reseed behavior
key-generation mode
root-seed vs private-key storage representation
seed/key self-test posture where applicable
secure-storage location
secure-boot/debug state
zeroization assumptions
rollback/clone/reprovision assumptions
wire bytes
stack / heap / static RAM / flash
CPU/latency measurements
registry/trust scaling where affected
replay/restart behavior
dependency inventory
transport / MTU / reliability context
```

Measurements without this context are implementation observations, not portable profile evidence.

## 6. Common-contract layers

### 6.1 Transport contract

ZK-ARCHE should be transport-independent. Candidate transports may include Ethernet, Wi-Fi, BLE, IEEE 802.15.4-class links, LoRa-class links, UART/serial, USB, CAN, TCP, UDP, CoAP mappings, IPC/shared memory, robotics middleware bridges, or proprietary industrial transports.

A transport adapter should expose only the information ZK-ARCHE requires, conceptually:

```text
send(peer/context, bytes)
receive() -> bytes + transport context
max_frame_size / MTU
ordering characteristics
reliability / retransmission characteristics
peer/channel metadata when trustworthy and available
```

Transport metadata is untrusted unless explicitly authenticated or channel-bound. Transport adapters must not become authority for protocol identity merely because they expose an address, MAC, socket, connection identifier, or radio endpoint.

A new transport should normally be added by implementing the adapter contract plus transport-specific conformance tests rather than by forking AUTH/TRUST semantics.

### 6.2 Mandatory cryptographic contract

Every conformant common-contract implementation must support at least one small, conservative, reviewed mandatory suite/floor defining:

```text
key agreement / shared-secret establishment
possession / authentication proof
KDF
MAC / key confirmation
AEAD where required by the profile
hash / transcript digest
CSPRNG / entropy requirements
canonical encoding
transcript rules
replay/freshness semantics
```

The exact primitive selection remains subject to existing cryptographic review, standards comparison, target measurements, and versioned suite governance.

Optional cryptographic suites may extend upward but may not silently replace the mandatory floor or make two otherwise conformant core peers unable to authenticate.

### 6.3 Capability and profile contract

Peers may advertise bounded capability information such as:

```text
protocol version
mandatory suite support
selected protocol/security profiles
max message / frame size
max pending sessions
persistent replay-state capability
secure-storage capability
hardware acceleration
monotonic counter support
resumption
commissioner capability
registry capability
private lookup
remote attestation
advanced credentials
PQ hybrid
DATA
trust-graph support
```

The negotiation contract must distinguish:

- runtime/resource properties;
- selected security profile;
- optional extensions/capabilities.

Capabilities cannot negotiate security away. If no mutually acceptable mandatory security profile exists, the exchange fails closed rather than silently selecting a weaker interpretation.

### 6.4 Identity and local-trust contract

Each peer must be able to make the mandatory trust/authentication decision from locally held or locally verifiable state. That state may have been provisioned, enrolled, delegated, synchronized, or updated through an authorized process, but AUTH itself must not require an external authority lookup.

The common contract must preserve:

- explicit identity/credential-key binding;
- possession proof;
- scoped role/authorization semantics;
- deployment/audience context;
- local policy/epoch/revocation evaluation;
- replay and continuity state;
- fail-closed behavior when required state is unavailable or stale beyond the profile's permitted bound.

### 6.5 Trust-extension contract

A high-end peer may maintain thousands of peers, a large trust graph, policy database, transparency log, search index, or fleet-management state while an MCU holds only a bounded local trust subset.

The constrained node should only need enough information to answer the mandatory local questions:

```text
Who/what key or commitment is proving possession?
Is this trust evidence valid for my local policy?
What scope/role is currently authorized?
Is the authorization sufficiently fresh for this profile?
Has this exchange been replayed or rolled back?
Can I derive/confirm the expected secure association?
```

The large peer's richer state must not become a hidden requirement for the constrained peer's correctness.

### 6.6 Extension contract

Extensions may add features such as TLS/DTLS binding, OSCORE-oriented mappings, private lookup, remote attestation, BBS/selective disclosure, PQ hybrids, DATA sovereignty, analytics, large trust graphs, or future capabilities.

Extensions must be:

- explicitly identified and negotiated;
- versioned where semantics affect interoperability;
- ignorable when non-critical and specified as such;
- fail-closed when critical and unsupported;
- isolated from the mandatory constrained floor;
- covered by deterministic Rust/C or cross-implementation conformance evidence where promoted.

A new extension must not require changing `ZK-ARCHE-CORE` unless an explicitly reviewed versioned migration is approved.

### 6.7 Reticulum-inspired development effort axes

Reticulum's roadmap separates long-lived **Primary Efforts** from individual release items. ZK-ARCHE adopts the same planning principle while adapting the effort areas to a security protocol and constrained-computing framework.

| ZK-ARCHE effort axis | Purpose | Typical roadmap ownership |
|---|---|---|
| **Comprehensibility & Specification** | make the protocol understandable and independently implementable | RFC-class specification, diagrams, annotated traces, threat model, examples, implementation guidance, TD-004 |
| **Universality & Portability** | maximize platform/language coverage without changing security semantics | Rust/C parity, MCU/Linux/edge targets, portable crypto/storage/RNG boundaries, future bindings, TD-002 |
| **Security Functionality & Lifecycle** | improve core authentication, authorization, trust, replay, rekey, revocation, resumption, and P2P behavior | zk211–zk224, zk239–zk241 |
| **Usability & Utility** | lower integration, diagnosis, deployment, and evidence-generation burden | wrappers, manifests, diagnostics, examples, benchmark tooling, reproducible clean-checkout workflows |
| **Interfaceability** | broaden physical, virtual, transport, and middleware integration through adapters rather than protocol forks | BLE, 802.15.4, LoRa-class links, UART, CAN, UDP/TCP, CoAP mappings, IPC, robotics/industrial adapters, zk227–zk230 |
| **Assurance & Verifiability** | continuously raise confidence without overstating claims | deterministic vectors, negative corpora, fuzzing, formal models, side-channel/RNG evidence, independent review, TD-001/TD-003 |

The first five axes parallel the useful roadmap separation demonstrated by Reticulum—comprehensibility, universality, functionality, usability/utility, and interfaceability. **Assurance & Verifiability** is an additional ZK-ARCHE axis because a cryptographic authentication protocol cannot treat security evidence as merely auxiliary work.

A capability can contribute to more than one axis. The axes are a planning/readability lens, not separate protocol layers and not permission to duplicate implementation.

### 6.8 Active-work selection and auxiliary-effort rule

For each development cycle or weekly request set, select a bounded set of active work areas from the effort axes according to current evidence, debt, dependencies, and explicit human priorities. Phase numbers remain ownership identifiers; the effort axes explain *why* the work advances the project.

The current near-term emphasis is:

| Priority | Active effort | Evidence/debt driver |
|---|---|---|
| P0 | Assurance & Verifiability | TD-001 custom proof review and TD-003 formal-model completeness |
| P0 | Comprehensibility & Specification | TD-004 RFC-class normative package and conformance material |
| P0 | Universality & Portability | TD-002 constrained-target measurements and Common Contract feasibility |
| P1 | Security Functionality & Lifecycle | authorization/revocation/resumption lifecycle, P2P local trust, mandatory floor |
| P1 | Interfaceability | prove transport adapters do not alter AUTH/TRUST semantics across materially different links |
| P2 | Usability & Utility | improve tooling/documentation only where it reduces reproducibility or deployment friction |
| Research-only | Advanced suites/features | BBS, PQ hybrids, large trust graphs, remote attestation, or other optional extensions until promotion evidence exists |

Reticulum also distinguishes auxiliary ecosystem efforts from core work. ZK-ARCHE adopts the equivalent rule: tools, bindings, dashboards, gateways, discovery services, policy administration, transport helpers, and ecosystem integrations may greatly improve reach and utility, but they must not become hidden prerequisites for the mandatory Common Contract unless a future reviewed profile explicitly promotes them.

## 7. Review policy

| Work type | Minimum review posture |
|---|---|
| Documentation cleanup, roadmap alignment, wrappers | lightweight review |
| CI repair without protocol semantics | normal review + final verification |
| Transport adapter implementation without security-semantic change | normal review + adapter/conformance verification |
| Wire parsing, negotiation, transcript, proof, KDF/MAC, replay, resumption, authorization, trust mutation, RNG/DRBG | checkpoint-style review |
| Common-contract or mandatory-profile semantics | checkpoint-style review + independent final verification |
| C memory safety, unsafe Rust, fuzz/sanitizer crash corrections | checkpoint-style review |
| Cross-language semantic/vector changes | checkpoint-style review |
| Release-candidate or field-readiness claim | independent evidence review |

## 8. Phase map

| Phase | Purpose | Exit evidence focus |
|---|---|---|
| zk201 | Unified repository baseline and validation inventory | reproducible repository map and validation entry points |
| zk202 | Parent-level CI wrappers and evidence normalization | clean-checkout Rust/C validation |
| zk203 | Replay-test automation and negative-case coverage | deterministic replay/reorder/restart fixtures |
| zk204 | Fuzzing automation, corpus layout, crash triage | retained corpus + reproducible crashes/fixes |
| zk205 | Rust/C deterministic-vector parity | byte- and decision-compatible vectors |
| zk206 | Provisioning/TOFU hardening | explicit local trust-establishment semantics and negative tests |
| zk207 | Formal-model expansion and implementation traceability | canonical state model + property/attacker matrix + traceability |
| zk208 | Side-channel, RNG, key-storage, and execution-context evidence | target-specific crypto execution manifests and review boundaries |
| zk209 | External review package and reproducibility bundle | proof-review package + reproducible assurance bundle |
| zk210 | Release-candidate evidence gate | claim matrix bounded by actual evidence |
| zk211 | Signed one-time late-enrollment grants | scoped/replay-safe grant semantics + vectors |
| zk212 | Delegated commissioner enrollment | authority-limited commissioner flow + audit evidence |
| zk213 | Authenticated rekey and re-registration | atomic lineage replacement + rollback/replay tests |
| zk214 | Enrollment replay, abuse, authorization, and revocation controls | authn/authz separation + convergence/freshness tests |
| zk215 | Optional privacy-preserving credential research | benchmark/review only; no baseline promotion |
| zk216 | IoT profile matrix, common-contract resource envelope, and benchmark harness | target manifests + exact profile resource budgets + mandatory-floor feasibility |
| zk217 | AUTH transcript v3 and complete context binding | selected profile + security context + transcript mutation coverage |
| zk218 | Strict AUTH state machine and observable-failure behavior | sequence/retry/error/privacy negative tests |
| zk219 | Stateless `AUTH_RETRY` and unauthenticated-work throttling | bounded pre-auth cost + privacy/DoS evidence |
| zk220 | Optional encrypted lookup hints | bounded prototype + privacy/rotation/DoS analysis |
| zk221 | Replay-safe 1-RTT session resumption | authorization-aware resumption record + invalidation/reuse tests |
| zk222 | AUTH metrics CI and assurance dashboard | reproducible metrics tied to manifests and claim boundaries |
| zk223 | Optional anonymous-credential migration evaluation | exact footprint/privacy/review comparison |
| zk224 | Optional PQ hybrid suite research | exact packet/resource/fragmentation/downgrade evidence |
| zk225 | Rust/C interop hardening and common-contract vector governance | profile/capability/extension/transport interoperability corpus |
| zk226 | RFC-class specification package and registry discipline | normative grammar/state/registry/common-contract text backed by the RFC-class evidence gate |
| zk227 | EDHOC/CoAP/OSCORE-inspired constrained-profile research | measured comparator, not dependency by analogy |
| zk228 | TLS/mTLS exporter-bound channel binding | unique AUTH-instance exporter context + negative fixtures |
| zk229 | Native datagram robustness and transport-adapter semantics | retry/replay/retransmit/address-context behavior + adapter evidence |
| zk230 | CORE/AUTH/LINK/TRUST/BIND/ENROLL/DATA common-contract decomposition | explicit module ownership + adapter boundary + cross-module lifecycle contracts |
| zk231 | Per-device data sovereignty architecture | threat/policy model and constrained ownership boundaries |
| zk232 | ZK-minimal proof-carrying data profile | bounded primitives and resource evidence |
| zk233 | Minimal `ZK-ARCHE-DATA` commit/release flow | deterministic positive/negative flows |
| zk234 | Policy-bound release tokens and revocable epochs | holder/audience/purpose/epoch lineage and revocation tests |
| zk235 | Local audit hash chain and transparency bridge | continuity/recovery/tamper evidence |
| zk236 | Sovereignty CI gates and footprint budgets | profile-specific measured budgets + deployment context |
| zk237 | Channel-bound sovereignty over secure transports | exporter/channel binding to release context |
| zk238 | Advanced sovereignty research | research-only isolation from constrained baseline |
| zk239 | Infrastructure-independent P2P trust and mutual authentication | local/non-transitive trust + offline operation + revocation/lineage evidence |
| zk240 | `p2p-iot-core` Common Contract profile | MCU↔MCU and MCU↔edge same-assurance interop across representative transports |
| zk241 | Conservative mandatory cryptographic/security floor | reviewed portable floor with constrained-target evidence and downgrade resistance |

## 9. Dependency and priority sequence

Phase numbers identify ownership; they do not imply that every phase must execute strictly serially. The Common Contract is cross-cutting and must shape earlier work rather than appearing only at the final P2P phases.

```text
zk201–zk205 reproducible baseline
        ↓
zk206 local trust semantics
        ↓
zk207 formal traceability ───────┐
zk208 target execution evidence ├─→ zk209 external-review bundle → zk210 claim gate
                                 │
zk211–zk214 enrollment / authorization / revocation lifecycle
        ↓
zk216 common-contract profile + resource evidence
        ↓
zk217 transcript / negotiation context
        ↓
zk218 state machine / failure privacy
        ↓
zk219 retry + zk220 lookup + zk221 resumption
        ↓
zk225 common-contract vector/interoperability governance
        ↓
zk226 RFC-class normative specification
        ↓
zk227–zk230 transport adapter / binding / module decomposition
        ↓
zk239 local P2P trust semantics
        ↓
zk240 p2p-iot-core cross-class interoperability
        ↓
zk241 mandatory reviewed security floor qualification
```

Data-sovereignty work may prototype in parallel but cannot claim specification-grade maturity while shared AUTH/TRUST/LINK/common-contract lifecycle semantics remain underspecified.

## 10. Baseline and assurance work — zk201–zk210

### zk201–zk205: reproducible implementation truth

The repository must have one clean-checkout path that reproduces Rust validation, C validation, canonical Rust vectors, C consumption of those vectors, parser/state negative tests, fuzz entry points, and evidence generation. Vector changes must be versioned and reviewed as protocol-impacting changes.

Cross-target portability work should begin collecting a target/transport matrix early so common-contract assumptions are tested before they become normative.

### zk206: local trust establishment

Provisioning, enrollment, bootstrap, or any TOFU-like experimental mode must be explicit. Normal AUTH remains NO-LEARNING. A peer may learn transport reachability or discovery information without treating it as trusted identity.

The constrained baseline should support locally provisioned/enrolled trust without a CA. Any future TOFU mode must be clearly labeled as a separate policy choice with explicit first-contact attack assumptions and cannot silently become the P2P default.

### zk207: formal assurance contract

Formal work must begin from a property/attacker matrix, not from tool choice alone. The target evidence includes:

- one canonical protocol state model or a mechanically synchronized model source;
- secrecy, authentication/agreement, replay/lifecycle, credential/reference misbinding, anonymity, and unlinkability properties where in scope;
- explicit compromise models;
- explicit infrastructure-loss/offline scenarios for P2P claims;
- no implicit trust transitivity unless delegation evidence is modeled;
- downgrade-resistance and profile-negotiation properties for the common contract;
- observable failure/abort behavior where privacy depends on it;
- retained successful proofs and counterexamples;
- mapping from model states/events to concrete Rust/C functions, fields, vectors, and persistent-state transitions.

Symbolic results must not be reported as proof of constant-time behavior, RNG quality, memory safety, or security of the custom role proof itself.

### zk208: execution-context and side-channel evidence

Target evidence must distinguish platform properties from measured ZK-ARCHE properties. Record crypto boundaries, accelerators, seed/private-key representation, entropy/DRBG posture, secure storage, zeroization, restart/rollback assumptions, transport context, and implementation versions. Define which operations are expected constant time and which dependencies provide those guarantees.

### zk209: independent review package

TD-001 must be converted into a concrete cryptographic review target covering at minimum:

- statement and witness relation;
- completeness and soundness/extractability assumptions;
- simulator behavior for OR composition;
- Fiat-Shamir challenge construction and domain separation;
- canonical serialization and role ordering;
- protocol/instance identifiers;
- scalar sampling and invalid-point/identity handling;
- witness-dependent control flow / constant-time boundaries;
- Rust/C deterministic and negative-vector parity;
- suitability of the reviewed primitive behavior for the mandatory constrained P2P floor.

Review findings must be retained, dispositioned, and converted into regression evidence when representable.

### zk210: claim gate

The release gate must separate at least:

```text
implementation tests pass
protocol/vector conformance
common-contract conformance
RFC-class documentation/conformance status
transport-adapter conformance
cryptographic review status
formal-model status
constrained-target evidence
cross-class P2P interoperability
platform/product deployment context
field evidence
certification status
```

No lower-level success may be summarized as a stronger claim.

## 11. Enrollment, authorization, and lifecycle — zk211–zk215

### Core semantic split

ZK-ARCHE must model three different operations:

1. **Authentication** — prove possession/credential/role membership relative to existing trusted state.
2. **Authorization** — decide what the authenticated peer may do now, under explicit audience, scope, role/policy, validity, epoch, and revocation state.
3. **Trust mutation** — create, replace, delegate, revoke, or otherwise change trusted state through explicit authorized workflows.

Normal AUTH is NO-LEARNING and cannot perform (3).

### zk211–zk212: enrollment grants and commissioners

Late enrollment remains separate from normal AUTH. Grants should be one-time/scoped and bind the intended peer/holder where possible, audience/deployment/domain, suite/profile compatibility, role/policy scope, issuer/authority, validity, nonce/replay state, and authorization lineage.

A commissioner is an optional authority role, not mandatory infrastructure. Commissioner authority must be bounded, auditable, non-transitive unless separately authorized, and unnecessary for two previously authorized peers to reauthenticate directly.

### zk213: rekey and lineage replacement

Rekey/re-registration must prove control of the current credential and new key, bind the operation to the current authorized session/context, and atomically replace or tombstone the predecessor. Negative evidence must cover replay, rollback, lost predecessor state, concurrent replacement, partial write, privilege expansion, and recovery.

### zk214: revocation as convergence

Revocation is not a local delete event. Offline-capable profiles need a versioned issuer/epoch-scoped revocation view with enough state to:

- reconcile full and differential updates;
- survive missed notifications and reconnect;
- detect rollback or stale snapshots;
- define a maximum stale-authorization window or freshness objective;
- invalidate dependent sessions, resumption credentials, derived keys, cached authorization, and DATA release state where applicable.

Always-online infrastructure must not become mandatory for the constrained baseline. If a peer is offline beyond the profile's permitted freshness bound and cannot safely decide, it must fail closed or restrict authorization according to the normative profile rather than silently assuming fresh authority state.

### zk215: advanced credentials remain isolated

BBS/selective-disclosure or other advanced credentials remain optional research until exact proof/signature bytes, pairing/CPU/RAM/flash cost, dependency maturity, revocation/issuance model, privacy gain, and external-review status justify a versioned optional suite.

## 12. AUTH hardening — zk216–zk224

### zk216: Common Contract profile and benchmark contract

Distinguish:

- local runtime/resource configuration;
- negotiated protocol/security profile;
- mandatory common-contract behavior;
- optional capability/extension bits.

A profile claim must include exact/bounded wire bytes, memory, CPU, persistent-state behavior, dependency cost, registry scaling, transport/MTU assumptions, and the `crypto_execution_context` manifest defined above.

The benchmark program must determine whether the mandatory floor is realistically implementable on the least-capable supported targets. If it is not, the response is to revise the mandatory floor through reviewed design—not to create an undocumented weaker security mode under the same profile.

### zk217: transcript v3

Every security-relevant semantic input should be unambiguously bound, including as applicable:

```text
protocol version
suite / method / selected profile
capabilities and critical extensions
session / sequence identifiers
ephemeral keys
peer identity / key commitment
role/policy and authorization context
deployment/domain and audience
transport/channel-binding label when used
fresh AUTH-instance identifier where required
canonical payload bytes
```

A profile/capability distinction must be explicit enough that a high-end peer cannot negotiate broad capabilities and then execute semantics inconsistent with the selected constrained profile. Downgrade from a stronger optional extension to the mandatory floor is acceptable only when the mandatory floor itself remains fully authenticated and policy-acceptable; downgrade below the floor is not.

### zk218: state machine and privacy observability

The state machine must reject wrong type, wrong sequence, stale/cross-session messages, reflected messages, invalid retransmissions, unsupported critical selections, wrong peer/context bindings, and inconsistent negotiated profile behavior. Error classes, response/no-response behavior, size buckets, retry behavior, and timing must be considered part of privacy assurance rather than purely operational detail.

### zk219: retry/source validation

`AUTH_RETRY` should provide a stateless or bounded MAC-based source-validation gate before expensive registry scans or proof verification in exposed datagram deployments. Evidence must include amplification bounds, CPU asymmetry, retry-token lifetime/replay behavior, malformed/unknown-device equivalence where intended, and interaction with privacy-normalized failures.

A retry cookie is an anti-abuse mechanism, not an external identity authority.

### zk220: optional encrypted lookup hint

The first prototype path may use a randomized standard-primitive encrypted opaque registry key as a **non-authoritative prefilter**. Full PID/proof/possession/transcript/state verification remains mandatory. Promotion requires exact wire/CPU/RAM cost, key-epoch rotation/revocation behavior, retry placement, passive-linkability analysis, and Rust/C deterministic/negative vectors.

Private lookup optimization must not become necessary for constrained P2P correctness.

### zk221: authorization-aware resumption

A resumption credential is not merely a secret. The future record should bind enough state to prove that privilege remains valid, such as:

```text
peer/holder
deployment/domain/audience
suite/profile
original security context
authorization lineage/generation
policy/registry/revocation epoch
issue + expiry
reuse/usage limits
privacy-relevant identifier state
```

On resumption, changed context that can affect authorization must be reevaluated locally. If safe reevaluation is impossible or required cached state is missing, fall back to full AUTH. Negative tests must cover revoked credentials, stale lineage, changed role/policy/audience/deployment, missing cache, rollback, excessive reuse, and repeated-identifier linkability.

General-purpose state-changing 0-RTT remains out of scope for the constrained baseline.

### zk222: metrics and evidence

Metrics must be tied to reproducible manifests and claim boundaries. Dashboards must not convert “test passed” into “secure,” “formally verified,” “common-contract conformant,” or “field ready.”

### zk223–zk224: optional advanced suites

Anonymous credentials and PQ hybrids remain research/edge profiles until target measurements and interoperability/review evidence justify promotion. PQ work must account for packet/MTU/fragmentation/loss pressure and downgrade/method-suite compatibility, not only primitive availability.

A high-end peer may use an advanced suite with another capable peer, but it must retain the mandatory common floor if it claims interoperability with constrained peers.

## 13. Rust/C and cross-platform conformance governance — zk225

Rust remains the canonical checked-in vector source unless a future reviewed migration changes that policy. C remains an independent implementation lane and the initial constrained portability anchor.

Required gates include:

- C passes current canonical Rust vectors;
- vector regeneration has no unexplained drift;
- wire/transcript/proof/KDF/MAC/state changes produce versioned vectors;
- unknown non-critical extension values remain ignorable where specified;
- unsupported critical selections fail closed;
- suite/method/profile compatibility is tested, not merely documented;
- deterministic GREASE-style reserved-value fixtures exercise extension points so they do not ossify;
- Rust/C produce the same accept/reject decision for the conformance corpus;
- transport adapters do not alter protocol-level identity or trust semantics;
- identical common-contract vectors can be exercised across representative target classes;
- MCU↔edge exchanges prove mutual rather than one-sided authentication;
- optional high-end capability advertisements do not cause constrained peers to weaken or misinterpret the mandatory floor.

Live random GREASE traffic is optional and must respect constrained-link budgets; deterministic CI coverage is the required baseline.

## 14. Specification maturity and module ownership — zk226–zk230

ZK-ARCHE should evolve toward an RFC-class package with normative grammar, registries, state machines, security/privacy considerations, implementation requirements, transport-adapter requirements, deterministic positive/negative vectors, annotated traces, and independent interoperability evidence. The detailed protocol-reference matrix and full RFC-class exit gate remain centralized in [`rfc-evolution-plan.md`](./rfc-evolution-plan.md).

Target suite ownership remains:

```text
ZK-ARCHE-CORE   common contract, wire format, canonical encoding, registries, transcript rules
ZK-ARCHE-AUTH   native device/role authentication and P2P mutual authentication
ZK-ARCHE-LINK   secure association, replay, key lifecycle, resumption, key export
ZK-ARCHE-TRUST  trusted records, scoped authorization evidence, lineage, revocation
ZK-ARCHE-BIND   transport/channel bindings and adapter-derived context
ZK-ARCHE-ENROLL setup, late enrollment, commissioner grants, rekey/revocation
ZK-ARCHE-DATA   encrypted data records, policy-bound release, auditability
```

Every normative security behavior needs at least one of: positive vector, negative vector, executable test, scoped formal-model result, reviewed proof argument, or explicit evidence gap.

EDHOC/OSCORE, TLS/mTLS, DTLS, ACE, IKEv2/IPsec, QUIC, RATS, MLS, WireGuard, Reticulum, and related systems are engineering comparators. They do not become dependencies merely because they motivate a requirement.

### zk230 Common Contract decomposition

`ZK-ARCHE-CORE` should define the stable interoperability contract and keep transport/platform details below it and optional security/application extensions above it.

Conceptually:

```text
                 optional extensions
      ┌──────────────────────────────────┐
      │ PQ │ BBS │ ATTEST │ DATA │ etc. │
      └────────────────┬─────────────────┘
                       │
              ZK-ARCHE Common Contract
      ┌──────────────────────────────────┐
      │ CORE AUTH LINK TRUST ENROLL BIND │
      └────────────────┬─────────────────┘
                       │
                 transport adapter
      ┌──────────────────────────────────┐
      │ UDP TCP BLE UART CAN 802.15.4…  │
      └────────────────┬─────────────────┘
                       │
                 hardware / OS
```

A new platform or transport should normally be integrated below the common contract. A new optional security capability should normally be integrated above it. Changes to the common contract require the strongest compatibility/review posture because they affect every conformant peer.

## 15. Transport and channel-binding work — zk227–zk230

### Transport-adapter conformance

For each promoted adapter, document and test:

```text
framing / message-boundary handling
MTU and fragmentation behavior
ordering / duplicate behavior
retransmission ownership
address/context volatility
maximum unauthenticated amplification
what transport metadata is available
which metadata is authenticated or untrusted
channel-binding availability
resource footprint
```

An address supplied by BLE, UDP, UART topology, CAN identifier, MAC address, or another transport cannot automatically become ZK-ARCHE identity.

### EDHOC/OSCORE-inspired constrained work

Use measured comparison for handshake bytes, flights, exporter/context semantics, retransmission behavior, parser/dependency footprint, and MCU resource cost. Do not optimize for “smaller than EDHOC”; explain what additional ZK-ARCHE bytes/cycles provide in privacy/authorization terms.

### TLS exporter binding

A future TLS binding must be unique to the **ZK-ARCHE AUTH instance**, not only to the underlying TLS connection. Define a normative exporter label/context including application/ALPN, deployment/domain, endpoint identity or commitment, suite/profile, fresh AUTH-instance identity, and relevant transcript state. Cross-instance, cross-application, wrong-endpoint, proxy/termination, and TLS-resumption cases need deterministic fixtures.

TLS is an optional binding. The core P2P profile must not require TLS or a PKI.

### Datagram robustness

Native UDP/DTLS-style behavior must define retry/source validation, replay windows, duplicate handling, retransmission, bounded response caches, session/epoch identity, reordering behavior, address changes where supported, amplification limits, and privacy-aware error behavior.

Equivalent robustness semantics must be adapted appropriately for other unreliable or constrained transports without forking AUTH/TRUST semantics.

## 16. Data sovereignty — zk231–zk238

Per-device data sovereignty means a device cryptographically controls release of protected data by recipient/holder, audience, purpose, data type, policy, time/epoch, and revocation state. Protected data is encrypted by default and release authorization remains separate from authentication.

The constrained proof floor remains bounded primitives such as Schnorr possession proofs, small role-membership proofs, MAC/signature authorization tickets, hash commitments, and measured fixed-depth inclusion proofs. General-purpose circuits and heavyweight credentials remain optional research.

A minimal flow may include `DATA_COMMIT`, `RELEASE_REQUEST`, `RELEASE_PROOF`, `RELEASE_KEY`, and `AUDIT_APPEND`. Promotion requires parser/replay/policy mutation/epoch invalidation/protected-plaintext/fixed-buffer tests plus target resource evidence.

Data sovereignty must also honor the Common Contract doctrine: a small device may use a smaller local policy/trust representation, but a larger peer cannot require a gateway-only policy engine merely to authenticate the device or request a bounded release under a profile the device supports.

Field-readiness evidence for sovereignty must include deployment context: platform update posture, provisioning/revocation authority, storage protections, external controls, and residual trust assumptions.

## 17. Infrastructure-independent P2P zero trust — zk239–zk241

### 17.1 P2P role model

P2P mode uses per-handshake `initiator` and `responder` roles rather than permanent client/server identity roles. Either side may initiate. Both sides authenticate, confirm the same transcript/security context, derive/confirm compatible secure-association material, and locally evaluate scoped trust/authorization evidence.

No peer becomes inherently more trusted because it has more CPU, memory, storage, network connectivity, or administrative functionality.

### 17.2 Local and non-transitive trust

Trust is local and not implicitly transitive.

```text
A trusts B
B trusts C
──────────
A does NOT automatically trust C
```

Delegation must be explicit and scope-, role-, audience-, depth-, validity-, issuer-, and epoch-limited and revocable. A constrained peer may support only a bounded subset/depth of delegation while remaining fully conformant to the mandatory authentication floor.

Credential/reference mappings belong in the threat model; a reference cannot be treated as security identity unless it is cryptographically bound to the exact key/commitment, authorization context, deployment/audience, epoch, and allowed operations.

### 17.3 No external root-of-trust dependency

For two peers that already possess sufficient locally authorized state:

```text
CA unavailable                 → core AUTH still works
Internet unavailable           → core AUTH still works
cloud service unavailable      → core AUTH still works
central registry unavailable   → core AUTH still works
DNS unavailable                → core AUTH still works
gateway unavailable            → direct P2P AUTH still works
```

The exact result may still depend on local policy freshness. If revocation/policy state is beyond the profile's allowed stale window, fail-closed behavior is permitted and may be required. That is a local security decision, not a dependency on an external authority to perform the cryptographic authentication exchange.

### 17.4 Cross-class same-assurance interoperability

`p2p-iot-core` is the mandatory cross-class interoperability profile. A higher-capability peer adapts to the constrained floor when the constrained floor is the mutually selected profile.

Required directionality includes both sides:

```text
STM32-class peer  ↔ STM32-class peer
STM32-class peer  ↔ ESP32-S3-class peer
STM32-class peer  ↔ Raspberry Pi-class peer
STM32-class peer  ↔ Jetson-class peer
Jetson-class peer ↔ STM32-class peer
```

The important property is not equal execution cost. It is equal mandatory authentication assurance and compatible security semantics.

### 17.5 Common P2P interoperability test matrix

Before claiming `p2p-iot-core` interoperability, retain evidence for at least:

| Initiator | Responder | Infrastructure condition | Required behavior |
|---|---|---|---|
| MCU-core | MCU-core | none | mutual authentication and secure association |
| MCU-core | MCU-plus | none | mutual authentication under same mandatory floor |
| MCU-core | Linux-edge | none | mutual authentication; edge adapts to constrained floor |
| MCU-core | accelerated-edge | none | mutual authentication; no gateway/CA dependency |
| accelerated-edge | MCU-core | none | reverse-direction initiation works with same assurance |
| MCU-core | MCU-core | Internet unavailable | core P2P remains functional |
| MCU-core | edge | CA/cloud unavailable | core P2P remains functional |
| MCU-core | edge | gateway unavailable | direct P2P remains functional when transport reachability exists |
| unknown/untrusted peer | MCU-core | none | reject; AUTH does not mutate trust |
| revoked peer | any | local state current | reject and invalidate dependent state as specified |
| peer with stale auth state | any | beyond permitted freshness | deterministic fail-closed/restricted behavior per profile |
| high-end peer advertises unsupported optional extensions | MCU-core | none | select common floor or fail closed; never weaken security |
| incompatible mandatory security floors | any | none | fail closed |
| transport changes while identity remains same | any | permitted profile | identity decision remains cryptographically bound, not address-bound |

### 17.6 `zk239` exit criteria — decentralized trust semantics

Exit evidence should include:

- local trust-record semantics;
- NO-LEARNING normal AUTH;
- explicit delegation model;
- non-transitivity tests;
- revocation/epoch convergence behavior;
- offline and infrastructure-loss scenarios;
- bounded local trust representation for constrained peers;
- formal/model traceability for trust/authorization decisions;
- threat model for credential/reference mapping.

### 17.7 `zk240` exit criteria — Common Contract P2P profile

Exit evidence should include:

- normative `p2p-iot-core` common contract;
- exact mandatory and optional feature split;
- target resource ceilings/budgets;
- MCU↔MCU and MCU↔edge bidirectional interoperability;
- at least two materially different transport/adaptation contexts when practical;
- fixed/bounded parsing and state behavior for the constrained implementation;
- profile/capability downgrade-resistance tests;
- no hidden CA/cloud/gateway dependency in the core path;
- same mandatory accept/reject semantics across Rust/C and declared target implementations;
- evidence that a higher-capability peer can adapt down without reducing the mandatory security properties.

### 17.8 `zk241` exit criteria — mandatory reviewed security floor

Exit evidence should include:

- exact mandatory suite and security-property definition;
- independent review status appropriate to custom proof behavior and suite integration;
- deterministic positive/negative vectors;
- constrained-target CPU/RAM/flash/wire measurements;
- entropy/RNG/key-storage execution-context evidence;
- replay/transcript/authorization binding tests;
- downgrade and incompatible-profile failure tests;
- no hardware-accelerator requirement for semantic correctness;
- no mandatory BBS/PQ/general-purpose ZK/certificate-chain/trust-graph dependency;
- confirmation that optional higher-end features do not weaken or replace the mandatory floor.

## 18. Compatibility philosophy

The roadmap adopts four compatibility rules.

### 18.1 Implement the contract, not the hardware

Hardware-specific optimizations and adapters are permitted. Hardware-specific protocol semantics are not the default architecture.

### 18.2 Adapt downward for interoperability

The less-capable peer may define the resource envelope of a mutually acceptable profile. It must not define a weaker authentication model.

### 18.3 Extend upward without fragmenting the protocol

Prefer:

```text
ZK-ARCHE Common Contract + OPTIONAL_X
```

over:

```text
ZK-ARCHE-for-STM32
ZK-ARCHE-for-ESP32
ZK-ARCHE-for-Jetson
```

Device-specific profiles may exist for measured resource tuning, but they must inherit the same normative common-contract semantics when they claim interoperability.

### 18.4 Infrastructure may assist but not own the root authentication decision

Discovery, synchronization, audit, registry acceleration, policy administration, or fleet tooling may rely on infrastructure in deployment-specific profiles. The mandatory P2P authentication floor must remain locally executable by the peers themselves.

## 19. Roadmap completion and claim rules

A roadmap phase is not “done” because code exists. Completion means the phase's declared evidence exists and repository claim language matches that evidence.

Use these distinctions consistently:

```text
DESIGNED              architecture/spec intent exists
IMPLEMENTED           code path exists
TESTED                deterministic tests pass
INTEROPERABLE         declared implementations agree
COMMON-CONFORMANT      mandatory common-contract corpus passes
RFC-CLASS DOCUMENTED   RFC-class specification/conformance gate in rfc-evolution-plan.md is satisfied
MEASURED              target/resource evidence exists
FORMALLY ANALYZED      scoped model result exists
EXTERNALLY REVIEWED    independent review exists
DEPLOYMENT-QUALIFIED   platform/product context and required field evidence exist
```

Never infer a stronger state from a weaker one.

`RFC-CLASS DOCUMENTED` is an internal engineering maturity label, not a claim that ZK-ARCHE is an RFC, Internet Standard, IETF product, or externally standardized protocol.

A device may be **platform-supported** without yet being **Common Contract conformant**. A transport adapter may be **implemented** without being **interop-qualified**. A P2P demo may work without proving **infrastructure independence**, **same-assurance cross-class interoperability**, **RFC-class documentation**, or **field readiness**.

## 20. Agent editing contract

Future automated edits must preserve:

- Rust/C byte-level and decision compatibility;
- canonical vector governance;
- the ZK-ARCHE Common Contract as the cross-platform interoperability boundary;
- bottom-up interoperability: capability scales upward, mandatory security does not scale downward;
- the RFC-class reference doctrine, with `rfc-evolution-plan.md` as the single detailed owner of RFC numbers, standards comparators, and the full RFC-class evidence gate;
- the Reticulum-inspired effort-axis model as a planning/readability lens while preventing auxiliary ecosystem work from becoming hidden core dependencies;
- NO-LEARNING normal AUTH;
- separation of authentication, authorization, and trust mutation;
- local/non-transitive P2P trust unless explicit delegation is verified;
- `iot-core` and `p2p-iot-core` as constrained interoperability floors with full mandatory assurance, not weak-security modes;
- no CA/cloud/gateway/Internet dependency in the mandatory P2P authentication path;
- transport independence and the rule that transport addresses are not automatically protocol identity;
- optional extension isolation and downgrade-resistant profile negotiation;
- high-end peers adapting to constrained peers rather than forcing high-end dependencies onto them;
- strict evidence/claim separation;
- no production/security/certification/field-readiness claims without corresponding evidence;
- no mandatory SNARK/STARK, advanced credential, PQ, large trust graph, certificate-chain, or remote-attestation work unless a future explicit charter promotes it into the constrained baseline with evidence;
- checkpoint-style review for protocol, common-contract, crypto, parsing, negotiation, replay/resumption, authorization/revocation, RNG/key lifecycle, memory safety, formal-model, privacy, side-channel, and interoperability changes.