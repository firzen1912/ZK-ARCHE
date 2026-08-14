# ZK-ARCHE Daily Research Pipeline

This document defines the operating contract for recurring automated research on ZK-ARCHE.

The pipeline is intentionally separated from protocol implementation. It may inspect the full repository to understand the current state, but its write authority is restricted to `docs/research/**` on the `dev` branch.

## Objective

Each daily run should identify high-value external developments that can materially improve ZK-ARCHE's privacy, security, interoperability, constrained-device feasibility, formal assurance, or protocol maturity, and translate them into traceable research input without silently changing the framework.

The workflow is:

```text
read current ZK-ARCHE dev state
        ↓
identify current gaps and active roadmap questions
        ↓
research current primary sources
        ↓
separate verified facts from ZK-ARCHE inference
        ↓
write daily report
        ↓
update research index
        ↓
update backlog only when justified
        ↓
one atomic commit → one dev ref update
```

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

It must never write to `main`, merge branches, rebase, force-push, publish releases, modify repository settings, or create/update pull requests or issues as part of the daily research run.

If a finding implies a change to code, tests, specifications, roadmaps, ADRs, CI, release governance, or assurance claims, record the recommendation in the daily report and, when justified, in `backlog.md` for explicit human promotion.

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
- `docs/roadmaps/improvement-roadmap.md`;
- `docs/roadmaps/rfc-evolution-plan.md`;
- `docs/technical-debt/README.md`;
- `docs/assurance/assurance-and-validation.md`;
- `spec/README.md` and relevant specification files;
- recent repository commits and implementation changes.

The purpose is to avoid generic cryptography news collection. Research should target actual ZK-ARCHE gaps, current implementation decisions, upcoming roadmap work, unresolved evidence needs, and recently changed protocol surfaces.

## Research channels

Prefer primary and authoritative sources. Daily runs should consider, where relevant:

- IETF RFCs, Internet-Drafts, working-group repositories, meeting material, and reference implementations;
- NIST standards, draft standards, guidance, and cryptographic-transition material;
- IACR ePrint and peer-reviewed CRYPTO/EUROCRYPT/ASIACRYPT work;
- IEEE, ACM, USENIX Security, NDSS, CCS, S&P, PETS, and related venues;
- official university or research-lab publications and code;
- official GitHub repositories, releases, commits, issues, benchmarks, and test suites;
- official documentation for libsodium, Rust cryptography libraries, embedded targets, RTOS platforms, secure elements, and MCU cryptographic accelerators;
- formal-methods tools and primary research for ProVerif, Tamarin, symbolic protocol analysis, and mechanized proof where relevant.

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

1. source title and source type;
2. publication/release/update date;
3. primary-source link or citation;
4. verified source-supported claim;
5. problem or bottleneck addressed;
6. strongest distinct engineering idea;
7. evidence maturity;
8. limitations and uncertainty;
9. direct ZK-ARCHE relevance;
10. likely impact on wire format, CPU, RAM, flash, dependencies, trust model, or privacy where applicable;
11. compatibility impact on Rust, C, deterministic vectors, and existing profiles;
12. recommended disposition.

Use the evidence maturity vocabulary:

```text
concept | formal | software | constrained-hardware | deployed | externally-reviewed
```

Use the disposition vocabulary:

```text
investigate | reproduce | benchmark | prototype | promote | defer | reject | research-only
```

## Promotion boundary

Research output is advisory. It must not directly edit the roadmap, specification, code, ADRs, assurance claims, or release posture.

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

A report may identify a likely roadmap phase, ADR need, spec section, test requirement, or benchmark, but only explicit subsequent human-reviewed work may modify those artifacts.

## Daily output contract

The daily report must follow `docs/research/daily/README.md` and should end with a synthesis containing:

- what is genuinely new relative to previous reports;
- what matters most to the current ZK-ARCHE implementation;
- what should be investigated or benchmarked next;
- what should remain research-only;
- which existing backlog items gained or lost support;
- the highest-priority primary sources for deeper follow-up.

Avoid padding. A smaller number of high-impact, well-supported findings is preferable to a large undifferentiated literature dump.

## Backlog update rule

Update `docs/research/backlog.md` only when a finding materially changes the status of an existing question or establishes a distinct new research question.

Do not create duplicate backlog entries for every paper or release. Consolidate multiple sources under the same engineering question where appropriate.

## Verification after publication

After updating `dev`, verify:

- the new commit's parent is the previous `dev` head;
- the daily report exists for the correct date;
- the research index links to it;
- any backlog modification is supported by the report;
- no file outside `docs/research/**` changed;
- `main` was not modified;
- only one commit and one `dev` ref update were used for that daily run.
