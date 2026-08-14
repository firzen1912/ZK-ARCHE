# ZK-ARCHE Unified Repository

ZK-ARCHE is a privacy-preserving authentication, authorization, and secure-session research framework for heterogeneous IoT and edge systems. This unified repository contains the Rust reference implementation, the C constrained-device implementation, shared protocol specifications, validation tooling, and evidence-gated improvement planning.

## Repository lineage

This workspace carries forward work from:

| Previous repository | Role in this unified iteration |
|---|---|
| <https://github.com/firzen1912/ZK-ARCHE-Rust.git> | Rust reference implementation, deterministic test vectors, protocol modeling, and higher-level validation. |
| <https://github.com/firzen1912/ZK-ARCHE-C.git> | Low-level C implementation for constrained and heterogeneous IoT targets. |
| <https://github.com/firzen1912/zk-arche-compare.git> | Cross-implementation comparison, interoperability notes, and validation planning. |

Use this repository as the canonical location for new ZK-ARCHE work unless a task explicitly targets a historical repository.

## Repository map

```text
ZK-ARCHE/
├── README.md                  # Project overview and entry point
├── ROADMAP.md                 # Short roadmap pointer / current focus
├── rust/                      # Rust reference implementation + canonical vectors
├── c/                         # C constrained-device implementation
├── spec/                      # Normative protocol/specification package
├── docs/
│   ├── README.md              # Documentation index and ownership map
│   ├── architecture/          # Architecture and repository boundaries
│   ├── research/              # External research intake, backlog, and daily reports
│   ├── roadmaps/              # Canonical improvement and standards-evolution plans
│   ├── assurance/             # Security, validation, interoperability, evidence policy
│   ├── adr/                   # Architecture Decision Records
│   ├── release/               # Maturity, claim, and release-governance rules
│   └── technical-debt/        # Explicit unresolved protocol/implementation/assurance debt
├── scripts/                   # Parent-level validation helpers
└── evidence/                  # Generated/curated validation evidence when present
```

Start with the [documentation index](docs/README.md) for navigation and the [architecture overview](docs/architecture/architecture-overview.md) for ownership boundaries.

## Documentation workflow

ZK-ARCHE separates discovery, planning, decisions, normative protocol requirements, implementation, and evidence so that research cannot silently become protocol behavior:

```text
external research
      ↓
docs/research/
      ↓ explicit promotion
docs/roadmaps/
      ↓ architecture/security decision when required
docs/adr/
      ↓ normative protocol requirement
spec/
      ↓ implementation
rust/ + c/
      ↓ validation
docs/assurance/ + evidence/
      ↓ evidence-gated claim
docs/release/
```

The [research archive](docs/research/README.md) is the intake point for future literature, standards, implementation, and cryptographic research. The [improvement roadmap](docs/roadmaps/improvement-roadmap.md) remains the canonical engineering plan.

## Target deployment posture

ZK-ARCHE is developed for heterogeneous IoT and edge environments including STM32-class MCUs, ESP32-S3-class devices, Raspberry Pi-class gateways, and Jetson Orin-class edge nodes. Protocol improvements should preserve a low-footprint constrained path and must not make heavyweight research features mandatory for the core IoT profile without measurements and review.

## Specification posture

The `spec/` directory is the normative staging area for ZK-ARCHE protocol behavior. It should evolve toward an RFC-like package with explicit message grammar, transcript construction, registries, profiles, state machines, security/privacy considerations, and conformance vectors.

EDHOC/OSCORE, TLS/mTLS, and DTLS are engineering references and optional binding targets; they are not claims that ZK-ARCHE replaces those protocols or is an IETF standard. See the [RFC-style evolution plan](docs/roadmaps/rfc-evolution-plan.md).

## Safety and assurance posture

Do not describe ZK-ARCHE as production-ready, formally verified, side-channel certified, externally reviewed, or field-ready unless the corresponding evidence is checked in and the applicable release gate permits that claim.

Cryptographic or protocol changes require evidence appropriate to their risk, especially changes to enrollment, authentication, replay protection, role-membership proofs, key derivation, parsers, RNG, transcript binding, anti-DoS behavior, resumption, transport/channel binding, or cryptographic suites.

See [assurance and validation](docs/assurance/assurance-and-validation.md), [cross-language validation](docs/assurance/cross-language-validation.md), and [release governance](docs/release/release-governance.md).

## Quick validation

Run both implementation lanes from the repository root:

```bash
./scripts/ci-all.sh
```

Rust only:

```bash
./scripts/ci-rust.sh
```

C only:

```bash
./scripts/ci-c.sh
```

Release qualification:

```bash
./scripts/ci-release-qualification.sh
```

## Cross-language test-vector anchor

Rust owns the canonical checked-in deterministic vectors:

```text
rust/test-vectors/0x0001/
```

The C implementation validates against those vectors:

```bash
cd c
make
./build/tests/test_vectors ../rust/test-vectors/0x0001
```

Rust remains the canonical vector source and C remains the independent constrained implementation used to test byte-level interoperability.
