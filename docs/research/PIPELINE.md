# ZK-ARCHE Daily Research Pipeline

This document defines the operating contract for recurring automated research on ZK-ARCHE.

The pipeline is intentionally separated from protocol implementation. It may inspect the full repository to understand the current state, but its write authority is restricted to `docs/research/**` on the `dev` branch.

## Objective

Each daily run should identify high-value external developments that can materially improve ZK-ARCHE's privacy, security, interoperability, constrained-device feasibility, formal assurance, or protocol maturity, and translate them into traceable research input **without silently changing the framework or creating implementation commitments**.

The workflow is:

```text
read current ZK-ARCHE dev state
        ↓
identify current gaps and active roadmap/debt questions
        ↓
research current primary sources
        ↓
classify novelty vs prior research
        ↓
separate verified facts from ZK-ARCHE inference
        ↓
write delta-oriented daily report
        ↓
update research index
        ↓
update backlog only when justified
        ↓
one atomic commit → one dev ref update
        ↓
optional weekly finding candidates for later human review
```

The daily automation stops at the research layer. It does not create or update weekly findings or requests.

## Branch and access policy

### Read scope

The pipeline may read and search the entire `firzen1912/ZK-ARCHE` repository on `dev`, including:

- Rust and C implementation code;
- tests, fuzzers, deterministic vectors, and formal models;
- `spec/`;
- `docs/architecture/`;
- `docs/roadmaps/`;
- `docs/assurance/`;
- `docs/adr/`;
- `docs/technical-debt/`;
- `docs/findings/`;
- `docs/requests/`;
- `docs/release/`;
- CI workflows and validation scripts;
- commit, issue, and pull-request history when relevant.

This access is for context only outside the research namespace.

### Write scope

The pipeline may create or update files only under:

```text
docs/research/**
```

Normal writable files are:

```text
docs/research/README.md
docs/research/backlog.md
docs/research/daily/YYYY-MM-DD.md
```

The pipeline must never create, modify, move, rename, overwrite, or delete anything outside `docs/research/**`.

In particular, the recurring daily pipeline must not write:

```text
docs/findings/**
docs/requests/**
docs/technical-debt/**
docs/roadmaps/**
docs/adr/**
spec/**
rust/**
c/**
```

It must never write to `main`, merge branches, rebase, force-push, publish releases, modify repository settings, or create/update pull requests or issues as part of the daily research run.

If a finding implies a change to code, tests, specifications, roadmaps, ADRs, CI, release governance, weekly findings, weekly requests, or assurance claims, record the recommendation in the daily report and, when justified, in `backlog.md` for explicit human promotion.

## One-commit / one-ref-update rule

Each daily run may produce at most one Git commit and one update of `refs/heads/dev`.

Before writing, the automation must:

1. read the current `dev` head;
2. inspect the current repository state relevant to the day's research;
3. read every research file that may be changed;
4. build the complete final contents of the daily report, index, and backlog before any write;
5. create the required blobs and one replacement tree;
6. create one commit with the previous `dev` head as its parent;
7. update the `dev` ref once without force;
8. verify that the resulting diff contains only `docs/research/**` paths.

Recommended commit message:

```text
research: add ZK-ARCHE daily research report YYYY-MM-DD
```

If the report already exists and a same-day rerun is deliberately performed:

```text
research: update ZK-ARCHE daily research report YYYY-MM-DD
```

If safe atomic publication cannot be completed, make no unrelated repository changes and return the research results with the GitHub-write failure stated clearly.

## Repository-first research rule

Every run must begin by inspecting the current `dev` state before searching externally.

At minimum, review the material most likely to determine the current research priorities:

- `docs/research/README.md`;
- `docs/research/backlog.md`;
- the most recent daily research reports;
- the current weekly findings/request files if they exist;
- `docs/roadmaps/improvement-roadmap.md`;
- `docs/roadmaps/rfc-evolution-plan.md`;
- `docs/technical-debt/README.md`;
- `docs/assurance/assurance-and-validation.md`;
- `spec/README.md` and relevant specification files;
- recent repository commits and implementation changes.

The purpose is to avoid generic cryptography news collection. Research should target actual ZK-ARCHE gaps, current implementation decisions, upcoming roadmap work, unresolved evidence needs, and recently changed protocol surfaces.

## Delta and compression discipline

The daily report should preserve evidence while minimizing repeated context.

If there is no meaningful repository change since the previous report:

- say so once in the run-identity/repository-delta section;
- name the still-controlling `R-*`/`TD-*` items;
- do not reproduce a long unchanged repository inventory merely for completeness.

For each source-supported finding, classify its relationship to prior research:

```text
new | corroborates | refines | contradicts | supersedes
```

Use `corroborates` when a source adds support but does not change the engineering decision. Use `refines` when it changes the threat model, evidence contract, constraints, or next experiment. Use `contradicts`/`supersedes` only when the report explicitly reconciles the older conclusion.

Do not repeatedly restate an older source or finding unless the new evidence changes its status, limitations, destination, or required evidence.

## Research channels

Prefer primary and authoritative sources. Daily runs should consider, where relevant:

- IETF RFCs, Internet-Drafts, working-group repositories, meeting material, and reference implementations;
- NIST standards, draft standards, guidance, and cryptographic-transition material;
- IACR ePrint and peer-reviewed CRYPTO/EUROCRYPT/ASIACRYPT work;
- IEEE, ACM, USENIX Security, NDSS, CCS, S&P, PETS, and related venues;
- official university or research-lab publications and code;
- official GitHub repositories, releases, commits, issues, benchmarks, and test suites;
- official documentation for libsodium, Rust cryptography libraries, embedded targets, RTOS platforms, secure elements, and MCU cryptographic accelerators;
- formal-methods tools and primary research for ProVerif, Tamarin, SAPIC+, symbolic protocol analysis, and mechanized proof where relevant.

