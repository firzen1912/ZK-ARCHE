# ZK-ARCHE Unified Repository

This repository is the latest unified iteration of ZK-ARCHE. It consolidates the previously separate Rust, C, Python, and comparison workspaces into one repository for protocol development, cross-language validation, and IoT-focused hardening.

## Repository lineage

This unified workspace carries forward the work from these earlier repositories:

| Previous repository | Role in this unified iteration |
|---|---|
| <https://github.com/firzen1912/ZK-ARCHE-Rust.git> | Rust reference implementation, deterministic test vectors, protocol modeling, and higher-level validation. |
| <https://github.com/firzen1912/ZK-ARCHE-C.git> | Low-level C implementation intended for constrained and heterogeneous IoT targets. |
| Local Python lane | Python reference implementation, CLI harness, and Rust-vector validation used for readable protocol experimentation. |
| <https://github.com/firzen1912/zk-arche-compare.git> | Cross-implementation comparison, interop notes, and validation planning. |

Use this repository as the canonical place for new ZK-ARCHE work unless a task explicitly targets one of the historical repositories. The older repositories remain useful for source lineage, audit trail, and implementation history, but the roadmap, shared protocol notes, and future hardening work should live here.

## Workspace layout

The repository is intentionally arranged as preserved implementation lanes plus shared validation and planning material:

```text
ZK-ARCHE/
├── rust/                 # Rust reference workspace and canonical test vectors
├── c/                    # C implementation, tests, fuzz harnesses, libsodium build
├── python/               # Python reference implementation, CLI, and vector tests
├── docs/                 # Combined roadmap, RFC-evolution plan, and validation notes
├── spec/                 # RFC-style specification skeleton and registries
├── scripts/              # Parent-level validation helpers
└── evidence/             # Parent-level validation logs
```

## Target deployment posture

ZK-ARCHE is being developed for heterogeneous IoT and edge environments, including STM32-class MCUs, ESP32-S3-class devices, Raspberry Pi-class gateways, and Jetson Orin-class edge nodes. Protocol improvements should preserve a low-footprint path for constrained devices and avoid making heavyweight research features mandatory for the core IoT profile.

The controlling roadmap defines which work belongs in the deployable IoT profiles versus optional research profiles:

```text
docs/improvement-roadmap.md
```

## RFC-style protocol evolution

ZK-ARCHE should evolve toward an RFC-like protocol package rather than only implementation-specific documentation. The standards-track work should define normative message grammar, transcript construction, suite registries, profile requirements, state machines, security considerations, privacy considerations, IANA-style registries, and implementation conformance tests.

The comparison targets are EDHOC-style compact authenticated key exchange for constrained IoT, TLS/mTLS-style transcript and endpoint-authentication rigor, and DTLS-style datagram robustness. This does not mean copying TLS or DTLS; it means adopting their specification discipline, downgrade resistance, alert/error taxonomy, extension negotiation, and interop-test culture. See:

```text
docs/rfc-evolution-plan.md
spec/
```

## Safety and assurance posture

Do not treat this repository as production-ready, formally verified, side-channel certified, externally reviewed, or field-ready unless the claim is backed by checked-in evidence.

Cryptographic/protocol changes should go through checkpoint-style review with explicit evidence. This includes changes to setup, auth, replay protection, role-membership proofs, key derivation, packet parsing, RNG, transcript binding, anti-DoS behavior, session resumption, and post-quantum research paths.

Consolidated security, hardening, replay, DRBG, external-review, and local-validation guidance lives in:

```text
docs/assurance-and-validation.md
```

## Quick validation

Run all implementation lanes from the parent repository:

```bash
./scripts/ci-all.sh
```

Run only Rust:

```bash
./scripts/ci-rust.sh
```

Run only C:

```bash
./scripts/ci-c.sh
```

Run only Python:

```bash
./scripts/ci-python.sh
```

## Cross-language test-vector anchor

The Rust implementation owns the checked-in deterministic vectors at:

```text
rust/test-vectors/0x0001/
```

The C vector harness should be run against that path from the C directory:

```bash
cd c
make
./build/tests/test_vectors ../rust/test-vectors/0x0001
```

The Python lane carries a mirrored vector fixture under `python/test-vectors/0x0001/` and should remain byte-compatible with the Rust vector semantics. Prefer adding a sync/check script before changing vector meanings.
