# ZK-ARCHE Daily Research Pipeline

This document defines the operating contract for recurring automated research on ZK-ARCHE.

The pipeline may inspect the full repository for context, but its write authority is restricted to `docs/research/**` on `dev`. The research backlog is a persistent **research-execution queue**; it is not an engineering implementation queue and it does not autonomously promote work into the roadmap, ADRs, specification, code, assurance claims, or release posture.

## Objective

Each daily run should continuously consume the highest-value unresolved research questions, gather and reconcile primary evidence, update the research queue, discover justified follow-on questions, and preserve traceable research provenance without silently changing ZK-ARCHE architecture or implementation commitments.

The workflow is:

```text
read current ZK-ARCHE dev state
        ↓
read backlog.md and prior daily reports
        ↓
select highest-priority unexhausted queue items
        ↓
inspect repository anchors for those items
        ↓
research current primary sources
        ↓
classify novelty vs prior evidence
        ↓
reproduce / benchmark / prototype where the research scope permits
        ↓
write delta-oriented daily report
        ↓
update backlog item status, evidence, next question, and last-reviewed date
        ↓
discover and enqueue distinct justified follow-on questions
        ↓
continue until the run budget is exhausted or no actionable queued item remains
        ↓
update research index
        ↓
one atomic research commit → one dev ref update
```

A run may consume more than one backlog item when time and evidence quality permit. It should prefer depth over paper count and should avoid repeatedly selecting an item that has no unresolved next-evidence question.

## Queue semantics

`docs/research/backlog.md` is the canonical persistent queue for recurring research.

Every active item must have:

- a stable `R-*` identifier;
- a concrete research question;
- a queue priority;
- a status;
- explicit evidence needed to advance or close the question;
- a potential destination if later promoted;
- a last-reviewed date;
- enough next-step detail that a later daily run can resume without rediscovering context.

### Queue priority

Use:

```text
P0 | P1 | P2 | P3
```

- `P0` — research directly blocks a security, conformance, formal-assurance, or claim boundary.
- `P1` — research blocks mandatory Common Contract, constrained-device, interoperability, lifecycle, or P2P maturity.
- `P2` — valuable core capability or evidence expansion after P0/P1 blockers.
- `P3` — optional, exploratory, migration, or research-only work that must not delay the mandatory baseline.

Daily selection order is normally P0 → P1 → P2 → P3, but a lower-priority item may be selected when it is dependency-unblocking, newly time-sensitive, or can be resolved efficiently with evidence already being reviewed.

### Status lifecycle

Use:

```text
queued
researching
reproduce
benchmark
prototype
promote
research-only
defer
reject
exhausted
```

`exhausted` means the current research question has no material unresolved research step under its present scope. It does **not** mean the associated engineering work is implemented or the destination is accepted.

An item may remain `reproduce`, `benchmark`, or `prototype` across multiple runs until its explicit evidence contract is satisfied.

### Queue consumption rule

At the start of each run:

1. load all non-terminal backlog items;
2. ignore `reject` and `exhausted` unless new contradictory/superseding evidence appears;
3. rank by priority, dependency value, staleness, and evidence gap;
4. select at least one highest-value actionable item;
5. record selected item IDs in the daily report;
6. research until the item can be advanced, refined, deferred, rejected, exhausted, or left active with a sharper next-evidence requirement;
7. only then move to the next queue item if run budget remains.

A run must not keep selecting the same active item merely because it is old. Each revisit should have a concrete unresolved question or new source trigger.

### Backlog replenishment

The pipeline should continuously replenish the queue when justified by:

- a distinct question discovered while resolving an existing item;
- a newly published primary source that changes the threat model or evidence contract;
- a repository change exposing a new research gap;
- a contradiction between implementation, specification, formal model, and external guidance;
- a benchmark or reproduction result that creates a separable follow-on question.

Do not create one backlog entry per paper. Consolidate sources under the engineering question they inform.

## Branch and access policy

### Read scope

The pipeline may read/search the entire repository on `dev`, including implementation, tests, vectors, formal models, `spec/`, architecture, roadmaps, assurance, ADRs, technical debt, findings, requests, release material, validation scripts, commit history, issues, and pull requests when relevant.

### Write scope

The recurring pipeline may create or update only:

```text
docs/research/**
```

Normal writable files are:

```text
docs/research/README.md
docs/research/backlog.md
docs/research/daily/YYYY-MM-DD.md
```

It must not write to `docs/findings/**`, `docs/requests/**`, `docs/technical-debt/**`, `docs/roadmaps/**`, `docs/adr/**`, `spec/**`, `rust/**`, `c/**`, `main`, repository settings, issues, PRs, or releases.

A daily research run may identify implementation or governance work, but it records that only as advisory research evidence or a promotion candidate.

## Repository-first research rule

Every run begins with current `dev` state. At minimum, inspect:

- `docs/research/README.md`;
- `docs/research/backlog.md`;
- the most recent daily reports;
- relevant weekly findings/requests;
- `docs/roadmaps/improvement-roadmap.md`;
- `docs/roadmaps/rfc-evolution-plan.md`;
- `docs/technical-debt/README.md`;
- `docs/assurance/assurance-and-validation.md`;
- relevant `spec/` and implementation/test/formal anchors;
- recent commits affecting selected queue items.

