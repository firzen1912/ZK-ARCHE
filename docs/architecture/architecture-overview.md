# ZK-ARCHE Architecture Overview

## Architecture

ZK-ARCHE is organized as a protocol/specification project with two implementation lanes and explicit separation between research input, planned work, normative behavior, implementation, and assurance evidence.

## Repository map

```text
ZK-ARCHE/
├── rust/                      Rust reference implementation
│   └── test-vectors/0x0001/  canonical deterministic vectors
├── c/                         C11/libsodium constrained implementation
├── spec/                      normative protocol staging package
├── docs/
│   ├── architecture/          stable architecture and ownership boundaries
│   ├── research/              research intake and candidate ideas
│   ├── roadmaps/              planned improvement and standards evolution
│   ├── assurance/             security and interoperability evidence policy
│   ├── adr/                   point-in-time architecture/protocol decisions
│   ├── release/               maturity and claim governance
│   └── technical-debt/        unresolved gaps
├── scripts/                   validation and release-qualification wrappers
└── evidence/                  generated or curated evidence when present
```

The Git tree is the authoritative exact file inventory. This overview documents stable ownership boundaries rather than duplicating every file path.

## Authority hierarchy

When documents disagree, use this precedence for protocol behavior:

1. versioned normative material and registries under `spec/`;
2. canonical deterministic test-vector semantics under `rust/test-vectors/` where the spec is still incomplete;
3. Rust and C interoperability tests and implementation behavior;
4. accepted ADRs explaining deliberate decisions;
5. roadmaps describing intended future work;
6. research reports describing external evidence or candidate directions.

Research and roadmap prose must never silently override implemented or specified protocol semantics.

## Implementation lanes

### Rust reference lane

`rust/` is the reference implementation and canonical deterministic-vector source. It is responsible for reproducible protocol semantics, higher-level tests, fuzz targets, and vector generation.

### C constrained lane

`c/` is an independent C11/libsodium implementation intended to preserve a realistic path toward constrained and heterogeneous IoT targets. Its interoperability against the Rust vector corpus is a release-relevant assurance signal.

Neither lane alone proves protocol security. Agreement demonstrates interoperability for covered cases, not formal correctness or production readiness.

## Protocol and specification boundary

`spec/` owns normative protocol structure: message grammar, registries, profiles, state machines, security/privacy considerations, implementation requirements, and vector rules.

`docs/architecture/` explains where components and responsibilities belong. `docs/roadmaps/` may propose future behavior, but a roadmap item is not normative until the applicable spec/ADR/test work is accepted.

## Constrained-device boundary

The mandatory IoT path must remain feasible for STM32/ESP32-S3-class devices. Gateway- or research-grade features may exist, but they must be profile/suite isolated and must not become hidden dependencies of constrained authentication.

High-end Raspberry Pi-, Jetson-, workstation-, or server-class peers should negotiate down to the constrained interoperability floor where required.

## Interoperability invariants

Protocol-impacting changes should preserve or deliberately version:

- packet and TLV encodings;
- transcript construction and domain separation;
- suite/profile identifiers;
- proof and key-confirmation semantics;
- deterministic test-vector meanings;
- replay and state-machine behavior;
- Rust/C byte-level compatibility wherever both lanes implement the behavior.

A deliberate break requires an ADR, versioned specification/vector changes, migration notes, and appropriate review.

## Documentation ownership

```text
research    = what external evidence suggests
roadmaps    = what ZK-ARCHE plans to investigate/build
ADRs        = why a consequential decision was made
spec        = what conforming protocol behavior means
code/tests  = what implementations currently do
assurance   = what evidence exists and what claims remain blocked
release     = what maturity language is allowed
```

This separation is intentional. It keeps future research useful without allowing research enthusiasm to bypass protocol review or evidence gates.
