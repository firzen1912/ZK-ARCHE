# ZK-ARCHE Unified Rust/C Improvement Roadmap

This roadmap defines an evidence-gated improvement path for the unified ZK-ARCHE Rust and C repository. It is designed for ordinary maintenance, automated-agent assistance, and checkpoint-style review when security-sensitive protocol or implementation changes are proposed.

The roadmap is evidence-gated, not calendar-gated. It does not claim production readiness, formal verification, side-channel resistance, complete replay resistance, IoT field readiness, external cryptographic review, or certification unless the required evidence exists as checked-in artifacts.

## Current Baseline

The repository contains two implementation lanes:

| Lane | Path | Role |
|---|---|---|
| Rust reference | `rust/` | Cargo workspace, protocol library, binaries, canonical deterministic test vectors, fuzz targets, symbolic-model skeletons, and higher-level validation |
| C implementation | `c/` | libsodium-based C11 implementation, public headers, unit/e2e/vector tests, fuzz harnesses, symbolic-model skeletons, and constrained-device implementation anchor |

Rust test vectors under `rust/test-vectors/0x0001/` are the canonical byte-level interoperability anchor. The C harness consumes them directly and must preserve the same wire, transcript, proof, KDF, and key-confirmation semantics.

## Non-Negotiable Boundaries

- Do not change cryptographic primitives, domain separators, packet formats, suite identifiers, test-vector meanings, replay semantics, or wire compatibility without checkpoint-style review and evidence.
- Rust and C must remain byte-compatible wherever both lanes implement the same vector, transcript, wire-header, TLV, proof, KDF, MAC, or protocol-state-machine behavior.
- Normal `AUTH` remains proof of prior enrollment. Unknown-device self-registration inside normal authentication is not allowed without an explicitly reviewed enrollment design.
- `iot-core` and `p2p-iot-core` remain ZK-minimal. General-purpose SNARK/STARK proving, large anonymous-credential provers, post-quantum hybrids, large certificate-chain parsers, and heap-heavy policy engines cannot become mandatory for MCU-class devices.
- C hot paths should use fixed or bounded buffers where practical, deterministic failure behavior, and no mandatory dynamic allocation after initialization for the constrained baseline.
- Rust must preserve `cargo fmt`, `cargo check`, `cargo test`, and `cargo clippy -D warnings` evidence; C must preserve strict compiler/static-analysis and sanitizer evidence.
- Fuzz targets and formal models are evidence producers, not proof of complete security by themselves.
- Automated edits may improve implementation quality, validation, documentation, and traceability, but must not weaken assurance-status truthfulness.

## IoT Capability and Assurance Contract

ZK-ARCHE must remain implementable across heterogeneous targets rather than drifting toward gateway-only assumptions.

| Class | Representative devices | Expected role |
|---|---|---|
| MCU-core | STM32-class bare-metal/RTOS | constrained peer/client, tiny trust store, fixed-buffer authentication |
| MCU-plus | ESP32/ESP32-S3-class RTOS | constrained peer, optional small registry or commissioner-lite behavior when benchmarked |
| Linux-edge | Raspberry Pi-class Linux | peer/server, commissioner, local gateway, test harness |
| Accelerated-edge | Jetson Orin-class Linux | peer/gateway/server, large-registry benchmarks, fuzz/formal/review artifact generation |

Profile rules:

- `iot-core` and `p2p-iot-core` are the constrained interoperability floors.
- High-end peers adapt downward to the constrained profile rather than forcing gateway-class features onto MCUs.
- Heavy credential, post-quantum, large-graph, and general-purpose ZK features stay suite-negotiated and research/edge-only until measured and reviewed.
- UDP profiles preserve a small-datagram design target; oversized extensions require an explicit TCP, fragmentation, or gateway-only profile.
- Hardware acceleration may optimize an implementation but must not define protocol correctness.

Every material feature must report, before maturity claims: wire bytes, RAM/stack/heap profile, CPU cost, registry/trust scaling where affected, replay behavior, transcript-binding mutation coverage, Rust/C interoperability evidence, and failure behavior.

## Review Policy

| Work type | Required posture |
|---|---|
| Documentation cleanup, roadmap alignment, wrapper scripts | Lightweight review |
| CI repair without protocol-semantic changes | Normal review plus final verification |
| Replay, wire parsing, transcripts, proof verification, KDF/MAC, RNG/DRBG | Checkpoint-style review |
| C memory-safety changes, unsafe Rust, sanitizer or fuzz-crash fixes | Checkpoint-style review |
| Cross-language compatibility changes | Checkpoint-style review |
| Release-candidate gate review | Independent final verification |

## Phase Map

