# ZK-ARCHE RFC-Style Protocol Evolution Plan

This document defines how ZK-ARCHE should evolve from a multi-language research implementation into an RFC-like protocol package while preserving deployability on heterogeneous IoT targets.

The goal is not to claim IETF adoption. The goal is to make ZK-ARCHE written, tested, and reviewed with the discipline expected from protocols such as EDHOC, TLS/mTLS, DTLS, and OSCORE.

## Scope and terminology

`DTLS` is the intended comparison target for datagram security. If internal notes mention `DLTS`, treat that as a typo unless a future document defines a different term.

ZK-ARCHE should remain a privacy-preserving device authentication and role-authorization protocol. It should not silently become a generic TLS replacement. Instead, the RFC-style work should specify:

- a native ZK-ARCHE wire protocol profile;
- an optional CoAP/OSCORE-oriented constrained-IoT profile;
- an optional TLS/mTLS or DTLS channel-binding profile for deployments that already require those transports;
- a conformance test suite shared by Rust, C, and Python.

## Standards used as design references

| Reference family | Relevant property to emulate | ZK-ARCHE implication |
|---|---|---|
| EDHOC / CoAP / OSCORE | compact authenticated key exchange, constrained-node orientation, COSE/CBOR discipline, security-context export | define a compact profile and a clean key-export/security-context interface |
| TLS 1.3 / mTLS | transcript binding, downgrade prevention, endpoint authentication, key schedule structure, alert taxonomy | bind all negotiated fields into finished MACs and define strict error/state behavior |
| DTLS 1.3 | datagram robustness, anti-amplification, retransmission, replay windowing, connection identifiers | make UDP behavior explicit rather than transport-ad hoc |
| BRSKI/FDO/Matter-style onboarding | separable commissioning/enrollment phase | keep late enrollment outside normal AUTH and require explicit grants |
| Privacy credentials | unlinkable authorization and selective disclosure | keep anonymous credentials optional until proof size, review, and IoT benchmarks exist |

## RFC-style deliverables

Create these artifacts before claiming that ZK-ARCHE is specification-grade:

| Artifact | Suggested path | Purpose |
|---|---|---|
| Protocol specification | `spec/zk-arche-protocol.md` | normative message flow, field definitions, cryptographic computations, state machines |
| Wire format registry | `spec/registries.md` | version, suite, extension, profile, alert, and transport-binding registries |
| IoT profile specification | `spec/iot-profiles.md` | `iot-core`, `iot-edge`, and `research-only` limits and conformance requirements |
| Security considerations | `spec/security-considerations.md` | replay, downgrade, UKS, DoS, privacy, side-channel, RNG, storage, and enrollment threats |
| Privacy considerations | `spec/privacy-considerations.md` | PID unlinkability, role privacy, lookup hints, credential issuance, metadata leakage |
| Test-vector specification | `spec/test-vectors.md` | deterministic vectors, transcript bytes, negative vectors, and regeneration procedure |
| Interop matrix | `evidence/interop-matrix.md` | Rust/C/Python feature and vector parity status |
| Implementation requirements | `spec/implementation-requirements.md` | constant-time boundaries, bounded parsing, RNG behavior, storage atomicity, error handling |

Do not place normative requirements only in source comments. If a behavior affects security or wire compatibility, it must appear in the specification package and in at least one test-vector or negative-test artifact.

## Candidate protocol profiles

### ZK-ARCHE-Core

Target: STM32-class and ESP32-S3-class constrained clients, with Linux-edge or capable MCU-plus servers where measured.

Requirements:

- fixed or bounded message buffers;
- no mandatory heap-heavy parser in C hot paths;
- no mandatory anonymous credential or post-quantum dependency;
- UDP-safe packet sizing by default;
- strict replay, sequence, and transcript mutation tests;
- deterministic test vectors across Rust, C, and Python where implemented.

### ZK-ARCHE-Edge

Target: Raspberry Pi-class gateways and Jetson Orin-class edge nodes.

Allowed additions:

- large registry benchmarks;
- encrypted lookup hints;
- commissioning services;
- evidence dashboards;
- optional TLS/DTLS channel binding;
- more expensive credential experiments when benchmarked.

### ZK-ARCHE-Research

Target: cryptographic evaluation, formal modeling, external review, and future suite work.

Allowed additions:

- post-quantum hybrid suites;
- BBS/Privacy Pass/DAA-style authorization experiments;
- alternative proof systems;
- mechanized proofs and model checking;
- wire-format alternatives such as CBOR/COSE profiles.

Research features must be explicitly negotiated and must not become hidden dependencies of `ZK-ARCHE-Core`.

## Native protocol specification shape

The native specification should be written in RFC style with sections equivalent to:

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
13. Transport bindings
14. Error handling and alerts
15. State machines
16. Test vectors
17. Security considerations
18. Privacy considerations
19. IANA-style registries or private registry policy
20. Implementation guidance for constrained devices

