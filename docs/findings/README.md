# ZK-ARCHE Weekly Findings Format

`docs/findings/` is the weekly repository-grounded findings layer for ZK-ARCHE.

It exists to keep three different things separate:

- `docs/research/` — external evidence, standards/papers/projects, hypotheses, and research backlog;
- `docs/findings/` — what the current repository, reproductions, measurements, reviews, or consolidated evidence actually establish;
- `docs/requests/` — explicit human-authorized work with acceptance conditions.

A research paper or daily report is not automatically a finding, and a finding is not automatically a request, roadmap item, specification requirement, or maturity claim.

## Weekly file rule

Create at most one findings document per active week:

```text
docs/findings/week-of-MM-DD-YYYY-findings.md
```

Do not pre-create empty future files. Do not create provider-specific, agent-specific, or `details-*` sidecars.

## Required structure

Each weekly findings file should use this stable shape:

0. **How to use this file**
1. **Week at a glance**
2. **Consolidated findings**
3. **Evidence and provenance map**
4. **Duplicate / rejected / superseded interpretations**
5. **Impact map**
6. **Carry-forward / unresolved questions**

The file may add specialized subsections, but the first six sections should remain recognizable.

## Finding contract

Use a stable weekly ID such as `F0824-01`.

Each material finding should state:

- **Status:** confirmed | partial | inconclusive | rejected | superseded
- **Evidence basis:** repository path, test, benchmark, review, daily research report, primary source, or retained artifact
- **Repository fact:** what is actually true in the current tree or retained evidence
- **Interpretation:** the narrow conclusion justified by that evidence
- **Affected work:** existing `R-*`, `TD-*`, roadmap phase, ADR, spec section, test/vector surface, or `none`
- **Required next evidence:** what would strengthen, falsify, or clear the finding
- **Claim boundary:** what the finding does *not* prove

Separate evidence from inference. Prefer one consolidated finding over repeating the same conclusion from multiple daily reports.

## Findings versus other documentation

- `docs/research/daily/` preserves dated external-research provenance.
- `docs/research/backlog.md` owns unresolved research questions.
- `docs/findings/` owns consolidated repository-grounded conclusions and execution/review outcomes.
- `docs/requests/` owns explicit requested work and acceptance criteria.
- `docs/technical-debt/` owns durable reproducible gaps that survive the week.
- `docs/roadmaps/` owns long-term sequencing and capability/specification maturity.
- `docs/adr/` owns consequential decisions.
- `spec/` owns normative protocol behavior.

A weekly findings file must not become a second research archive or roadmap. Link to the source material instead of copying full daily reports.

## Research handoff rule

A daily research item may be promoted into weekly findings only when the weekly synthesis can state a concrete repository implication and its evidence basis. If the evidence is still only a promising external idea, keep it in `docs/research/`.

A weekly finding may become a request only after explicit human intent or an already-authorized engineering process makes that work actionable. This prevents recurring research automation from silently creating implementation commitments.

## Agent reading rule

Read the current weekly findings file before rereading many daily reports. Jump to the cited daily report, source, test, or repository path only when the finding needs deeper evidence.

Current source/tests/vectors, `spec/`, retained assurance evidence, active technical debt, ADRs, and later explicit human decisions override stale weekly findings.