| Range | Purpose |
|---|---|
| zk201 | Unified repository baseline and validation inventory |
| zk202 | Parent-level CI wrappers and evidence normalization |
| zk203 | Replay-test automation and negative-case coverage |
| zk204 | Fuzzing automation, corpus layout, and crash triage |
| zk205 | Rust/C deterministic-vector parity and interoperability |
| zk206 | TOFU/provisioning hardening |
| zk207 | Formal-model expansion and implementation traceability |
| zk208 | Side-channel and RNG evidence |
| zk209 | External review package and reproducibility bundle |
| zk210 | Release-candidate evidence gate |
| zk211 | Signed one-time late-enrollment grants |
| zk212 | Delegated commissioner enrollment |
| zk213 | Authenticated rekey and re-registration |
| zk214 | Enrollment replay, abuse, and authorization controls |
| zk215 | Optional privacy-preserving credential research |
| zk216 | IoT profile matrix and benchmark harness |
| zk217 | AUTH transcript v3 and complete context binding |
| zk218 | Strict AUTH state-machine and sequence validation |
| zk219 | Stateless `AUTH_RETRY` cookies and unauthenticated-work throttling |
| zk220 | Optional encrypted lookup hints |
| zk221 | Replay-safe 1-RTT session resumption |
| zk222 | AUTH metrics CI and security-assurance dashboard |
| zk223 | Optional reviewed anonymous-credential migration evaluation |
| zk224 | Optional post-quantum hybrid suite research |
| zk225 | Rust/C interoperability hardening and vector-governance consolidation |
| zk226 | RFC-style specification package and registry discipline |
| zk227 | EDHOC/CoAP/OSCORE-inspired constrained-profile research |
| zk228 | TLS/mTLS exporter-bound channel-binding profile |
| zk229 | DTLS-style datagram robustness |
| zk230 | Protocol-suite decomposition into CORE/AUTH/LINK/TRUST/BIND/ENROLL/DATA |
| zk231 | Per-device data sovereignty architecture |
| zk232 | ZK-minimal proof-carrying data profile |
| zk233 | Minimal `ZK-ARCHE-DATA` commit/release flow |
| zk234 | Policy-bound release tokens and revocable epochs |
| zk235 | Local audit hash chain and gateway transparency bridge |
| zk236 | Sovereignty CI gates and footprint budgets |
| zk237 | Channel-bound sovereignty over secure transports |
| zk238 | Advanced sovereignty research kept outside the constrained baseline |
| zk239 | P2P zero-trust trust graph and initiator/responder mutual authentication |
| zk240 | P2P IoT profile contract |
| zk241 | Conservative mandatory crypto baseline |

## Baseline and Interoperability Work — zk201–zk210

The first ten phases establish navigability, repeatable CI, deterministic-vector governance, replay/fuzz/formal evidence, platform assurance, and a reproducible external-review package. The release-candidate gate requires current Rust and C CI evidence, passing C validation against the canonical Rust vectors, replay/fuzz status, scoped formal-model status, side-channel/RNG status, and truthful external-review status.

Blocked claims remain: production-ready cryptographic security, complete replay resistance, complete side-channel resistance, formal verification of the full implementation, IoT field readiness, certification, or completed external review without supporting evidence.

## Enrollment and Lifecycle — zk211–zk215

Late enrollment must remain a separate authorized act from normal `AUTH`. The preferred direction is a signed, one-time `EnrollmentGrant` bound to suite, server/domain, device public-key commitment where known, role policy, validity window, nonce, issuer, and replay state. Grant use must require device proof of possession and reject expired, wrong-server, wrong-suite, wrong-role, malformed, unsigned, and replayed grants.

Delegated commissioner enrollment may allow an already-authenticated and authorized peer/operator to request a scoped grant for a new device. Commissioner authority must be role-limited, auditable, rate-limited, and unable to transfer existing device secrets.

Authenticated rekey must prove control of both the current credential and new key, bind the operation to the authenticated session, and atomically tombstone/revoke the old record. Replay, rollback, concurrency, partial-write, and privilege-expansion cases require negative tests.

Privacy-preserving credential alternatives remain research until their privacy gain, wire size, CPU/RAM footprint, dependency cost, suite isolation, migration path, and external-review status are explicit.

## AUTH Hardening — zk216–zk224

The constrained profile must report exact or bounded AUTH bytes, stack/heap, CPU, persistent-storage behavior, and registry lookup costs. AUTH transcript v3 should bind every negotiated or security-relevant field that affects semantics, including version/suite/profile, capabilities, sequence/session fields, ephemeral keys, server identity, role-policy hash, transport label, deployment/domain identifier, and canonical payload bytes.

The state machine must reject wrong type, wrong sequence, stale or cross-session packets, reflected messages, wrong peer/address bindings, and invalid retransmissions. Pending-session and response-cache state must remain bounded.

`AUTH_RETRY` should provide a stateless or bounded MAC-only source-validation gate before expensive registry scans or proof verification in exposed datagram deployments. Encrypted lookup hints may provide randomized server-readable candidate lookup without exposing a stable cleartext device identifier, while preserving the existing privacy-oriented scan fallback.

