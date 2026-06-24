# ZK-ARCHE Unified Repository

This repository is the latest unified iteration of ZK-ARCHE. It consolidates the previously separate Rust, C, and comparison workspaces into one Commander-ready repository for protocol development, cross-language validation, and IoT-focused hardening.

## Repository lineage

This unified workspace carries forward the work from these earlier repositories:

| Previous repository | Role in this unified iteration |
|---|---|
| <https://github.com/firzen1912/ZK-ARCHE-Rust.git> | Rust reference implementation, deterministic test vectors, protocol modeling, and higher-level validation. |
| <https://github.com/firzen1912/ZK-ARCHE-C.git> | Low-level C implementation intended for constrained and heterogeneous IoT targets. |
| <https://github.com/firzen1912/zk-arche-compare.git> | Cross-implementation comparison, interop notes, and validation planning. |

Use this repository as the canonical place for new ZK-ARCHE work unless a task explicitly targets one of the historical repositories. The older repositories remain useful for source lineage, audit trail, and implementation history, but the roadmap, shared protocol notes, and future hardening work should live here.

## Workspace layout

The repository is intentionally arranged as preserved implementation lanes plus shared validation and planning material:

```text
ZK-ARCHE/
├── rust/                 # Rust reference workspace and test vectors
├── c/                    # C implementation, tests, fuzz harnesses, libsodium build
├── docs/                 # Combined roadmap and cross-language validation notes
├── scripts/              # Parent-level validation helpers
├── evidence/             # Parent-level validation logs
└── shared-context/       # Hermes Legion Commander repo graph / prompt context outputs
```

## Target deployment posture

ZK-ARCHE is being developed for heterogeneous IoT and edge environments, including STM32-class MCUs, ESP32-S3-class devices, Raspberry Pi-class gateways, and Jetson Orin-class edge nodes. Protocol improvements should preserve a low-footprint path for constrained devices and avoid making heavyweight research features mandatory for the core IoT profile.

The controlling roadmap defines which work belongs in the deployable IoT profiles versus optional research profiles:

```text
docs/improvement-roadmap.md
```

## Safety and assurance posture

Do not treat this repository as production-ready, formally verified, side-channel certified, externally reviewed, or field-ready unless the claim is backed by checked-in evidence.

Cryptographic/protocol changes should go through Hermes Legion Commander checkpoint competition, not alternating-only mode. This includes changes to setup, auth, replay protection, role-membership proofs, key derivation, packet parsing, RNG, transcript binding, anti-DoS behavior, session resumption, and post-quantum research paths.

## Quick validation

Run both implementation lanes from the parent repository:

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

## Hermes Legion Commander usage

Build repo graph context first:

```powershell
$CommanderExe = "$env:LOCALAPPDATA\HermesLegionCommander\venv\Scripts\hermes-legion-commander.exe"
$Repo = "C:\Users\firze\OneDrive\Documents\GitHub\ZK-ARCHE"

& $CommanderExe repo-graph build `
  $Repo `
  --out "$Repo\shared-context\repo-map" `
  --task "ZK-ARCHE unified Rust/C baseline validation and security roadmap"
```

Use alternating mode for low-risk validation infrastructure and documentation. Use checkpoint competition for protocol, crypto, parsing, replay, RNG, side-channel, memory-safety, and interop changes.
