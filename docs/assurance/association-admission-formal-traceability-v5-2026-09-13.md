# Association-admission formal traceability v5 — 2026-09-13

Status: **TEMPORAL RETAINED-AUTHORITY MODEL EXTENDED; FRESH EXACT-HEAD PROVERIF RESULT REQUIRED FOR FORMAL-RESULT CLAIMS**

This checkpoint supersedes `association-admission-formal-traceability-v4-2026-09-05.md` for the current CORE/LINK retained-association authority surface. It records the repository-owned temporal model now present in synchronized Rust/C copies and keeps the evidence boundary explicit: this is model/spec/implementation traceability, not a fresh proof result.

## 1. Temporal retained-authority property surface

`rust/models/proverif/zk_arche_retained_authority_draft.pv` and its synchronized C mirror model the rule that protected application use is authorized only while a retained association remains under current lifecycle authority.

The model requires any `ProtectedUseAllowed(a)` event to correspond to both `AssociationEstablished(a)` and `AuthorityCurrent(a)`. It also excludes protected use when any of these authoritative lifecycle conditions has become unsafe:

1. authorization generation stale;
2. revocation state stale;
3. explicit revocation observed;
4. lineage stale;
5. replay continuity lost;
6. restart continuity lost;
7. usage-counter continuity lost;
8. required binding lost.

These are deliberately modeled as authoritative Boolean lifecycle outcomes. The symbolic model does not derive revocation convergence, persistent monotonicity, channel cryptography, or storage rollback resistance.

## 2. Model → specification → implementation/evidence traceability

| Temporal property | Specification owner | Executable implementation/evidence surface | Symbolic boundary |
|---|---|---|---|
| retained use requires established association/current authority | `spec/core-association-admission.md` | association-admission lifecycle classifier and shared canonical corpus | models authorization decision, not traffic transport |
| stale authorization generation invalidates retained authority | authorization-generation lifecycle + CORE association spec | association-admission classifier/corpus | consumes generation-currentness result |
| stale revocation state invalidates retained authority | revocation lifecycle + CORE association spec | association-admission classifier/corpus | does not prove propagation latency |
| explicit revocation invalidates retained authority | revocation lifecycle + CORE association spec | association-admission classifier/corpus | does not prove convergence timing |
| stale lineage invalidates retained authority | lineage/revocation lifecycle specs | association-admission classifier/corpus | does not prove durable lineage storage |
| replay-continuity loss invalidates retained authority | replay continuity + CORE association spec | association-admission classifier/corpus | does not prove replay-store persistence |
| restart-continuity loss invalidates retained authority | lifecycle persistence/recovery specs | association-admission classifier/corpus | does not prove crash consistency |
| usage-counter-continuity loss invalidates retained authority | key-usage/lifecycle specs | association-admission classifier/corpus | does not prove monotonic storage |
| required-binding loss invalidates retained authority | binding/association specs | association-admission classifier/corpus | does not prove channel/exporter cryptography |

The current canonical association-admission corpus additionally qualifies deterministic fail-closed precedence among these lifecycle conditions. The ProVerif model proves/excludes property classes, not implementation-specific reason precedence.

## 3. Repository-owned formal gate

`scripts/ci-formal.sh` binds formal execution to exact repository HEAD and tracked model blob, verifies Rust/C model synchronization first, and now expects exactly **10** true results for the retained-authority model.

A valid `FORMALLY ANALYZED` claim for this model requires a fresh exact-head execution artifact from that gate. ProVerif was not executed by the cloud runner for this checkpoint; cloud-runner validation is unavailable while the user-confirmed local project-validation baseline remains green. No PASS is inherited from older model blobs or repository heads.

## 4. Explicit abstraction gaps

This temporal model does **not** establish:

- role-membership proof soundness or computational security;
- parser/wire correctness;
- constant-time behavior, memory safety, RNG quality, or entropy health;
- durable monotonic storage, atomicity, or physical rollback resistance;
- revocation-distribution convergence bounds;
- key erasure or transport teardown after authority invalidation;
- physical target behavior or MCU resource measurements;
- privacy/unlinkability beyond the modeled events;
- independent cryptographic review;
- RFC/IETF status or deployment qualification.

TD-003 therefore advances through stronger synchronized temporal coverage and traceability, while exact-head formal results and the wider model-to-code assurance envelope remain separate evidence obligations.
