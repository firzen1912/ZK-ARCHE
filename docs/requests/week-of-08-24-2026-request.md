# Requests — week of 2026-08-24

> **Primary theme: research-to-engineering traceability and documentation control.** This week’s explicit request is documentation/governance work only. No cryptographic primitive, protocol wire behavior, roadmap phase, maturity claim, or implementation requirement is being promoted by this file.

## 0. How to use this file

**Human coder:** read §§1–6 and §10 first. Use §7 for exact documentation constraints.

**Coding agent:** read §§1–4, own one bounded documentation packet, and use the current repository as authority. Do not interpret research findings as permission to modify protocol/spec/code unless a separate explicit request authorizes that work.

Related records:

- current findings: `../findings/week-of-08-24-2026-findings.md`
- prior synthesized findings: `../findings/week-of-08-17-2026-findings.md`
- research archive: `../research/README.md`
- research backlog: `../research/backlog.md`
- active technical debt: `../technical-debt/README.md`
- improvement roadmap: `../roadmaps/improvement-roadmap.md`

Current source/tests/vectors, `spec/`, assurance evidence, technical debt, ADRs, and later explicit human decisions override stale request text.

---

## 1. Week at a glance

| Field | State |
|---|---|
| North star | Keep daily research high-fidelity while making conclusions and requested work faster for humans and coding agents to consume |
| Human request | review current ZK-ARCHE daily research, improve its content model, and add HIVEAS-style findings/requests folders |
| Scope | documentation/governance only |
| Branch | `dev` |
| Historical-report rule | preserve existing dated research reports as provenance; improve the forward template rather than rewriting history |
| Research-automation rule | recurring daily research remains write-limited to `docs/research/**` |
| Promotion rule | research/findings do not become implementation scope without explicit human authorization |
| Protocol/wire impact | none |
| Maturity impact | none |

---

## 2. Pass condition and status vocabulary

This documentation request is **Done** only when all of the following are true:

```text
CURRENT REPO + DAILY ARCHIVE AUDITED
+ HIVEAS FINDINGS/REQUESTS PATTERN COMPARED
+ docs/findings/ CREATED WITH STABLE WEEKLY CONTRACT
+ docs/requests/ CREATED WITH STABLE WEEKLY CONTRACT
+ PRIOR RESEARCH CONSOLIDATED WITHOUT ALTERING HISTORY
+ FUTURE DAILY TEMPLATE MADE DELTA/ACTION ORIENTED
+ DAILY AUTOMATION LEAST-WRITE BOUNDARY PRESERVED
+ DOCS NAVIGATION/OWNERSHIP UPDATED
= DONE
```

Status vocabulary follows `README.md` in this directory.

---

## 3. Scoreboard

| ID | Objective | Status | Acceptance focus |
|---|---|---|---|
| Z24-01 | Audit daily research 2026-08-15 through 2026-08-25 | Done | identify repeated context, evidence strengths, and missing handoff layer |
| Z24-02 | Compare HIVEAS findings/request workflow | Done | preserve useful weekly control-board separation without copying HIVEAS-specific mission semantics |
| Z24-03 | Add weekly findings workflow | Done | stable README + synthesized weekly files + evidence/claim boundaries |
| Z24-04 | Add weekly request workflow | Done | explicit human intent, pass conditions, execution packets, evidence and closeout |
| Z24-05 | Improve future daily research format | Done | delta/novelty classification, concise repo context, finding index, actionability/weekly handoff |
| Z24-06 | Preserve research automation boundary | Done | no automatic writes to findings/requests; handoff candidates only |
| Z24-07 | Update documentation navigation and ownership | Done | research/findings/requests/debt/roadmap roles are unambiguous |

---

## 4. Priority execution packets

### P0 — Separate evidence, findings, and requests

Establish these distinct ownership layers:

```text
external source / research hypothesis
        ↓
docs/research/daily/
        ↓
docs/research/backlog.md
        ↓ human-reviewed consolidation
docs/findings/week-of-*-findings.md
        ↓ explicit human intent
docs/requests/week-of-*-request.md
        ↓
implementation / spec / assurance work
        ↓
findings + debt + roadmap/ADR/spec as appropriate
```

The arrows are not automatic promotions. Each boundary requires the evidence or authorization described by its owner.

### P1 — Make future daily reports delta-oriented

Future daily reports should explicitly state whether each item is **new**, **corroborating**, **refining**, **contradicting**, or **superseding** prior research. If the repository is unchanged, say so once and link to the relevant current state instead of reproducing the same long context block.

### P2 — Keep weekly documents compact enough to drive work

The weekly findings file should collapse duplicate daily conclusions. The weekly request file should expose status, acceptance conditions, and bounded packets near the top. Deep technical provenance remains linked in research/spec/tests rather than copied repeatedly.

---

## 5. Human request / intent timeline

