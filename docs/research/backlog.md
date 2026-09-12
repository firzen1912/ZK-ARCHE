# ZK-ARCHE Research Backlog

This file is the canonical persistent **research-execution queue** for recurring ZK-ARCHE research. Items here are research questions and evidence gaps, not engineering commitments.

The daily pipeline in `PIPELINE.md` should continuously select the highest-value actionable items from this queue, research them, update their evidence/status, enqueue distinct follow-on questions when justified, and continue until the run budget is exhausted or no actionable item remains.

A backlog item becoming `promote` or `exhausted` does **not** modify the roadmap, ADRs, specification, code, technical debt, assurance posture, or release claims. Those remain explicitly human-gated.

## Queue priority

Use the following priority order for daily selection:

- `P0` — blocks security, conformance, formal-assurance, or claim boundaries.
- `P1` — blocks mandatory Common Contract, constrained-device, lifecycle, interoperability, or P2P maturity.
- `P2` — valuable core capability/evidence expansion after P0/P1 blockers.
- `P3` — optional, exploratory, migration, or research-only work.

Current queue-priority mapping:

| Priority | Backlog items | Rationale |
|---|---|---|
| P0 | R-004, R-007, R-009, R-010, R-013, R-015 | formal/security proof boundaries, cryptographic review, replay/resumption/channel-binding correctness, trust-mutation and privacy claims |
| P1 | R-001, R-005, R-008, R-011, R-012, R-014 | constrained Common Contract, hardware evidence, authn/authz/enrollment, interoperability/extension agility, revocation convergence, DoS/lookup behavior |
| P2 | none currently | reserve for non-blocking core evidence/capability expansion |
| P3 | R-002, R-003, R-006 | anonymous credentials, PQ hybrids, optional attestation; useful but not mandatory baseline blockers |

The priority map may be changed by a daily report only when the report explains the dependency/evidence reason. Priority is about research execution order, not feature importance.

## Status vocabulary

- `queued` — worth investigating; no sufficient assessment yet.
- `researching` — sources are being reviewed.
- `reproduce` — a claim/result needs local reproduction or independent validation.
- `benchmark` — feasibility depends on measured wire/RAM/CPU/storage/latency evidence.
- `prototype` — bounded experimental implementation is justified.
- `promote` — evidence is sufficient to propose explicit human engineering review.
- `research-only` — useful context/experiment, intentionally outside the mandatory baseline.
- `defer` — potentially useful, but current cost/priority/evidence is insufficient.
- `reject` — incompatible, unsafe, redundant, or not valuable for ZK-ARCHE.
- `exhausted` — the current research question has no material unresolved research step under its present scope.

`exhausted` does not mean engineering implementation is complete. New contradictory or superseding evidence may reactivate an exhausted item.

## Daily queue-consumption contract

For every daily run:

1. read all non-terminal items;
2. rank by priority, dependency value, staleness, and explicit evidence gap;
3. select at least one actionable item;
4. record selected IDs and rationale in the daily report;
5. gather the exact next evidence requested by the item;
6. update status/evidence/last-reviewed only when justified;
7. enqueue a new `R-*` item only for a genuinely distinct question;
8. continue to another item if run budget remains;
9. if all items are non-actionable/exhausted, perform a bounded discovery pass rather than inventing low-value work.

Do not keep revisiting an item without a concrete unresolved next question or a new evidence trigger.

## Backlog

