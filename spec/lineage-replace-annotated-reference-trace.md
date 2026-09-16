# ZK-ARCHE LINEAGE_REPLACE Annotated Reference Trace

Status: **draft normative trace / storage-neutral / wire-unassigned**.

This document is an independently readable trace companion to `spec/lineage-replace-state-machine.md`. It does not allocate a wire message, registry value, storage format, or cryptographic construction. It explains the already-defined logical transitions using the canonical `rust/test-vectors/replay/lineage-replace-states-v1.txt` corpus so an independent implementation can reproduce the same transition decisions without inferring behavior from Rust or C source.

Normative keywords **MUST** and **MUST NOT** are used in the BCP 14 sense where they restate behavior already defined by the controlling state-machine contract.

## 1. Trace notation

Each trace has the form:

```text
pre-state + event + plan-class -> transitioned? + post-state
```

The canonical plan classes are descriptive qualification labels:

- `full`: every required semantic consequence is present;
- `none`: no accepted replacement plan is supplied;
- `partial`: one or more required consequences are absent;
- `resumption_not_invalidated`: the plan omits predecessor-bound resumption invalidation.

A complete plan requires predecessor retirement, successor activation, and invalidation of predecessor-bound session keys, resumption state, authorization cache, identity-attribution cache, channel binding, and replay state. These labels do not define a wire encoding.

## 2. Successful staging — LS-01

```text
ACTIVE_PREDECESSOR + BEGIN + full
    -> transitioned = true
    -> REPLACEMENT_PENDING
```

Interpretation:

1. replacement admissibility has already been decided by the upstream authorization predicate;
2. `BEGIN` receives the complete accepted semantic plan;
3. the machine enters internal `REPLACEMENT_PENDING` state;
4. predecessor retirement and successor activation are not yet represented as committed;
5. ordinary AUTH cannot generate this transition.

An implementation MUST NOT treat `REPLACEMENT_PENDING` as remote proof that durable mutation succeeded. It is internal lifecycle state only.

## 3. Incomplete staging fails closed — LS-02, LS-03, LS-15

The following all leave the predecessor active:

```text
LS-02: ACTIVE_PREDECESSOR + BEGIN + none
    -> false + ACTIVE_PREDECESSOR

LS-03: ACTIVE_PREDECESSOR + BEGIN + partial
    -> false + ACTIVE_PREDECESSOR

LS-15: ACTIVE_PREDECESSOR + BEGIN + resumption_not_invalidated
    -> false + ACTIVE_PREDECESSOR
```

LS-15 is security-significant: predecessor-bound resumption state is dependent authority state. A replacement plan that would preserve it is incomplete and MUST NOT begin replacement.

The same rule applies to every other required consequence; the named corpus case highlights resumption because stale resumption state could otherwise preserve predecessor authority after lineage change.

## 4. Direct commit is forbidden — LS-04

```text
ACTIVE_PREDECESSOR + COMMIT + full
    -> false + ACTIVE_PREDECESSOR
```

A complete plan does not authorize skipping staging. `COMMIT` is valid only from `REPLACEMENT_PENDING`. This prevents an implementation from collapsing the logical ordering merely because all semantic flags are present.

## 5. Successful logical commit — LS-06

```text
REPLACEMENT_PENDING + COMMIT + full
    -> true + ACTIVE_SUCCESSOR_PREDECESSOR_RETIRED
```

The post-state means the logical contract jointly requires:

```text
predecessor retired
successor active
predecessor session keys invalid
predecessor resumption state invalid
predecessor authorization cache invalid
predecessor identity-attribution cache invalid
predecessor channel binding invalid
predecessor replay state invalid
```

This is a semantic commitment, not evidence that a physical store performed an atomic or rollback-resistant transaction. Durable persistence and crash recovery remain separate qualification obligations.

## 6. Incomplete commit fails closed — LS-07, LS-08, LS-16

```text
LS-07: REPLACEMENT_PENDING + COMMIT + none
    -> false + REPLACEMENT_PENDING

LS-08: REPLACEMENT_PENDING + COMMIT + partial
    -> false + REPLACEMENT_PENDING

LS-16: REPLACEMENT_PENDING + COMMIT + resumption_not_invalidated
    -> false + REPLACEMENT_PENDING
```

The machine MUST NOT report successor activation when any required invalidation is absent. In particular, LS-16 prevents a staged replacement from committing while predecessor resumption authority survives.

A target-specific durable implementation must map an inability to complete required persistence to its reviewed recovery contract; this logical layer does not invent recovery from a partial durable write.

## 7. Interruption is terminal at this layer — LS-10 through LS-12

```text
LS-10: REPLACEMENT_PENDING + INTERRUPT + none
    -> true + CONTINUITY_BROKEN

LS-11: CONTINUITY_BROKEN + BEGIN + full
    -> false + CONTINUITY_BROKEN

LS-12: CONTINUITY_BROKEN + COMMIT + full
    -> false + CONTINUITY_BROKEN
```

Once replacement has begun, an interruption moves this storage-neutral layer to `CONTINUITY_BROKEN`. Neither a new `BEGIN` nor `COMMIT` repairs it. A higher-level authenticated recovery mechanism may eventually be specified, but this machine MUST NOT synthesize one.

By contrast, LS-05 confirms that an unrelated `INTERRUPT` while still in `ACTIVE_PREDECESSOR` does not create a replacement transition or continuity break.

## 8. No duplicate replacement after activation — LS-13 and LS-14

```text
LS-13: ACTIVE_SUCCESSOR_PREDECESSOR_RETIRED + BEGIN + full
    -> false + ACTIVE_SUCCESSOR_PREDECESSOR_RETIRED

LS-14: ACTIVE_SUCCESSOR_PREDECESSOR_RETIRED + COMMIT + full
    -> false + ACTIVE_SUCCESSOR_PREDECESSOR_RETIRED
```

The completed state is terminal for this logical replacement instance. A later lineage transition requires a separately authorized lifecycle instance whose predecessor is the then-current active lineage; it is not a replay of the completed transition.

## 9. Deterministic independent-implementation checklist

An implementation claiming this logical state-machine behavior MUST be able to reproduce all canonical cases with the same transition bit and post-state. Qualification should additionally verify that:

- normal AUTH cannot supply authoritative `BEGIN` or `COMMIT`;
- `REPLACEMENT_PENDING` is not remotely selectable;
- every required plan consequence participates in completeness;
- a missing resumption invalidation is rejected at both staging and commit;
- `CONTINUITY_BROKEN` has no recovery transition in this layer;
- successful logical commit cannot be interpreted as physical atomicity evidence.

The canonical corpus remains the executable semantic authority for these trace cases. This document explains the cases; it does not fork or replace their meanings.

## 10. Claim boundary

This trace advances RFC-class independently implementable documentation for the existing storage-neutral lineage-replacement state machine. It does **not** establish:

- a LINEAGE_REPLACE wire grammar or registry allocation;
- durable atomic storage;
- power-loss recovery correctness;
- rollback-resistant persistence or hardware monotonicity;
- secure erasure;
- traffic-key rekey/exhaustion semantics;
- cryptographic authorization correctness;
- privacy or unlinkability;
- physical-target measurements;
- a fresh Rust/C qualification PASS;
- formal-analysis success;
- external review;
- RFC or IETF status;
- deployment qualification.

Those claims require their own declared evidence. If future normative lifecycle behavior changes, this trace and the canonical corpus MUST be reconciled together before the new behavior is promoted as conformant.
