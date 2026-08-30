# ZK-ARCHE Authenticated Replay-Epoch Recovery Requirements

Status: **draft normative requirements / implementation-blocked**. This document defines the security and state-transition requirements that any future authenticated fresh replay-epoch mechanism must satisfy. It does not define a wire message, make `iot-core` selectable, or claim that Rust/C currently implement replay-epoch recovery.

Normative keywords **MUST**, **MUST NOT**, **SHOULD**, and **MAY** are used in the BCP 14 sense where behavior is precise enough to test once an owning recovery flow is implemented.

## 1. Scope and ownership

A fresh replay epoch exists to recover safely from `CONTINUITY_BROKEN` without treating lost replay memory as permission to forget previously accepted traffic.

Replay-epoch recovery is a **trust/lifecycle transition**, not normal AUTH. The owning operation MUST be an explicit reviewed rekey, re-registration, reprovisioning, or equivalent recovery flow that is authorized to replace security lineage. Normal AUTH remains NO-LEARNING and MUST NOT create a replay epoch merely because authentication succeeds.

This document defines the minimum recovery contract shared by future owners such as zk213 and the replay lifecycle. Exact wire grammar, message identifiers, transcript encoding, registry values, and executable transition APIs remain future TD-004/implementation work.

## 2. Terms

**Predecessor replay domain**: the replay-continuity domain whose trustworthy replay state is unavailable, stale, rolled back, or otherwise unusable.

**Successor replay epoch**: a new replay domain established only by a successful authenticated lineage-replacement transition.

**Recovery authority**: locally accepted authority permitted to replace the affected credential/security lineage. It may be the current/predecessor credential holder proving possession through the recovery flow or a separately provisioned recovery/reprovisioning authority. Merely being reachable over a transport is not authority.

**Lineage replacement**: an explicit state transition that binds the predecessor security identity/context to approved successor state and retires the predecessor replay domain.

## 3. Authorization boundary

A peer MUST NOT leave `CONTINUITY_BROKEN` because of any of the following alone:

```text
process restart
power cycle
new socket or transport connection
new UDP/TCP/BLE/CAN address or handle
new outer session identifier
new unauthenticated nonce
ordinary capability/profile negotiation
ordinary successful AUTH
empty replay-cache initialization
```

A successor replay epoch MUST be established only by a recovery flow whose authority is already represented by locally trusted state or by an explicit reviewed reprovisioning policy. Core recovery for already-authorized peers MUST NOT require CA, DNS, Internet connectivity, cloud identity, a central registry lookup, blockchain, manufacturer cloud, or gateway/controller approval.

If the peer cannot establish that the recovery authority is permitted to replace the affected lineage, recovery MUST fail closed and the domain MUST remain `CONTINUITY_BROKEN`.

## 4. Mandatory authenticated binding

Before activating a successor replay epoch, the recovery flow MUST authenticate and bind every input on which the replacement decision depends. At minimum, where applicable to the selected profile, the authenticated transition MUST bind:

```text
predecessor peer / credential identity
predecessor credential or authorization generation
successor peer / credential identity
successor authentication key or commitment
successor replay-epoch identifier or equivalent context
protocol version
suite / method
selected profile
recovery operation identity
fresh recovery challenge / nonce material
audience / deployment / domain
authorization generation and policy epoch
revocation epoch or equivalent freshness state
```

The successor replay-epoch identifier MAY be public, but it MUST be fresh for the replacement operation and cryptographically authenticated by the transition. An attacker MUST NOT be able to choose a new epoch by changing transport metadata or replaying an old recovery transcript.

Transport addresses are operational metadata, not protocol identity.

## 5. Atomic activation and predecessor retirement

A conforming implementation MUST NOT expose the successor epoch as usable until the authenticated recovery transition has completed successfully.

Successful activation MUST be one logical commit that establishes the successor epoch and the replay state required to admit new AUTH under it while retiring or tombstoning the predecessor replay domain so that the two domains cannot be confused.

