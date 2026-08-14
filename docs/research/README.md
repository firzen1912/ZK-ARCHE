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

A dated report should be created only after research for that date has actually been performed. Do not pre-create empty future reports.

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
- formal verification and symbolic analysis with tools such as ProVerif and Tamarin;
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

Each meaningful finding should record:

1. source, publication/release date, and source type;
2. verified claim supported by the source;
3. protocol/security/privacy problem addressed;
4. strongest distinct engineering idea;
5. evidence maturity: concept, formal, simulation, software, constrained-hardware, deployed, or externally reviewed;
6. assumptions, limitations, uncertainty, and reproduction caveats;
7. likely ZK-ARCHE impact area;
8. expected wire/RAM/CPU/dependency implications where relevant;
9. compatibility or migration implications for Rust/C and existing vectors;
10. recommended disposition: investigate, reproduce, benchmark, prototype, promote, defer, reject, or research-only.

Separate **source-supported facts** from **ZK-ARCHE inference** explicitly.

## Promotion boundary

Research does not directly become a protocol requirement.

Use this progression:

```text
research finding
    ↓
research backlog entry
    ↓ evidence sufficient?
roadmap candidate
    ↓ consequential architecture/protocol choice?
ADR
    ↓ normative behavior approved?
spec change + versioned vectors/tests
    ↓
Rust/C implementation and validation
```

Each report may include a promotion record:

```yaml
roadmap_impact:
  candidate_phase: null
  recommendation: investigate | reproduce | benchmark | prototype | promote | defer | reject | research-only
  evidence_maturity: concept | formal | software | hardware | deployed | externally-reviewed
  protocol_impact: none | compatible | extension | versioned-breaking-change
  required_next_evidence: null
  roadmap_action: none
  promotion_requirement: explicit review
```

A research report must not claim that a roadmap item, ADR, spec requirement, or maturity gate changed unless that separate artifact was deliberately updated and reviewed.

## Research backlog

[`backlog.md`](backlog.md) tracks unresolved research questions and promising leads. The backlog is not a roadmap. Its entries can be closed as rejected or deferred without implementation.

When evidence becomes actionable, promote the item by linking it to the appropriate roadmap phase or ADR rather than copying research prose into normative documents.

## Automated daily pipeline

The recurring research workflow is governed by [`PIPELINE.md`](PIPELINE.md).

The automation operates on the `dev` branch under a strict least-write-access rule:

- it may read the entire repository to understand current code, specification, roadmap, assurance, and technical-debt state;
- everything outside `docs/research/**` is read-only context;
- it may write only `docs/research/**`;
- it must never write to `main`;
- each completed daily run may create at most one commit and one `dev` ref update;
- findings that imply code, spec, roadmap, ADR, CI, or release changes are recorded as recommendations for explicit review rather than applied automatically.

The report, index update, and any justified backlog update must be assembled into the same atomic commit. The automation must verify after writing that no path outside `docs/research/**` changed.

## Index

No dated research reports are registered yet in this directory. Add each completed report to this index when it is created.

| Date | Report | Focus | Promotion status |
|---|---|---|---|
| — | — | — | — |
