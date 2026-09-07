# ZK-ARCHE Roadmaps

This directory contains the canonical long-horizon engineering roadmaps for ZK-ARCHE:

- [`improvement-roadmap.md`](./improvement-roadmap.md) — canonical phase plan (`zk201`–`zk241`) for implementation, assurance, constrained-device viability, lifecycle security, interoperability, data sovereignty, and infrastructure-independent P2P operation.
- [`rfc-evolution-plan.md`](./rfc-evolution-plan.md) — RFC-class specification, interoperability, security-analysis, registry, change-control, and standards-reference discipline.

The two documents are complementary. The improvement roadmap owns the canonical phase sequence and exit evidence; the RFC evolution plan sharpens the specification-quality gates that apply to those phases.

## Roadmap progress

> **Current evidence-based completion: 46.34%**<br>
> **Remaining to full roadmap: 53.66%**<br>
> Progress snapshot reviewed: **2026-09-06**<br>
> Evidence basis: exact-current `dev` repository evidence through `eaf9222`; executable cloud-runner qualification unavailable in this run; user-confirmed local validation baseline remains green.

`46.34%` is a roadmap-tracking metric, **not** a claim that ZK-ARCHE is 46.34% secure, production-ready, RFC-standardized, externally reviewed, or deployment-qualified.

The score is the arithmetic mean of the evidence-completion scores for canonical phases `zk201`–`zk241`. Each phase is evaluated only against its declared exit evidence.

| Roadmap group | Phases | Completion |
|---|---:|---:|
| Baseline / reproducible truth | `zk201`–`zk205` | **85.00%** |
| Assurance / review / claim gate | `zk206`–`zk210` | **55.00%** |
| Enrollment / authorization / lifecycle | `zk211`–`zk215` | **45.00%** |
| AUTH hardening / IoT profiles | `zk216`–`zk224` | **36.11%** |
| Interop / RFC / transport / decomposition | `zk225`–`zk230` | **50.00%** |
| Data sovereignty | `zk231`–`zk238` | **31.25%** |
| Infrastructure-independent P2P Common Contract | `zk239`–`zk241` | **33.33%** |
| **Overall** | `zk201`–`zk241` | **46.34%** |

### Scoring rubric

| Score | Meaning |
|---:|---|
| **0%** | Roadmap intent only; no material exit evidence exists. |
| **25%** | Design/specification or initial implementation/evidence exists, but major required surfaces are absent. |
| **50%** | Material implementation plus some required tests/evidence exists; important exit criteria remain incomplete. |
| **75%** | Substantially implemented; most exit evidence exists, with bounded qualification/evidence gaps remaining. |
| **100%** | The phase's declared exit evidence actually exists and repository claim language matches that evidence. |

Scores are not based on commit count, lines of code, elapsed time, or effort. Required independent review, physical-target measurements, formal results, interoperability evidence, RFC-class documentation, field evidence, and other declared exit artifacts must actually exist before the associated phase receives credit.

### Phase scorecard

The 41 phase scores now sum to `1900`; `1900 / 41 = 46.34%`.

| Roadmap group | Phase scores |
|---|---|
| Baseline / reproducible truth | `zk201=100`, `zk202=100`, `zk203=75`, `zk204=75`, `zk205=75` |
| Assurance / review / claim gate | `zk206=75`, `zk207=50`, `zk208=25`, `zk209=25`, `zk210=100` |
| Enrollment / authorization / lifecycle | `zk211=50`, `zk212=50`, `zk213=75`, `zk214=50`, `zk215=0` |
| AUTH hardening / IoT profiles | `zk216=25`, `zk217=75`, `zk218=75`, `zk219=25`, `zk220=0`, `zk221=75`, `zk222=50`, `zk223=0`, `zk224=0` |
| Interop / RFC / transport / decomposition | `zk225=75`, `zk226=50`, `zk227=25`, `zk228=50`, `zk229=25`, `zk230=75` |
| Data sovereignty | `zk231=50`, `zk232=0`, `zk233=25`, `zk234=75`, `zk235=50`, `zk236=25`, `zk237=25`, `zk238=0` |
| Infrastructure-independent P2P Common Contract | `zk239=50`, `zk240=25`, `zk241=25` |

### 2026-09-06 score change

`zk235` moved from **0 → 50**. Exact-current `dev` now contains a bounded, domain-separated local DATA audit-chain primitive in both Rust and C plus a normative local-chain contract and matching deterministic continuity/tamper fixtures. The primitive commits only to release-context/lifecycle state and is explicitly non-authoritative for release or trust decisions.

`zk235` remains below 75 because durable restart recovery, rollback-resistant persistence anchoring, power-loss evidence, transparency publication/bridge behavior, constrained target measurements, and an executed exact-head Rust/C qualification result for this new surface are still absent.

No other phase crosses a rubric threshold in this snapshot. Recent work inside unchanged phases remains bounded by its prior score until the corresponding declared exit evidence exists.

## Validation and evidence posture

The latest retained clean executable qualification predates this exact head. In this run the cloud execution environment could not clone `github.com`, so Cargo/C/ProVerif and the repository-wide qualification wrappers were not executed against `eaf9222`.

