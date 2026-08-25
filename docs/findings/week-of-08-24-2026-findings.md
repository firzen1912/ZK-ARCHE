# Findings — week of 2026-08-24

## 0. How to use this file

This file is the weekly synthesis/control surface for repository-relevant findings beginning 2026-08-24. Source-level research remains in `../research/daily/`; explicit authorized work belongs in `../requests/`.

## 1. Week at a glance

| Field | State |
|---|---|
| Starting research state | 15 active research-backlog questions (`R-001` through `R-015`) |
| Active technical debt | TD-001 through TD-004 remain open |
| Implementation lanes | Rust reference + independent C implementation |
| Main technical theme | authorization/revocation/resumption lifecycle and stronger evidence context |
| Main documentation theme | separate daily research provenance from weekly findings and explicit requests |
| Mandatory-suite / wire impact | none from this synthesis |
| Maturity impact | none |

## 2. Consolidated findings

### F0824-01 — Revocation is a convergence problem with a bounded stale-authorization window

- **Status:** confirmed
- **Evidence basis:** `../research/daily/2026-08-24.md`, R-008, R-012
- **Repository fact:** the roadmap already contains revocation, resumption, delegated enrollment, policy epochs, P2P trust, and DATA authorization, but current normative text does not define how disconnected peers converge on revocation state.
- **Interpretation:** future revocation design needs durable issuer/epoch-scoped state, full/differential reconciliation, reconnect behavior, rollback resistance, and a per-profile freshness objective or maximum stale-authorization window. “Revoked” cannot imply instantaneous global awareness in offline-capable deployments.
- **Affected work:** R-008, R-012, zk213/zk214/zk221/zk230/zk234/zk235/zk239, TD-004
- **Required next evidence:** convergence fixtures, rollback/offline/reconnect tests, privacy analysis of revocation identifiers
- **Claim boundary:** ACE revocation patterns are comparators; they do not mandate a centralized always-online authority.

### F0824-02 — Resumption must carry valid authorization context, not merely a valid secret

- **Status:** confirmed
- **Evidence basis:** `../research/daily/2026-08-25.md`, R-009, R-012, R-015
- **Repository fact:** zk221 already intends replay-safe privilege-preserving 1-RTT resumption and invalidation on relevant registry/role/policy changes.
- **Interpretation:** resumption credentials need explicit issue/expiry/reuse policy plus cached authorization lineage/context. If role, audience, deployment, policy epoch, revocation state, or other decision inputs change and cannot be safely reevaluated, the peer should fall back to full AUTH rather than inheriting stale privilege. Repeated ticket/PSK identifiers also belong in unlinkability testing.
- **Affected work:** R-009, R-012, R-015, zk213/zk214/zk217/zk221/zk226/zk239
- **Required next evidence:** resumption-record schema and negative vectors for changed/missing/stale authorization context
- **Claim boundary:** the research does not choose a final ticket format or require TLS/EDHOC resumption semantics.

### F0824-03 — Constrained benchmark results require a pinned cryptographic execution context

- **Status:** confirmed
- **Evidence basis:** `../research/daily/2026-08-25.md`, R-003, R-005, TD-002
- **Repository fact:** target measurements remain a blocking evidence gap for MCU field-readiness claims.
- **Interpretation:** every target benchmark should record implementation/library version, crypto boundary, hardware/firmware accelerator path, seed/private-key representation, entropy source, DRBG/reseed rules, self-test posture, secure-storage location, zeroization/rollback assumptions, and software fallbacks. Measurements without this context are local observations, not portable profile evidence.
- **Affected work:** R-003, R-005, TD-002, zk208/zk216/zk222/zk224/zk236/zk241
- **Required next evidence:** versioned `crypto_execution_context` benchmark manifest plus raw measurements
- **Claim boundary:** adopting this evidence taxonomy does not imply FIPS 140-3 certification.

### F0824-04 — Protocol conformance and field readiness must remain separate claims

- **Status:** confirmed
- **Evidence basis:** `../research/daily/2026-08-25.md`, TD-002, TD-004
- **Repository fact:** current profile and assurance documents correctly gate claims on measurements, but they do not yet define a structured deployment-context evidence layer.
- **Interpretation:** a future readiness package should identify the tested ZK-ARCHE profile plus surrounding product/platform security capabilities, provisioning/revocation authority, lifecycle/update posture, external controls, and residual trust assumptions. Passing protocol vectors cannot be reported as whole-product security.
- **Affected work:** R-005, TD-002, TD-004, zk209/zk210/zk216/zk222/zk226/zk236
- **Required next evidence:** deployment-context evidence schema and claim-boundary checklist
- **Claim boundary:** no new runtime protocol dependency follows from this finding.

### F0824-05 — The research archive needs a weekly control layer to prevent evidence from becoming de facto work planning

- **Status:** confirmed
- **Evidence basis:** audit of `../research/daily/2026-08-15.md` through `2026-08-25.md`, `../research/README.md`, `../research/PIPELINE.md`, and the HIVEAS `docs/findings/` + `docs/requests/` pattern
- **Repository fact:** ZK-ARCHE daily reports are detailed and source-disciplined, but they repeatedly restate stable repository context and combine source findings, repository inference, backlog movement, and candidate work in the same dated document.
- **Interpretation:** preserve daily reports as research provenance, but make future reports delta-oriented and route consolidated repository conclusions into weekly `docs/findings/`. Explicit human-authorized work and acceptance criteria belong in weekly `docs/requests/`. The recurring research automation should remain unable to write either sibling directory.
- **Affected work:** documentation workflow only
- **Required next evidence:** stable templates, current-week files, documentation index updates, and a preserved least-write research boundary
- **Claim boundary:** this reorganization does not promote any research item into code/spec/roadmap work.

## 3. Evidence and provenance map

| Finding | Primary provenance | Existing owner |
|---|---|---|
| F0824-01 | daily 2026-08-24 | R-008 / R-012 |
| F0824-02 | daily 2026-08-25 | R-009 / R-012 / R-015 |
| F0824-03 | daily 2026-08-25 | R-003 / R-005 / TD-002 |
| F0824-04 | daily 2026-08-25 | R-005 / TD-002 / TD-004 |
| F0824-05 | daily archive + HIVEAS documentation pattern | docs governance |

## 4. Duplicate / rejected / superseded interpretations

- Repeating the same unchanged TD-001–TD-004 context in every future daily report is unnecessary; a concise “no repository delta” statement plus exact links is sufficient.
- A daily `roadmap_impact` record is advisory metadata, not an implementation request.
- A backlog status change is not evidence that a roadmap phase, spec, or maturity state changed.
- External standards guidance strengthens evidence requirements but does not automatically impose certification or dependency requirements.

## 5. Impact map

This week strengthens existing research/debt and adds documentation control surfaces. No research item is promoted into normative implementation by this findings file.

The explicit human documentation request for this week is recorded separately in `../requests/week-of-08-24-2026-request.md`.

## 6. Carry-forward / unresolved questions

- Determine which R-* items, if any, should receive an explicit human implementation request after review.
- Keep TD-001 through TD-004 open until their clearing evidence exists.
- Use the improved daily template to reduce repeated context and make novelty/corroboration explicit.
- Keep the daily automation write-limited to `docs/research/**`; weekly findings/requests require a separate human-reviewed update.