# LINEAGE_REPLACE Logical State-Machine Checkpoint — 2026-08-30

## Scope

This checkpoint covers one bounded zk213 implementation packet: a shared storage-neutral predecessor→successor logical state machine downstream of the already implemented `LINEAGE_REPLACE` decision predicate and commit planner.

It does not allocate wire behavior, perform durable writes, mutate a production trust store, claim crash-safe persistence, or close TD-003/TD-004.

## Preconditions consumed

- `spec/replay-epoch-transition-owner.md` owns the semantic transition and requires `ACTIVE(predecessor) -> REPLACEMENT_PENDING -> ACTIVE(successor) + RETIRED(predecessor)` with fail-closed ambiguity handling.
- `spec/lineage-replace-decision-contract.md` and the RE-01..RE-20 corpus own normalized replacement admissibility.
- the existing commit planner requires predecessor retirement, successor activation, and invalidation of predecessor-bound session/resumption/authz/attribution/channel/replay state.
- normal AUTH remains NO-LEARNING and is not a lifecycle transition authority.

## Changed surfaces

```text
rust/crates/proto/src/lineage_replace.rs
rust/crates/proto/tests/lineage_replace_corpus.rs
c/include/auth/lineage_replace.h
c/src/proto/lineage_replace.c
c/tests/test_lineage_replace.c
rust/test-vectors/replay/lineage-replace-states-v1.txt
spec/lineage-replace-state-machine.md
```

## Shared state contract

Both implementations expose the same logical states:

```text
ACTIVE_PREDECESSOR
REPLACEMENT_PENDING
ACTIVE_SUCCESSOR_PREDECESSOR_RETIRED
CONTINUITY_BROKEN
```

Both expose the same internal events:

```text
BEGIN
COMMIT
INTERRUPT
```

A complete accepted commit plan is required to enter `REPLACEMENT_PENDING` and again to logically commit the successor. Missing or partial plans do not advance state. Interruption from pending moves to `CONTINUITY_BROKEN`. This layer intentionally has no recovery event from `CONTINUITY_BROKEN`.

## Negative evidence

The shared LS-01..LS-14 corpus explicitly falsifies these unsafe shortcuts:

- commit without staging;
- staging or commit with no plan;
- staging or commit with an incomplete invalidation plan;
- repeated staging while already pending;
- recovery from `CONTINUITY_BROKEN` without separately specified authority/storage evidence;
- new replacement activity after successor activation through this one-shot machine;
- treating an unrelated interruption while the predecessor is active as a replacement transition.

The earlier RE corpus continues to reject restart, transport change, ordinary AUTH, stale/replayed transitions, wrong lineage/context, competing successors, downgrade, rollback, and ambiguous storage before any commit plan can be produced.

## Validation boundary

A narrow standalone C compilation of the new state-machine implementation shape was executed with:

```text
gcc -std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Werror
```

and the accepted begin/commit path, interruption-to-broken path, and no-recovery-from-broken path passed in the available environment.

A clean repository checkout and local Rust toolchain were unavailable because this runtime could not resolve `github.com`; therefore no local full C suite, sanitizer, Rust, formal, or release-qualification result is claimed here. Hosted exact-head CI after publication is authoritative.

## Claim boundary

Before hosted exact-head qualification completes, the new packet is only **implemented / CI-pending**.

Even after successful qualification, the evidence is limited to storage-neutral logical state compatibility. It does not establish durable atomicity, power-loss recovery, rollback-resistant persistence, secure erasure, physical-target behavior, wire-level recovery interoperability, PFS/PCS/KCI, external review, Common Contract conformance, RFC-class completion, or deployment qualification.

TD-001, TD-002, TD-003, and TD-004 remain open.
