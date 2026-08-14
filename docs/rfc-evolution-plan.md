# ZK-ARCHE RFC-Style Protocol Evolution Plan

This document defines how ZK-ARCHE should evolve from a Rust/C research implementation into an RFC-like protocol package while preserving deployability on heterogeneous IoT targets.

The goal is not to claim IETF adoption. The goal is to make ZK-ARCHE written, tested, and reviewed with the discipline expected from protocols such as EDHOC, TLS/mTLS, DTLS, and OSCORE.

## Scope and terminology

ZK-ARCHE remains a privacy-preserving device authentication and role-authorization protocol. It should not silently become a generic TLS replacement. The specification effort should define:

- a native ZK-ARCHE wire-protocol profile;
- an optional CoAP/OSCORE-oriented constrained-IoT profile;
- optional TLS/mTLS or DTLS channel-binding profiles for deployments that already require those transports;
- a conformance and interoperability suite shared by the Rust and C implementations.

`DTLS` is the intended comparison target for datagram security. Internal references to `DLTS` should be treated as typographical errors unless a future document explicitly defines another term.

## Standards used as design references

| Reference family | Property to emulate | ZK-ARCHE implication |
|---|---|---|
| EDHOC / CoAP / OSCORE | compact authenticated key exchange, constrained-node orientation, canonical encodings, security-context export | define a compact profile and clean key-export/context interface |
| TLS 1.3 / mTLS | transcript binding, downgrade resistance, endpoint authentication, structured key schedule, alert taxonomy | bind negotiated fields into finished/binder calculations and define strict state/error behavior |
| DTLS 1.3 | datagram robustness, anti-amplification, retransmission, replay windows, connection identifiers | specify native UDP behavior explicitly |
| BRSKI/FDO/Matter-style onboarding | separable commissioning and enrollment | keep late enrollment outside normal `AUTH` and require explicit authorization |
| Privacy credentials | unlinkable authorization and selective disclosure | keep advanced credential systems optional until size, review, and IoT benchmarks exist |

## RFC-style deliverables

Before specification-grade claims, maintain at least:

| Artifact | Path | Purpose |
|---|---|---|
| Protocol specification | `spec/zk-arche-protocol.md` | normative flows, fields, cryptographic computations, state machines |
| Registries | `spec/registries.md` | version, suite, extension, profile, alert, transport-binding identifiers |
| IoT profiles | `spec/iot-profiles.md` | constrained, edge, and research profile limits |
| Security considerations | `spec/security-considerations.md` | replay, downgrade, UKS, DoS, RNG, storage, enrollment, side-channel threats |
| Privacy considerations | `spec/privacy-considerations.md` | pseudonymity, role privacy, lookup behavior, metadata leakage |
| Test-vector specification | `spec/test-vectors.md` | canonical vectors, negative vectors, regeneration rules |
| Implementation requirements | `spec/implementation-requirements.md` | bounded parsing, RNG, storage, constant-time boundaries, failure behavior |
| Interop evidence | `evidence/interop-matrix.md` | Rust/C feature and vector parity status when curated evidence is checked in |

Security- or wire-relevant normative behavior must not exist only in source comments. It belongs in the specification package and should be backed by at least one test, vector, negative vector, or explicit evidence gap.

## Candidate protocol profiles

### ZK-ARCHE-Core

Target: STM32-class and ESP32-S3-class constrained devices interoperating with capable MCU or Linux peers.

Requirements:

- fixed or bounded message buffers;
- no mandatory heap-heavy parser in C hot paths;
- no mandatory anonymous-credential, post-quantum, or general-purpose ZK dependency;
- UDP-safe packet sizing by default;
- strict replay, sequence, and transcript mutation tests;
- deterministic Rust vectors with C validation against the same semantics.

### ZK-ARCHE-Edge

Target: Raspberry Pi-class gateways and Jetson Orin-class edge nodes.

Allowed additions include larger registries, commissioner services, encrypted lookup hints, evidence dashboards, optional secure-channel bindings, and more expensive credential experiments when explicitly benchmarked.

### ZK-ARCHE-Research

Target: cryptographic evaluation, formal modeling, external review, and future suite work.

Allowed additions include post-quantum hybrids, advanced privacy credentials, alternative proof systems, mechanized proofs/model checking, and alternative encodings. Research features must be suite/profile negotiated and cannot become hidden dependencies of the constrained core.

## Native specification shape

The native specification should eventually include sections equivalent to:

