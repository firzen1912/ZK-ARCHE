# LINEAGE_REPLACE verified commit contract

Status: implementation-owned, wire-neutral lifecycle contract. This text does not allocate a packet, field, registry value, cryptographic primitive, or production AUTH-v3 negotiation behavior.

## 1. Purpose

This contract composes the existing verified lifecycle evidence chain with the ordered storage transaction so that a lifecycle request cannot reach durable mutation through independently manufactured booleans.

The required order is:

```text
verified current-credential possession
+ verified successor-key possession
+ exact AUTH session/context binding
+ current local iot-core authorization/attribution
+ predecessor binding + no privilege expansion
        -> AUTHORIZED_REPLACEMENT
        -> normalized LINEAGE_REPLACE predicate
        -> complete replacement plan
        -> PERSIST_PENDING
        -> ACTIVATE_SUCCESSOR
        -> RETIRE_PREDECESSOR
        -> INVALIDATE_DEPENDENT_STATE
        -> CLEAR_PENDING
```

## 2. Pre-storage hard gate

A conforming lifecycle commit entrypoint MUST NOT invoke any storage mutation callback unless all of the following hold:

- the typed possession results are valid for the current session and exact predecessor/successor references;
- the current AUTH completion, protocol version, suite, profile, session identifier, authorization-context hash, and channel-binding hash match the expected authenticated session;
- the local authorization and attribution record is current and identifies the expected peer/predecessor;
- the requested successor scope does not exceed the currently authorized scope;
- the normalized lifecycle predicate accepts predecessor, successor, context/dependent-state, freshness, replay, concurrency, rollback, and storage-safety facts;
- a complete replacement plan is produced.

The caller-supplied `authority_valid` field in the lower-level normalized facts is not authoritative at this entrypoint. Authorization is derived by the verified bound `iot-core` classifier and overwrites that bit before evaluation.

Any pre-storage rejection MUST produce an empty storage trace. AUTH remains NO-LEARNING; rejection cannot create, expand, repair, or infer trust.

## 3. Storage failure behavior

Once `PERSIST_PENDING` is attempted, storage results follow `lineage-replace-storage-transaction-contract.md`. The orchestrator MUST preserve the exact storage result. It MUST NOT compensate, clear pending, reactivate the predecessor, or infer commit after an intermediate failure.

`STORAGE_COMMITTED` means only that the supplied adapter reported every logical durability step successful in order. It does not establish physical atomicity, power-loss survival, malicious rollback resistance, secure erasure, or deployment qualification.

## 4. Shared evidence

`rust/test-vectors/replay/lineage-replace-verified-commit-v1.txt` is the canonical decision/trace corpus for this composition boundary. Rust and C consumers must agree on authorization result, normalized decision, storage result, and callback trace.

The negative corpus includes failed possession verification, failed authenticated-session completion, privilege expansion, stale freshness, replay rejection, and storage failure before/after the pending marker.

## 5. Explicit remaining gaps

This contract consumes typed upstream possession-verification results; it does not define the cryptographic lifecycle request that produces them. It also remains storage-neutral. Target-specific persistence, power-cut/restart behavior, trusted freshness/high-water mapping, flash wear/endurance, secure-element semantics, and malicious snapshot rollback require separate evidence. Independent cryptographic review, complete formal coverage, and RFC-class documentation remain separate gates.