Resumption should be 1-RTT, transcript-bound, privilege-preserving, replay-aware, and invalidated by relevant registry/role/policy changes. General-purpose state-changing 0-RTT is out of scope for the constrained baseline.

Anonymous-credential and post-quantum suites remain optional research/edge profiles until exact packet, RAM, CPU, implementation, and external-review evidence justifies broader use.

## Rust/C Vector Governance — zk225

Rust remains the canonical checked-in vector source. C is the independent implementation lane used to validate the same byte-level semantics.

Required gates:

- C harness passes against `rust/test-vectors/0x0001/`.
- Rust vector regeneration produces no unreviewed drift.
- Vector changes are versioned and reviewed as protocol-impacting changes.
- Transcript, PID, proof, rerandomization, KDF, MAC/key-confirmation, wire-header, and TLV parity are tested wherever implemented in both lanes.
- Interoperability failures block release qualification.

## Specification Maturity — zk226–zk230

ZK-ARCHE should evolve toward an RFC-like protocol package with normative message grammar, registries, state machines, security/privacy considerations, implementation requirements, and deterministic positive/negative vectors. EDHOC/OSCORE, TLS/mTLS, and DTLS are engineering references and optional binding directions, not claims that ZK-ARCHE replaces those protocols.

The target suite decomposition is:

```text
ZK-ARCHE-CORE   wire format, canonical encoding, registries, transcript rules
ZK-ARCHE-AUTH   native role/device authentication and P2P mutual authentication
ZK-ARCHE-LINK   secure peer association, replay windows, resumption, key export
ZK-ARCHE-TRUST  peer records, scoped trust evidence, grants, revocation epochs
ZK-ARCHE-BIND   secure-channel and transport bindings
ZK-ARCHE-ENROLL setup, late enrollment, commissioner grants, rekey, revocation
ZK-ARCHE-DATA   encrypted data records, policy-bound release, auditability
```

Every normative security behavior needs a positive vector, negative vector, executable test, scoped formal-model claim, or explicit evidence gap.

## Data Sovereignty — zk231–zk238

Per-device data sovereignty means each device cryptographically controls what protected data is released, to whom, for what purpose, during what epoch, and under what policy. Protected data is encrypted by default; release material binds recipient, purpose, data type, time range, policy hash, device/registry/policy epochs, replay nonce/counter, and channel context when present.

The constrained proof profile is limited to bounded primitives such as Schnorr possession proofs, small role-membership proofs, MAC/signature authorization tickets, hash commitments, and small fixed-depth inclusion proofs where measured. General-purpose circuit proving and large credential systems are not mandatory constrained features.

A minimal data flow may include `DATA_COMMIT`, `RELEASE_REQUEST`, `RELEASE_PROOF`, `RELEASE_KEY`, and `AUDIT_APPEND`. Parsers, replay behavior, policy mutation, epoch revocation, protected-plaintext prevention, proof-size limits, and fixed-buffer constraints require CI evidence before feature-complete claims.

## P2P Zero Trust — zk239–zk241

P2P mode replaces permanent client/server identity roles with per-handshake `initiator` and `responder` roles. Either peer may initiate; both peers mutually authenticate, confirm the same transcript, and locally evaluate scoped trust evidence. Trust is not implicitly transitive. Delegation is policy-, scope-, depth-, role-, and epoch-limited and revocable.

Every P2P implementation must support `p2p-iot-core`. High-end peers must be able to authenticate constrained peers using the same mandatory floor without requiring a full trust-graph evaluator, certificate-chain parser, heavyweight credential prover, post-quantum hybrid, or general-purpose ZK prover on the MCU.

The conservative mandatory crypto direction uses standard portable building blocks: Curve25519/Ristretto-compatible ECDH or equivalent reviewed baseline, Ed25519 or bounded Schnorr-style possession proof, HKDF-SHA256, a standard MAC, ChaCha20-Poly1305 or measured hardware-aligned AEAD profile, SHA-256/BLAKE2-family hashing as profiled, canonical encoding, transcript binding, replay protection, and deterministic cross-language vectors.

## Agent Editing Contract

Future automated edits must preserve:

- Rust/C byte-level compatibility;
- the Rust vector corpus as the canonical checked-in interop anchor unless a checkpoint-approved migration changes that policy;
- `iot-core` and `p2p-iot-core` as constrained interoperability floors;
- conservative primitive-based authentication as the implementation baseline;
- strict separation between evidence and claims;
- no production/security/certification claims without evidence;
- no mandatory SNARK/STARK implementation work unless a future charter explicitly reopens that scope;
- checkpoint-style review for protocol, crypto, parsing, replay, RNG, memory-safety, formal-model, side-channel, and interoperability changes.