Secondary articles may help discovery, but claims promoted into the report should be traced to the strongest available primary source.

## Priority research domains

Daily research should prioritize distinct developments in:

- privacy-preserving device authentication and authorization;
- constrained Sigma/Schnorr-style proof systems;
- anonymous credentials and selective disclosure;
- unlinkability and metadata-leakage reduction;
- EDHOC, OSCORE, CoAP, DTLS 1.3, TLS 1.3, mTLS, exporters, and channel binding;
- IoT enrollment, commissioning, ownership transfer, late enrollment, rekey, and revocation;
- replay protection, retry cookies, anti-amplification, resumption, state exhaustion, and DoS resistance;
- transcript design, downgrade resistance, reflection/UKS resistance, and strict state machines;
- deterministic vectors, differential testing, fuzzing, mutation testing, and implementation interoperability;
- formal verification, symbolic analysis, and implementation-to-model traceability;
- constant-time implementation, side-channel hardening, RNG/DRBG design, secure storage, and key lifecycle;
- STM32 and ESP32-S3 CPU/RAM/flash/wire-size constraints;
- Rust/C cryptographic implementation safety and portability;
- optional post-quantum or hybrid profiles when constrained-device measurements exist;
- P2P zero-trust trust graphs, scoped delegation, revocation epochs, and mutual authentication;
- privacy-preserving data sovereignty and policy-bound release.

## Finding contract

Each meaningful finding must include:

1. stable per-report finding ID;
2. novelty classification relative to prior research;
3. source title and source type;
4. publication/release/update date;
5. primary-source link or citation;
6. verified source-supported claim;
7. exact existing owner when one exists (`R-*`, `TD-*`, roadmap phase, spec section, or `none`);
8. concrete repository fact or implementation/spec/test anchor;
9. problem or bottleneck addressed;
10. strongest distinct engineering idea;
11. evidence maturity;
12. limitations and uncertainty;
13. direct ZK-ARCHE relevance;
14. likely impact on wire format, CPU, RAM, flash, dependencies, trust model, or privacy where applicable;
15. compatibility impact on Rust, C, deterministic vectors, and existing profiles;
16. required next evidence;
17. recommended disposition;
18. whether the item is a weekly finding candidate.

Use the evidence maturity vocabulary:

```text
concept | formal | software | constrained-hardware | deployed | externally-reviewed
```

Use the disposition vocabulary:

```text
investigate | reproduce | benchmark | prototype | promote | defer | reject | research-only
```

Separate **source-supported facts**, **repository facts**, and **ZK-ARCHE inference** explicitly.

## Promotion boundary

Research output is advisory. It must not directly edit the roadmap, specification, code, ADRs, assurance claims, release posture, weekly findings, weekly requests, or technical-debt status.

Each potentially actionable finding should include:

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

A report may identify a likely roadmap phase, ADR need, spec section, test requirement, benchmark, or weekly finding candidate, but only explicit subsequent human-reviewed work may modify those artifacts.

## Weekly handoff boundary

ZK-ARCHE now has separate weekly layers:

```text
docs/findings/week-of-MM-DD-YYYY-findings.md
docs/requests/week-of-MM-DD-YYYY-request.md
```

The daily automation may **read** these files for context but may not write them.

A daily report may list:

- **weekly finding candidates** — conclusions that a later human-reviewed synthesis could consolidate into `docs/findings/`;
- **request candidates** — normally `none`; a request requires explicit human intent or another already-authorized engineering process.

The weekly findings process should collapse duplicate daily conclusions and retain exact provenance. The weekly request process should record only explicitly authorized work with acceptance conditions and evidence requirements.

This separation is deliberate: recurring research must not become a de facto roadmap or autonomous work queue.

## Daily output contract

The daily report must follow `docs/research/daily/README.md` and should contain:

- run identity and repository delta;
- a concise executive synthesis;
- a finding index with novelty, owner, disposition, and weekly-handoff classification;
- only the repository context actually needed to support the findings;
- detailed source-supported findings;
- cross-source synthesis;
- an actionability matrix;
- justified backlog updates or an explicit `none`;
- weekly finding/request handoff candidates;
- an explicit no-change/claim boundary;
- high-priority follow-up primary sources.

The report should make clear:

- what is genuinely new relative to previous reports;
- what only corroborates/refines existing backlog work;
- what matters most to the current ZK-ARCHE implementation;
- what should be investigated or benchmarked next;
- what should remain research-only;
- which existing backlog items gained or lost support;
- what this report did **not** change.

Avoid padding. A smaller number of high-impact, well-supported findings is preferable to a large undifferentiated literature dump.

## Backlog update rule

Update `docs/research/backlog.md` only when a finding materially changes the status/evidence contract of an existing question or establishes a distinct new research question.

Do not create duplicate backlog entries for every paper or release. Consolidate multiple sources under the same engineering question where appropriate.

When a finding only corroborates an existing item, update the row only if the new source changes the evidence required, status, destination, or review date in a meaningful way.

## Verification after publication

After updating `dev`, verify:

- the new commit's parent is the previous `dev` head;
- the daily report exists for the correct date;
- the research index links to it;
- any backlog modification is supported by the report;
- no file outside `docs/research/**` changed;
- `docs/findings/**` and `docs/requests/**` were not modified by the daily run;
- `main` was not modified;
- only one commit and one `dev` ref update were used for that daily run.