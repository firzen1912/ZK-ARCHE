# Hermes Legion Commander Usage for ZK-ARCHE

This repository is security-sensitive. Use Commander modes conservatively.

## Build repo graph

```powershell
$CommanderExe = "$env:LOCALAPPDATA\HermesLegionCommander\venv\Scripts\hermes-legion-commander.exe"
$Repo = "C:\Users\firze\OneDrive\Documents\GitHub\ZK-ARCHE"

& $CommanderExe repo-graph build `
  $Repo `
  --out "$Repo\shared-context\repo-map" `
  --task "ZK-ARCHE Rust/C roadmap and validation"
```

## Alternating mode for low-risk tasks

```powershell
& $CommanderExe council `
  --config .\config\model_council.zk-arche.local.toml `
  campaign `
  --from-version 201 `
  --to-version 202 `
  --strategy alternating `
  --run-id "zk-arche-v201-v202-alternating"
```

## Checkpoint competition for security-sensitive tasks

```powershell
& $CommanderExe checkpoint `
  --config .\config\checkpoint_competition.zk-arche.local.toml `
  --repo $Repo `
  run `
  --from-version 203 `
  --to-version 203
```

## Prompt guardrail

```text
Treat docs/improvement-roadmap.md as the controlling roadmap. Do not claim production readiness, formal verification, side-channel resistance, replay-resistance completeness, external review completion, certification, or IoT field readiness unless checked-in evidence supports the claim. Use checkpoint competition for protocol, crypto, wire parsing, replay, RNG, memory-safety, and Rust/C interop changes.
```
