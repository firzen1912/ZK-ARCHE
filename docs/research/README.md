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

Research should prioritize developments that can materially affect ZK-ARCHE architecture, security, privacy, interoperability, constrained-device feasibility, or specification maturity, including:

- privacy-preserving authentication and authorization;
- zero-knowledge and Sigma/Schnorr-style proofs suitable for constrained systems;
- anonymous/selective-disclosure credentials and unlinkable authorization;
- EDHOC, OSCORE, CoAP, TLS 1.3/mTLS, DTLS 1.3, and secure-channel binding;
- IoT onboarding, commissioning, enrollment, rekey, revocation, and lifecycle security;
- replay resistance, anti-amplification, retry cookies, resumption, and denial-of-service controls;
- transcript binding, downgrade/UKS/reflection resistance, and protocol state-machine design;
- deterministic vectors, differential testing, parser fuzzing, and interoperability methodology;
- formal verification and symbolic analysis with tools such as ProVerif, Tamarin, and SAPIC+ when useful;
- embedded cryptography, constant-time behavior, RNG/DRBG design, secure storage, and side-channel considerations;
- Rust and C cryptographic implementation practices;
- STM32/ESP32-class footprint and performance constraints;
- post-quantum/hybrid cryptography only as explicitly measured optional research unless promoted by review;
- data-sovereignty, policy-bound release, auditability, and privacy-preserving data access;
- P2P zero-trust authentication, scoped trust evidence, delegation, and revocation.

## Source discipline

Prefer primary and authoritative sources:

- IETF RFCs, Internet-Drafts, working-group material, and reference implementations;
- NIST publications, standards, and cryptographic guidance;
- peer-reviewed IEEE/ACM/USENIX/NDSS/CRYPTO/EUROCRYPT and related publications;
- arXiv or ePrint preprints when clearly labeled as pre-review/preprint evidence;
- official project, university-lab, standards-body, vendor, or government material;
- source repositories, releases, commits, issues, benchmarks, and test suites for relevant implementations;
- original hardware/MCU and cryptographic-library documentation.

A repository issue, benchmark screenshot, blog post, or single implementation behavior is an engineering signal, not proof of a general protocol property unless independently verified.

## Daily finding contract

Future daily reports follow [`daily/README.md`](daily/README.md). Each material finding should record:

1. stable per-report ID and novelty class (`new`, `corroborates`, `refines`, `contradicts`, `supersedes`);
2. source, publication/release date, source type, and primary link;
3. verified source-supported claim;
4. exact existing owner when one exists (`R-*`, `TD-*`, phase/spec section, or `none`);
5. concrete repository fact or implementation/spec/test anchor;
6. protocol/security/privacy problem addressed;
7. strongest distinct engineering idea;
8. evidence maturity: `concept`, `formal`, `software`, `constrained-hardware`, `deployed`, or `externally-reviewed`;
9. assumptions, limitations, uncertainty, and reproduction caveats;
10. likely wire/RAM/CPU/flash/dependency/trust-model implications where relevant;
11. compatibility or migration implications for Rust/C and existing vectors;
12. required next evidence and recommended disposition;
13. whether it is a candidate for later weekly findings consolidation.

Separate **source-supported facts**, **repository facts**, and **ZK-ARCHE inference** explicitly.

## Promotion boundary

Research does not directly become a protocol requirement or engineering request.

Use this progression:

```text
external source / research idea
        ↓
docs/research/daily/YYYY-MM-DD.md
        ↓
docs/research/backlog.md
        ↓  human-reviewed consolidation when repo implication is concrete
docs/findings/week-of-*-findings.md
        ↓  explicit human intent / authorized work
docs/requests/week-of-*-request.md
        ↓  long-term sequencing or decision required?
docs/roadmaps/ and/or docs/adr/
        ↓  normative behavior approved?
spec change + versioned vectors/tests
        ↓
Rust/C implementation and validation
        ↓
docs/assurance/ + retained evidence
        ↓
docs/release/
```

A finding may stop at any stage. Rejection, deferral, research-only classification, or an explicit evidence gap are valid outcomes.

Each daily report may include advisory promotion metadata:

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

A research report must not claim that a weekly request, roadmap item, ADR, spec requirement, debt status, or maturity gate changed unless that separate artifact was deliberately updated through the appropriate process.

## Research backlog

[`backlog.md`](backlog.md) tracks unresolved research questions and promising leads. The backlog is not a roadmap or weekly request queue. Its entries can be closed as rejected or deferred without implementation.

When evidence becomes actionable, first consolidate the repository implication in weekly findings where useful; then promote work only through explicit human request/review and link the appropriate roadmap phase or ADR rather than copying research prose into normative documents.

## Automated daily pipeline

The recurring research workflow is governed by [`PIPELINE.md`](PIPELINE.md).

The automation operates on the `dev` branch under a strict least-write-access rule:

- it may read the entire repository, including current weekly findings/requests, to understand code, specification, roadmap, assurance, technical-debt, and current human intent;
- everything outside `docs/research/**` is read-only context;
- it may write only `docs/research/**`;
- it must never write to `main`;
- it must not create/update `docs/findings/**` or `docs/requests/**`;
- each completed daily run may create at most one commit and one `dev` ref update;
- findings that imply code, spec, roadmap, ADR, CI, release, findings, request, or assurance changes are recorded as recommendations/hand-off candidates rather than applied automatically.

The report, index update, and any justified backlog update must be assembled into the same atomic commit. The automation must verify after writing that no path outside `docs/research/**` changed.

## Daily-report quality direction

The historical 2026-08-15 through 2026-08-25 reports remain valid provenance. Future reports should be more delta-oriented:

- state repository change once rather than repeating unchanged context;
- distinguish new evidence from corroboration/refinement;
- map findings to exact R-/TD-/spec/phase owners;
- synthesize sources that support one engineering conclusion;
- include an actionability matrix and explicit claim/no-change boundary;
- identify weekly finding candidates without autonomously creating them.

## Index

| Date | Report | Focus | Promotion status |
|---|---|---|---|
| 2026-08-27 | [Daily research](daily/2026-08-27.md) | immutable profile-ID semantics, deterministic unknown fixtures vs live GREASE, AUTH-v3 subcontext canonicalization/criticality boundary | reproduce; explicit review required |
| 2026-08-26 | [Daily research](daily/2026-08-26.md) | replay-state lifetime/formal-runtime fidelity, Rust/C eviction parity, pre-authentication source validation and DoS contention | reproduce / benchmark; explicit review required |
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
