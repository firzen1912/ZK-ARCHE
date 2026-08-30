# LINEAGE_REPLACE transition-plan checkpoint — 2026-08-30

Status: **checkpoint / implementation evidence for a pure planner only**.

## Scope

This checkpoint records the smallest implementation step after the shared `RE-01`…`RE-20` decision corpus: Rust and C now derive the same wire-neutral predecessor→successor commit plan **only after** `LINEAGE_REPLACE` has already produced `ACCEPT_SUCCESSOR`.

The planner is intentionally non-mutating. It does not allocate a wire message, authenticate authority evidence, perform durable storage, activate trust, change replay state, or make normal AUTH trust-mutating.

## Affected surfaces

- `rust/crates/proto/src/lineage_replace.rs`
- `rust/crates/proto/tests/lineage_replace_corpus.rs`
- `c/include/auth/lineage_replace.h`
- `c/src/proto/lineage_replace.c`
- `c/tests/test_lineage_replace.c`
- `rust/test-vectors/replay/lineage-replace-plans-v1.txt`

No cryptographic primitive, domain separator, packet grammar, registry value, profile identifier, or formal model changed.

## Canonical planner invariant

For an internal decision other than `ACCEPT_SUCCESSOR`, no commit plan exists.

For `ACCEPT_SUCCESSOR`, both implementations produce the same conservative plan:

```text
retire_predecessor             = true
activate_successor             = true
invalidate_session_keys        = true
invalidate_resumption          = true
invalidate_authorization_cache = true
invalidate_attribution_cache   = true
invalidate_channel_binding     = true
invalidate_replay_state        = true
```

This is the fail-closed constrained-floor behavior. The specification permits explicit revalidation of some predecessor-bound state in the future, but such revalidation is not inferred here. It requires separately specified and tested lifecycle semantics.

## Negative/security properties retained

- ordinary AUTH still cannot produce an accepted replacement decision through the shared decision predicate;
- rejected replacement decisions cannot produce a commit plan;
- the C planner rejects a null output destination rather than silently treating it as success;
- the C planner clears the output plan before returning rejection, preventing stale caller state from appearing to authorize a commit;
- no planner path performs trust-store writes or durable mutation;
- no transport address, restart event, or session identifier becomes protocol identity or recovery authority.

## Evidence boundary

This packet advances **IMPLEMENTED + TESTED parity for the pure transition planner** once exact-head hosted CI passes.

It does **not** establish:

- authenticated proof of predecessor/successor credential control;
- durable atomicity, power-loss recovery, rollback-resistant storage, or crash consistency;
- an actual predecessor→successor runtime state transition;
- a wire encoding or registry allocation;
- replay-epoch profile promotion or `iot-core` selectability;
- formal verification of implementation behavior;
- forward secrecy, post-compromise security, KCI resistance, constant-time behavior, RNG quality, or memory safety beyond existing lane evidence;
- TD-001 independent cryptographic review;
- TD-002 physical constrained-target measurements;
- completion of TD-003 or TD-004;
- Common Contract completion, RFC-class status, or deployment qualification.

## Next dependency

The next lifecycle implementation step is a storage-neutral atomic transition executor/state machine that consumes this plan and models `ACTIVE(predecessor) -> REPLACEMENT_PENDING -> ACTIVE(successor)+RETIRED(predecessor)` without yet claiming persistence or allocating a network message. Durable storage and crash/rollback qualification must remain separate evidence gates.
