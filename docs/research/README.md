# ZK-ARCHE Research Archive

This directory is the intake layer for recurring external research relevant to ZK-ARCHE. Its purpose is to turn standards, papers, implementations, benchmarks, formal-analysis results, and hardware/security developments into traceable engineering input **without silently converting external work into ZK-ARCHE requirements**.

## Structure

```text
docs/research/
├── README.md
├── PIPELINE.md
├── backlog.md
└── daily/
    ├── README.md
    ├── YYYY-MM-DD.md
    └── ...
```

Related sibling control layers are deliberately outside the daily automation write scope:

```text
docs/findings/   # weekly repository-grounded conclusions/reproductions/reviews
docs/requests/   # weekly explicit human-authorized work and acceptance conditions
```

A dated report should be created only after research for that date has actually been performed. Do not pre-create empty future reports.

## Documentation ownership boundary

ZK-ARCHE uses four distinct layers so research does not turn into an autonomous work queue:

| Layer | Owns | Does not own |
|---|---|---|
| `docs/research/` | external evidence, hypotheses, source provenance, unresolved research questions | implementation commitments or maturity claims |
| `docs/findings/` | consolidated repository-grounded conclusions, reproduced/measured/review outcomes | human work authorization or long-term sequencing |
| `docs/requests/` | explicit requested work, acceptance criteria, execution status/control | durable architecture ownership or evidence-free completion claims |
| `docs/technical-debt/` | durable reproducible gaps and clearing evidence | weekly task planning |

`docs/roadmaps/`, `docs/adr/`, `spec/`, implementation/tests, assurance evidence, and release governance retain their existing authority.

## Research scope

Research should prioritize developments that can materially affect ZK-ARCHE architecture, security, privacy, interoperability, constrained-device feasibility, or specification maturity, including privacy-preserving authentication/authorization; constrained Sigma/Schnorr proofs; anonymous/selective-disclosure credentials; EDHOC/OSCORE/CoAP/TLS/DTLS and channel binding; enrollment/rekey/revocation; replay/DoS/resumption; transcript and state-machine design; vectors/differential testing/fuzzing; formal verification; embedded cryptography/RNG/storage/side channels; Rust/C implementation; STM32/ESP32 constraints; optional measured PQ/hybrid profiles; data sovereignty; and P2P zero-trust delegation/revocation.

## Source discipline

Prefer primary and authoritative sources: IETF RFCs and drafts, NIST guidance, peer-reviewed security/cryptography venues, clearly labeled preprints, official research/project repositories, formal-methods projects, and original hardware/library documentation. Secondary sources are discovery aids; material claims should be traced to primary evidence.

## Daily finding contract

Future daily reports follow [`daily/README.md`](daily/README.md). Each material finding records a stable finding ID and novelty class; source/date/type/link; verified claim; exact R-/TD-/phase/spec owner; repository anchor; problem and engineering idea; evidence maturity; limitations; likely resource/trust/privacy impact; Rust/C/vector compatibility; required next evidence; disposition; and weekly-finding candidacy. Source-supported facts, repository facts, and ZK-ARCHE inference must remain explicit.

## Promotion boundary

Research does not directly become a protocol requirement or engineering request.

```text
external source / research idea
        ↓
docs/research/daily/YYYY-MM-DD.md
        ↓
docs/research/backlog.md
        ↓  human-reviewed consolidation
docs/findings/week-of-*-findings.md
        ↓  explicit human intent
docs/requests/week-of-*-request.md
        ↓
docs/roadmaps/ and/or docs/adr/
        ↓
spec + vectors/tests
        ↓
Rust/C implementation and validation
        ↓
docs/assurance/ + retained evidence
        ↓
docs/release/
```

A finding may stop at any stage. Rejection, deferral, research-only classification, or an explicit evidence gap are valid outcomes. Promotion always requires explicit human review; daily research must not claim that roadmap/spec/ADR/debt/maturity state changed unless that separate artifact was deliberately updated through its governing process.

## Research backlog

[`backlog.md`](backlog.md) is the persistent research-execution queue, not a roadmap or engineering request queue. When evidence becomes actionable, repository implications should first be consolidated through the human-reviewed findings/request/promotion path rather than copied directly into normative artifacts.

## Automated daily pipeline

