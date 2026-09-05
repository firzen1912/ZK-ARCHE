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
| TD-002 | Constrained-device | Target-class byte/RAM/CPU evidence is incomplete for STM32/ESP32-S3-class deployments | Blocks IoT field-readiness claims | Reproducible target benchmark reports and profile limits | unassigned | open |
| TD-003 | Assurance | Formal assurance now has synchronized AUTH-v3 and replay-continuity models, retained scoped ProVerif results, and partial model→spec→Rust/C traceability, but complete property coverage, parser/model and runtime/model equivalence, privacy/compromise semantics, and several lifecycle properties remain unresolved or normatively blocked | Blocks full formal-verification and complete model-to-code assurance claims | Complete the remaining property/attacker coverage and exact model→spec→Rust/C/test mapping; retain exact-model successful results/counterexamples; close abstraction/equivalence gaps only where repository semantics actually exist | unassigned | open |
| TD-004 | Protocol | RFC-style specification work has progressed beyond a skeleton, including canonical context/profile text and a testable implementation-requirements contract, but the package still lacks complete independently implementable state machines, negotiation/downgrade behavior, error/privacy semantics, lifecycle rules, registries/change control, annotated traces, and full conformance integration | Blocks RFC-class documented and specification-grade/conformance claims | Complete normative wire/state/lifecycle/error/privacy text, registries/change control, mandatory-floor semantics, positive/negative vectors, annotated traces, and independent Rust/C conformance evidence | unassigned | open |
| TD-005 | Tooling | Exact-head executable qualification is not enforced on every path that can advance `dev`. The pre-commit gate does not compile either lane; the pre-push hook that runs `scripts/ci-all.sh` for `dev` is per-clone local configuration (`core.hooksPath`) whose source `.githooks/` is tracked only on `main`, so a `dev` checkout installs no gate by default; commits reaching `dev` through the GitHub API or web editor bypass local hooks entirely; and `dev` intentionally runs no hosted Actions | Weakens every "exact-current `dev` health" and "clean exact-head qualification" statement used as the evidence basis for the roadmap dashboard, daily research reports, and assurance checkpoints. Realized 2026-09-03 to 2026-09-05, when `dev` HEAD did not build after `251c987` while two daily reports asserted exact-head health | A per-HEAD qualification record for `dev` equivalent to the existing per-HEAD formal manifests under `evidence/formal/`, so an unqualified HEAD is detectable rather than assumed; plus a gate reachable from `dev` itself — tracked hook source with a documented install step, a compile/test step in the pre-commit gate, or an enforced post-merge verification pass for commits that bypass local hooks | unassigned | open |

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