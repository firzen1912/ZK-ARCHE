# ZK-ARCHE Replay Continuity Contract

Status: **draft normative work**. This document defines fail-closed replay-continuity behavior for constrained profiles when accepted-message replay state is unavailable, stale, or suspected of rollback. It does not define the authenticated fresh-epoch transition that would safely recover from a continuity break, and it does not make `iot-core` selectable.

## 1. Scope

This contract applies to verifier-side replay state used to reject duplicate AUTH inputs after they have crossed the profile-defined acceptance boundary. It complements the bounded replay-window semantics in `iot-profiles.md`.

The contract separates three questions that MUST NOT be conflated:

1. how many accepted replay identifiers are retained during normal operation;
2. what happens when that retained state is lost or cannot be trusted after restart;
3. what authenticated event is allowed to establish a genuinely fresh replay epoch.

The current `iot-core` profile resolves item 1 with a minimum 64-entry FIFO window. This document resolves the fail-closed behavior for item 2. Item 3 remains unresolved and therefore still blocks profile promotion.

## 2. Terminology

**Replay continuity state**: the verifier state needed to preserve the profile's duplicate-rejection decision across execution interruptions.

**Trusted restored window**: replay continuity state that the implementation can establish is at least as recent as the last replay state it committed before the interruption, under the platform's documented persistence and rollback assumptions.

**Continuity break**: a state in which the verifier cannot establish that restored replay state is sufficiently recent to continue making the profile's replay claim.

**Fresh replay epoch**: a new replay-continuity domain whose creation is authenticated and bound into the protocol security context so that replay state from the predecessor domain cannot be confused with the new domain.

A process restart, power cycle, transport reconnect, socket replacement, address change, or outer `session_id` change is **not** by itself a fresh replay epoch.

## 3. Normal replay-continuity states

A conformant verifier implementing this contract has three abstract replay-continuity states:

```text
TRUSTED
  |
  | restart / reload
  v
RESTORING
  |                 \
  | trusted state    \ state missing, stale,
  | established       \ unverifiable, or rollback suspected
  v                     v
TRUSTED          CONTINUITY_BROKEN
                       |
                       | authenticated fresh-epoch transition
                       | (not yet defined for iot-core)
                       v
                    TRUSTED
```

`TRUSTED` means the verifier may apply the profile replay-window rules and may admit new AUTH inputs subject to all other authentication and authorization checks.

`RESTORING` is a local startup/recovery state. The verifier MUST NOT accept a new AUTH input for the affected replay-continuity domain while restoration is incomplete.

`CONTINUITY_BROKEN` is fail-closed. The verifier MUST NOT accept a new AUTH input under the affected profile/domain until an authenticated fresh-epoch transition defined by that profile succeeds.

## 4. Restart and state-loss requirements

Before the first post-restart AUTH acceptance in an existing replay-continuity domain, a verifier MUST establish one of the following:

1. a trusted restored replay window is available and satisfies the selected profile's minimum replay-retention semantics; or
2. replay continuity is broken.

A verifier MUST NOT initialize an empty replay window after restart and continue accepting AUTH as though prior accepted identifiers had never existed.

If replay continuity state is missing, corrupt, incomplete, older than the verifier can safely establish, or otherwise unverifiable, the verifier MUST enter `CONTINUITY_BROKEN` for that replay-continuity domain.

If the implementation detects or reasonably suspects rollback of the replay state, persistent generation metadata, storage journal, or other mechanism used to establish freshness, it MUST treat that condition as a continuity break. It MUST NOT attempt to guess which older replay entries remain safe.

The exact storage mechanism is implementation-specific. This specification does not require flash, secure elements, TPMs, monotonic hardware counters, a cloud service, or a gateway. However, an implementation cannot claim this restart contract merely because it stores replay entries somewhere; it must retain evidence that its chosen mechanism establishes the freshness property required above under the stated platform assumptions.

## 5. Fail-closed behavior during a continuity break

While a replay-continuity domain is `CONTINUITY_BROKEN`:

- the verifier MUST reject or decline admission of AUTH inputs that would rely on the affected replay state;
- it MUST NOT silently create an empty replay cache and resume normal AUTH;
- it MUST NOT treat a fresh outer transport connection, UDP source tuple, TCP connection, sequence number, or `session_id` as recovery;
- it MUST NOT mutate trust merely to escape the continuity break;
- it MUST preserve the distinction between authentication failure, authorization failure, and unavailable replay continuity;
- it MAY continue local administration, explicit provisioning, diagnostics, discovery, or other operations that do not assert successful AUTH under the affected replay domain, subject to their own security policy.

Error signaling for this state belongs to the protocol privacy/error contract. Implementations MUST NOT introduce a new remotely distinguishable oracle solely for convenience without the associated privacy review and Rust/C response-class evidence.

## 6. Fresh replay epoch requirements

