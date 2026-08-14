# ZK-ARCHE Documentation Index

This is the documentation map for ZK-ARCHE. The structure intentionally follows the management pattern used by HIVEAS: architecture defines boundaries, research captures external input, roadmaps own planned improvement, ADRs preserve decisions, assurance owns evidence and claim limits, and release governance controls maturity language.

Start with the repository-level [README](../README.md) and the [architecture overview](architecture/architecture-overview.md).

## Architecture

- [Architecture overview](architecture/architecture-overview.md) — repository map, implementation lanes, authority hierarchy, protocol/spec boundaries, and documentation ownership.
- [`spec/`](../spec/) — normative protocol staging package. Design notes in `docs/` do not override `spec/`.

## Research and future improvement

- [Research archive](research/README.md) — source/evidence contract and promotion rules.
- [Research backlog](research/backlog.md) — queued questions, technologies, standards, papers, and implementation leads.
- [Daily research](research/daily/) — dated reports created only when research is actually performed.
- [Improvement roadmap](roadmaps/improvement-roadmap.md) — canonical evidence-gated engineering plan.
- [RFC-style evolution plan](roadmaps/rfc-evolution-plan.md) — protocol/specification maturity plan.

Research is input, not authority. A paper, repository, RFC, draft, benchmark, or implementation does not become a ZK-ARCHE requirement until it is explicitly promoted into a roadmap/ADR/spec change.

## Assurance and security

- [Assurance and validation](assurance/assurance-and-validation.md) — threat model, security goals, validation runbook, replay tests, side-channel/RNG checklist, and external-review guidance.
- [Cross-language validation](assurance/cross-language-validation.md) — Rust/C deterministic-vector and release-qualification workflow.
- [`spec/security-considerations.md`](../spec/security-considerations.md) — normative security considerations staging document.
- [`spec/privacy-considerations.md`](../spec/privacy-considerations.md) — normative privacy considerations staging document.

## Decisions and governance

- [Architecture Decision Records](adr/) — immutable point-in-time records for protocol, crypto, architecture, interoperability, or trust-model decisions.
- [Release governance](release/release-governance.md) — maturity states, evidence gates, and allowed claims.
- [Technical debt](technical-debt/README.md) — unresolved protocol, implementation, assurance, tooling, and documentation gaps.

## Implementation lanes

- [`rust/`](../rust/) — reference implementation and canonical deterministic vector source.
- [`c/`](../c/) — independent C11/libsodium constrained-device implementation and interoperability lane.
- [`scripts/`](../scripts/) — parent validation and release-qualification helpers.

## Directory guide

| Directory | Scope |
|---|---|
| `architecture/` | Stable component, repository, authority, and ownership boundaries. |
| `research/` | External research intake, daily reports, research queue, source assessment, and promotion candidates. |
| `roadmaps/` | Canonical future engineering and protocol-maturity plans. |
| `assurance/` | Threat model, validation, interoperability, evidence, and review guidance. |
| `adr/` | Point-in-time decisions that explain why architecture/protocol choices were made. |
| `release/` | Claim gates, maturity vocabulary, qualification requirements, and release governance. |
| `technical-debt/` | Explicit unresolved gaps; debt is tracked instead of being hidden in prose. |

## Canonical ownership rules

To prevent documentation drift, each kind of information has one primary owner:

| Information | Canonical owner |
|---|---|
| Project overview / quick start | `README.md` |
| Current roadmap entry point | `ROADMAP.md` |
| External research and candidate ideas | `docs/research/` |
| Planned engineering work | `docs/roadmaps/` |
| Architecture/protocol decisions | `docs/adr/` |
| Stable architecture map | `docs/architecture/` |
| Normative protocol behavior | `spec/` |
| Rust implementation behavior | `rust/` + tests/vectors |
| C implementation behavior | `c/` + tests |
| Security/interop evidence policy | `docs/assurance/` |
| Known unresolved gaps | `docs/technical-debt/` |
| Maturity and release claims | `docs/release/` |
| Exact repository file list | Git tree, not a manually maintained Markdown/text inventory |

## Research-to-implementation flow

```text
Research source / new idea
        ↓
docs/research/daily/YYYY-MM-DD.md
        ↓
docs/research/backlog.md
        ↓  explicit promotion
docs/roadmaps/
        ↓  decision required?
docs/adr/NNNN-*.md
        ↓  normative behavior
spec/
        ↓
rust/ + c/
        ↓
docs/assurance/ + evidence/
        ↓
docs/release/
```

A finding may stop at any stage. Rejection, deferral, research-only classification, or an explicit evidence gap are valid outcomes and should be recorded rather than converted into unsupported implementation work.
