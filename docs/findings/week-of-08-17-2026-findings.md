# Findings — week of 2026-08-17

## 0. How to use this file

This file consolidates the dated research reports from 2026-08-17 through 2026-08-23 into repository-relevant conclusions. The 2026-08-15 and 2026-08-16 reports are treated as baseline context because they established the first EDHOC/profile/transcript/benchmark questions.

This is not a replacement for the daily reports. Use the daily files for source-level detail and provenance; use this file to understand what the week changed for ZK-ARCHE.

## 1. Week at a glance

| Field | State |
|---|---|
| Repository condition | Implementation-stable across the week; the research series did not change Rust/C protocol behavior, vectors, roadmap, spec, assurance, or maturity claims |
| Active debt | TD-001 through TD-004 remained open |
| Main conclusion | The highest-value work is assurance, lifecycle, specification precision, and constrained evidence—not wholesale primitive replacement |
| Strongest new research directions | role-proof review contract; constrained benchmark contract; authn/authz separation; trust mutation policy; registry/profile agility; replay/resumption lifecycle; formal/privacy modeling; bounded private lookup |
| Mandatory-suite impact | none |
| Maturity impact | none |

## 2. Consolidated findings

### F0817-01 — The custom role-membership proof now has a concrete review contract

- **Status:** confirmed
- **Evidence basis:** `../research/daily/2026-08-17.md`, R-007, TD-001, current Rust proof implementation
- **Repository fact:** ZK-ARCHE uses a custom CDS-style OR-composed role-membership proof with simulated branches, one real branch, and Fiat-Shamir challenge composition.
- **Interpretation:** TD-001 should not be treated as a vague “get crypto review” task. The review target can be decomposed into statement/witness relation, simulator correctness, completeness/soundness assumptions, canonical serialization, role ordering, protocol/instance identifiers, challenge/domain separation, scalar sampling, constant-time witness handling, and Rust/C negative-vector parity.
- **Affected work:** R-007, TD-001, zk209/zk217/zk225/zk226
- **Required next evidence:** independent cryptographic review package plus regression vectors for review findings
- **Claim boundary:** CFRG Sigma/Fiat-Shamir guidance is a review framework; it does not prove the existing construction secure.

### F0817-02 — “MCU benchmark” must include execution and key-lifecycle context, not just timing

- **Status:** confirmed
- **Evidence basis:** `../research/daily/2026-08-17.md`, `2026-08-19.md`, R-005, TD-002
- **Repository fact:** target-class measurements remain incomplete and existing profile claims are evidence-gated.
- **Interpretation:** STM32/ESP32-S3 evidence should record entropy source state, DRBG/reseed behavior, root-seed versus private-key storage, secure-key/eFuse use, accelerator configuration, secure-boot/debug posture, rollback/reprovision behavior, stack/heap/flash, packet bytes, and latency. A number without its execution context is not portable profile evidence.
- **Affected work:** R-005, R-003, TD-002, zk208/zk216/zk222/zk224
- **Required next evidence:** reproducible per-target benchmark manifest and raw results
- **Claim boundary:** vendor documentation defines capabilities and constraints; it is not measured ZK-ARCHE performance.

### F0817-03 — Authentication, authorization, and trust mutation need separate semantics

- **Status:** confirmed
- **Evidence basis:** `../research/daily/2026-08-18.md`, `2026-08-21.md`, `2026-08-22.md`, R-008, R-013
- **Repository fact:** normal AUTH already assumes prior enrollment, while future roadmap work adds grants, policy-bound release, P2P trust, revocation, and delegation.
- **Interpretation:** a valid role/device proof should establish authentication relative to existing trusted state; it should not silently create trust or imply indefinite authorization. Authorization needs explicit audience/scope, granted role/policy, validity, epoch/revocation, and holder/session binding. Trust-store mutation should occur only through explicit ENROLL/commissioner/grant operations.
- **Affected work:** R-008, R-013, zk211/zk212/zk214/zk230/zk234/zk239
- **Required next evidence:** authn/authz contract, NO-LEARNING baseline tests, scoped authorization negative vectors
- **Claim boundary:** ACE/EDHOC patterns are semantic comparators, not mandatory dependencies.

### F0817-04 — Profile, registry, and extension semantics need compatibility and anti-ossification tests

- **Status:** confirmed
- **Evidence basis:** `../research/daily/2026-08-20.md`, `2026-08-21.md`, plus the 2026-08-16 profile-negotiation baseline; R-001 and R-011
- **Repository fact:** ZK-ARCHE has capability bits, local runtime `ProfileKind`, future protocol-profile names, and registry skeletons, but these concepts are not yet fully normalized into normative selected-profile and compatibility behavior.
- **Interpretation:** the specification should distinguish runtime resource profile, negotiated protocol/security profile, and optional capability bits; define critical versus ignorable unknown values; encode suite/method/profile compatibility; and retain deterministic GREASE-style differential tests so Rust/C extension points do not ossify.
- **Affected work:** R-001, R-011, zk217/zk225/zk226/zk229/zk240, TD-004
- **Required next evidence:** profile-semantics matrix and Rust/C unknown/unsupported-value corpus
- **Claim boundary:** test-only GREASE does not require live random GREASE traffic on constrained links.

### F0817-05 — Replay, key usage, channel binding, rekey, and resumption form one lifecycle problem

