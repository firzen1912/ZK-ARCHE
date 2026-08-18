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
| R-001 | EDHOC/OSCORE-inspired constrained binding, exporter, and explicit profile-negotiation semantics | benchmark | exact ZK-ARCHE-vs-EDHOC wire/footprint comparison; profile ID/selection and prescriptive-parameter model; transcript binding; downgrade/unknown-profile behavior; interop implications | `docs/roadmaps/rfc-evolution-plan.md` / `spec/` | 2026-08-16 |
| R-002 | Reviewed anonymous/selective-disclosure credential options for role authorization | researching | BBS/property comparison; exact proof/signature bytes; BLS12-381 pairing CPU/RAM/flash/dependency footprint; issuance/revocation model; Rust/C library maturity; extension status; external-review status | research-only or future suite ADR | 2026-08-17 |
| R-003 | Post-quantum hybrid key establishment for edge/gateway profiles | benchmark | full packet budget using 1216-byte MLKEM768-X25519 encapsulation key and 1120-byte ciphertext; RAM/CPU/flash; MTU/fragmentation/loss behavior; protocol-specific hybrid KDF/transcript review; downgrade model | research-only / optional suite roadmap | 2026-08-17 |
| R-004 | Formal verification expansion beyond current symbolic skeletons | reproduce | pinned Tamarin 1.12 environment and dependencies; executable AUTH model; property/assumption matrix; model-to-code traceability; reproducible positive and failed lemma outputs | assurance roadmap / evidence | 2026-08-18 |
| R-005 | STM32/ESP32-class AUTH and P2P benchmark methodology | benchmark | target hardware/build profile; entropy-source state; DRBG/reseed and RNG-failure behavior; eFuse/flash key handling; secure-boot/debug state; acceleration configuration; stack/heap/flash; packet and latency data | IoT profile roadmap / assurance evidence | 2026-08-17 |
| R-006 | Optional remote-attestation evidence bound to authenticated sessions | research-only | threat-model justification, hardware root-of-trust availability, EAT/evidence sizes, freshness/privacy analysis, verifier architecture, MCU/edge RAM/CPU/flash/wire measurements | future optional profile/extension only if justified | 2026-08-16 |
| R-007 | CFRG-informed conformance/security review of the CDS OR-composed role-membership proof | reproduce | mathematical proof contract; simulator/completeness/soundness/ZK assumption matrix; canonical serialization and role ordering; protocol/instance identifiers; CSPRNG and constant-time review; Rust/C differential and negative vectors; independent cryptographic review | TD-001 / assurance / future proof ADR/spec text | 2026-08-17 |
| R-008 | Explicit authentication-vs-authorization context binding and scoped/revocable authorization semantics | researching | authn/authz contract; compact audience/scope/policy/epoch/revocation fields; session/channel binding; privacy analysis; negative vectors; `iot-core` byte/state budget | zk230/zk234/zk239 / future AUTH-TRUST-DATA spec work | 2026-08-18 |
| R-009 | Per-suite AEAD key-usage accounting and exhaustion-triggered rekey behavior | researching | suite-specific usage-limit derivation; sender/receiver counter semantics; warning/hard-stop thresholds; persistence/rollback design; rekey/resumption interaction; MCU flash-wear analysis; threshold negative tests | zk213/zk221/zk230/zk233 / assurance | 2026-08-18 |

Add new items when a research question is concrete enough to state what evidence would change a ZK-ARCHE decision. Do not use this table as a feature wishlist.

## Promotion record

When an item is promoted, retain the row and link the destination so research provenance is preserved. Example:

```text
R-00X → docs/roadmaps/...#phase → docs/adr/NNNN-...md → spec/...md
```

Promotion means “ready for explicit engineering review,” not “automatically accepted.”
