# ZK-ARCHE Weekly Request Format

`docs/requests/` is the explicit human-intent and bounded execution-control layer for ZK-ARCHE.

Each active week has at most one request document:

```text
docs/requests/week-of-MM-DD-YYYY-request.md
```

Do not create `details-*`, daily-request, provider-specific, model-specific, or agent-specific sidecars. Preserve necessary technical detail in the weekly file; put external research in `docs/research/`, observed/reproduced outcomes in `docs/findings/`, and durable unresolved gaps in `docs/technical-debt/`.

Do not pre-create future weekly files. A request file should exist only when there is explicit human intent or an already-authorized recurring engineering process for that week.

## Required weekly structure

Every weekly request follows the same control-board pattern:

0. **How to use this file**
1. **Week at a glance**
2. **Pass condition and status vocabulary**
3. **Scoreboard**
4. **Priority execution packets**
5. **Human request / intent timeline**
6. **Problem → diagnosis → fix / remaining index**
7. **Detailed requirements and engineering notes**
8. **Validation and evidence**
9. **Session update record**
10. **Week-end closeout / carry-forward**

The headings are stable. Their contents may be specialized to the week.

## Status vocabulary

- **Done** — acceptance conditions and required evidence are complete.
- **Built, not evidenced** — implementation exists but required validation/review/evidence does not.
- **Partial** — coherent progress exists but one or more acceptance obligations remain.
- **Blocked** — a real external/hardware/review dependency prevents completion.
- **Open** — requested work has not been completed.
- **Checkpointed** — bounded stopping point with evidence, rejected attempts, and next experiment recorded.
- **Withdrawn** — apparent success was invalidated by stronger evidence, review, or changed human intent.

For protocol/security work, “tests pass” is not automatically “Done” when the request also requires vectors, cross-language parity, formal evidence, external review, constrained measurements, or a claim gate.

## Reading rule for coding agents

Read §§0–4 first. Own one bounded packet, then jump to the matching §7 subsection. Avoid rereading the entire research archive, roadmap, or unrelated weekly findings merely to reconstruct one task.

Current repository source/tests/vectors, `spec/`, `AGENTS.md` when present, ADRs, retained assurance evidence, active technical debt, and later explicit human decisions override stale weekly request text.

## Security-sensitive editing rule

A weekly request can authorize work, but it does not waive the repository’s checkpoint-style review boundaries.

Changes to cryptographic primitives, proof semantics, domain separators, transcript construction, packet/wire grammar, replay/resumption semantics, trust or authorization behavior, parser acceptance, RNG/key lifecycle, or Rust/C interoperability remain security-sensitive. They require the review/evidence posture defined by the roadmap and assurance documents.

A request must never manufacture a maturity claim. Production readiness, external cryptographic review, formal verification, constrained-device field readiness, side-channel resistance, or certification can be marked complete only when the required evidence exists.

## Request versus findings versus research versus debt

- `docs/requests/week-of-*.md` — requested work, acceptance conditions, status/control context, and necessary rationale.
- `docs/findings/week-of-*-findings.md` — consolidated measurements, reviews, reproductions, repository observations, rejected hypotheses, and what actually happened.
- `docs/research/` — external source intake and unresolved research questions.
- `docs/technical-debt/` — durable reproducible gaps that survive the week.
- `docs/roadmaps/` — long-term sequencing and capability/specification ownership.
- `docs/adr/` — consequential architecture/protocol decisions.
- `spec/` — normative protocol behavior.

A weekly request must not become a second roadmap or a duplicate research archive. Preserve detail when it changes acceptance conditions or prevents repeated failure; otherwise link to the canonical artifact.

## Promotion rule

Research does not create requests automatically.

A research item may appear in a weekly request only when:

1. a human explicitly asks for the work; or
2. an already-authorized recurring engineering process owns that class of work; and
3. the request defines a bounded acceptance condition and the evidence needed to mark it complete.

This keeps the daily research automation advisory and prevents interesting external work from silently becoming engineering scope.