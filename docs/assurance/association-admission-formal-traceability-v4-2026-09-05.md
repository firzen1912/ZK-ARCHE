# Association-admission formal traceability v4 — 2026-09-05

Status: **MODEL/SPEC/IMPLEMENTATION TRACEABILITY RECONCILED; FRESH EXACT-HEAD PROVERIF RESULT REQUIRED FOR FORMAL-RESULT CLAIMS**

This checkpoint supersedes `association-admission-formal-traceability-v3-2026-09-03.md` for the current CORE association-admission surface. It records what the synchronized ProVerif abstraction actually covers, what the Rust/C decision implementations and canonical corpus cover, and what remains outside the model after the newer association-authority-loss specification work.

This document is an assurance/traceability artifact. It is not a new formal proof result and does not inherit a PASS from an older model blob or repository HEAD.

## 1. Current synchronized symbolic surface

The canonical Rust/C association-admission ProVerif copies model the wire-neutral CORE/LINK admission composition after owning subsystems have already produced authoritative facts. The current model contains 16 queries.

An `AssociationEstablished(e)` event requires correspondence with:

1. `AuthComplete(e)`;
2. `PreexistingTrustRecord(e)`;
3. `AuthorizationPresent(e)`;
4. `AuthorizationFresh(e)`;
5. `AuthorizationGenerationBound(e)`;
6. `AuthorizationGenerationCurrent(e)`;
7. `RevocationCurrent(e)`;
8. `HolderNotRevoked(e)`;
9. `LineageCurrent(e)`;
10. `ReplayContinuityCurrent(e)`;
11. `RestartContinuityCurrent(e)`;
12. `UsageCounterContinuityCurrent(e)`;
13. `BindingSatisfied(e)`;
14. `RollbackClear(e)`.

The model additionally requires that establishment cannot coincide with:

15. `TrustMutationRequested(e)`; or
16. `ExplicitRevocationObserved(e)`.

The v4 change relative to the older traceability checkpoint is the explicit `UsageCounterContinuityCurrent` prerequisite. Loss of that continuity is treated as a nonce/key-reuse hazard independent of replay-window continuity and restart continuity.

## 2. Repository-owned formal gate

`scripts/ci-formal.sh` binds execution to the exact repository HEAD and tracked model blob. For `association-admission` it requires exactly 16 `RESULT ... is true.` lines and fails if ProVerif exits non-zero, reports a false/unproved result, the model differs from exact HEAD, the Rust/C model copies are unsynchronized, or the expected result count is not observed.

That gate is the authority for a fresh executable formal-result claim. This document does not replace it.

Cloud-runner limitation in this checkpoint: ProVerif execution is not available in the current cloud environment. The user-confirmed local project-validation baseline remains green, but no new ProVerif PASS is asserted here without an exact-head retained run artifact.

## 3. Model → specification → implementation → corpus traceability

