# Replay-Continuity Formal Traceability

Status: draft TD-003 evidence mapping. This artifact maps the approved replay-continuity state semantics into a synchronized symbolic model without claiming that the model has been executed or that replay persistence is formally verified.

## Scope

Canonical symbolic source:

- `rust/models/proverif/zk_arche_replay_continuity_draft.pv`

Synchronized mirror:

- `c/models/proverif/zk_arche_replay_continuity_draft.pv`

Synchronization owner:

- `scripts/sync-formal-models.sh`

The model covers only the state semantics already approved in `spec/replay-continuity.md` and implemented in the Rust/C replay-continuity primitives:

```text
TRUSTED
  -- restart --> RESTORING

RESTORING
  -- trusted restored window --> TRUSTED
  -- missing/corrupt/stale/rollback-suspected --> CONTINUITY_BROKEN
  -- AUTH attempt --> blocked, remain RESTORING

CONTINUITY_BROKEN
  -- AUTH attempt --> blocked, remain CONTINUITY_BROKEN
  -- empty-cache reset --> remain CONTINUITY_BROKEN
  -- fresh outer session --> remain CONTINUITY_BROKEN
```

No authenticated fresh replay-epoch recovery transition exists in the model. That omission is intentional because `replay_epoch_rule` remains unresolved for draft `iot-core`.

## Property mapping

| Formal event/query | Normative owner | Rust owner | C owner | Current evidence meaning |
|---|---|---|---|---|
| `AuthAdmitted(d) ==> TrustedStateEstablished(d)` | `spec/replay-continuity.md` §§3–5 | `ReplayContinuity::auth_admission_allowed()` in `rust/crates/proto/src/replay_continuity.rs` | `auth_replay_continuity_auth_admission_allowed()` / state primitive in `c/src/store/replay_continuity.c` | model construction admits AUTH only from the trusted-state dispatcher |
| `RestoredTrustedWindow(d) ==> RestoringEntered(d)` | §4 | `RestoredTrustedWindow` transition | corresponding C restored-window transition | restoration success is reachable only after restart/restoring state |
| `ContinuityBroken(d) ==> RestoringEntered(d)` | §§3–4 | missing/corrupt/stale/rollback events | corresponding C events | continuity break is entered only from restoration failure paths in this model |
| `AuthBlockedRestoring(d) ==> RestoringEntered(d)` | §§3,5 | AUTH admission false while `Restoring` | same C decision | AUTH attempts during restoration remain fail closed |
| `AuthBlockedBroken(d) ==> ContinuityBroken(d)` | §5 | AUTH admission false while `ContinuityBroken` | same C decision | AUTH attempts cannot escape broken continuity |
| `EmptyCacheResetStayedBroken(d) ==> ContinuityBroken(d)` | §5 / RC-05 | `EmptyCacheReset` self-transition | same C event | empty replay state is not recovery |
| `FreshOuterSessionStayedBroken(d) ==> ContinuityBroken(d)` | §5 / RC-06 | `FreshOuterSession` self-transition | same C event | transport/session churn is not recovery |
| failed-AUTH preservation correspondences | §9 / RC-07 | `FailedAuth` self-transition | same C event | failed AUTH does not advance replay-continuity state |

## Attacker and abstraction boundary

The public ProVerif command channel gives the environment control over operation ordering. Internal state tokens are carried only on private channels so symbolic AUTH admission occurs only when the abstract state is `TRUSTED`.

This model does **not** represent:

- contents of the 64-entry FIFO replay window;
- bounded eviction;
- physical persistence or flash/journal behavior;
- integrity/freshness validation of restored state;
- how rollback suspicion is detected;
- power-loss atomicity;
- fresh replay epochs;
- epoch identifiers or transcript/key-schedule binding;
- cryptographic soundness;
- canonical wire parsing;
- memory safety, RNG quality, constant-time behavior, or target hardware behavior.

The existing AUTH-v3 model remains the owner for currently modeled handshake agreement/context/replay-table properties. This replay-continuity model is a separate state abstraction intended to narrow the restart gap identified by R-004 and R-009; it does not make the persistent/unbounded AUTH-v3 replay table equivalent to runtime replay storage.

## Evidence state

At creation of this artifact:

```text
MODEL SOURCE PRESENT
+ RUST/C MODEL MIRROR GOVERNED BY SYNC CHECK
+ MODEL→SPEC→RUST/C MAPPING PRESENT
!= PROVERIF RESULT RETAINED
!= FORMALLY ANALYZED
!= FORMALLY VERIFIED
!= RUNTIME PERSISTENCE PROVEN
!= REPLAY-EPOCH RECOVERY DEFINED
```

The next evidence step is to run this exact model under the pinned ProVerif lane, retain every query result with model hash/tool version/commit, and fail closed on any false or unproved query. Only after a successful retained run may the corresponding rows be described as symbolically analyzed, and even then only within the abstraction boundary above.
