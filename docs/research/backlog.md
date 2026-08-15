# ZK-ARCHE Research Backlog

This file tracks research questions and external technologies that may affect ZK-ARCHE but are **not yet engineering commitments**.

## Status vocabulary

- `queued` — worth investigating; no sufficient assessment yet.
- `researching` — sources are being reviewed.
- `reproduce` — a claim/result needs local reproduction or independent validation.
- `benchmark` — feasibility depends on measured wire/RAM/CPU/storage/latency evidence.
- `prototype` — bounded experimental implementation is justified.
- `promote` — evidence is sufficient to propose a roadmap/ADR/spec change.
- `research-only` — useful context or experiment, but intentionally outside the baseline.
- `defer` — potentially useful, but current cost/priority/evidence is insufficient.
- `reject` — incompatible, unsafe, redundant, or not valuable for ZK-ARCHE.

## Backlog

| ID | Topic / question | Status | Evidence needed | Potential destination | Last reviewed |
|---|---|---|---|---|---|
| R-001 | EDHOC/OSCORE-inspired constrained binding and exporter semantics | benchmark | exact ZK-ARCHE-vs-EDHOC wire/footprint comparison, transcript/exporter mapping, interop implications | `docs/roadmaps/rfc-evolution-plan.md` / `spec/` | 2026-08-15 |
| R-002 | Reviewed anonymous/selective-disclosure credential options for role authorization | researching | BBS/property comparison, proof size, RAM/CPU/flash, dependency footprint, external-review status | research-only or future suite ADR | 2026-08-15 |
| R-003 | Post-quantum hybrid key establishment for edge/gateway profiles | benchmark | ML-KEM + traditional hybrid wire sizes, RAM/CPU, MTU/fragmentation impact, hybrid-KDF review, downgrade model | research-only / optional suite roadmap | 2026-08-15 |
| R-004 | Formal verification expansion beyond current symbolic skeletons | queued | property list, model/code traceability, reproducible ProVerif/Tamarin results | assurance roadmap / evidence | — |
| R-005 | STM32/ESP32-class AUTH and P2P benchmark methodology | benchmark | target hardware/build profile, acceleration/RNG/key-storage configuration, stack/heap/flash measurement, packet and latency data | IoT profile roadmap / assurance evidence | 2026-08-15 |

Add new items when a research question is concrete enough to state what evidence would change a ZK-ARCHE decision. Do not use this table as a feature wishlist.

## Promotion record

When an item is promoted, retain the row and link the destination so research provenance is preserved. Example:

```text
R-00X → docs/roadmaps/...#phase → docs/adr/NNNN-...md → spec/...md
```

Promotion means “ready for explicit engineering review,” not “automatically accepted.”
