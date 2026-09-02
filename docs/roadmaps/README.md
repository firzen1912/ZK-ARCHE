# ZK-ARCHE Roadmaps

This directory contains the canonical long-horizon engineering roadmaps for ZK-ARCHE:

- [`improvement-roadmap.md`](./improvement-roadmap.md) — canonical phase plan (`zk201`–`zk241`) for implementation, assurance, constrained-device viability, lifecycle security, interoperability, data sovereignty, and infrastructure-independent P2P operation.
- [`rfc-evolution-plan.md`](./rfc-evolution-plan.md) — RFC-class specification, interoperability, security-analysis, registry, change-control, and standards-reference discipline.

The two documents are complementary. The improvement roadmap owns the canonical phase sequence and exit evidence; the RFC evolution plan sharpens the specification-quality gates that apply to those phases.

## Roadmap progress

> **Current evidence-based completion: 42.07%**<br>
> **Remaining to full roadmap: 57.93%**<br>
> Progress snapshot reviewed: **2026-09-02**<br>
> Evidence basis: exact `dev` at `fa0a1770969e8deb341a1bffd69d11eff9d3a813`, which passed repository release qualification with the local ProVerif 2.05 lane.

`42.07%` is a roadmap-tracking metric, **not** a claim that ZK-ARCHE is 42.07% secure, production-ready, RFC-standardized, externally reviewed, or deployment-qualified.

The score is the arithmetic mean of the evidence-completion scores for canonical phases `zk201`–`zk241`. Each phase is evaluated only against its declared exit evidence.

| Roadmap group | Phases | Completion |
|---|---:|---:|
| Baseline / reproducible truth | `zk201`–`zk205` | **85.00%** |
| Assurance / review / claim gate | `zk206`–`zk210` | **55.00%** |
| Enrollment / authorization / lifecycle | `zk211`–`zk215` | **35.00%** |
| AUTH hardening / IoT profiles | `zk216`–`zk224` | **33.33%** |
| Interop / RFC / transport / decomposition | `zk225`–`zk230` | **45.83%** |
| Data sovereignty | `zk231`–`zk238` | **21.88%** |
| Infrastructure-independent P2P Common Contract | `zk239`–`zk241` | **33.33%** |
| **Overall** | `zk201`–`zk241` | **42.07%** |

### Scoring rubric

Each phase is scored using the same evidence rubric:

| Score | Meaning |
|---:|---|
| **0%** | Roadmap intent only; no material exit evidence exists. |
| **25%** | Design/specification or initial implementation/evidence exists, but major required surfaces are absent. |
| **50%** | Material implementation plus some required tests/evidence exists; important exit criteria remain incomplete. |
| **75%** | Substantially implemented; most exit evidence exists, with bounded qualification/evidence gaps remaining. |
| **100%** | The phase's declared exit evidence actually exists and repository claim language matches that evidence. |

Scores are **not** based on commit count, lines of code, document count, elapsed time, or implementation effort. A phase does not reach 100% merely because code exists.

Required independent review, physical-target measurements, formal results, interoperability evidence, RFC-class documentation, field evidence, or other declared exit artifacts must actually exist before the associated phase receives credit for them.

### Phase scorecard

This scorecard makes the grouped arithmetic reproducible. Each phase is scored against the declared exit evidence in [`improvement-roadmap.md`](./improvement-roadmap.md), not against implementation effort. The 41 phase scores sum to `1725`; `1725 / 41 = 42.07%`.

| Roadmap group | Phase scores |
|---|---|
| Baseline / reproducible truth | `zk201=100`, `zk202=100`, `zk203=75`, `zk204=75`, `zk205=75` |
| Assurance / review / claim gate | `zk206=75`, `zk207=50`, `zk208=25`, `zk209=25`, `zk210=100` |
| Enrollment / authorization / lifecycle | `zk211=25`, `zk212=25`, `zk213=75`, `zk214=50`, `zk215=0` |
| AUTH hardening / IoT profiles | `zk216=25`, `zk217=75`, `zk218=75`, `zk219=25`, `zk220=0`, `zk221=50`, `zk222=50`, `zk223=0`, `zk224=0` |
| Interop / RFC / transport / decomposition | `zk225=75`, `zk226=50`, `zk227=25`, `zk228=50`, `zk229=25`, `zk230=50` |
| Data sovereignty | `zk231=50`, `zk232=0`, `zk233=25`, `zk234=50`, `zk235=0`, `zk236=25`, `zk237=25`, `zk238=0` |
| Infrastructure-independent P2P Common Contract | `zk239=50`, `zk240=25`, `zk241=25` |

The 2026-09-01 decision checkpoints justify the threshold increases for lifecycle, DATA, and local P2P decision surfaces: they contain Rust/C classifiers and deterministic corpora, not full wire protocols, target evidence, or independent qualification. In particular, `p2p-iot-core` remains draft and the qualification corpus intentionally retains unexecuted/blocked cross-class cases; that evidence does not promote `zk240` or `zk241` beyond 25%.

### Exact-current validation status

