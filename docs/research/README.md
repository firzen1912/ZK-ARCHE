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

Related sibling control layers are deliberately outside the daily automation write scope: `docs/findings/` owns human-reviewed conclusions and `docs/requests/` owns explicit human-authorized work. Roadmaps, ADRs, `spec/`, implementation/tests, assurance evidence, technical debt, and release governance retain their existing authority.

A dated report should be created only after research for that date has actually been performed. Do not pre-create empty future reports.

## Research and promotion boundary

Research should prioritize developments that materially affect ZK-ARCHE architecture, security, privacy, interoperability, constrained-device feasibility, or specification maturity. Prefer primary and authoritative sources: IETF RFCs/drafts, NIST, peer-reviewed security/cryptography venues, official project repositories, formal-methods projects, and original hardware/library documentation. Secondary sources are discovery aids; material claims should be traced to primary evidence.

Future daily reports follow [`daily/README.md`](daily/README.md) and the recurring workflow in [`PIPELINE.md`](PIPELINE.md). Each material finding records source provenance, verified claim, exact owner, repository anchor, engineering idea, evidence maturity, limitations, resource/trust/privacy impact, Rust/C/vector compatibility, required next evidence, disposition, and roadmap-impact record. Source-supported facts, repository facts, and ZK-ARCHE inference remain explicit.

Research does not directly become a protocol requirement or engineering request. Promotion flows through human-reviewed findings/requests and then, when explicitly accepted, roadmaps/ADRs/specification/implementation/assurance/release governance. [`backlog.md`](backlog.md) is a research-execution queue, not a roadmap or implementation queue.

The daily pipeline may read the full `dev` repository but may write only `docs/research/**`; it must never write `main`, findings, requests, roadmaps, ADRs, specs, implementation, assurance, debt, CI, issues, PRs, or releases. Each completed run may create at most one commit and one `dev` ref update.

## Index

| Date | Report | Focus | Promotion status |
|---|---|---|---|
| 2026-09-26 | [Daily research](daily/2026-09-26.md) | immutable profile identity vs per-session state; support advertisement vs authoritative selection; ordered/unordered negotiation semantics and exporter-parameter ownership | benchmark; R-001 evidence contract refined; explicit review required |
| 2026-09-25 | [Daily research](daily/2026-09-25.md) | compiler/target-aware side-channel qualification for the CDS role proof: libsodium hardening evidence, exact Dalek/subtle scope, artifact-level leakage provenance | reproduce; R-007 evidence contract refined; explicit review required |
| 2026-09-24 | [Daily research](daily/2026-09-24.md) | constrained entropy/RNG lifecycle and health: ESP32-S3 readiness transitions, STM32 seed/fault handling, Rust/C backend provenance and fail-closed operation atomicity | benchmark / reproduce; explicit review required |
| 2026-09-23 | [Daily research](daily/2026-09-23.md) | pre-auth return-routability before state/HPKE/lookup/proof work; independent amplification/state/CPU budgets; HPKE-vs-VOPRF observer/privacy and DoS-cost boundary | prototype / benchmark; explicit review required |
| 2026-09-22 | [Daily research](daily/2026-09-22.md) | sender/channel binding vs authorization/revocation currentness; measured revocation-convergence budget; bounded disconnected authority lifetime | reproduce / benchmark; explicit review required |
| 2026-09-21 | [Daily research](daily/2026-09-21.md) | non-authoritative/non-unique credential selectors; exact proof-bound TrustRecord attribution; constrained trust-mutation capability and storage bounds | reproduce; explicit review required |
| 2026-09-20 | [Daily research](daily/2026-09-20.md) | privacy-handle rotation as lifecycle/replay state; migration prepare/activate boundary; identifier anti-reuse persistence and constrained storage cost | reproduce / benchmark; explicit review required |
| 2026-09-19 | [Daily research](daily/2026-09-19.md) | formal-toolchain trust boundary: SAPIC state-translation semantics; Tamarin privacy/equivalence patch-level qualification; replayable formal-proof artifacts | reproduce; explicit review required |
| 2026-09-18 | [Daily research](daily/2026-09-18.md) | CDS OR-proof witness-index side-channel schedule; canonical unique role-set semantics; Rust/C malformed scalar/point validation parity and Fiat–Shamir proof-context separation | reproduce; explicit review required |
| 2026-09-17 | [Daily research](daily/2026-09-17.md) | exporter context vs exporter-generation currentness; bounded previous-generation processability vs authorization authority; adapter-local epoch semantics | reproduce; explicit review required |
| 2026-09-16 | [Daily research](daily/2026-09-16.md) | bounded delegation attenuation/key continuity; descendant revocation and audit-vs-authority separation; offline stale-revocation bounds | reproduce / benchmark; explicit review required |
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