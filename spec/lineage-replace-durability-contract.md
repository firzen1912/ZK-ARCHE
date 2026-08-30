# ZK-ARCHE LINEAGE_REPLACE Durability Observation Contract

Status: **draft normative logical recovery contract / storage-neutral / wire-unassigned**.

This document defines how a future durable `LINEAGE_REPLACE` implementation must classify restart observations before it may resume protocol operation. It does not define a storage engine, journal format, filesystem primitive, flash transaction, monotonic counter, secure element, physical rollback resistance, or recovery wire message.

Normative keywords **MUST** and **MUST NOT** are used in the BCP 14 sense where behavior is precise and testable.

## 1. Recovery principle

A restart MUST recover to an active lineage only when the persisted observation is unambiguous and internally consistent with one of two stable logical states:

```text
ACTIVE_PREDECESSOR
ACTIVE_SUCCESSOR_PREDECESSOR_RETIRED
```

`REPLACEMENT_PENDING` MUST NOT be reconstructed as an operable state after restart. A pending, mixed, partial, corrupt, empty, or otherwise ambiguous observation MUST recover as `CONTINUITY_BROKEN`.

This rule prevents restart from becoming an implicit trust-mutation or rollback path. Normal AUTH remains NO-LEARNING and MUST NOT repair or advance lineage state.

## 2. Normalized restart observation

The shared Rust/C classifier consumes only normalized facts:

```text
record_integrity_valid
predecessor_active
replacement_pending
successor_active
predecessor_retired
invalidations_complete
```

These fields are semantic observations supplied by a future storage adapter. They are not a prescribed on-disk encoding.

## 3. Stable predecessor recovery

Recovery MAY return `ACTIVE_PREDECESSOR` only when all of the following hold:

```text
record_integrity_valid = true
predecessor_active = true
replacement_pending = false
successor_active = false
predecessor_retired = false
invalidations_complete = false
```

Any extra replacement evidence makes the observation ambiguous and therefore fail-closed.

## 4. Stable successor recovery

Recovery MAY return `ACTIVE_SUCCESSOR_PREDECESSOR_RETIRED` only when all of the following hold:

```text
record_integrity_valid = true
predecessor_active = false
replacement_pending = false
successor_active = true
predecessor_retired = true
invalidations_complete = true
```

The `invalidations_complete` observation means the future storage adapter has evidence that the predecessor-bound invalidation set required by the commit planner was included in the same completed logical transition. It does not prove secure erasure or physical rollback resistance.

## 5. Fail-closed observations

All other observations MUST map to `CONTINUITY_BROKEN`, including:

- any integrity failure;
- any surviving `REPLACEMENT_PENDING` marker;
- predecessor and successor simultaneously active;
- successor active without predecessor retirement;
- predecessor retired without successor activation;
- successor activation with incomplete dependent-state invalidation;
- empty or missing normalized state;
- contradictory predecessor state and completed invalidation evidence.

The classifier intentionally does not attempt to infer which write happened first or to complete an interrupted transition.

## 6. Canonical cross-language corpus

`rust/test-vectors/replay/lineage-replace-recovery-v1.txt` is the canonical storage-neutral restart corpus. Rust and C MUST consume the same cases and produce the same recovered logical state.

The corpus is decision/state evidence only. It is not a storage-format vector and MUST NOT be used to claim crash-safe persistence on any target.

## 7. Claim boundary

Hosted Rust/C qualification may establish **IMPLEMENTED + TESTED + cross-language recovery-decision compatible** for this normalized classifier.

It does not establish:

- durable atomic storage;
- filesystem or flash crash consistency;
- physical power-loss survival;
- rollback-resistant storage or monotonic hardware;
- secure erasure;
- production trust-store mutation;
- authenticated recovery wire behavior;
- PFS, PCS, or KCI resistance;
- complete zk213 fulfillment;
- Common Contract conformance;
- RFC-class completion;
- deployment qualification.

A future storage adapter must retain target-specific evidence showing how real persistence observations are normalized into this classifier without weakening its fail-closed behavior.