| Date | Human intent | Resulting requirement |
|---|---|---|
| 2026-08-25 | Review the current state of `ZK-ARCHE/dev/docs/research/daily` and improve the content | audit the full dated series and improve future report structure without erasing provenance |
| 2026-08-25 | Look at HIVEAS and create similar findings/requests folders in ZK-ARCHE | add weekly `docs/findings/` and `docs/requests/` control layers adapted to a cryptographic/protocol framework |

---

## 6. Problem → diagnosis → fix / remaining index

| Problem / risk | Diagnosis | Required response |
|---|---|---|
| Daily reports are long and repeat stable repo state | source discipline is strong, but unchanged context is reproduced every day | make future reports delta-oriented and link stable context |
| External research and repository conclusions are mixed | daily report currently carries source claims, ZK inference, backlog movement, and action candidates together | add weekly findings as the repository-grounded consolidation layer |
| Candidate work can look like implied engineering scope | `roadmap_impact` metadata is useful but can read like a task list | add explicit weekly requests owned by human intent; research automation cannot create them |
| Agents may reread many 15–25 KB daily reports | there is no compact weekly synthesis/control surface | read weekly findings first, then jump to cited daily/source material only as needed |
| Evidence vocabulary is slightly inconsistent | research docs used variants such as `hardware`, `constrained-hardware`, and `externally reviewed` | normalize future contracts to `concept | formal | software | constrained-hardware | deployed | externally-reviewed` |
| Workflow change could weaken least-write automation | daily pipeline currently protects the repo by writing only `docs/research/**` | preserve that boundary and make findings/requests a separate human-reviewed process |

No implementation or protocol problem is authorized for correction by this request.

---

## 7. Detailed requirements and engineering notes

### 7.1 Preserve research history

Do not rewrite the 2026-08-15 through 2026-08-25 daily reports merely to make them match the new template. They are dated research provenance. Consolidate them into weekly findings and improve the template for subsequent reports.

### 7.2 Findings are conclusions, not a second research archive

A finding must identify the repository fact or reproduced/retained evidence that makes the conclusion relevant. Multiple daily reports supporting the same conclusion should become one weekly finding with multiple provenance links.

A standards draft, paper, or vendor document can support a finding, but external text by itself remains research until the repository implication is concrete.

### 7.3 Requests require human intent

A request may point to R-*, TD-*, roadmap phases, specs, or findings, but it must define what is actually being asked and how completion is evidenced. Do not generate a request just because an item is interesting, `reproduce`, `benchmark`, or `prototype` in the research backlog.

### 7.4 Security-sensitive boundaries remain intact

This documentation workflow does not relax checkpoint-style review for proof semantics, transcript/domain separation, wire grammar, replay/resumption, authorization/trust behavior, parser acceptance, RNG/key lifecycle, C memory safety, unsafe Rust, or Rust/C interoperability.

### 7.5 Future daily report compression

A future report should be able to answer quickly:

- what changed in the repo since the last report;
- what is genuinely new in external evidence;
- what merely corroborates/refines an existing R-/TD- question;
- what exact repository surface is affected;
- what evidence would change the decision;
- whether the item is a candidate weekly finding;
- whether explicit human review is needed before any request exists.

The detailed source-level field contract remains available for high-value findings.

---

## 8. Validation and evidence

Validation for this request is documentation-structural:

- `docs/findings/README.md` defines weekly finding ownership and evidence fields.
- `docs/findings/week-of-08-17-2026-findings.md` consolidates the prior full research week.
- `docs/findings/week-of-08-24-2026-findings.md` consolidates the current week to date.
- `docs/requests/README.md` defines weekly request/control-board ownership.
- this file records the explicit human request that authorized the documentation change.
- `docs/research/daily/README.md` defines the improved future daily format.
- `docs/research/PIPELINE.md` retains least-write daily automation and adds a weekly handoff boundary.
- documentation indexes/ownership maps link the new directories.

No Rust, C, deterministic vector, formal model, spec, ADR, roadmap, assurance, release, technical-debt status, or maturity claim is changed by this request.

---

## 9. Session update record

### 2026-08-25

- Audited the dated research archive from 2026-08-15 through 2026-08-25.
- Confirmed the recurring controlling debt remains TD-001 through TD-004.
- Compared HIVEAS `docs/findings/` and `docs/requests/`, especially its weekly request control-board format.
- Chose not to rewrite historical daily reports.
- Added a weekly synthesis layer and a human-intent request layer.
- Tightened the future daily report contract around delta, novelty, provenance, and handoff.

---

## 10. Week-end closeout / carry-forward

This documentation request is complete when the atomic `dev` update containing the files above is verified.

Carry-forward rule for future weeks:

- do not create a weekly request file unless explicit human intent/authorized work exists;
- do not create empty future findings files;
- let recurring daily research continue writing only under `docs/research/**`;
- consolidate repeated evidence into findings rather than expanding the research archive into a de facto roadmap;
- promote technical work only through explicit requests and the repository’s existing review/evidence gates.