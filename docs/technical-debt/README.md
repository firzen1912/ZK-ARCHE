# ZK-ARCHE Technical Debt Register

This directory makes unresolved work explicit instead of burying it inside roadmaps, source comments, or maturity claims. The register is inspired by HIVEAS's separate technical-debt area but uses categories appropriate to a cryptographic/protocol framework.

## Debt categories

| Category | Examples |
|---|---|
| Protocol debt | underspecified messages, incomplete state machines, unversioned extension behavior, ambiguous error/replay semantics |
| Cryptographic debt | custom proof requiring review, incomplete domain-separation analysis, missing primitive migration design |
| Interoperability debt | Rust/C mismatch, missing vectors, unsupported feature in one lane, non-reproducible interop test |
| Constrained-device debt | missing STM32/ESP32 measurements, heap use, packet-size excess, flash-write or RNG assumptions |
| Assurance debt | missing fuzz/replay/mutation/formal/side-channel/external-review evidence |
| Tooling debt | non-reproducible CI, missing benchmark harness, evidence collection gaps, qualification gates that do not cover every path into a branch |
| Documentation/spec debt | implemented behavior not captured normatively, stale registry/profile/security text |

## Active register

| ID | Category | Gap | Impact / blocked claim | Clearing evidence | Owner | Status |
|---|---|---|---|---|---|---|
| TD-001 | Assurance | Custom role-membership proof still requires independent cryptographic review before strong production/privacy claims | Blocks externally reviewed / production-grade proof claims | External review memo plus resolved findings and regression vectors | unassigned | open |
| TD-002 | Constrained-device | The repository now defines a versioned constrained-target benchmark-manifest schema that captures target identity, toolchain/profile, crypto/entropy/storage/security posture, transport context, resource/latency measurements, restart/replay/rollback/revocation observations, raw-artifact references, and provenance. Actual reproducible STM32/ESP32-S3-class physical measurements remain absent/incomplete. Schema validity or planned manifests are not measurement evidence. | Blocks IoT field-readiness and physical target-profile claims | Populate the repository-owned manifest contract with reproducible physical STM32/ESP32-S3-class runs, retained raw artifacts, and profile limits; review the resulting evidence without promoting host/simulation observations to physical-target claims | unassigned | open |
| TD-003 | Assurance | Formal assurance now has synchronized AUTH-v3, replay-continuity, retained-authority, and lineage-replacement models, including fail-closed predecessor reconciliation, plus retained scoped ProVerif results and partial model→spec→Rust/C traceability. Complete property/attacker coverage, parser/model and runtime/model equivalence, privacy/compromise semantics, exact-head execution of newer lifecycle models, and several lifecycle properties remain unresolved or normatively blocked. | Blocks full formal-verification and complete model-to-code assurance claims | Complete the remaining property/attacker coverage and exact model→spec→Rust/C/test mapping; execute and retain exact-model successful results/counterexamples for the newer lifecycle models; close abstraction/equivalence gaps only where repository semantics actually exist | unassigned | open |
| TD-004 | Protocol | RFC-style specification work has progressed beyond a skeleton, including canonical context/profile text, a testable implementation-requirements contract, bounded P2P delegation semantics, an implementation-backed annotated enrollment-grant reference trace, and an annotated lineage-replacement trace tied to the normative lifecycle machine and canonical negative corpus. The package still lacks complete independently implementable state machines across all major flows, negotiation/downgrade behavior, complete error/privacy semantics, remaining lifecycle rules, complete registries/change control, sufficient annotated traces across other major flows, and full conformance integration. | Blocks RFC-class documented and specification-grade/conformance claims | Complete normative wire/state/lifecycle/error/privacy text, registries/change control, mandatory-floor semantics, positive/negative vectors, annotated traces across the remaining major flows, and independent Rust/C conformance evidence | unassigned | open |
| TD-005 | Tooling | Exact-head qualification infrastructure now exists on `dev`: `scripts/ci-all.sh` records a clean exact-HEAD PASS through `scripts/record-qualification.sh`, and the tracked `.githooks/pre-push` runs `scripts/ci-all.sh` for pushes targeting `dev`. Enforcement is still incomplete because hook activation depends on per-clone `core.hooksPath`, GitHub API/web updates bypass local hooks, and `dev` intentionally runs no hosted Actions. Therefore a `dev` HEAD without a retained qualification record remains possible and must not be assumed healthy solely from branch position. | Weakens every "exact-current `dev` health" and "clean exact-head qualification" statement used as the evidence basis for the roadmap dashboard, daily research reports, and assurance checkpoints. Realized 2026-09-03 to 2026-09-05, when `dev` HEAD did not build after `251c987` while two daily reports asserted exact-head health | Make absence of a retained exact-HEAD qualification record detectable and blocking for every path that can advance or qualify `dev`, without relying on GitHub Actions on `dev`; this may be satisfied by an enforced repository-owned post-update verifier or equivalent branch-reachable mechanism that covers API/web updates as well as locally hooked pushes. Preserve the existing exact-HEAD manifest/log contract. | unassigned | open |

## Status vocabulary

- `open` — unresolved and relevant.
- `in-progress` — clearing work is underway.
- `blocked` — requires external/hardware/human dependency.
- `accepted` — consciously tolerated with documented reason and scope.
- `cleared` — required evidence exists and links to it are recorded.
- `superseded` — replaced by another debt item or architectural decision.

## Adding debt

A debt entry should state what is missing, what claim or capability it blocks, and what evidence clears it. Avoid vague TODOs such as “improve security.”

Use:

```text
TD-NNN
category
gap
impact / blocked claim
clearing evidence
owner
status
links: research / roadmap / ADR / spec / issue / evidence
```

Roadmap work may clear debt, but a roadmap checkbox alone is not evidence that the debt is cleared.