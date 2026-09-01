# ZK-ARCHE

ZK-ARCHE is a privacy-preserving authentication, authorization, and secure-association research framework for heterogeneous IoT and edge systems. The project is converging on a **Common Contract Architecture**: the least-capable conformant peer and a higher-capability peer should be able to mutually authenticate under the same mandatory security floor without requiring a CA, cloud identity provider, central registry lookup, gateway approval, DNS, Internet connectivity, blockchain, or other always-online external infrastructure.

The repository contains two active implementation lanes:

- **Rust** — reference implementation, canonical deterministic vectors, higher-level validation, fuzzing, and formal-model integration.
- **C11/libsodium** — independent constrained-device implementation used to prove that the protocol contract can remain portable and byte/decision compatible on smaller systems.

ZK-ARCHE is **not production-ready and not an IETF standard**. Security, formal-analysis, constrained-target, interoperability, and release claims remain evidence-gated.

## Current development snapshot

The current `dev` branch has advanced well beyond the earlier unified-repository baseline. As of 2026-08-31, the implementation/specification state includes:

| Area | Current state |
|---|---|
| Stable compatibility baseline | Existing v2 wire behavior remains the compatibility baseline. |
| AUTH v3 | A non-advertised candidate is specified and implemented far enough for deterministic Rust/C reference primitives, canonical context encoding/parsing, reflection resistance, and `iot-core` authorization/attribution testing. It is **not promoted for production selection**. |
| Authentication context | Canonical AUTH v3 context encoding, bounded parsers, negative corpora, allocation/resource checks, and cross-language decision tests are present. |
| Replay lifecycle | Replay cache behavior, replay guards, replay continuity, epoch recovery, and restart/rollback-oriented lifecycle contracts are implemented/tested across the active lanes where applicable. |
| Error handling | Wire-error normalization and registry parity checks now fail closed on unknown or drifting error mappings rather than silently accepting inconsistent behavior. |
| Trust mutation | Normal AUTH remains **NO-LEARNING**. Trust mutation is separated from authentication and is being modeled through explicit lifecycle operations rather than implicit authentication side effects. |
| LINEAGE_REPLACE | The repository now carries a bounded replacement/recovery state machine with contracts and Rust/C coverage for attempt binding, freshness, possession, authorization, bound auth context, session binding, storage capability/transactions, recovery, reconciliation, convergence, provenance, and verified commit ordering. |
| Cross-language conformance | Shared deterministic corpora/vectors cover AUTH v3, profile behavior, replay continuity, lineage replacement, and wire/error semantics across Rust and C. |
| Formal assurance | Synchronized ProVerif models exist for AUTH v3, replay continuity, and LINEAGE_REPLACE commit ordering. The formal qualification gate binds evidence to the exact repository HEAD, model blob, tool version, query count, and retained output and fails closed when prerequisites are unavailable. |
| Specification maturity | `spec/` is now an active draft normative package rather than only a skeleton, with AUTH v3, profile, replay, privacy/security, registry, and lineage-replacement contracts under review. |
| Research/governance | Daily research, weekly findings, weekly requests, roadmaps, ADRs, assurance checkpoints, technical debt, and release governance are separated so external research cannot silently become protocol behavior. |

The strongest remaining evidence ceilings still include independent review of the custom role-membership proof, reproducible STM32/ESP32-S3-class measurements, complete model-to-code formal traceability, and RFC-class normative completeness. See the [improvement roadmap](docs/roadmaps/improvement-roadmap.md) and [technical debt register](docs/technical-debt/README.md).

## Common Contract doctrine

ZK-ARCHE follows a bottom-up interoperability rule:

> **Scale functionality upward from the least-capable conformant peer; do not scale security downward from the most-capable peer.**

A constrained peer must still make its mandatory authentication decision locally. A gateway-class peer may contribute richer policy, indexing, telemetry, storage, audit, acceleration, or optional privacy features, but those capabilities must not become hidden prerequisites for core peer authentication.

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

This is an engineering doctrine, not a claim that ZK-ARCHE replaces those protocols or depends on Reticulum/WireGuard.

## Repository map

```text
ZK-ARCHE/
├── README.md                  # Project overview and current implementation snapshot
├── ROADMAP.md                 # Short roadmap entry point
├── rust/                      # Rust reference implementation + canonical vectors
├── c/                         # C constrained-device implementation
├── spec/                      # Draft normative protocol/specification package
├── docs/
│   ├── README.md              # Documentation index and ownership map
│   ├── architecture/          # Stable architecture and repository boundaries
│   ├── research/              # External research intake, backlog, and daily reports
│   ├── findings/              # Human-reviewed repository/research synthesis
│   ├── requests/              # Explicit human-authorized execution work
│   ├── roadmaps/              # Canonical improvement and RFC-evolution plans
│   ├── assurance/             # Security, formal, interoperability, and evidence checkpoints
│   ├── adr/                   # Architecture Decision Records
│   ├── release/               # Maturity, claim, and release-governance rules
│   └── technical-debt/        # Durable unresolved protocol/implementation/assurance gaps
├── scripts/                   # Validation, release-qualification, and model-sync helpers
└── evidence/                  # Generated/curated validation evidence when present
```

Start with the [documentation index](docs/README.md), the [architecture overview](docs/architecture/architecture-overview.md), the [root roadmap](ROADMAP.md), and the [draft normative protocol](spec/zk-arche-protocol.md).

## Documentation and change-control flow

ZK-ARCHE separates discovery, intent, normative behavior, implementation, and evidence:

