# ZK-ARCHE Roadmaps

This directory contains the canonical long-horizon engineering roadmaps for ZK-ARCHE:

- [`improvement-roadmap.md`](./improvement-roadmap.md) — canonical phase plan (`zk201`–`zk241`) for implementation, assurance, constrained-device viability, lifecycle security, interoperability, data sovereignty, and infrastructure-independent P2P operation.
- [`rfc-evolution-plan.md`](./rfc-evolution-plan.md) — RFC-class specification, interoperability, security-analysis, registry, change-control, and standards-reference discipline.

The two documents are complementary. The improvement roadmap owns the canonical phase sequence and exit evidence; the RFC evolution plan sharpens the specification-quality gates that apply to those phases.

## Roadmap progress

> **Current evidence-based completion: 35.98%**  
> **Remaining to full roadmap: 64.02%**  
> Progress snapshot reviewed: **2026-08-29**  
> Evidence basis: `dev` through `c7b2c647549d0410f70281c39acdb9b1f8188a2f`

`35.98%` is a roadmap-tracking metric, **not** a claim that ZK-ARCHE is 35.98% secure, production-ready, RFC-standardized, externally reviewed, or deployment-qualified.

The score is the arithmetic mean of the evidence-completion scores for canonical phases `zk201`–`zk241`. Each phase is evaluated only against its declared exit evidence.

| Roadmap group | Phases | Completion |
|---|---:|---:|
| Baseline / reproducible truth | `zk201`–`zk205` | **85.00%** |
| Assurance / review / claim gate | `zk206`–`zk210` | **55.00%** |
| Enrollment / authorization / lifecycle | `zk211`–`zk215` | **15.00%** |
| AUTH hardening / IoT profiles | `zk216`–`zk224` | **33.33%** |
| Interop / RFC / transport / decomposition | `zk225`–`zk230` | **45.83%** |
| Data sovereignty | `zk231`–`zk238` | **6.25%** |
| Infrastructure-independent P2P Common Contract | `zk239`–`zk241` | **25.00%** |
| **Overall** | `zk201`–`zk241` | **35.98%** |

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

## Current evidence posture

ZK-ARCHE deliberately keeps maturity states separate. Evidence for one state must not be used to imply another.

| State | Current posture |
|---|---|
| `IMPLEMENTED` | Material Rust/C protocol implementation exists for the current baseline, but the full roadmap is not implemented. |
| `TESTED` | Strong automated Rust/C, deterministic-vector, negative-path, and qualification coverage exists for implemented surfaces; future roadmap surfaces remain incomplete. |
| `INTEROPERABLE` | Rust/C interoperability evidence exists for shared implemented behavior; this does not cover every future profile, transport, lifecycle, or P2P requirement. |
| `FORMALLY ANALYZED` | Scoped formal properties have retained evidence; TD-003 remains open because full property coverage and model-to-spec/code traceability are incomplete. |
| `MEASURED` | Available software/environment evidence exists, but required physical STM32/ESP32-S3-class evidence remains incomplete under TD-002. |
| `EXTERNALLY REVIEWED` | **Not complete.** TD-001 independent cryptographic review remains an external evidence blocker. |
| `RFC-CLASS DOCUMENTED` | **Not complete.** TD-004 and the RFC-class evidence gate remain open. |
| `COMMON-CONFORMANT` | **Not complete.** The complete constrained Common Contract and P2P qualification matrix remain unfinished. |
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

## Current execution priority

Subject to the exact-current `dev` GitHub Actions green gate, roadmap work should continue in this order:

1. Preserve reproducible Rust/C truth and deterministic/negative conformance evidence.
2. Finish TD-003 formal property coverage and model→spec→code traceability.
3. Advance TD-004 toward independently implementable RFC-class normative documentation.
4. Build TD-002 constrained Common Contract measurement/scaffolding and collect only real environment/target evidence.
5. Complete AUTH/TRUST/LINK lifecycle semantics: authentication ≠ authorization ≠ trust mutation, NO-LEARNING AUTH, transcript/context binding, replay, retry, rekey, revocation convergence, authorization-aware resumption, and downgrade resistance.
6. Qualify the infrastructure-independent `p2p-iot-core` Common Contract across constrained and higher-capability peers with the same mandatory authentication assurance floor.
7. Advance transport/interface adapters and bindings only when their shared security prerequisites are stable.
8. Advance data-sovereignty work after shared AUTH/TRUST/LINK prerequisites are sufficiently specified and tested.
9. Keep BBS/PQ, large trust graphs, attestation, and private lookup optional/research-only unless explicitly promoted and dependency-ready.

## Bottom-up Common Contract

The roadmap's interoperability north star is:

> **The least-capable supported conformant peer defines the resource envelope, not a weaker security model.**

A high-capability peer must adapt to the constrained mandatory floor without lowering assurance. A constrained peer must locally verify the mandatory authentication decision. Core AUTH between already-authorized peers must not depend on a CA, cloud identity provider, central registry lookup, DNS, Internet connectivity, blockchain, manufacturer cloud, or gateway/controller approval.

Trust is local and non-transitive by default. Delegation is explicit, bounded, revocable, and scoped. Normal AUTH is NO-LEARNING. Optional higher-end functionality may scale upward, but it must remain isolated from the mandatory constrained security floor.

## Updating this progress snapshot

When roadmap progress is recalculated:

1. Resolve the exact current `dev` HEAD and require exact-head CI to be green before ordinary roadmap publication.
2. Re-evaluate every canonical phase `zk201`–`zk241` against its declared exit criteria using the 0/25/50/75/100 rubric above.
3. Reconcile phase scores with the RFC-class exit requirements in [`rfc-evolution-plan.md`](./rfc-evolution-plan.md).
4. Update the overall arithmetic mean, grouped percentages, evidence-basis commit, blockers, and evidence posture in this README.
5. Record score decreases when newer evidence invalidates an older claim; do not preserve a percentage merely for monotonic appearance.
6. If useful work occurs inside a phase without crossing a 25-point threshold, record that progress in the run report without inflating this completion score.

For normative requirements and detailed exit criteria, the roadmap documents remain authoritative over this dashboard.