Normative keyword discipline should be used only after the behavior is testable. Until then, use design notes and TODOs rather than premature MUST/SHOULD language.

## EDHOC-like constrained profile

ZK-ARCHE should learn from EDHOC without becoming EDHOC unless a later checkpoint decides to define a true EDHOC method/profile.

Candidate work:

- define a compact binary profile with canonical field ordering;
- evaluate CBOR/CDDL for a future constrained specification layer;
- define a security-context export function analogous to an OSCORE context exporter;
- support CoAP transport mapping only if message size, retransmission, and replay behavior are measured;
- keep the existing Rust/C/Python vectors as the primary semantic anchor during migration.

Acceptance gates:

- compact profile message sizes are measured for `R=1`, `R=4`, and `R=8` role branches;
- packet sizes remain within configured `iot-core` UDP limits or require explicit fragmentation/TCP;
- COSE/CBOR dependencies are optional until code size and RAM impact are measured on constrained targets;
- any new encoding has differential parser tests and fuzz targets.

## TLS/mTLS-style binding profile

ZK-ARCHE can coexist with TLS/mTLS in two ways:

1. ZK-ARCHE as application-layer authentication over a TLS channel.
2. ZK-ARCHE as an additional privacy-preserving device-auth proof bound to a TLS exporter value.

The second option is preferred when TLS is already present but certificate-based client identity is too linkable.

Candidate binding:

```text
zk_arche_tls_channel_binding = TLS-Exporter("EXPORTER-ZK-ARCHE-v1", transcript_context, 32)
AUTH transcript includes zk_arche_tls_channel_binding
```

Policy:

- do not treat mTLS client certificates as privacy preserving;
- do not let TLS termination proxies silently break ZK-ARCHE peer authentication;
- define whether the ZK-ARCHE peer is the TLS endpoint or an application identity behind it;
- bind server identity, ALPN/application profile, suite id, and deployment id into the ZK-ARCHE transcript.

Acceptance gates:

- channel-binding mutation tests exist;
- wrong TLS endpoint, wrong deployment id, wrong ALPN/profile, and wrong exporter value are rejected;
- mTLS mode documents metadata leakage from client certificates;
- TLS use is optional for `iot-core` unless the target deployment explicitly provides it.

## DTLS-style datagram profile

The native UDP profile should adopt DTLS-style discipline:

- explicit epoch/session identifiers;
- anti-amplification policy before address validation;
- stateless retry cookie support;
- replay windows and duplicate suppression;
- retransmission behavior for lossy links;
- bounded response cache behavior;
- explicit alert/error mapping without leaking unnecessary private state.

Acceptance gates:

- unauthenticated invalid packets do not trigger registry scans or proof verification after retry-cookie mode is enabled;
- amplification before source validation is bounded by policy;
- duplicate, stale, reordered, cross-session, and wrong-sequence packets are rejected or handled exactly as specified;
- response-cache memory use is bounded under attack.

## Specification-grade security assurance

Before using RFC-like language in release notes, require evidence for:

- transcript coverage mutation tests;
- downgrade and cross-suite tests;
- replay and reorder negative tests;
- parser fuzzing and differential parser tests;
- malformed and oversized input tests;
- RNG failure handling;
- storage atomicity and recovery tests;
- side-channel review checklist for constant-time code boundaries;
- formal model traceability to implementation assertions;
- Rust/C/Python vector parity where implemented;
- external cryptographic review for custom proofs and new credential suites.

## Python lane role

The Python implementation is now part of the unified repository as `python/`.

It should be used for:

- readable reference behavior;
- quick protocol experiments;
- CLI-level demos;
- deterministic vector validation;
- negative-test prototyping;
- specification examples.

It should not be used as the constrained-device target. `iot-core` feasibility is governed by the C lane and target measurements. Python can still block semantic changes by failing vector or state-machine tests.

## Migration sequence

Recommended order:

1. Integrate Python as a third implementation lane.
2. Add parent-level Python CI and evidence logging.
3. Create `spec/` skeleton with protocol, registries, profiles, security, privacy, and vectors documents.
4. Convert existing wire/header/TLV and AUTH/SETUP behavior into normative spec text.
5. Add negative vectors and mutation-test vectors.
6. Add TLS-exporter and DTLS-style UDP binding design notes as optional profiles.
7. Add CoAP/OSCORE/EDHOC-inspired constrained profile research notes.
8. Add conformance labels only after Rust/C/Python tests agree on the claimed behavior.

## Non-goals

- Do not claim ZK-ARCHE is an IETF RFC unless it actually goes through the IETF process.
- Do not make TLS, DTLS, CBOR, COSE, anonymous credentials, or post-quantum cryptography mandatory for the constrained core profile without measured target evidence.
- Do not replace the current test vectors casually; add versioned vectors and migration notes.
- Do not use RFC-style prose to hide missing tests or unresolved security assumptions.
