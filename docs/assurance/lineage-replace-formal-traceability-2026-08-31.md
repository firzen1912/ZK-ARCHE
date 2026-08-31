# LINEAGE_REPLACE formal traceability checkpoint — 2026-08-31

Status: **MODELED + IMPLEMENTATION-TRACEABLE; NOT FORMALLY ANALYZED**.

This checkpoint advances TD-003 by adding a synchronized symbolic model for the existing verified LINEAGE_REPLACE commit ordering. It does not claim a successful ProVerif run for the new model, cryptographic proof soundness, physical storage atomicity, rollback resistance, external review, RFC-class status, Common Contract completion, or deployment qualification.

## 1. Exact modeled boundary

Canonical model:

- `rust/models/proverif/zk_arche_lineage_replace_commit_draft.pv`

Synchronized mirror:

- `c/models/proverif/zk_arche_lineage_replace_commit_draft.pv`

Synchronization owner:

- `scripts/sync-formal-models.sh`

The model represents only the already-owned semantic chain:

```text
AUTHORIZED_REPLACEMENT
        -> complete accepted replacement plan
        -> PERSIST_PENDING
        -> ACTIVATE_SUCCESSOR
        -> RETIRE_PREDECESSOR
        -> INVALIDATE_DEPENDENT_STATE
        -> CLEAR_PENDING
```

It deliberately does not model the cryptographic construction of current-credential or successor-key possession proofs, AUTH-v3 parser correctness, concrete durable storage, flash/filesystem behavior, power interruption physics, rollback-resistant freshness anchors, secure erasure, or implementation memory safety.

## 2. Property inventory

The new model has six correspondence queries.

| ID | Symbolic property | Specification owner | Concrete implementation owner |
|---|---|---|---|
| LR-FM-01 | `PendingPersisted(d) ==> ReplacementAuthorized(d)` | `spec/lineage-replace-verified-commit-contract.md` §2 | `rust/crates/proto/src/lineage_replace_verified_commit.rs`; `c/src/proto/lineage_replace_verified_commit.c` |
| LR-FM-02 | `SuccessorActivated(d) ==> PendingPersisted(d)` | verified-commit contract §1; storage-transaction contract | Rust/C storage transaction executors |
| LR-FM-03 | `PredecessorRetired(d) ==> SuccessorActivated(d)` | storage-transaction contract | Rust/C storage transaction executors |
| LR-FM-04 | `DependentStateInvalidated(d) ==> PredecessorRetired(d)` | storage-transaction contract | Rust/C storage transaction executors |
| LR-FM-05 | `PendingCleared(d) ==> DependentStateInvalidated(d)` | storage-transaction contract | Rust/C storage transaction executors |
| LR-FM-06 | `PendingCleared(d) ==> ReplacementAuthorized(d)` | verified-commit contract §2–3 | verified-commit orchestrators + storage transaction executors |

These are ordering/correspondence properties only. They do not prove that an adapter reporting success actually persisted data, that persisted data survives a crash, or that an attacker cannot restore an older valid snapshot.

## 3. Model → spec → code → deterministic evidence

### Authorization before mutation

Normative/implementation-owned requirement:

- `spec/lineage-replace-verified-commit-contract.md` states that any pre-storage rejection must produce an empty storage trace and that storage mutation is unreachable until verified possession, exact AUTH session binding, current authorization/attribution, predecessor/scope checks, normalized lifecycle acceptance, and a complete plan succeed.

Rust/C owner:

- `lineage_replace_execute_verified_bound_iot_core_commit` / corresponding Rust function evaluates verified authorization and the authorized lifecycle decision before invoking `lineage_replace_execute_storage_transaction`.

Deterministic evidence:

- `rust/test-vectors/replay/lineage-replace-verified-commit-v1.txt`
- Rust/C verified-commit corpus consumers.

Symbolic owner:

- LR-FM-01 and LR-FM-06.

### Ordered durable transaction

Normative/implementation-owned requirement:

```text
PERSIST_PENDING
ACTIVATE_SUCCESSOR
RETIRE_PREDECESSOR
INVALIDATE_DEPENDENT_STATE
CLEAR_PENDING
```

Rust/C owner:

- `rust/crates/proto/src/lineage_replace_storage_transaction.rs`
- `c/src/proto/lineage_replace_storage_transaction.c`

Deterministic evidence:

- `rust/test-vectors/replay/lineage-replace-storage-transaction-v1.txt`
- Rust/C storage-transaction corpus consumers.

Symbolic owner:

- LR-FM-02 through LR-FM-05.

### Failure behavior

The symbolic state machine exposes explicit failure events after each durable intermediate state and has no transition from those failure branches to `PendingCleared`. This mirrors the implementation rule that an intermediate storage failure stops execution and does not compensate or infer commit.

This is a modeled control-flow boundary, not a retained theorem about real crash behavior. A successful ProVerif run is still required before the new model may be called FORMALLY ANALYZED.

## 4. Attacker and abstraction boundary

The public model command channel represents an attacker able to choose whether and when an abstract lifecycle path advances or fails. Private state channels represent accepted internal state transitions, not secret cryptographic material.

The model assumes that `ReplacementAuthorized` is emitted only after the concrete verified-authorization boundary has accepted. Therefore it cannot establish:

- computational soundness of the custom role proof;
- validity of possession-proof cryptography;
- AUTH-v3 parser/model equivalence;
- authorization-policy correctness outside the cited concrete contract;
- constant-time behavior or side-channel resistance;
- RNG quality, key storage, or secure erasure;
- filesystem/flash atomicity or wear behavior;
- power-loss survival;
- freshness/high-water integrity or malicious rollback resistance.

Those remain separate TD-001/TD-002/TD-003 qualification surfaces.

## 5. Qualification state

Repository evidence at creation time supports only:

```text
MODEL TEXT PRESENT
+ RUST/C MODEL MIRROR GOVERNED BY sync-formal-models.sh
+ MODEL→SPEC→RUST/C→VECTOR TRACEABILITY RECORDED
!= PROVERIF RESULT RETAINED
!= FORMALLY ANALYZED
!= IMPLEMENTATION VERIFIED
!= CRYPTOGRAPHICALLY PROVEN
!= STORAGE MEASURED
!= ROLLBACK RESISTANT
```

The execution environment for this checkpoint did not provide ProVerif, and a clean checkout could not be obtained because direct `github.com` DNS resolution failed. Accordingly no ProVerif pass/fail is claimed for the new model in this checkpoint.

## 6. Required next evidence

Before this model can advance from MODELED to FORMALLY ANALYZED:

1. run `scripts/sync-formal-models.sh --check` in a clean checkout;
2. run ProVerif against the exact canonical model with the repository-pinned/retained tool version;
3. retain the exact repository commit, model blob SHA, tool/version, query count, complete output summary, and any counterexample;
4. fail closed if any correspondence is unproved or the model/parser fails;
5. separately run the Rust/C deterministic verified-commit and storage-transaction corpora at the same exact repository state.

A green symbolic result must not be used to close physical persistence, rollback-freshness, computational-cryptography, memory-safety, or external-review gaps.