1. Introduction
2. Terminology
3. Protocol overview
4. Protocol constants and registries
5. Cryptographic suites
6. Message encoding and canonicalization
7. HELLO negotiation
8. SETUP / enrollment
9. AUTH
10. Late enrollment and commissioner grants
11. Rekey and re-registration
12. Session resumption
13. Transport and secure-channel bindings
14. Error handling and alerts
15. State machines
16. Test vectors and negative vectors
17. Security considerations
18. Privacy considerations
19. Registry policy
20. Implementation guidance for constrained devices

Normative keyword discipline should be used only after behavior is sufficiently specified and testable. Until then, design notes and explicit TODOs are preferable to premature MUST/SHOULD language.

## EDHOC/OSCORE-inspired constrained profile

ZK-ARCHE may borrow constrained-protocol design discipline without claiming to be EDHOC or OSCORE.

Candidate work:

- define compact canonical field ordering;
- evaluate optional CBOR/CDDL descriptions for specification clarity;
- define a security-context export interface;
- support CoAP transport mapping only after message size, retransmission, and replay behavior are measured;
- preserve current Rust/C vector semantics during any encoding research unless a checkpoint-approved versioned migration is introduced.

Any additional parser must receive malformed-input tests, differential tests where applicable, and fuzz coverage. CBOR/COSE dependencies stay optional until their code-size/RAM/CPU cost is acceptable on constrained targets.

## TLS/mTLS-style channel-binding profile

ZK-ARCHE may run as application-layer authentication over TLS or bind its proof transcript to TLS exporter material when TLS already exists.

Candidate binding:

```text
zk_arche_tls_channel_binding = TLS-Exporter("EXPORTER-ZK-ARCHE-v1", transcript_context, 32)
AUTH transcript includes zk_arche_tls_channel_binding
```

Requirements:

- mTLS client certificates must not be described as unlinkable when they expose stable identity;
- TLS termination/proxy assumptions must be explicit;
- server identity, application/ALPN context, suite, deployment/domain, and exporter material are transcript-bound;
- wrong exporter, endpoint, deployment, suite, or application context is rejected;
- TLS remains optional for constrained standalone profiles unless the deployment specifically requires it.

## DTLS-style datagram profile

Native UDP should adopt explicit datagram-security discipline:

- epoch/session identifiers;
- address validation and stateless retry-cookie policy;
- bounded pre-validation amplification;
- replay windows and duplicate suppression;
- retransmission rules for lossy links;
- bounded response caches;
- precise alerts/errors without unnecessary privacy leakage.

Unauthenticated invalid packets should not trigger expensive registry scans or proof verification when retry-cookie policy is enabled. Duplicate, stale, reordered, cross-session, wrong-sequence, and wrong-address packets require deterministic tests.

## Specification-grade assurance

Before stronger maturity language, require evidence for:

- transcript-coverage mutation tests;
- downgrade and cross-suite tests;
- replay/reorder/reflection tests;
- parser fuzzing and malformed/oversized input tests;
- RNG failure handling;
- storage atomicity and recovery behavior;
- constant-time/side-channel review boundaries;
- formal-model traceability to implementation assertions;
- Rust/C deterministic-vector interoperability wherever both lanes implement the feature;
- external cryptographic review for custom proofs and any new credential suites.

## Migration sequence

Recommended sequence:

1. Keep Rust as the canonical deterministic-vector source and C as an independent interoperability lane.
2. Keep parent-level Rust/C CI and release qualification reproducible from a clean checkout.
3. Maintain the `spec/` package for protocol, registries, profiles, security, privacy, vectors, and implementation requirements.
4. Convert existing wire/header/TLV and AUTH/SETUP behavior into testable normative text.
5. Add negative vectors and mutation-test vectors.
6. Define optional TLS-exporter and DTLS-style UDP binding profiles.
7. Add CoAP/OSCORE/EDHOC-inspired constrained-profile research only with target measurements.
8. Add conformance labels only after the relevant Rust/C tests and vectors agree on the claimed behavior.

## Non-goals

- Do not claim ZK-ARCHE is an IETF RFC unless it actually goes through the IETF process.
- Do not make TLS, DTLS, CBOR, COSE, anonymous credentials, post-quantum cryptography, or general-purpose ZK systems mandatory for the constrained core without measured target evidence and review.
- Do not replace canonical vectors casually; use versioned vectors and migration notes for protocol-impacting changes.
- Do not use RFC-style prose to hide missing tests or unresolved security assumptions.