- **Status:** confirmed
- **Evidence basis:** `../research/daily/2026-08-18.md`, `2026-08-19.md`, R-009, R-010
- **Repository fact:** replay persistence, rekey, TLS exporter binding, and future resumption are separately represented in current plans.
- **Interpretation:** they should be validated as a connected lifecycle: per-suite usage accounting, exhaustion-triggered rekey, durable replay continuity, continuity-break behavior after state loss, upper-layer AUTH-instance uniqueness for channel binding, secure key/seed lifetime, and deterministic rollback/restart tests.
- **Affected work:** R-009, R-010, zk213/zk217/zk221/zk228/zk230/zk235
- **Required next evidence:** lifecycle state matrix plus restart/rollback/cross-instance negative tests
- **Claim boundary:** current research does not specify the final wire format for rekey or resumption.

### F0817-06 — O(1) private registry lookup has a bounded standard-primitive prototype path

- **Status:** partial
- **Evidence basis:** `../research/daily/2026-08-22.md`, R-014, current Rust registry/AUTH lookup behavior
- **Repository fact:** the registry store is O(1) by stored device key, but current privacy-preserving PID matching requires O(n) candidate scans during AUTH.
- **Interpretation:** an optional HPKE-encrypted opaque lookup hint is a reasonable first prototype because it can recover O(1) candidate selection without making the hint authoritative identity or authorization. VOPRF/POPRF remains a comparator for stronger privacy models, not the default design.
- **Affected work:** R-014, zk219/zk220/zk225/zk226
- **Required next evidence:** bounded prototype, exact wire/CPU/RAM cost, rotation/revocation model, privacy and DoS analysis, Rust/C negative vectors
- **Claim boundary:** this is not ready for `iot-core` promotion and does not remove full proof/PID verification.

### F0817-07 — Formal assurance should use one canonical state model and explicit privacy properties

- **Status:** confirmed
- **Evidence basis:** `../research/daily/2026-08-18.md`, `2026-08-23.md`, R-004, R-015, TD-003
- **Repository fact:** current symbolic models are intentionally incomplete skeletons and do not model the persistent registry, retry/error behavior, repeated sessions, lifecycle state, or privacy equivalence.
- **Interpretation:** formal work should define a property/attacker matrix first, then keep ProVerif/Tamarin semantics synchronized through one canonical model source or mechanically synchronized representation. Anonymity and unlinkability must be separate claims, and observable abort/error/timing behavior belongs in the privacy model.
- **Affected work:** R-004, R-015, TD-003, zk207/zk209/zk217/zk218/zk219
- **Required next evidence:** pinned reproducible formal environment, canonical state model, privacy equivalence properties, model-to-code traceability, retained proofs/counterexamples
- **Claim boundary:** symbolic verification does not validate constant time, RNG, memory safety, custom proof security, or real timing leakage.

### F0817-08 — BBS and PQ/hybrid work remain optional research, not constrained-baseline replacements

- **Status:** confirmed
- **Evidence basis:** `../research/daily/2026-08-17.md` and earlier 2026-08-15 baseline; R-002 and R-003
- **Repository fact:** `iot-core` remains the constrained interoperability floor and current roadmap already isolates heavyweight credential/PQ work.
- **Interpretation:** available evidence strengthens the benchmark case but also reinforces packet/CPU/RAM pressure. No weekly evidence justifies changing the mandatory suite or role-proof baseline.
- **Affected work:** R-002, R-003, zk223/zk224
- **Required next evidence:** exact proof/ciphertext bytes, CPU/RAM/flash, fragmentation/loss behavior, dependency maturity, external review
- **Claim boundary:** research relevance is not promotion.

## 3. Evidence and provenance map

| Finding | Daily provenance | Primary repository destinations |
|---|---|---|
| F0817-01 | 08-17 | R-007 / TD-001 |
| F0817-02 | 08-17, 08-19 | R-005 / TD-002 |
| F0817-03 | 08-18, 08-21, 08-22 | R-008 / R-013 |
| F0817-04 | 08-16 baseline, 08-20, 08-21 | R-001 / R-011 / TD-004 |
| F0817-05 | 08-18, 08-19 | R-009 / R-010 |
| F0817-06 | 08-22 | R-014 |
| F0817-07 | 08-18, 08-23 | R-004 / R-015 / TD-003 |
| F0817-08 | 08-15 baseline, 08-17 | R-002 / R-003 |

## 4. Duplicate / rejected / superseded interpretations

- “Add a new primitive because a standards draft exists” is rejected. The repeated evidence instead supports measurement, review, and versioned optional profiles.
- “PID changes imply unlinkability” is rejected as an assurance shortcut; explicit attacker models and observable-failure behavior are required.
- “A successful role proof is authorization” is rejected as an overly broad interpretation.
- “An MCU timing benchmark is enough for field readiness” is rejected; entropy, storage, accelerator, restart, and execution context are part of the evidence.

## 5. Impact map

The week strengthens existing backlog/debt rather than creating a parallel roadmap. The most direct debt linkage remains:

- TD-001 ← F0817-01
- TD-002 ← F0817-02
- TD-003 ← F0817-07
- TD-004 ← F0817-03/F0817-04/F0817-05

No debt item is cleared by research alone.

## 6. Carry-forward / unresolved questions

Carry into the next week:

1. revocation convergence and maximum stale-authorization windows;
2. resumption authorization revalidation and identifier reuse/privacy;
3. benchmark `crypto_execution_context` metadata;
4. deployment-context evidence before field/readiness claims;
5. whether any research item has enough evidence and explicit human intent to become a bounded weekly request.