# Daily Research Reports

Completed recurring ZK-ARCHE research reports live here using ISO dates:

```text
docs/research/daily/YYYY-MM-DD.md
```

Do not create empty future reports. Historical reports remain dated provenance and should not be rewritten merely to match a newer template.

## Daily report objective

A daily report should answer six questions quickly:

1. What changed in the repository since the prior run?
2. Which active `R-*` backlog items were selected, and why?
3. What external evidence is genuinely new?
4. What status/evidence transition did each selected backlog item earn?
5. What new follow-on research question, if any, was discovered?
6. What evidence is still required before engineering promotion?

The daily report is research execution and evidence intake, not an implementation request. It may consume and replenish `../backlog.md`, but it must not autonomously create engineering commitments.

## Queue vocabulary

### Priority

```text
P0 | P1 | P2 | P3
```

P0/P1 items normally outrank P2/P3 items. See `../PIPELINE.md` for exact semantics.

### Status

```text
queued | researching | reproduce | benchmark | prototype | promote | research-only | defer | reject | exhausted
```

`exhausted` means the current research question has no material unresolved research step under its present scope. It does not mean engineering implementation or roadmap acceptance is complete.

## Novelty vocabulary

```text
new | corroborates | refines | contradicts | supersedes
```

Do not create a new backlog item merely because a new paper corroborates an existing question.

## Evidence maturity

```text
concept | formal | software | constrained-hardware | deployed | externally-reviewed
```

## Report template

````markdown
# ZK-ARCHE Research — YYYY-MM-DD

## 0. Run identity and repository delta

| Field | Value |
|---|---|
| Starting `dev` HEAD | `<sha>` |
| Previous daily report | `YYYY-MM-DD` or `none` |
| Repository delta since prior report | concise list or `none` |
| Research focus | concise description |

## 1. Queue snapshot and selection

### Active queue summary

| Priority | queued/researching | reproduce | benchmark | prototype | promote | deferred/research-only |
|---|---:|---:|---:|---:|---:|---:|
| P0 | 0 | 0 | 0 | 0 | 0 | 0 |
| P1 | 0 | 0 | 0 | 0 | 0 | 0 |
| P2 | 0 | 0 | 0 | 0 | 0 | 0 |
| P3 | 0 | 0 | 0 | 0 | 0 | 0 |

### Selected backlog items

| ID | Priority | Starting status | Why selected today | Intended evidence step |
|---|---|---|---|---|
| R-xxx | P0 | reproduce | dependency/security reason | exact next evidence |

## 2. Executive synthesis

Keep this decision-dense. Prefer 3–6 conclusions:

- what changed;
- why it matters;
- what advanced or failed to advance;
- what should happen next;
- what did **not** change.

## 3. Finding index

| ID | Finding | Novelty | Backlog owner | Disposition | Weekly finding candidate? |
|---|---|---|---|---|---|
| DYYYYMMDD-F01 | short title | new/refines/... | R-xxx | reproduce/... | yes/no |

## 4. Repository context reviewed

Record only relevant changed/controlling surfaces and direct implementation/spec/test/formal anchors.

## 5. Findings

### DYYYYMMDD-F01 — Finding title

- **Novelty:** new | corroborates | refines | contradicts | supersedes
- **Backlog owner:** R-xxx / none
- **Source:**
- **Primary link:**
- **Source type / venue:**
- **Publication or release date:**
- **Verified claim:**
- **Existing engineering owner:** TD-xxx / roadmap phase / spec section / none
- **Repository fact:**
- **Problem addressed:**
- **Strongest distinct engineering idea:**
- **Evidence maturity:** concept | formal | software | constrained-hardware | deployed | externally-reviewed
- **Limitations / uncertainty:**
- **ZK-ARCHE inference:**
- **Wire / RAM / CPU / flash / dependency / trust-model impact:**
- **Rust/C / vector compatibility impact:**
- **Required next evidence:**
- **Recommended disposition:** investigate | reproduce | benchmark | prototype | promote | defer | reject | research-only
- **Weekly finding candidate:** yes | no

```yaml
roadmap_impact:
  candidate_phase: null
  recommendation: investigate
  evidence_maturity: concept
  protocol_impact: none
  required_next_evidence: null
  roadmap_action: none
  promotion_requirement: explicit human review
```

## 6. Cross-source synthesis

State the combined conclusion once. Reconcile corroboration, disagreement, assumptions, and any changed decision boundary.

## 7. Backlog transitions

| ID | Start | End | Evidence gained today | Remaining unresolved question |
|---|---|---|---|---|
| R-xxx | reproduce | reproduce/benchmark/promote/etc. | concise evidence | exact next step |

If no transition is justified, state `none`.

## 8. Newly enqueued follow-ons

| New ID | Priority | Question | Why distinct | Evidence that would change a decision |
|---|---|---|---|---|

Only add a new item when it is genuinely distinct from existing backlog questions. Otherwise update the existing item.

## 9. Actionability matrix

| Target | Today’s change | Required next evidence | Destination |
|---|---|---|---|
| R-xxx / TD-xxx / none | refinement | exact evidence | backlog / weekly findings candidate / explicit human review |

## 10. Weekly handoff candidates

### Findings candidates

List conclusions suitable for later human-reviewed consolidation into `docs/findings/`.

### Request candidates

Normally `none` for automated research.

## 11. Claim / no-change boundary

State what this report did **not** establish: no wire change, no suite change, no roadmap/spec/ADR change, no debt cleared, no readiness/certification claim changed, unless a separate authorized process actually made that change.

## 12. Next queue recommendations

List the highest-value unexhausted backlog items for the next run, in priority order, with one-line rationale.

## 13. Follow-up sources

Prioritize only primary sources that deserve deeper review next.
````

## Quality rules

- Consume backlog items by decision value, not age alone.
- Prefer P0/P1 when actionable.
- A revisit must have a concrete unresolved question or new-source trigger.
- Do not fabricate status transitions to show progress.
- Do not create one backlog item per paper.
- Consolidate convergent sources under one engineering question.
- Preserve source provenance and limitations.
- Prefer a few high-impact findings over a literature dump.
- If all active items are exhausted, perform a bounded discovery pass and enqueue only distinct questions with explicit evidence contracts.

## Promotion boundary

Daily research may mark an item `promote`, but this means only that the evidence is ready for explicit human engineering review.

The daily process remains write-limited to `docs/research/**` under `../PIPELINE.md` and must not itself modify roadmap, ADR, spec, implementation, assurance, release, technical-debt, findings, or request state.
