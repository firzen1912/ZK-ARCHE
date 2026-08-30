# ZK-ARCHE LINEAGE_REPLACE Logical Storage Transaction Contract

Status: **draft normative logical transaction contract / storage-neutral / wire-unassigned**.

This contract defines the ordering that a future storage adapter must preserve when persisting an accepted `LINEAGE_REPLACE` commit plan. It is intentionally weaker than a claim of filesystem, flash, journal, secure-element, or physical power-loss atomicity.

## 1. Required logical order

A future adapter MUST make the replacement-pending condition durable before making any successor state visible. It MUST NOT clear that pending condition until all of the following logical effects are durably represented by the adapter:

1. successor active;
2. predecessor retired;
3. predecessor-bound invalidation set complete.

Conceptually:

```text
stable predecessor
  -> pending marker
  -> successor activation
  -> predecessor retirement
  -> dependent-state invalidations
  -> pending marker clear
  -> stable committed successor
```

The ordering is a specification contract, not a prescribed storage format.

## 2. Restart semantics at every cut

`rust/test-vectors/replay/lineage-replace-write-cuts-v1.txt` defines canonical logical write cuts. Rust and C independently map each cut into the normalized facts consumed by the restart classifier.

Only the two stable endpoints may recover operably:

- before the transaction begins: `ACTIVE_PREDECESSOR`;
- after the final pending-marker clear: `ACTIVE_SUCCESSOR_PREDECESSOR_RETIRED`.

Every intermediate cut MUST recover as `CONTINUITY_BROKEN`. The implementation MUST NOT infer completion, resume a pending mutation, or use normal AUTH to repair lineage state.

## 3. Adapter obligation

A production adapter MUST document how its actual persistence primitives implement or strengthen this logical order. If the adapter can expose an intermediate, torn, reordered, corrupt, or otherwise ambiguous observation after restart, that observation MUST normalize to a fail-closed classifier input.

An adapter MUST NOT claim rollback resistance merely because it satisfies this ordering. Rollback resistance requires separate target-specific freshness evidence.

## 4. Claim boundary

The deterministic model can establish Rust/C agreement about expected restart classification at logical write cuts. It does not establish:

- real storage atomicity or write ordering;
- power-loss survival;
- filesystem journal guarantees;
- flash-program/erase behavior;
- secure-element transaction guarantees;
- anti-rollback monotonicity;
- secure erasure;
- production trust-store integration;
- deployment qualification.

Those remain target-specific evidence obligations under zk213/TD-002 and the RFC-class evidence gate.