Recovery from `CONTINUITY_BROKEN` requires an authenticated fresh replay epoch. The exact `iot-core` fresh-epoch mechanism is **not yet defined**.

Any future mechanism promoted for this purpose MUST satisfy all of the following before `iot-core` becomes selectable:

1. the epoch transition is authenticated by already-authorized local trust state or by an explicit reviewed rekey/reprovisioning authority;
2. the resulting epoch identifier or equivalent context is cryptographically bound into the AUTH security context or into keying material that uniquely determines that security context;
3. an attacker cannot create a fresh epoch merely by restarting a peer, changing transport addresses, replaying an old negotiation, or choosing a new outer session identifier;
4. both peers can determine which replay epoch is active for the security decision they are making;
5. Rust and C agree on transition accept/reject behavior and deterministic context/vector material where applicable;
6. rollback of the epoch-transition state either remains detectable or results in another fail-closed continuity break;
7. the mechanism does not require a CA, Internet access, cloud identity provider, central registry lookup, or gateway approval for already-authorized peers unless a later profile explicitly defines such infrastructure as optional assistance rather than the root authentication decision.

Until those conditions are normatively specified and tested, `replay_epoch_rule` remains unresolved in the `iot-core` machine-readable profile contract.

## 7. Persistence and evidence boundary

This specification intentionally defines a security property rather than a storage implementation.

Evidence for a concrete target/profile MUST record at least:

```text
replay-window capacity and eviction policy
persistent vs volatile representation
commit/update ordering
power-loss behavior
restart restore procedure
freshness/rollback assumptions
clone/reimage/reprovision assumptions
storage integrity mechanism if present
flash-write/endurance implications where relevant
behavior when restore validation fails
```

Host tests can establish parser/state-machine decisions. They do not establish MCU flash durability, secure-storage integrity, rollback resistance, or field power-loss behavior.

Physical target measurements or HIL evidence that are not available MUST remain explicit TD-002 blockers rather than being inferred from this normative text.

## 8. Formal-model boundary

The current AUTH-v3 ProVerif replay table is persistent and unbounded. It therefore models a stronger replay-memory abstraction than the bounded runtime caches and this restart contract.

A TRUE symbolic replay correspondence result does not prove:

- persistence across device restart;
- bounded-cache eviction safety outside the retained window;
- rollback detection;
- durable storage correctness;
- atomicity under power loss;
- safe fresh-epoch recovery.

Formal run records MUST continue to state this abstraction difference until a model explicitly represents the promoted replay epoch/restart semantics.

## 9. Conformance scenarios required before profile promotion

At minimum, Rust and C conformance evidence for the final replay-continuity design MUST cover:

```text
RC-01 retained replay window restored successfully
RC-02 replay state absent after restart -> CONTINUITY_BROKEN
RC-03 replay state corrupt/unverifiable -> CONTINUITY_BROKEN
RC-04 older/rollback-suspected state -> CONTINUITY_BROKEN
RC-05 empty-cache reset is not accepted as recovery
RC-06 fresh transport/session identifier does not recover continuity
RC-07 failed AUTH does not advance replay continuity state
RC-08 authenticated fresh-epoch transition succeeds only when its final design conditions hold
RC-09 replay from predecessor epoch is rejected or cryptographically non-applicable in the new epoch
RC-10 interrupted epoch transition fails closed
```

RC-08 through RC-10 remain design-blocked until the authenticated fresh-epoch mechanism is specified.

## 10. `iot-core` status

This document supports the following `iot-core` interpretation:

```text
replay_policy       = accepted-auth1-fifo-window
replay_min_entries  = 64
restart behavior    = restore trusted state or enter CONTINUITY_BROKEN
fresh epoch rule    = unresolved
selectable          = 0
```

The machine-readable `iot-core` profile contract now binds `restart_replay_rule=restore-trusted-state-or-continuity-broken` and retains `replay_epoch_rule=unresolved` with `selectable=0`. Rust and C profile-contract fixtures independently enforce those same markers. Therefore the restart rule is no longer an unresolved profile-contract field; only the authenticated fresh replay-epoch rule remains unresolved at this layer.

Therefore this document advances TD-004 specification precision but does not close R-009, TD-003, TD-002, TD-004, or any deployment-readiness claim.

## 11. Informative reference discipline

The fail-closed direction is consistent with constrained-security precedents without importing their wire formats or trust assumptions:

- RFC 8613 (OSCORE) requires endpoints to avoid accepting previously received messages after mutable security-context loss and describes either persistent state or establishment of a fresh security context.
- RFC 9031 (CoJP) strengthens that lesson for a constrained join profile by requiring replay-window persistence across reboots in its deployment context.
- RFC 9147 (DTLS 1.3) treats replay windows as epoch-specific and changes epochs when keying material changes.

These RFCs are engineering comparators. They do not make OSCORE, CoJP, or DTLS dependencies of ZK-ARCHE.