If storage failure, power loss, crash, rollback suspicion, or partial update prevents the implementation from proving which epoch is active, the affected domain MUST remain or return to `CONTINUITY_BROKEN`.

Implementations MUST NOT recover by simultaneously accepting both the predecessor and successor replay domains merely to improve availability.

## 6. Dependent-state invalidation

Replay-epoch replacement MUST NOT silently preserve dependent state whose security context belonged to the predecessor lineage.

Unless a future normative owner explicitly defines safe rebinding, the transition MUST invalidate or force revalidation of predecessor-bound:

```text
session / traffic keys
resumption secrets or tickets
cached authorization decisions
cached identity-attribution results
replay-window state
policy/revocation freshness assumptions
transport/channel bindings
```

The transition MUST NOT expand trust or authorization scope. Authorization after recovery must remain bounded by locally accepted audience, deployment/domain, role/policy, generation, epoch, and revocation state.

## 7. Rollback and replay behavior

Replaying an old recovery transcript MUST NOT reactivate a retired predecessor epoch or recreate a previously superseded successor epoch.

Rollback of epoch-transition metadata MUST either be detectable or cause another fail-closed continuity break. An implementation MUST NOT guess that an older stored epoch is safe.

Traffic from the predecessor replay domain MUST be rejected or cryptographically non-applicable after successor activation. The exact enforcement mechanism belongs to the final wire/key-schedule design and MUST receive Rust/C parity evidence before profile promotion.

## 8. Rust/C conformance evidence required before implementation claim

Before any implementation is reported as supporting authenticated replay-epoch recovery, Rust and C MUST have matching positive/negative decision evidence for at least:

```text
RE-01 authorized lineage replacement activates one successor epoch
RE-02 unauthorized replacement authority is rejected
RE-03 restart alone cannot create an epoch
RE-04 transport/address/session-id change cannot create an epoch
RE-05 ordinary AUTH cannot create an epoch
RE-06 replayed recovery transcript is rejected
RE-07 predecessor-domain traffic is rejected/non-applicable after activation
RE-08 interrupted/partial activation remains fail closed
RE-09 rollback-suspected epoch metadata returns to CONTINUITY_BROKEN
RE-10 stale authorization/revocation context cannot be preserved by recovery
RE-11 predecessor-bound resumption state is invalidated or explicitly revalidated
RE-12 Rust/C agree on canonical transition inputs and accept/reject outcomes
```

A profile contract MAY identify this recovery policy as resolved only after its exact transition owner, canonical authenticated inputs, and decision evidence are registered. `iot-core` remains non-selectable while those executable surfaces are absent.

## 9. Formal-analysis boundary

This contract provides normative prerequisites for later FM-04/FM-22 modeling. It does not itself prove forward secrecy, post-compromise security, key-compromise impersonation resistance, rollback resistance, durable storage correctness, or implementation equivalence.

Any future compromise/recovery model MUST state which secret or state is compromised, when corruption occurs, what successor state is fresh, and which authenticated recovery event is assumed to restore a guarantee. Symbolic success MUST NOT be generalized to physical secure erasure, RNG quality, memory safety, constant-time behavior, or durable MCU storage.

## 10. Current qualification state

At the current repository state:

```text
restart/state-loss fail-closed behavior      IMPLEMENTED + TESTED in Rust/C
replay continuity symbolic model             SCOPED FORMALLY ANALYZED
authenticated recovery requirements          SPECIFIED in this document
exact recovery wire/state owner              NOT IMPLEMENTED
successor epoch transition in Rust/C          NOT IMPLEMENTED
shared RE-01..RE-12 executable corpus         NOT PRESENT
iot-core replay_epoch_rule                    remains unresolved
selectable                                     0
```

Therefore this document advances TD-004/lifecycle specification precision but does not close TD-003, TD-004, zk213, profile promotion, Common Contract conformance, or deployment qualification.
