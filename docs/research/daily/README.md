# Daily Research Reports

Completed recurring ZK-ARCHE research reports live here using ISO dates:

```text
docs/research/daily/YYYY-MM-DD.md
```

Do not create empty reports for future dates. A file exists only when research was actually performed.

Historical reports are dated provenance. Do not rewrite old reports merely to make them match a newer template; improve the forward contract and consolidate older conclusions through `docs/findings/` when needed.

## Daily report objective

A daily report should answer four questions quickly:

1. What changed in the repository since the previous research run?
2. What external evidence is genuinely new rather than repeated?
3. Which existing `R-*`, `TD-*`, spec/roadmap/assurance question does it affect?
4. What evidence should be gathered next before any engineering promotion occurs?

The daily report is **research intake**, not an implementation request. Candidate weekly findings may be identified, but the recurring research automation must not create or update `docs/findings/` or `docs/requests/`.

## Novelty vocabulary

Each material finding must classify its relationship to prior research:

```text
new | corroborates | refines | contradicts | supersedes
```

- `new` — establishes a distinct engineering/security question not already captured.
- `corroborates` — adds independent support without materially changing the existing conclusion.
- `refines` — changes the evidence contract, constraints, threat model, or recommended next step for an existing question.
- `contradicts` — conflicts with a prior assumption or finding and requires reconciliation.
- `supersedes` — newer/stronger evidence replaces a previous conclusion or source basis.

Do not create a new backlog item for every `corroborates` or `refines` result.

## Evidence maturity vocabulary

Use one normalized vocabulary across future reports:

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

If there is no meaningful implementation/spec/roadmap/assurance delta, say so once. Do not repeat a long unchanged repository inventory.

## 1. Executive synthesis

Keep this decision-dense. Prefer 3–6 numbered conclusions:

- what is genuinely new or stronger;
- why it matters to the current ZK-ARCHE state;
- what should be reproduced, benchmarked, prototyped, deferred, rejected, or kept research-only;
- what did **not** change (wire, suite, maturity, roadmap, etc.).

## 2. Finding index

| ID | Finding | Novelty | Existing owner | Disposition | Weekly finding candidate? |
|---|---|---|---|---|---|
| DYYYYMMDD-F01 | short title | new/refines/... | R-xxx / TD-xxx / phase / none | reproduce/... | yes/no |

Use stable per-report IDs such as `D20260826-F01`.

## 3. Repository context reviewed

Record only the repository surfaces needed to support the findings.

Separate:

- **changed since prior report**;
- **unchanged but controlling** (for example an open debt item);
- **direct implementation/spec/test anchors actually inspected**.

Link or name exact paths instead of restating large amounts of stable prose.

## 4. Findings

### DYYYYMMDD-F01 — Finding title

- **Novelty:** new | corroborates | refines | contradicts | supersedes
- **Source:**
- **Primary link:**
- **Source type / venue:**
- **Publication or release date:**
- **Verified claim:**
- **Existing owner:** R-xxx / TD-xxx / roadmap phase / spec section / none
- **Repository fact:** what is actually true in current ZK-ARCHE
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

## 5. Cross-source synthesis

State the combined conclusion once. Distinguish:

- corroboration;
- disagreement;
- unresolved assumptions;
- evidence that changes the prior decision boundary.

Avoid restating each source independently when they support the same conclusion.

## 6. Actionability matrix

| Target | Today’s change | Required next evidence | Destination |
|---|---|---|---|
| R-xxx / TD-xxx / none | status/evidence refinement | exact evidence | research backlog / weekly findings candidate / explicit human review |

This table is advisory. It does not create a request, roadmap change, ADR, spec requirement, or maturity claim.

## 7. Candidate backlog updates

List only material status/evidence changes to `../backlog.md` or distinct new research questions.

Explicitly say `none` when nothing should change.

## 8. Weekly handoff candidates

### Findings candidates

List conclusions suitable for consolidation into `../../findings/week-of-*-findings.md` during a separate human-reviewed weekly synthesis.

### Request candidates

Normally `none` for automated research. A request requires explicit human intent or another already-authorized engineering process.

## 9. Claim / no-change boundary

State what this report did **not** establish, for example:

- no mandatory-suite change;
- no wire-format change;
- no roadmap/spec/ADR change;
- no debt cleared;
- no maturity/readiness/certification claim changed.

## 10. Follow-up sources

Prioritize the primary sources that deserve deeper review next. Do not create a long generic reading list.
````

## Quality rules

The goal is not to maximize paper count or report length. Spend detail on sources that can change an architecture decision, security assumption, protocol mechanism, benchmark target, implementation priority, interoperability requirement, or research hypothesis.

Prefer these compression rules:

- If the repo is unchanged, state the controlling gaps once rather than rewriting the same paragraph every day.
- If a source only corroborates an existing backlog item, record the stronger evidence and avoid duplicating the entire prior rationale.
- If several sources converge on one engineering conclusion, synthesize them under one finding or cross-source conclusion.
- Preserve exact source provenance and limitations even when the executive summary is short.
- A smaller number of high-impact, well-supported findings is preferable to a large undifferentiated literature dump.

## Weekly handoff boundary

The daily research process may propose `weekly finding candidates`, but it remains write-limited to `docs/research/**` under `../PIPELINE.md`.

A separate human-reviewed process may consolidate daily research into:

```text
docs/findings/week-of-MM-DD-YYYY-findings.md
```

Explicit human-authorized work may then be recorded in:

```text
docs/requests/week-of-MM-DD-YYYY-request.md
```

Neither step is automatic research promotion.