Research must target actual ZK-ARCHE gaps rather than generic cryptography or IoT news.

## Research channels

Prefer primary sources:

- IETF RFCs, Internet-Drafts, working-group material, and reference implementations;
- NIST standards/guidance;
- IACR ePrint and peer-reviewed cryptography work;
- IEEE, ACM, USENIX Security, NDSS, CCS, S&P, PETS, and similar venues;
- university/research-lab publications and code;
- official source repositories, releases, benchmarks, test suites, and hardware documentation;
- formal-methods primary sources for ProVerif, Tamarin, SAPIC+, and related tools.

Secondary sources may aid discovery, but substantive claims should be traced to the strongest available primary source.

## Priority research domains

The queue should continue to cover:

- privacy-preserving authentication/authorization;
- constrained Sigma/Schnorr-style proofs;
- anonymous/selective-disclosure credentials;
- unlinkability and metadata leakage;
- EDHOC, OSCORE, CoAP, DTLS/TLS/mTLS/exporters/channel binding;
- enrollment, commissioning, rekey, revocation, ownership transfer;
- replay, retry cookies, anti-amplification, resumption, state exhaustion, DoS;
- transcript design, downgrade/reflection/UKS resistance, strict state machines;
- vectors, differential testing, fuzzing, mutation testing, Rust/C interoperability;
- formal verification and model-to-code traceability;
- constant-time behavior, side channels, RNG/DRBG, secure storage, key lifecycle;
- STM32/ESP32-S3 CPU/RAM/flash/wire constraints;
- optional PQ/hybrid profiles when measured;
- P2P zero-trust, scoped delegation, revocation, mutual authentication;
- data sovereignty, policy-bound release, auditability, and privacy-preserving data access.

## Finding contract

Each meaningful finding must include:

1. stable per-report finding ID;
2. selected backlog item(s) affected;
3. novelty classification: `new | corroborates | refines | contradicts | supersedes`;
4. source title/type/date/primary link;
5. verified source-supported claim;
6. exact owner (`R-*`, `TD-*`, roadmap phase, spec section, or `none`);
7. concrete repository anchor;
8. problem addressed and strongest distinct engineering idea;
9. evidence maturity: `concept | formal | software | constrained-hardware | deployed | externally-reviewed`;
10. limitations/uncertainty;
11. ZK-ARCHE inference;
12. likely wire/CPU/RAM/flash/dependency/trust/privacy impact;
13. Rust/C/vector compatibility impact;
14. required next evidence;
15. recommended disposition;
16. whether it is a weekly-finding candidate.

Separate source-supported facts, repository facts, and ZK-ARCHE inference explicitly.

## Promotion boundary

Research may move a backlog item to `promote`, but `promote` means only **ready for explicit human engineering review**.

The pipeline must not directly edit roadmap/spec/code/ADR/assurance/debt/release/weekly request state.

Use:

```yaml
roadmap_impact:
  candidate_phase: null
  recommendation: investigate | reproduce | benchmark | prototype | promote | defer | reject | research-only
  evidence_maturity: concept | formal | software | constrained-hardware | deployed | externally-reviewed
  protocol_impact: none | compatible | extension | versioned-breaking-change
  required_next_evidence: null
  roadmap_action: none
  promotion_requirement: explicit human review
```

## Daily output contract

Every daily report must include:

- run identity and repository delta;
- **queue snapshot**: active counts by priority/status;
- **selected backlog items** and why they were chosen;
- concise executive synthesis;
- finding index;
- repository context inspected;
- detailed findings;
- cross-source synthesis;
- actionability matrix;
- **backlog transitions** (`old status → new status`, evidence gained, remaining question);
- **newly enqueued follow-ons**, if any;
- weekly finding/request handoff candidates;
- explicit claim/no-change boundary;
- next recommended queue items.

If no backlog transition is justified, state `none`; do not fabricate progress.

## Exhaustion behavior

When no actionable non-terminal backlog item remains:

1. confirm the queue is exhausted rather than merely stale;
2. run a bounded discovery pass across the priority research domains and recent primary sources;
3. enqueue only distinct questions with an explicit decision-changing evidence contract;
4. if no justified question is found, record `queue exhausted; no material new research question discovered` in the daily report.

The pipeline should never generate low-value backlog entries merely to keep itself busy.

## One-commit / one-ref-update rule

Each automated daily run may produce at most one Git commit and one `dev` ref update. The report, README index update, and backlog transitions for that run should be assembled atomically.

Recommended messages:

```text
research: add ZK-ARCHE daily research report YYYY-MM-DD
research: update ZK-ARCHE daily research report YYYY-MM-DD
```

If atomic publication fails, do not make unrelated changes.

## Verification after publication

Verify that:

- the report exists for the correct date;
- the research index links to it;
- selected backlog items and transitions match the report;
- newly enqueued items have a distinct question and evidence contract;
- no path outside `docs/research/**` changed;
- `docs/findings/**` and `docs/requests/**` were untouched;
- `main` was untouched;
- the daily run used one research commit / one `dev` ref update.