The recurring research workflow is governed by [`PIPELINE.md`](PIPELINE.md). It may read the full `dev` repository but may write only `docs/research/**`; it must never write `main`, findings, requests, roadmaps, ADRs, specs, implementation, assurance, debt, CI, issues, PRs, or releases. Each completed run may create at most one commit and one `dev` ref update. The report, index, and any justified backlog transition are assembled atomically, then verified so no path outside `docs/research/**` changed.

## Daily-report quality direction

Historical reports remain dated provenance. New reports should be delta-oriented, map findings to exact owners, synthesize convergent sources, include actionability and claim/no-change boundaries, and identify weekly-finding candidates without autonomously creating them.

## Index

| Date | Report | Focus | Promotion status |
|---|---|---|---|
| 2026-09-15 | [Daily research](daily/2026-09-15.md) | downgrade-resistant negotiation: selected-tuple vs preference/offer integrity; forged retry-hint policy isolation; authenticated same-conversation fallback evidence | reproduce / benchmark; explicit review required |
| 2026-09-14 | [Daily research](daily/2026-09-14.md) | semantic association vs rotatable wire-visible session handles; path validation without identity promotion; full-AUTH fallback as fresh authentication/privacy context | reproduce / prototype; explicit review required |
| 2026-09-13 | [Daily research](daily/2026-09-13.md) | stateful formal modeling of authorized trust mutation; verify-then-commit constrained trust replacement; internal-vs-external error/privacy separation | reproduce; explicit review required |
| 2026-09-12 | [Daily research](daily/2026-09-12.md) | replay-continuity detector vs response evidence; key-generation transition isolation; fresh AUTH/rebinding non-substitution for stale authorization/revocation authority | reproduce; explicit review required |
| 2026-09-11 | [Daily research](daily/2026-09-11.md) | fresh channel rebinding during resumption; resumption-credential lineage vs new traffic-key epoch; cumulative full-AUTH/resumption-lineage lifetime | reproduce; explicit review required |
| 2026-09-10 | [Daily research](daily/2026-09-10.md) | dedicated per-AUTH TLS exporter binding vs RFC 9266 fixed channel binding; exporter-secret generation/currentness separation for long-lived connections | reproduce; explicit review required |
| 2026-09-09 | [Daily research](daily/2026-09-09.md) | terminal AUTH consume-before-verify state-deletion DoS on spoofable transports; forgery-resistance-normalized terminal disposition | reproduce; explicit review required |
| 2026-09-08 | [Daily research](daily/2026-09-08.md) | AUTH-v3 critical-extension semantic processing vs ID recognition; canonical singleton/order-independent occurrence boundary and versioning rule | reproduce; explicit review required |
| 2026-09-07 | [Daily research](daily/2026-09-07.md) | DATA audit claim boundary: local hash-chain integrity vs durable rollback resistance/external consistency; privacy-minimized audit/transparency evidence | reproduce / investigate; explicit review required |
| 2026-09-06 | [Daily research](daily/2026-09-06.md) | association-scoped resumption privacy epochs, authorization/channel-authority non-substitution and observable-failure separation, optional attested-key PoP binding | reproduce / research-only; explicit review required |
| 2026-09-05 | [Daily research](daily/2026-09-05.md) | AEAD key-usage continuity semantics: directional q/v budgets and message-length bound, conservative monotonic sender-use estimation, exact-attempt ENROLL authorization binding | reproduce / benchmark; explicit review required |
| 2026-09-04 | [Daily research](daily/2026-09-04.md) | IESG-approved TLS/DTLS IoT profile: constrained resource ceilings, resumption/0-RTT separation, application/service-context binding, target hardware evidence boundary | benchmark / reproduce / investigate; explicit review required |
| 2026-09-03 | [Daily research](daily/2026-09-03.md) | generation-bound delegation as an ordered key-continuous attenuating chain; exact descendant revocation; one-successor bounded continuation/replacement | reproduce; explicit review required |
| 2026-09-02 | [Daily research](daily/2026-09-02.md) | exact ENROLL attempt-context freshness, nonce-vs-expiry separation, enrollment→association→DATA authority non-implication, optional assisted-enrollment boundary | reproduce / investigate; explicit review required |
| 2026-09-01 | [Daily research](daily/2026-09-01.md) | exporter-generation/channel-binding lifecycle, authentication-domain binding semantics, resumption recovery interaction, optional attestation freshness separation | reproduce / research-only; explicit review required |
| 2026-08-31 | [Daily research](daily/2026-08-31.md) | target storage-service semantics for lineage durability/freshness: PSA/TF-M replay protection, ESP32-S3 NVS/eFuse boundaries, STM32H5 EPOCH anti-rollback mapping | benchmark / investigate; explicit review required |
| 2026-08-30 | [Daily research](daily/2026-08-30.md) | distributed LINEAGE_REPLACE convergence/key-confirmation boundary, rollback-freshness anchor for stable persisted lineage state | reproduce / investigate; explicit review required |
| 2026-08-29 | [Daily research](daily/2026-08-29.md) | non-redundant UKS/misbinding identity attribution, dynamic-corruption and compositional formal-analysis boundaries | reproduce / investigate; explicit review required |
| 2026-08-28 | [Daily research](daily/2026-08-28.md) | hostile-count parser resource bounds, authorization authority namespace, authenticated fresh replay-epoch recovery | reproduce; explicit review required |
| 2026-08-27 | [Daily research](daily/2026-08-27.md) | immutable profile-ID semantics, deterministic unknown fixtures vs live GREASE, AUTH-v3 subcontext canonicalization/criticality boundary | reproduce; explicit review required |
| 2026-08-26 | [Daily research](daily/2026-08-26.md) | replay-state lifetime/formal-runtime fidelity, Rust/C eviction parity, pre-authentication source validation and DoS contention | reproduce; explicit review required |
| 2026-08-25 | [Daily research](daily/2026-08-25.md) | resumption authorization revalidation and PSK/ticket reuse, crypto execution-context evidence, IoT deployment-context claim boundaries | reproduce / benchmark; explicit review required |
| 2026-08-24 | [Daily research](daily/2026-08-24.md) | revocation convergence and stale-window bounds, scoped/expiring role authorization, dynamic rights lineage, correlation metadata privacy | reproduce; explicit review required |
| 2026-08-23 | [Daily research](daily/2026-08-23.md) | SAPIC+ single-source multi-backend formal verification, anonymity-vs-unlinkability and observable-failure privacy contract, Ascon/COSE benchmark maturity | reproduce / benchmark; explicit review required |
| 2026-08-22 | [Daily research](daily/2026-08-22.md) | HPKE encrypted O(1) lookup hints, VOPRF/POPRF comparator, holder-of-key + audience authorization binding | prototype / reproduce / research-only; explicit review required |
| 2026-08-21 | [Daily research](daily/2026-08-21.md) | NO-LEARNING trust-store policy, credential-reference misbinding formal analysis, method/suite registry compatibility for PQ transition | reproduce / benchmark; explicit review required |
| 2026-08-20 | [Daily research](daily/2026-08-20.md) | GREASE-style extension agility, constrained enrollment authority placement, credential/authorization invalidation propagation | reproduce / investigate; explicit review required |
| 2026-08-19 | [Daily research](daily/2026-08-19.md) | TLS-exporter AUTH-instance uniqueness, constrained key-generation/seed-storage contract, replay continuity-break recovery | reproduce / benchmark; explicit review required |
| 2026-08-18 | [Daily research](daily/2026-08-18.md) | Authentication/authorization context separation, Tamarin 1.12 reproducibility baseline, AEAD key-usage exhaustion and rekey triggers | investigate / reproduce; explicit review required |
| 2026-08-17 | [Daily research](daily/2026-08-17.md) | CDS OR-proof review contract, ESP32-S3 entropy/key-storage assurance, concrete PQ/T packet budget, BBS maturity/footprint | reproduce / benchmark; explicit review required |
| 2026-08-16 | [Daily research](daily/2026-08-16.md) | Explicit profile negotiation, reboot-safe rekey lifecycle, optional remote attestation, formal-model reproducibility | reproduce / benchmark / research-only; explicit review required |
| 2026-08-15 | [Daily research](daily/2026-08-15.md) | EDHOC benchmark baseline, Fiat–Shamir transcript audit, Ascon, BBS credentials, hybrid PQ/T KEMs, ESP32-S3 target benchmarking | benchmark / research-only; explicit review required |
