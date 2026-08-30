# Replay-Epoch Recovery Checkpoint — 2026-08-29

Status: **checkpoint-style normative review record**. This record scopes the addition of `spec/replay-epoch-recovery.md`. It does not claim an implemented recovery transition.

## 1. Why this packet exists

The current replay-continuity contract correctly fails closed when replay state is missing, stale, corrupt, or rollback-suspected, but the authenticated fresh-epoch transition is intentionally unresolved. The TD-003 closure audit also states that new compromise/recovery theorems would risk inventing policy until downstream lifecycle semantics exist.

This packet therefore defines the minimum authority, binding, atomicity, predecessor-retirement, dependent-state invalidation, rollback, and evidence requirements that a future recovery implementation must satisfy.

## 2. Affected evidence surfaces

| Surface | Effect |
|---|---|
| `spec/replay-epoch-recovery.md` | new draft normative recovery requirements |
| `spec/replay-continuity.md` | unchanged; remains authoritative for current fail-closed runtime behavior |
| `spec/implementation-requirements.md` | unchanged; its statement that an implementation must not synthesize a fresh epoch remains valid |
| `rust/crates/proto/src/replay_continuity.rs` | unchanged; no recovery event added |
| `c/include/auth/replay_continuity.h` + `c/src/store/replay_continuity.c` | unchanged; no recovery event added |
| shared replay-continuity corpus/tests | unchanged; shortcut recovery remains negative evidence |
| `rust/test-vectors/profiles/iot-core-v1.profile` | unchanged; `replay_epoch_rule=unresolved`, `selectable=0` |
| formal models | unchanged; no new theorem or compromise claim |

## 3. Preserved negative invariants

Existing Rust/C behavior continues to require that empty-cache reset and a fresh outer session do not recover `CONTINUITY_BROKEN`. The new requirements additionally prohibit restart, address/transport change, ordinary AUTH, ordinary negotiation, and unauthenticated fresh values from becoming implicit epoch transitions.

No existing test or assertion is weakened. No profile is made selectable. No fallback security mode is introduced.

## 4. Required future implementation checkpoint

A future implementation packet must not merely add a `FreshEpoch` event. Before runtime promotion it must identify the exact owning rekey/re-registration/recovery flow, canonical authenticated inputs, predecessor/successor state transition, crash/rollback behavior, dependent-state invalidation, Rust/C parity, and executable RE-01 through RE-12 positive/negative evidence.

Wire-visible behavior must receive TD-004 grammar/registry/vector review. Trust mutation must remain explicit and normal AUTH must remain NO-LEARNING.

## 5. Claim boundary

This checkpoint supports only:

```text
AUTHENTICATED RECOVERY REQUIREMENTS: SPECIFIED
RECOVERY IMPLEMENTATION:             NO
RECOVERY INTEROPERABILITY:            NO
FM-22 COMPROMISE/RECOVERY PROOF:      NO
IOT-CORE SELECTABLE:                  NO
TD-003 CLOSED:                        NO
TD-004 CLOSED:                        NO
COMMON-CONFORMANT:                    NO
RFC-CLASS DOCUMENTED:                 NO
DEPLOYMENT-QUALIFIED:                 NO
```

TD-001 independent review and TD-002 physical constrained-target evidence remain unaffected and open.