```text
external research / hypothesis
        ↓
docs/research/
        ↓ human-reviewed synthesis
docs/findings/
        ↓ explicit authorized work
docs/requests/
        ↓ long-horizon sequencing / consequential decision
docs/roadmaps/ + docs/adr/
        ↓ normative behavior
spec/
        ↓ implementation
rust/ + c/ + deterministic vectors/tests
        ↓ validation
docs/assurance/ + retained evidence
        ↓ evidence-gated claim
docs/release/
```

Research is input, not authority. A paper, RFC, repository, benchmark, or prototype does not become ZK-ARCHE behavior until it passes the appropriate promotion and review boundary.

## Protocol posture

The `spec/` directory is the normative staging area for ZK-ARCHE protocol behavior. The target is RFC-class engineering quality with explicit message grammar, transcript construction, registries, profiles, state machines, error semantics, lifecycle behavior, security/privacy considerations, and conformance vectors.

Important current boundaries:

- Existing **v2** wire behavior remains the compatibility baseline.
- **AUTH v3** is a candidate and MUST NOT be advertised/selected for production use until its promotion gates are satisfied.
- Normal **AUTH is NO-LEARNING**: successful authentication does not itself create or expand trust.
- Authentication, authorization, and trust mutation are distinct operations.
- `iot-core` / future constrained P2P profiles are security floors, not permission to negotiate weaker mandatory authentication properties.
- Rust and C must remain byte- and decision-compatible wherever both claim the same behavior.

See [`spec/README.md`](spec/README.md), [`spec/zk-arche-protocol.md`](spec/zk-arche-protocol.md), and [`spec/security-considerations.md`](spec/security-considerations.md).

## Target deployment posture

Representative targets include STM32-class and Nordic-class MCUs, ESP32/ESP32-S3-class devices, Raspberry Pi-class gateways, Jetson-class edge nodes, industrial controllers, robotics companion computers, sensors, actuators, UAVs, UGVs, and x86/ARM64 hosts.

A new platform should normally require a platform/transport adapter, build profile, cryptographic backend integration, storage/RNG integration, and conformance evidence—not a new ZK-ARCHE protocol variant.

## Validation

Run both active implementation lanes from the repository root:

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

Formal qualification for the currently governed ProVerif models:

```bash
./scripts/ci-formal.sh
```

Full release qualification:

```bash
./scripts/ci-release-qualification.sh
```

Enable the tracked local quality gates once per clone:

```bash
make hooks
```

The pre-commit hook runs fast formatting and contract-consistency checks. The pre-push hook runs Rust/C/conformance qualification for pushes to `dev`, and the complete fail-closed release qualification (including ProVerif 2.05 and `cargo-audit`) for pushes to `main`. Installation copies the hooks into the clone's Git directory so they remain active when switching between `dev`, `main`, and topic branches; rerun `make hooks` after hook updates.

The hosted formal lane builds the CLI-only ProVerif 2.05 release from its checksum-pinned official source archive. A local machine with OCaml installed can create the same verifier under a temporary directory with `./scripts/install-proverif.sh <destination>` and add `<destination>/bin` to `PATH` before main qualification.

The formal lane intentionally fails closed when required tooling or exact-head provenance is unavailable. An unavailable or synthetic run is not a formal PASS.

## Cross-language vectors and corpora

Rust remains the canonical deterministic-vector source. The long-standing v2 anchor is:

```text
rust/test-vectors/0x0001/
```

Newer corpora cover AUTH v3 context/reference behavior, `iot-core` authorization and attribution, replay continuity, LINEAGE_REPLACE state/recovery semantics, profiles, and wire-error normalization. C independently consumes the shared behavior to detect byte-level and decision-level drift.

For the legacy vector anchor:

```bash
cd c
make
./build/tests/test_vectors ../rust/test-vectors/0x0001
```

## Branch and CI policy

- `dev` is the rapid development/integration branch.
- GitHub Actions workflows are intentionally absent from `dev` to conserve Actions usage during rapid prototyping.
- Local validation and evidence scripts remain available on `dev` and should be used before promoting stable work.
- `main` is the stable branch and retains the GitHub Actions CI/release-qualification workflow.
- Pull requests targeting `main` and pushes to `main` must pass the aggregate `CI complete — required capability gate` check before merge/release.

The absence of a GitHub Actions run on `dev` is therefore **not** a qualification failure by itself. Stable promotion to `main` is expected to restore/run the main-branch CI gates.

## Assurance and claim boundaries

Do not describe ZK-ARCHE as production-ready, formally verified, side-channel certified, independently cryptographically reviewed, RFC-complete, constrained-target validated, or field-ready unless the corresponding retained evidence and release gate permit that exact claim.

Cryptographic or protocol changes require evidence appropriate to their risk, especially changes to enrollment, authentication, replay protection, role-membership proofs, key derivation, parsers, entropy/RNG behavior, transcript/context binding, anti-DoS behavior, resumption, revocation, trust mutation, storage rollback, transport/channel binding, or cryptographic suites.

Key references:

- [Assurance and validation](docs/assurance/assurance-and-validation.md)
- [Formal model contract](docs/assurance/formal-model-contract.md)
- [Cross-language validation](docs/assurance/cross-language-validation.md)
- [Release governance](docs/release/release-governance.md)
- [Improvement roadmap](docs/roadmaps/improvement-roadmap.md)
- [RFC-style evolution plan](docs/roadmaps/rfc-evolution-plan.md)
