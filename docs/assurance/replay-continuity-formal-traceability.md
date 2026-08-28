# Replay-Continuity Formal Traceability

Status: draft TD-003 assurance evidence. The approved replay-continuity state semantics are mapped into synchronized Rust/C symbolic sources and were executed successfully in the pinned ProVerif 2.05 CI lane at commit `b44b0c5961ace180f795fdba1b9a162c7c8d4ce3`. This is a scoped `FORMALLY ANALYZED` result for the correspondence properties below, not a claim that replay persistence, rollback resistance, fresh-epoch recovery, or the full implementation is formally verified.

## Scope

Canonical symbolic source:

- `rust/models/proverif/zk_arche_replay_continuity_draft.pv`

Synchronized mirror:

- `c/models/proverif/zk_arche_replay_continuity_draft.pv`

Synchronization owner:

- `scripts/sync-formal-models.sh`

At the analyzed commit, both model paths resolve to the same Git blob:

```text
f44640a7ae7960db57a8782ab7ee260ac83c768d
```

The CI evidence log also records SHA-256 hashes of both synchronized model files before invoking ProVerif. The exact SHA-256 values and complete textual query output are retained in the `formal-proverif-evidence` artifact for CI run 45; this document uses the repository blob identity plus retained-artifact provenance rather than transcribing an unverified digest.

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

## Retained formal run

| Field | Retained evidence |
|---|---|
| Commit analyzed | `b44b0c5961ace180f795fdba1b9a162c7c8d4ce3` |
| CI workflow/run | `ZK-ARCHE CI`, run 45 |
| Formal job | `Formal lane — legacy v2 + AUTH v3 + replay continuity gates` |
| Tool | ProVerif 2.05 |
| Replay-continuity step | `Run replay continuity model and fail closed` |
| Canonical/mirror Git blob | `f44640a7ae7960db57a8782ab7ee260ac83c768d` for both model paths |
| Declared correspondence queries | 9 |
| Gate behavior | fail if zero `RESULT` lines; fail if any `RESULT` is false or cannot be proved |
| Step result | success |
| Formal job result | success |
| Whole CI result | success |
| Retained log | `evidence/formal/proverif-replay-continuity-draft.log` inside `formal-proverif-evidence` |
| Artifact ID | `9679829179` |
| Artifact digest | `sha256:c81eb047de507117b36e6dac9e4ca20f79838bf6d73a445c863f81cd692db868` |

The model contains nine explicit correspondence queries. CI run 45 completed the replay-continuity step successfully under a gate that rejects zero result lines and rejects every false or unproved result. Therefore the retained run is valid evidence that the executed correspondence-query set produced no false or unproved ProVerif result under this model and tool version. The artifact remains the authority for the exact textual `RESULT` lines.

## Property mapping

| Formal event/query | Normative owner | Rust owner | C owner | Current evidence meaning |
|---|---|---|---|---|
| `AuthAdmitted(d) ==> TrustedStateEstablished(d)` | `spec/replay-continuity.md` §§3–5 | `ReplayContinuity::auth_admission_allowed()` in `rust/crates/proto/src/replay_continuity.rs` | `auth_replay_continuity_auth_admission_allowed()` / state primitive in `c/src/store/replay_continuity.c` | symbolically analyzed: admitted AUTH has a trusted-state predecessor in this abstraction |
| `RestoredTrustedWindow(d) ==> RestoringEntered(d)` | §4 | `RestoredTrustedWindow` transition | corresponding C restored-window transition | symbolically analyzed: trusted-window restoration has a restoring-state predecessor |
| `ContinuityBroken(d) ==> RestoringEntered(d)` | §§3–4 | missing/corrupt/stale/rollback events | corresponding C events | symbolically analyzed: continuity-break entry has a restoring-state predecessor in this model |
| `AuthBlockedRestoring(d) ==> RestoringEntered(d)` | §§3,5 | AUTH admission false while `Restoring` | same C decision | symbolically analyzed: AUTH-block event while restoring has the expected state predecessor |
| `AuthBlockedBroken(d) ==> ContinuityBroken(d)` | §5 | AUTH admission false while `ContinuityBroken` | same C decision | symbolically analyzed: broken-state AUTH-block event has continuity-break predecessor |
| `EmptyCacheResetStayedBroken(d) ==> ContinuityBroken(d)` | §5 / RC-05 | `EmptyCacheReset` self-transition | same C event | symbolically analyzed: empty replay state is not modeled as recovery |
| `FreshOuterSessionStayedBroken(d) ==> ContinuityBroken(d)` | §5 / RC-06 | `FreshOuterSession` self-transition | same C event | symbolically analyzed: transport/session churn is not modeled as recovery |
| `FailedAuthPreservedRestoring(d) ==> RestoringEntered(d)` | §9 / RC-07 | `FailedAuth` self-transition | same C event | symbolically analyzed: failed AUTH preserves restoring-state lineage |
| `FailedAuthPreservedBroken(d) ==> ContinuityBroken(d)` | §9 / RC-07 | `FailedAuth` self-transition | same C event | symbolically analyzed: failed AUTH preserves broken-state lineage |

`FailedAuthPreservedTrusted` is emitted by the trusted dispatcher but is not a separate correspondence query in this model. That event therefore remains implementation/model-structure evidence rather than a separately retained ProVerif correspondence result.

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

After CI run 45:

```text
MODEL SOURCE PRESENT
+ RUST/C MODEL MIRROR GOVERNED BY SYNC CHECK
+ MODEL→SPEC→RUST/C MAPPING PRESENT
+ PINNED PROVERIF 2.05 RUN RETAINED
+ ZERO-RESULT / FALSE / UNPROVED FAIL-CLOSED GATE
+ SCOPED CORRESPONDENCE SET FORMALLY ANALYZED
!= FORMALLY VERIFIED IMPLEMENTATION
!= BOUNDED FIFO CACHE FORMALLY MODELED
!= RUNTIME PERSISTENCE PROVEN
!= ROLLBACK RESISTANCE PROVEN
!= REPLAY-EPOCH RECOVERY DEFINED OR ANALYZED
!= CRYPTOGRAPHIC SOUNDNESS PROVEN
```

This advances the modeled replay-continuity correspondence set to narrowly scoped `FORMALLY ANALYZED` evidence. TD-003 remains open because the repository still lacks complete attacker/property coverage and full model-to-runtime fidelity, including bounded replay-cache semantics, canonical parser/subcontext assumptions, restart-storage mechanics, rollback handling, and the unresolved authenticated fresh replay-epoch transition.

The next TD-003 packet should address a remaining model/runtime fidelity gap rather than broaden this claim. The highest-value current candidates are the AUTH-v3 canonicalization/parser assumption boundary from R-004/R-011 or a bounded replay-cache abstraction that explicitly represents capacity/eviction without pretending to establish physical persistence.