| Formal fact/property | Specification owner | Rust/C implementation surface | Canonical executable evidence | Formal abstraction note |
|---|---|---|---|---|
| AUTH complete | `spec/core-association-admission.md` | association-admission classifier | `association-admission-v4.txt` | consumes authoritative AUTH completion fact |
| pre-existing trust | `spec/core-association-admission.md`, `spec/auth-trust-mutation-boundary.md` | association-admission classifier | `association-admission-v4.txt` | models local trust fact, not trust-store persistence |
| authorization present/fresh | `spec/core-association-admission.md`, authorization lifecycle specs | association-admission classifier | `association-admission-v4.txt` | does not model policy-distribution mechanics |
| authorization generation bound/current | `spec/authorization-generation-lifecycle.md`, `spec/core-association-admission.md` | association-admission classifier | `association-admission-v4.txt` | provenance/currentness are independent facts |
| revocation current + holder not revoked | `spec/core-association-admission.md`, revocation lifecycle specs | association-admission classifier | `association-admission-v4.txt` | does not model revocation dissemination latency |
| lineage current | lineage/revocation lifecycle specs | association-admission classifier | `association-admission-v4.txt` | does not model durable lineage-store physics |
| replay continuity current | `spec/replay-continuity.md`, `spec/core-association-admission.md` | association-admission classifier | `association-admission-v4.txt` | consumes replay-state continuity fact |
| restart continuity current | lifecycle persistence/recovery specs | association-admission classifier | `association-admission-v4.txt` | does not prove crash-consistent storage |
| usage-counter continuity current | lifecycle persistence/key-usage specs, `spec/core-association-admission.md` | association-admission classifier | `association-admission-v4.txt` | consumes counter-continuity fact; does not prove monotonic storage |
| binding satisfied | binding/association specs | association-admission classifier | `association-admission-v4.txt` | does not prove underlying transport/channel cryptography |
| rollback clear | lifecycle persistence/recovery specs | association-admission classifier | `association-admission-v4.txt` | consumes rollback assessment rather than modeling physical rollback resistance |
| AUTH cannot mutate trust | `spec/auth-trust-mutation-boundary.md`, `spec/core-association-admission.md` | association-admission classifier | negative corpus case(s) | symbolic exclusion covers the decision composition only |
| explicitly revoked holder cannot establish | revocation specs, `spec/core-association-admission.md` | association-admission classifier | negative corpus case(s) | symbolic exclusion does not prove convergence timing |

The table is intentionally directional: the ProVerif model consumes authoritative Boolean lifecycle facts. It does not cryptographically derive those facts and therefore cannot substitute for the implementation, persistence, transport, or dissemination evidence owned by their respective subsystems.

## 4. Newly explicit temporal abstraction gap: retained-association authority loss

`spec/core-association-admission.md` now makes a retained association's authority fail closed when re-evaluation returns `FAIL_CLOSED`: new security-sensitive application traffic must not remain authorized merely because transport state or traffic keys still exist. Reauthentication alone cannot repair stale lifecycle facts. A profile may retain transient key material only for narrowly scoped shutdown/cleanup mechanics without restoring application authority.

The current association-admission ProVerif model **does not model this temporal post-establishment lifecycle**. Its principal event is `AssociationEstablished(e)` and its queries constrain prerequisites for that event. It does not currently express a trace such as:

`AssociationEstablished -> lifecycle fact becomes stale/revoked -> AuthorityInvalidated -> no subsequent ApplicationUseAuthorized`

Therefore this checkpoint MUST NOT claim that ProVerif has established retained-association authority invalidation, key erasure, transport teardown, or prevention of post-invalidation application use.

A dependency-ready TD-003 follow-up is to add a small temporal/lifecycle model (or safely extend the existing model) with explicit events for retained-association re-evaluation, authority invalidation, and application-use authorization, then bind the resulting property to Rust/C lifecycle tests. That work should land only with synchronized models and fresh retained ProVerif evidence.

## 5. Other explicit abstraction boundaries

The current model also does not establish:

- cryptographic soundness of the role-membership proof;
- parser or wire-decoding correctness;
- constant-time behavior;
- RNG or entropy quality;
- memory safety;
- storage atomicity, monotonic-counter persistence, or rollback resistance;
- revocation-distribution convergence bounds;
- authorization-generation allocation/advancement correctness;
- transport-channel security or exporter correctness;
- physical MCU behavior or resource measurements;
- full privacy/unlinkability properties;
- independent cryptographic review;
- RFC/IETF status or deployment qualification.

Those remain separate evidence obligations under TD-001 through TD-004 and the relevant roadmap phases.

## 6. Claim discipline

The exact-current association-admission surface can be described as **implementation-linked and symbolically modeled** for the 16 listed admission prerequisites/exclusions. A stronger **FORMALLY ANALYZED at exact current HEAD** claim requires retained execution evidence from `scripts/ci-formal.sh` for the exact model blob and repository HEAD.

The newer retained-association authority-loss rule is currently **SPECIFIED**, but this checkpoint intentionally records it as not yet temporally modeled. That distinction is required to keep TD-003 progress evidence-honest.
