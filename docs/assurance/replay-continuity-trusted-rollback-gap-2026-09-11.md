# Replay-continuity trusted-rollback formal gap

Status: TD-003 traceability addendum for exact-current `dev` after the runtime fail-closed change that makes `TRUSTED + ROLLBACK_SUSPECTED -> CONTINUITY_BROKEN` in both Rust and C.

This document records an exact model/runtime fidelity gap. It does **not** claim a new formal proof or a fresh ProVerif result.

## Runtime behavior now owned

The Rust and C replay-continuity implementations on current `dev` fail closed when rollback suspicion is raised while replay continuity is already `TRUSTED`. The transition is:

```text
TRUSTED + ROLLBACK_SUSPECTED -> CONTINUITY_BROKEN
```

AUTH admission is then denied while the domain remains `CONTINUITY_BROKEN`.

This behavior is distinct from restart restoration. Rollback suspicion may be raised against an already trusted continuity state; it does not require a preceding `TRUSTED -> RESTORING` transition.

## Current symbolic-model mismatch

The synchronized replay-continuity ProVerif pair:

- `rust/models/proverif/zk_arche_replay_continuity_draft.pv`
- `c/models/proverif/zk_arche_replay_continuity_draft.pv`

still handles `cmd_rollback_suspected` only in `RestoringDispatcher`. `TrustedDispatcher` has no rollback-suspected branch, so the symbolic model currently leaves that command on the default trusted-state self-loop.

Therefore the retained replay-continuity ProVerif result at model blob `f44640a7ae7960db57a8782ab7ee260ac83c768d` does **not** establish the newly implemented trusted-state rollback transition.

The retained nine-query result remains valid evidence for the exact historical model that was executed, but it must not be generalized to the stronger current runtime semantics.

## Required synchronization packet

A future formal packet must update both synchronized model copies together so that trusted-state rollback suspicion enters the broken state and AUTH remains fail closed afterward. The model update must preserve the distinction between:

1. continuity break caused while restoring missing/corrupt/stale/rollback-suspected persisted state; and
2. continuity break caused directly from an already trusted state by rollback suspicion.

The existing correspondence

```text
ContinuityBroken(d) ==> RestoringEntered(d)
```

cannot simply remain unchanged after that update because a direct trusted-state rollback break has no restoring predecessor. The revised property set should distinguish cause-specific break events rather than invent a synthetic restoring transition that runtime does not perform.

At minimum the synchronized model should make auditable these facts:

```text
trusted rollback suspicion -> continuity broken
broken continuity -> AUTH not admitted
restoring-origin continuity break -> restoring predecessor
trusted-origin rollback break -> trusted predecessor
```

After editing the model, a new exact-model ProVerif 2.05 run must be retained before the edited model can be described as `FORMALLY ANALYZED`. Per `docs/assurance/formal-model-contract.md`, an older retained run cannot be inherited by changed model text.

## Evidence boundaries

Even a successful revised symbolic result would not establish:

- correctness of physical rollback detection;
- rollback-resistant flash/journal persistence;
- power-loss atomicity;
- secure monotonic counters or hardware roots of trust;
- bounded replay-cache equivalence;
- computational cryptographic soundness;
- authenticated fresh replay-epoch recovery;
- deployment qualification.

Those remain separate TD-002/TD-003/runtime evidence obligations.

## Traceability impact

Until the model pair is synchronized and re-executed:

```text
RUST/C TRUSTED-ROLLBACK FAIL-CLOSED BEHAVIOR     IMPLEMENTED
SHARED EXECUTABLE TRANSITION CORPUS              PRESENT
CURRENT SYMBOLIC MODEL FOR THAT TRANSITION       GAP
RETAINED FORMAL RESULT FOR HISTORICAL MODEL      PRESENT
FORMALLY ANALYZED TRUSTED-ROLLBACK TRANSITION    NOT YET
PHYSICAL ROLLBACK RESISTANCE                     NOT ESTABLISHED
```

This addendum is intentionally narrow: it prevents the existing retained formal result from being over-claimed while preserving the exact next formalization target.