# ZK-ARCHE Documentation Index

This is the documentation map for ZK-ARCHE. The structure intentionally follows a separation-of-concerns pattern: architecture defines boundaries, research captures external input, findings consolidate repository-grounded conclusions, requests capture explicit human-authorized work, roadmaps own planned improvement, ADRs preserve decisions, assurance owns evidence and claim limits, technical debt owns durable unresolved gaps, and release governance controls maturity language.

Start with the repository-level [README](../README.md) and the [architecture overview](architecture/architecture-overview.md).

## Research, findings, requests, and future improvement

- [Research archive](research/README.md) — primary-source/evidence intake contract and research promotion boundaries.
- [Research backlog](research/backlog.md) — unresolved questions, technologies, standards, papers, and implementation leads that are not yet engineering commitments.
- [Daily research](research/daily/) — dated source-level reports created only when research is actually performed.
- [Weekly findings](findings/) — consolidated repository-grounded conclusions, reproductions, measurements, reviews, and weekly research synthesis.
- [Weekly requests](requests/) — explicit human-authorized work, acceptance conditions, execution status, and week-level control context.
- [Improvement roadmap](roadmaps/improvement-roadmap.md) — canonical evidence-gated engineering plan.
- [RFC-style evolution plan](roadmaps/rfc-evolution-plan.md) — protocol/specification maturity plan.

Research is input, not authority. A paper, repository, RFC, draft, benchmark, or implementation does not become a ZK-ARCHE requirement merely because it appears in a daily report or research backlog. Weekly findings consolidate what the evidence means for the current repository; weekly requests record what a human has actually authorized to be worked. Roadmaps/ADRs/specification remain separate promotion steps.

## Assurance and security

- [Assurance and validation](assurance/assurance-and-validation.md) — threat model, security goals, validation runbook, replay tests, side-channel/RNG checklist, and external-review guidance.
- [Cross-language validation](assurance/cross-language-validation.md) — Rust/C deterministic-vector and release-qualification workflow.
- [`spec/security-considerations.md`](../spec/security-considerations.md) — normative security considerations staging document.
- [`spec/privacy-considerations.md`](../spec/privacy-considerations.md) — normative privacy considerations staging document.

## Decisions and governance

- [Architecture Decision Records](adr/) — point-in-time records for protocol, crypto, architecture, interoperability, or trust-model decisions.
- [Release governance](release/release-governance.md) — maturity states, evidence gates, and allowed claims.
- [Technical debt](technical-debt/README.md) — unresolved protocol, implementation, assurance, tooling, and documentation gaps plus clearing evidence.

## Implementation lanes

- [`rust/`](../rust/) — reference implementation and canonical deterministic vector source.
- [`c/`](../c/) — independent C11/libsodium constrained-device implementation and interoperability lane.
- [`scripts/`](../scripts/) — parent validation and release-qualification helpers.

## Directory guide

| Directory | Scope |
|---|---|
| `architecture/` | Stable component, repository, authority, and ownership boundaries. |
| `research/` | External research intake, daily reports, research backlog, source assessment, and advisory promotion metadata. |
| `findings/` | Weekly consolidated repository facts, reproductions, measurements, review outcomes, and research-to-repo synthesis. |
| `requests/` | Weekly explicit human intent, bounded work packets, acceptance conditions, status, and evidence requirements. |
| `roadmaps/` | Canonical future engineering and protocol-maturity plans. |
| `assurance/` | Threat model, validation, interoperability, evidence, and review guidance. |
| `adr/` | Point-in-time decisions that explain why architecture/protocol choices were made. |
| `release/` | Claim gates, maturity vocabulary, qualification requirements, and release governance. |
| `technical-debt/` | Explicit durable unresolved gaps; debt is tracked instead of being hidden in prose. |

## Canonical ownership rules

To prevent documentation drift, each kind of information has one primary owner:

| Information | Canonical owner |
|---|---|
| Project overview / quick start | `README.md` |
| Current roadmap entry point | `ROADMAP.md` |
| External research and candidate ideas | `docs/research/` |
| Consolidated repository findings / reproduced or measured outcomes | `docs/findings/` |
| Explicit human-authorized weekly work and acceptance criteria | `docs/requests/` |
| Planned long-term engineering work | `docs/roadmaps/` |
| Architecture/protocol decisions | `docs/adr/` |
| Stable architecture map | `docs/architecture/` |
| Normative protocol behavior | `spec/` |
| Rust implementation behavior | `rust/` + tests/vectors |
| C implementation behavior | `c/` + tests |
| Security/interop evidence policy | `docs/assurance/` |
| Durable known unresolved gaps | `docs/technical-debt/` |
| Maturity and release claims | `docs/release/` |
| Exact repository file list | Git tree, not a manually maintained Markdown/text inventory |

## Research-to-implementation flow

```text
External research source / new idea
        ↓
docs/research/daily/YYYY-MM-DD.md
        ↓
docs/research/backlog.md
        ↓  human-reviewed consolidation when repository implication is concrete
docs/findings/week-of-*-findings.md
        ↓  explicit human intent / authorized work
docs/requests/week-of-*-request.md
        ↓  long-term sequencing or consequential decision?
docs/roadmaps/ and/or docs/adr/
        ↓  normative behavior approved?
spec/
        ↓
rust/ + c/ + deterministic vectors/tests
        ↓
docs/assurance/ + retained evidence
        ↓
docs/release/
```

A finding may stop at any stage. Rejection, deferral, research-only classification, an explicit evidence gap, or a request that is later withdrawn are valid outcomes and should be recorded rather than converted into unsupported implementation or maturity claims.

## Automation boundary

The recurring daily research pipeline may read the full repository—including current findings and requests—to stay repo-aware, but its writes remain restricted to `docs/research/**` on `dev`. Weekly findings and requests are separate human-reviewed control layers and must not be generated by the daily research job merely because a source looks promising.