| ID | Topic / question | Status | Evidence needed | Potential destination | Last reviewed |
|---|---|---|---|---|---|
| R-001 | EDHOC/OSCORE-inspired constrained binding, exporter, and explicit profile-negotiation semantics | benchmark | exact ZK-ARCHE-vs-EDHOC wire/footprint comparison; immutable profile-definition semantics for stable profile IDs; prescriptive-vs-non-prescriptive profile parameter classification; profile ID/selection and prescriptive-parameter model; same-ID Rust/C/cross-build semantic parity; transcript binding; downgrade/unknown-profile behavior; versioned replacement/deprecation behavior; interop implications | `docs/roadmaps/rfc-evolution-plan.md` / `spec/` | 2026-08-27 |
| R-002 | Reviewed anonymous/selective-disclosure credential options for role authorization | researching | BBS/property comparison; exact proof/signature bytes; BLS12-381 pairing CPU/RAM/flash/dependency footprint; issuance/revocation model; Rust/C library maturity; extension status; external-review status | research-only or future suite ADR | 2026-08-17 |
| R-003 | Post-quantum hybrid key establishment for edge/gateway profiles | benchmark | full packet budget using current ML-KEM sizes; RAM/CPU/flash; MTU/fragmentation/loss behavior; protocol-specific hybrid KDF/transcript review; downgrade model; root-seed/private-key storage implications; cryptographic-module boundary; accelerator family/microarchitecture; seed-derived-key/self-test posture; implementation versions; method/suite/profile compatibility matrix; comparison of PQ key-exchange with KEM-authentication message/state requirements and identity-protection timing | research-only / optional suite roadmap | 2026-08-25 |
| R-004 | Formal verification expansion beyond current symbolic skeletons | reproduce | pinned SAPIC+/ProVerif/Tamarin environment (or equivalent mechanically synchronized model source); one canonical AUTH state-machine model; property/attacker matrix covering secrecy, agreement, anonymity, unlinkability, compromise and SNDL variants; model-to-code traceability; stateful credential/reference database; mapping-substitution/minimal-compromise scenarios; explicit replay-state abstraction covering persistence, boundedness/eviction, restart/epoch semantics, and any implementation-strengthening assumptions; explicit AUTH-v3 canonicalization/parser assumption boundary for authorization/critical-extension/channel-binding hashes; duplicate/non-canonical context cases converted to Rust/C negative tests; retained successful proofs and counterexamples; representable attacks converted to Rust/C negative tests | assurance roadmap / evidence | 2026-08-27 |
| R-005 | STM32/ESP32-class AUTH and P2P benchmark methodology | benchmark | target hardware/build profile; cryptographic-module boundary; implementation/library versions; accelerator family/microarchitecture and software fallback paths; entropy-source state; DRBG/reseed and RNG-failure behavior; key-generation mode; root-seed vs private-key storage; seed-expansion/derivation-domain contract; seed-derived key-pair self-test posture; clone/rollback/reprovisioning behavior; eFuse/flash key handling; secure-boot/debug state; zeroization assumptions; stack/heap/flash; packet and latency data; deployment-context metadata before field-readiness use | IoT profile roadmap / assurance evidence | 2026-08-25 |
| R-006 | Optional remote-attestation evidence bound to authenticated sessions | research-only | threat-model justification; hardware root-of-trust availability; EAT/evidence sizes; freshness/privacy analysis; verifier architecture; cross-session/relay channel-binding analysis; MCU/edge RAM/CPU/flash/wire measurements | future optional profile/extension only if justified | 2026-08-16 |
| R-007 | CFRG-informed conformance/security review of the CDS OR-composed role-membership proof | reproduce | mathematical proof contract; simulator/completeness/soundness/ZK assumption matrix; canonical serialization and role ordering; protocol/instance identifiers; CSPRNG and constant-time review; Rust/C differential and negative vectors; independent cryptographic review | TD-001 / assurance / future proof ADR/spec text | 2026-08-17 |
| R-008 | Explicit authentication-vs-authorization context binding, scoped/revocable authorization, constrained enrollment-authority placement, and holder-of-key/audience semantics | reproduce | authn/authz contract; compact audience/group scope and requested-vs-granted role semantics; expiration/validity and policy epoch; exact holder key or authenticated-session confirmation binding; key/profile operation compatibility; three-role joining-peer/commissioner/enrollment-authority trust model; compact grant/voucher byte budget; constrained-link vs backend work split; offline/degraded-mode behavior; `NO-LEARNING` normal-AUTH/P2P baseline with explicit learning exceptions; role downgrade/expiry/revocation transition tests; decentralized variant; negative vectors; `iot-core` byte/state budget | zk211/zk212/zk230/zk234/zk239 / future AUTH-TRUST-ENROLL-DATA spec work | 2026-08-24 |
| R-009 | Per-suite AEAD key-usage accounting, durable replay continuity, resumption-secret lifecycle, and exhaustion-triggered rekey behavior | reproduce | suite-specific usage-limit derivation; sender/receiver counter semantics; warning/hard-stop thresholds; explicit replay epoch/lifetime and per-profile retention/eviction semantics; persistence or authenticated fresh-context transition after restart/replay-state loss; continuity-break state and rollback detection; Rust/C replay-capacity/eviction decision parity; authenticated resynchronization or key-rotation recovery; resumption-secret issue/expiry/reuse counters; secure erasure and excessive-reuse policy; separation of resumption-credential derivation lineage from newly established association traffic-key generation; non-resettable origin/full-AUTH lineage lifetime or generation budget across successor resumption credentials; restart/rollback protection for that cumulative bound; rekey/resumption interaction; separate rejection/continuity audit evidence; MCU flash-wear analysis; eviction/restart/rollback/epoch-transition negative tests; formal-model-to-runtime replay/resumption-state traceability | zk213/zk221/zk230/zk235 / assurance | 2026-09-11 |
| R-010 | TLS-exporter upper-layer AUTH-instance uniqueness and ZK-ARCHE channel-binding context semantics | reproduce | normative `EXPORTER-ZK-ARCHE-v1` label/context construction; application/ALPN and deployment/domain binding; endpoint identity/commitment binding; fresh AUTH-instance/session identifier; multiple-AUTH-per-TLS policy; TLS resumption/proxy/termination assumptions; explicit separation of retained binding requirements/identity from fresh resumed-association channel-binding evidence; predecessor binding bytes must not substitute for current resumed-channel proof; deterministic fixtures and cross-instance/cross-protocol/resumption-rebinding negative tests | zk217/zk221/zk228 / future BIND spec text | 2026-09-11 |
| R-011 | Registry/extension agility and GREASE-style anti-ossification conformance testing | reproduce | critical-vs-ignorable unknown-value semantics; explicit separation of deterministic reserved unknown-value fixture ranges from any future live GREASE; reserved suite/extension/version test ranges; deterministic Rust/C unknown-value corpus; unsupported-selected-suite fail-closed tests; method/suite/profile compatibility matrix; incompatible-but-known combination fail-closed tests; if live GREASE is proposed: sparse distributed reserved values, ordinary-unknown offered behavior, never-selected semantics, duplicate-free valid advertisements, privacy/MTU/fingerprinting budget; canonical critical-extension-set encoding and empty representation; duplicate/multiplicity/non-canonical context tests; extension-order/variability tests where legal | zk225/zk226/zk229 / registries and conformance vectors | 2026-08-27 |
| R-012 | Credential/authorization invalidation propagation, authorization-lineage replacement, and revocation convergence across sessions/resumption/derived keys/cached authorization/DATA state | reproduce | dependency/invalidation matrix; credential/registry/policy/key epoch propagation; issuer/audience-scoped authorization lineage and monotonic generation; atomic replace-vs-new semantics; missing-predecessor recovery after storage loss; full/diff revocation-view reconciliation; missed-update/offline reconnect behavior; maximum stale-authorization window per profile; rollback-resistant revocation persistence; secure cached authorization context for resumption; changed-context reevaluation; mandatory full-AUTH fallback when safe resumption authorization cannot be established; group/member removal rekey semantics; stale-resumption and stale-authorization negative vectors; bounded persistent metadata model; Tamarin/ProVerif revocation-state properties | zk213/zk214/zk221/zk230/zk234/zk235/zk239 / assurance and future spec text | 2026-08-25 |
| R-013 | Credential-learning/trust-store mutation policy and credential-reference-to-key/role/policy binding | reproduce | `NO-LEARNING` baseline for normal AUTH/P2P; explicit authorized-learning exceptions for ENROLL/commissioner; credential/reference binding to exact public key or commitment, role/policy context, audience/deployment/domain, epoch, authorization provenance, and permitted key operations/profile; mismatch/misconfigured-store negative tests; auditable trust-store mutations; stateful Tamarin/ProVerif model with attacker/misconfiguration influence over mappings | zk207/zk211/zk212/zk214/zk217/zk220/zk226/zk239 / future TRUST/ENROLL spec and assurance | 2026-08-22 |
| R-014 | Privacy-preserving O(1) registry lookup hints for large AUTH registries | prototype | bounded HPKE encrypted-hint prototype using an opaque registry key as non-authoritative prefilter; exact extension/info/AAD/padding encoding; lookup-key epoch/rotation/revocation model; stateless/bounded `AUTH_RETRY` source validation placed before session reservation, registry scan and expensive proof verification in exposed datagram profiles; O(n) PID scan vs O(1)+HPKE benchmarks at 10/100/1k/10k records; adversarial mixed UDP/TCP contention, replay-lock wait, session-slot occupancy and amplification measurements; duplicate-race tests proving exactly one accepted state transition; Rust/C deterministic and negative vectors; passive-linkability analysis; comparison against enrollment-issued random opaque handles and VOPRF/POPRF; threat model required before any VOPRF promotion | zk219/zk220/zk225/zk226/zk229 / optional lookup-hint extension | 2026-08-26 |
| R-015 | Privacy-observability and active-unlinkability contract for AUTH/ENROLL/P2P failure behavior, correlation metadata, and resumption identifiers | reproduce | attacker-model-specific anonymity/unlinkability definitions; externally observable failure matrix covering response/no-response, alert type, size bucket, retry behavior and timing; known-vs-unknown lookup/credential/reference and allowed-vs-disallowed role oracle tests; interaction with `AUTH_RETRY`; correlation-surface inventory covering session/connection IDs, capability/suite/profile lists and ordering, extension sets, retry tokens, packet sizes, lower-layer identifiers, and resumption ticket/PSK identifiers; identifier-rotation and bounded-reuse policy; Rust/C response-class, repeated-session, and repeated-resumption metadata equivalence fixtures; realistic timing/fingerprint measurements; DoS-cost analysis for normalization/dummy-work policy | zk203/zk217/zk218/zk219/zk220/zk221/zk225/zk226/zk229/zk239 / privacy and error/state-machine spec + assurance | 2026-08-25 |

Add new items only when a research question is concrete enough to state what evidence would change a ZK-ARCHE decision. Do not use this table as a feature wishlist.

## Promotion record

When an item is promoted, retain the row and link the destination so research provenance survives:

```text
R-00X → docs/roadmaps/...#phase → docs/adr/NNNN-...md → spec/...md
```

Promotion means “ready for explicit engineering review,” not “automatically accepted.”
