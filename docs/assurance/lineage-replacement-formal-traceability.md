# Lineage-Replacement Formal Traceability Record

Status: **scoped traceability evidence; not a fresh formal result**.

This record binds the synchronized lineage-replacement ProVerif abstraction to the normative and executable repository surfaces it is intended to represent. It advances TD-003 model→spec→Rust/C→test traceability only. It does not claim that ProVerif was executed for the current model, prove storage atomicity, prove rollback resistance, establish computational cryptographic security, or close lineage/rekey lifecycle qualification.

## 1. Reconciliation base

```text
repository_commit = 4a1236d19c7b6c0dadff8f69247a62657885d4f5
branch            = dev
model paths       = rust/models/proverif/zk_arche_lineage_replace_commit_draft.pv
                    c/models/proverif/zk_arche_lineage_replace_commit_draft.pv
model blob SHA    = 50f4874ed3bfa91ef2f46454fade16c57bea1d25
```

The Rust and C model files are byte-identical synchronized copies. They are one symbolic model mirrored into both implementation lanes, not independent formal implementations.

No exact-current ProVerif execution is asserted by this record. Until repository-owned formal qualification executes this exact model blob and retains its tool/version/query/output provenance, the properties below are **modeled queries awaiting fresh execution**, not fresh `FORMALLY ANALYZED` evidence.

## 2. Modeled lifecycle boundary

The model deliberately begins after a replacement request has already passed the concrete authorization/admission checks. Its modeled durable transaction is:

```text
ReplacementAuthorized
        ↓
PendingPersisted
        ↓
SuccessorActivated
        ↓
PredecessorRetired
        ↓
DependentStateInvalidated
        ↓
PendingCleared
```

The intended concrete dependent-state invalidation boundary includes predecessor-bound session keys, resumption state, authorization/attribution caches, channel-binding state, and replay state where owned by the lineage-replacement implementation. The symbolic model collapses those concrete invalidations into one `DependentStateInvalidated` event; it therefore proves no per-store erasure, persistence, atomicity, or crash-recovery property.

The recovery submodel separately distinguishes a legitimate interrupted replacement from an invalid replacement attempt:

```text
local AwaitingConfirmation + peer AwaitingConfirmation
        → PairPredecessorReady

invalid replacement attempt
        → ReconciliationRequired
```

An invalid attempt has no modeled transition to `PairPredecessorReady`.

## 3. Property-to-repository map

| Property/query | Specification/design meaning | Rust/C implementation/test anchor | Evidence boundary |
|---|---|---|---|
| `PendingPersisted(d) ==> ReplacementAuthorized(d)` | no durable lineage mutation before accepted replacement authorization | lineage-replacement decision/plan and storage-transaction surfaces in both lanes | symbolic ordering only; concrete authorization correctness remains executable/spec evidence |
| `SuccessorActivated(d) ==> PendingPersisted(d)` | successor activation cannot precede durable pending evidence | Rust/C lineage replacement storage transaction | no filesystem/flash atomicity claim |
| `PredecessorRetired(d) ==> SuccessorActivated(d)` | predecessor is not retired before successor activation | Rust/C replacement transaction and transaction corpus | no power-loss or rollback-resistant persistence claim |
| `DependentStateInvalidated(d) ==> PredecessorRetired(d)` | predecessor-bound derived authority/state is invalidated only after predecessor retirement | Rust/C invalidation plan covering session/resumption/authz/channel/replay state | symbolic event aggregates multiple concrete stores |
| `PendingCleared(d) ==> DependentStateInvalidated(d)` | commit marker cannot clear before dependent state invalidation | Rust/C storage transaction and interruption cases | no physical durable-write guarantee |
| `PendingCleared(d) ==> ReplacementAuthorized(d)` | a committed replacement remains downstream of authorization | replacement plan + storage transaction | authorization provenance/cryptography not modeled |
| `PairPredecessorReady(d) ==> LocalAwaitingConfirmation(d)` | recovered predecessor continuity is usable only for a legitimate local in-flight attempt | Rust/C asymmetric-durable reconciliation classifier | current model does not distinguish every concrete invalid-attempt reason |
| `PairPredecessorReady(d) ==> PeerAwaitingConfirmation(d)` | both peers must agree that replacement is awaiting authenticated confirmation | Rust/C asymmetric-durable reconciliation classifier | peer transport/authentication details abstracted |
| `ReconciliationRequired(d) ==> InvalidReplacementAttempt(d)` | invalid replacement attempts fail closed instead of becoming healthy predecessor continuity | shared asymmetric-durable negative corpus, including the fail-closed attempt-identity case | one bounded invalid-attempt abstraction, not full fault taxonomy |

## 4. Attacker and abstraction gaps

This model is a lifecycle/order abstraction, not a complete network/cryptographic protocol model. In particular it does not establish:

- correctness or unforgeability of the replacement authorization or role-membership proof;
- holder, issuer, audience, deployment, authorization-generation, revocation-epoch, or lineage cryptographic binding;
- constant-time behavior, RNG/DRBG quality, memory safety, zeroization, secure key storage, or side-channel resistance;
- filesystem/flash transaction atomicity, torn-write behavior, power-loss behavior, rollback-resistant freshness, monotonic counters, or clone resistance;
- that every concrete predecessor-bound key/cache/store is erased or durably invalidated;
- rekey usage thresholds, traffic-key exhaustion behavior, ticket/PSK lifetime, or application/DATA-key lifecycle;
- revocation convergence across disconnected peers;
- transport identity/channel-exporter correctness;
- physical constrained-target behavior or resource bounds.

Those remain separate implementation, qualification, TD-002, TD-003, TD-004, or external-review obligations.

## 5. Traceability rule for future changes

A change to lineage replacement is security-significant if it changes authorization-before-mutation, transaction order, predecessor/successor activation semantics, dependent-state invalidation, reconciliation classification, or the conditions under which predecessor continuity is usable.

Such a change must keep these evidence lanes synchronized:

```text
normative lifecycle/specification
        ↕
Rust implementation
        ↕
C implementation
        ↕
shared deterministic positive/negative corpus
        ↕
synchronized formal model copies
        ↕
retained exact-model formal result
```

A model edit invalidates any claim that an older retained ProVerif run analyzes the edited model. A code/test edit without a corresponding semantic model change may leave the model text unchanged, but this record must not be used to imply model-to-code equivalence. Conversely, a successful symbolic run cannot establish concrete storage atomicity, computational soundness, target behavior, or deployment qualification.

## 6. Current TD-003 disposition

This record closes a **traceability documentation gap** for the existing lineage-replacement model. TD-003 remains open. The next evidence-producing steps are:

1. execute repository-owned formal qualification against the exact synchronized model blob and retain tool/version/query/output provenance;
2. expand the bounded `InvalidReplacementAttempt` abstraction only when the concrete/spec fault taxonomy is stable enough to avoid the model outrunning behavior;
3. connect lineage/replacement properties to the canonical formal property/attacker matrix;
4. preserve explicit separation between symbolic lifecycle ordering and concrete crash/rollback/storage qualification;
5. add traffic-key usage/exhaustion/rekey properties only after those lifecycle semantics exist normatively and in Rust/C.
