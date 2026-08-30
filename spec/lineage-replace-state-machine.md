# ZK-ARCHE LINEAGE_REPLACE Logical State Machine

Status: **draft normative logical-state contract / wire-unassigned / storage-neutral**.

This document defines the minimum logical state machine exercised by the shared Rust/C `LINEAGE_REPLACE` implementation. It is downstream of the authorization/decision predicate and commit planner. It does not define durable storage, crash recovery, rollback-resistant hardware, a network message, a registry allocation, or production trust-store mutation.

Normative keywords **MUST** and **MUST NOT** are used in the BCP 14 sense where behavior is precise and testable.

## 1. State vocabulary

The storage-neutral machine has exactly these logical states:

```text
ACTIVE_PREDECESSOR
REPLACEMENT_PENDING
ACTIVE_SUCCESSOR_PREDECESSOR_RETIRED
CONTINUITY_BROKEN
```

`REPLACEMENT_PENDING` is internal lifecycle state. It MUST NOT be remotely selectable. `CONTINUITY_BROKEN` is fail-closed: this layer has no transition out of it.

## 2. Events

The machine recognizes three internal events:

```text
BEGIN
COMMIT
INTERRUPT
```

`BEGIN` and `COMMIT` MUST be supplied with a complete commit plan produced only after `ACCEPT_SUCCESSOR`. A missing or incomplete plan MUST NOT advance state.

`INTERRUPT` represents an interruption after replacement has entered `REPLACEMENT_PENDING`. It does not prove a specific durable-storage failure mode; it provides the common fail-closed logical consequence required before durable storage is designed.

Normal AUTH MUST NOT generate `BEGIN` or `COMMIT`. Restart, transport change, ordinary negotiation, empty replay state, and ordinary successful AUTH remain non-authoritative for trust mutation.

## 3. Allowed transitions

Only these transitions are valid:

```text
ACTIVE_PREDECESSOR
  -- BEGIN + complete accepted plan -->
REPLACEMENT_PENDING

REPLACEMENT_PENDING
  -- COMMIT + complete accepted plan -->
ACTIVE_SUCCESSOR_PREDECESSOR_RETIRED

REPLACEMENT_PENDING
  -- INTERRUPT -->
CONTINUITY_BROKEN
```

Every other state/event combination MUST leave state unchanged and report that no transition occurred.

The logical `COMMIT` state records only that the planned semantic consequences are jointly required. It does not claim they were durably written, made rollback-resistant, or atomically persisted on a physical target.

## 4. Plan completeness

A plan is complete only when all currently required consequences are present:

```text
retire predecessor
activate successor
invalidate predecessor-bound session keys
invalidate predecessor-bound resumption state
invalidate authorization cache
invalidate identity-attribution cache
invalidate channel binding
invalidate replay state
```

A partially populated plan MUST fail closed at both `BEGIN` and `COMMIT`.

## 5. Shared Rust/C conformance corpus

`rust/test-vectors/replay/lineage-replace-states-v1.txt` is the canonical logical-state corpus. Both implementations MUST consume the same cases.

The corpus covers:

- accepted staging from `ACTIVE_PREDECESSOR`;
- rejection of missing and partial plans;
- rejection of direct commit before staging;
- accepted logical commit from `REPLACEMENT_PENDING`;
- fail-closed interruption to `CONTINUITY_BROKEN`;
- no recovery from `CONTINUITY_BROKEN` in this layer;
- no second begin/commit after successor activation;
- no unrelated interruption from the predecessor-active state.

This corpus is semantic evidence only. It MUST NOT be treated as a wire grammar.

## 6. Evidence and claim boundary

After Rust/C hosted qualification succeeds, this state machine may be described as **IMPLEMENTED + TESTED + cross-language decision/state compatible** for its storage-neutral semantics.

It does not establish:

- durable atomic replacement;
- power-loss recovery correctness;
- rollback-resistant persistence;
- hardware monotonicity;
- secure erasure;
- a production successor-activation API;
- wire interoperability for recovery;
- PFS, PCS, or KCI resistance;
- complete zk213 fulfillment;
- Common Contract conformance;
- RFC-class documentation;
- deployment qualification.

Durable storage/recovery must map target-specific persistence behavior back to this logical state machine without weakening its fail-closed transitions.
