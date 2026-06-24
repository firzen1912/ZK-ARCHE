# ZK-ARCHE Unified Repository

This is the canonical unified ZK-ARCHE workspace. It brings the earlier Rust reference lane, C implementation lane, and comparison material into one repository for protocol development, cross-language validation, and IoT-focused hardening.

## Repository Lineage

These repositories are historical sources for the unified workspace:

| Previous repository | Carried forward here |
|---|---|
| <https://github.com/firzen1912/ZK-ARCHE-Rust.git> | Rust reference implementation, deterministic test vectors, protocol modeling, and validation harnesses. |
| <https://github.com/firzen1912/ZK-ARCHE-C.git> | C11/libsodium implementation for constrained and heterogeneous IoT targets. |
| <https://github.com/firzen1912/zk-arche-compare.git> | Cross-implementation comparison notes and validation planning. |

Use this repository for new ZK-ARCHE work unless a task explicitly needs to inspect historical source lineage.

## Workspace Layout

```text
ZK-ARCHE/
|-- rust/                 Rust reference workspace and test vectors
|-- c/                    C implementation, tests, fuzz harnesses, and headers
|-- docs/                 Shared roadmap, validation, and assurance notes
|-- scripts/              Parent-level validation helpers
`-- evidence/             Generated validation logs, ignored by default
```

## Key Documents

| Document | Purpose |
|---|---|
| `docs/improvement-roadmap.md` | Evidence-gated roadmap for protocol, implementation, and IoT profile work. |
| `docs/cross-language-validation.md` | Rust/C interop anchor and baseline validation commands. |
| `docs/assurance-and-validation.md` | Unified security, hardening, replay, DRBG, review, and local validation guidance. |

## Target Deployment Posture

ZK-ARCHE is being developed for heterogeneous IoT and edge environments, including STM32-class MCUs, ESP32-S3-class devices, Raspberry Pi-class gateways, and Jetson Orin-class edge nodes. Protocol improvements should preserve a low-footprint path for constrained devices and avoid making heavyweight research features mandatory for the core IoT profile.

The controlling roadmap defines which work belongs in deployable IoT profiles versus optional research profiles:

```text
docs/improvement-roadmap.md
```

## Safety and Assurance Posture

Do not treat this repository as production-ready, formally verified, side-channel certified, externally reviewed, or field-ready unless the claim is backed by checked-in evidence.

Cryptographic and protocol changes should go through checkpoint-style review with explicit evidence. This includes changes to setup, auth, replay protection, role-membership proofs, key derivation, packet parsing, RNG, transcript binding, anti-DoS behavior, session resumption, and post-quantum research paths.

## Quick Validation

Run both implementation lanes from the repository root:

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

## Cross-Language Test-Vector Anchor

The Rust implementation owns the checked-in deterministic vectors:

```text
rust/test-vectors/0x0001/
```

Run the C vector harness against that path:

```bash
cd c
make
./build/tests/test_vectors ../rust/test-vectors/0x0001
```