Record this state as:

`cloud-runner validation unavailable; user-confirmed local baseline green`

This is **not** a RED result and does not weaken the user-confirmed local baseline. It also does not permit a fresh TESTED/INTEROPERABLE/FORMALLY ANALYZED claim for the new exact head.

ZK-ARCHE deliberately keeps maturity states separate:

| State | Current posture |
|---|---|
| `IMPLEMENTED` | Material Rust/C protocol implementation exists for the current baseline, including initial DATA audit-chain semantics; the full roadmap is not implemented. |
| `TESTED` | Strong automated Rust/C, deterministic-vector, negative-path, formal-model, and qualification coverage exists for previously executed surfaces. Exact-current execution is not independently retained for this head. |
| `INTEROPERABLE` | Rust/C interoperability evidence exists for shared implemented behavior; it does not yet cover every profile, transport, lifecycle, sovereignty, or P2P requirement. |
| `FORMALLY ANALYZED` | Scoped formal properties have retained evidence; TD-003 remains open because complete property coverage and model→spec→code traceability are incomplete. |
| `MEASURED` | Required physical STM32/ESP32-S3-class evidence remains incomplete under TD-002. |
| `EXTERNALLY REVIEWED` | **Not complete.** TD-001 independent cryptographic review remains an external evidence blocker. |
| `RFC-CLASS DOCUMENTED` | **Not complete.** TD-004 and the RFC-class evidence gate remain open. |
| `COMMON-CONFORMANT` | **Not complete.** The complete constrained Common Contract and executable P2P qualification matrix remain unfinished. |
| `DEPLOYMENT-QUALIFIED` | **Not claimed.** Protocol conformance remains distinct from field/product readiness. |

## Principal blockers

1. **TD-001 — independent cryptographic review**  
   Custom role-membership proof behavior still requires actual independent review and dispositioned findings.

2. **TD-002 — constrained-target evidence**  
   Reproducible STM32/ESP32-S3-class execution-context, wire, RAM, flash, CPU/latency, entropy/key-storage, restart/rollback, revocation, and sovereignty evidence remains required. No physical evidence may be inferred from host-side tests.

3. **TD-003 — formal traceability**  
   Scoped formal analysis exists, but the canonical/synchronized model, property/attacker matrix, privacy/lifecycle coverage, compromise models, and complete model→spec→Rust/C→test traceability remain unfinished.

4. **TD-004 — RFC-class normative specification**  
   Normative grammar, complete state machines, registries/change control, requirement language, Security/Privacy Considerations, annotated traces, and independent-implementation conformance evidence remain incomplete.

5. **TD-005 — exact-head qualification enforcement**  
   Repository-owned qualification exists, but `dev` intentionally has no hosted Actions and API/web updates bypass local hooks. A given `dev` HEAD must not be described as freshly qualified unless repository-owned validation actually ran for that head.

6. **Data-sovereignty qualification**  
   Policy-bound release decisions and the initial local audit chain now exist, but the full `DATA_COMMIT` / `RELEASE_REQUEST` / `RELEASE_PROOF` / `RELEASE_KEY` / `AUDIT_APPEND` lifecycle, recovery/rollback persistence, target footprint evidence, and transparency bridge remain incomplete.

7. **P2P Common Contract qualification**  
   `p2p-iot-core` remains draft/non-selectable. Executable constrained↔constrained and constrained↔higher-capability evidence, bounded stale-authorization semantics, target budgets, and no-infrastructure runtime evidence remain unfinished.

## Bottom-up Common Contract

The roadmap's interoperability north star remains:

> **The least-capable supported conformant peer defines the resource envelope, not a weaker security model.**

A high-capability peer must adapt to the constrained mandatory floor without lowering assurance. A constrained peer must locally verify the mandatory authentication decision. Core AUTH between already-authorized peers must not depend on a CA, cloud identity provider, central registry lookup, DNS, Internet connectivity, blockchain, manufacturer cloud, or gateway/controller approval.

Trust is local and non-transitive by default. Delegation is explicit, bounded, revocable, and scoped. Normal AUTH is NO-LEARNING. Optional higher-end functionality may scale upward but must remain isolated from the mandatory constrained security floor.

## Updating this progress snapshot

When roadmap progress is recalculated:

1. Resolve exact-current `dev` HEAD and establish development health using repository-owned validation executable in the available environment; unavailable lanes remain explicitly unavailable rather than RED.
2. Re-evaluate every canonical phase `zk201`–`zk241` against its declared exit criteria using the 0/25/50/75/100 rubric.
3. Reconcile phase scores with the RFC-class exit requirements in [`rfc-evolution-plan.md`](./rfc-evolution-plan.md).
4. Update the arithmetic mean, grouped percentages, evidence basis, blockers, and evidence posture only when materially changed.
5. Record score decreases when newer evidence invalidates an older claim; do not preserve a percentage for monotonic appearance.
6. Record useful within-phase work in run reports without inflating scores when a 25-point evidence threshold is not crossed.

For normative requirements and detailed exit criteria, the roadmap documents remain authoritative over this dashboard.