At `fa0a177` on 2026-09-02, `scripts/ci-release-qualification.sh` passed its clean exact-head preflight and postflight. The run covered Rust formatting, check, tests, Clippy, dependency audit; C normal, ASan, and UBSan builds/tests; vector regeneration/drift; contract/corpus checks; and all scoped ProVerif models (37 true queries across AUTH-v3, replay continuity, lineage replacement, and association admission).

This restores `zk202` to 100% and the baseline group to 85%. It does not close the roadmap's independent-review, constrained-target, comprehensive formal-traceability, or RFC-class evidence gaps.

## Current evidence posture

ZK-ARCHE deliberately keeps maturity states separate. Evidence for one state must not be used to imply another.

| State | Current posture |
|---|---|
| `IMPLEMENTED` | Material Rust/C protocol implementation exists for the current baseline, but the full roadmap is not implemented. |
| `TESTED` | Strong automated Rust/C, deterministic-vector, negative-path, formal-model, and qualification coverage exists for implemented surfaces; exact-current release qualification passed. Future roadmap surfaces remain incomplete. |
| `INTEROPERABLE` | Rust/C interoperability evidence exists for shared implemented behavior; this does not cover every future profile, transport, lifecycle, or P2P requirement. |
| `FORMALLY ANALYZED` | Scoped formal properties have retained evidence; TD-003 remains open because full property coverage and model-to-spec/code traceability are incomplete. |
| `MEASURED` | Available software/environment evidence exists, but required physical STM32/ESP32-S3-class evidence remains incomplete under TD-002. |
| `EXTERNALLY REVIEWED` | **Not complete.** TD-001 independent cryptographic review remains an external evidence blocker. |
| `RFC-CLASS DOCUMENTED` | **Not complete.** TD-004 and the RFC-class evidence gate remain open. |
| `COMMON-CONFORMANT` | **Not complete.** The complete constrained Common Contract and executable P2P qualification matrix remain unfinished. |
| `DEPLOYMENT-QUALIFIED` | **Not claimed.** Protocol conformance is intentionally distinct from field/product readiness. |

## Principal blockers

The main evidence ceilings remain:

1. **TD-001 — independent cryptographic review**  
   The custom role-membership proof can be prepared for review and regression-tested internally, but independent review cannot be self-declared complete.

2. **TD-002 — constrained-target evidence**  
   Reproducible STM32/ESP32-S3-class execution-context, wire, RAM, flash, CPU/latency, entropy/key-storage, restart/rollback, and related target evidence remains required. Unavailable physical measurements must not be invented.

3. **TD-003 — formal traceability**  
   Scoped formal analysis is advancing, including retained AUTH-v3/FM-06 evidence, but the canonical/synchronized model, property/attacker matrix, privacy/lifecycle coverage, and model→spec→code traceability are not yet complete.

4. **TD-004 — RFC-class normative specification**  
   The normative grammar, complete state machines, registries, requirement language, Security/Privacy Considerations, annotated traces, conformance/change-control package, and independent-implementation evidence remain incomplete.

5. **P2P Common Contract qualification**  
   `p2p-iot-core` remains draft/non-selectable. The repository now owns a fail-closed cross-class qualification corpus, but executable constrained↔constrained and constrained↔higher-capability evidence, bounded stale-authorization semantics, target budgets, and no-infrastructure runtime evidence remain unfinished.

## Current execution priority

Roadmap execution uses the repository's balanced lane rotation. Every run first establishes exact-current `dev` health using executable repository-owned validation available in the environment. Hosted GitHub Actions are intentionally absent from `dev` and are not a development qualification authority.

Within the designated dependency-ready lane, preserve the bottom-up Common Contract and evidence rules: do not promote a draft profile, invent hardware/formal/external-review evidence, or weaken mandatory security to obtain a green result.

## Bottom-up Common Contract

The roadmap's interoperability north star is:

> **The least-capable supported conformant peer defines the resource envelope, not a weaker security model.**

A high-capability peer must adapt to the constrained mandatory floor without lowering assurance. A constrained peer must locally verify the mandatory authentication decision. Core AUTH between already-authorized peers must not depend on a CA, cloud identity provider, central registry lookup, DNS, Internet connectivity, blockchain, manufacturer cloud, or gateway/controller approval.

Trust is local and non-transitive by default. Delegation is explicit, bounded, revocable, and scoped. Normal AUTH is NO-LEARNING. Optional higher-end functionality may scale upward, but it must remain isolated from the mandatory constrained security floor.

## Updating this progress snapshot

When roadmap progress is recalculated:

1. Resolve exact current `dev` HEAD and establish development health using repository-owned validation executable in the available environment; unavailable lanes remain explicitly unavailable.
2. Re-evaluate every canonical phase `zk201`–`zk241` against its declared exit criteria using the 0/25/50/75/100 rubric above.
3. Reconcile phase scores with the RFC-class exit requirements in [`rfc-evolution-plan.md`](./rfc-evolution-plan.md).
4. Update the overall arithmetic mean, grouped percentages, evidence-basis commit, blockers, and evidence posture in this README when those materially change.
5. Record score decreases when newer evidence invalidates an older claim; do not preserve a percentage merely for monotonic appearance.
6. If useful work occurs inside a phase without crossing a 25-point threshold, record that progress in the run report without inflating this completion score.

For normative requirements and detailed exit criteria, the roadmap documents remain authoritative over this dashboard.
