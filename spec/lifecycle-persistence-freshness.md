# ZK-ARCHE Lifecycle Persistence and Freshness Contract

Status: **draft normative work**. This document defines the minimum fail-closed relationship between persisted lifecycle state, authorization freshness, revocation freshness, restart recovery, and rollback suspicion. It does not define a storage format, create a fresh replay epoch, make `iot-core` or `p2p-iot-core` selectable, or establish physical-target evidence.

Normative keywords **MUST**, **MUST NOT**, **SHOULD**, and **MAY** are used in the BCP 14 sense when the stated behavior is testable.

## 1. Scope

A successful cryptographic AUTH is not sufficient to admit an operation when the local authorization or revocation view on which that decision depends cannot be established as sufficiently fresh. Implementations therefore MUST treat persisted lifecycle state as part of the security decision rather than as an optimization cache.

This contract applies to locally retained state that can affect whether an already-known peer remains authorized after restart, including as applicable:

```text
credential / trust generation
authorization generation or policy epoch
revocation epoch / accepted revocation view
lineage or replacement generation
replay-continuity generation
resumption invalidation generation
locally required freshness / rollback metadata
```

The exact representation is implementation-specific. No CA, cloud service, DNS service, central registry, gateway, blockchain, or manufacturer service is required by this contract.

## 2. Security-state tuple

For a concrete profile, an implementation MUST define the subset of the following tuple that is security-relevant to its local admission decision:

```text
L = (
  credential_generation,
  authorization_generation,
  revocation_epoch,
  lineage_generation,
  replay_generation,
  resumption_generation,
  persistence_generation
)
```

A field that is not used by a profile MAY be absent. An implementation MUST NOT silently treat an unknown required component as zero, current, or fresh.

The tuple is local security state. It is not a new wire structure and does not create a global generation namespace.

## 3. Freshness states

For each affected local security domain, the implementation MUST distinguish at least:

```text
FRESH
RESTORING
STALE_OR_UNKNOWN
ROLLBACK_SUSPECTED
```

`FRESH` means the implementation has established, under its documented persistence assumptions, that the local state is usable for the admission decision.

`RESTORING` means startup/recovery validation is incomplete. Security decisions that depend on the affected state MUST NOT be admitted while it remains `RESTORING`.

`STALE_OR_UNKNOWN` means required state is missing, incomplete, corrupt, or cannot be established as sufficiently current. Affected authorization MUST fail closed.

`ROLLBACK_SUSPECTED` means the implementation has evidence or a documented detection condition indicating that persisted security state may predate a previously committed state. Affected authorization MUST fail closed and MUST NOT be repaired by normal AUTH.

## 4. Restart admission rule

Before the first post-restart operation is admitted under an existing authorization/trust context, the verifier MUST establish all locally required lifecycle components as `FRESH`.

A restart, power cycle, process replacement, transport reconnect, address change, new socket, or new outer session identifier MUST NOT by itself reset authorization, revocation, replay, lineage, or resumption generations.

If any required lifecycle component is `RESTORING`, `STALE_OR_UNKNOWN`, or `ROLLBACK_SUSPECTED`, the verifier MUST NOT admit an operation whose authorization depends on that component.

Normal AUTH MUST remain NO-LEARNING during recovery. Successful proof or possession verification MUST NOT synthesize missing authorization state, advance a revocation epoch, create a replacement lineage, clear rollback suspicion, or create a fresh replay epoch.

## 5. Atomicity and composition

Where multiple lifecycle components must change as one security transition, the implementation MUST either commit them atomically or use a recoverable protocol that cannot expose a partially advanced state as fully current after restart.

In particular, a transition that invalidates an older credential, authorization grant, lineage predecessor, replay domain, or resumption secret MUST NOT make the successor usable while leaving the invalidation state durably older in a way that can restore the predecessor after interruption.

If atomic composition cannot be established after restart, the affected domain MUST enter `STALE_OR_UNKNOWN` or `ROLLBACK_SUSPECTED` rather than choosing the most permissive recovered component.

A storage journal, dual-bank scheme, monotonic counter, secure element, filesystem transaction, flash record, or other persistence mechanism MAY implement this property. The mechanism is not normative; the fail-closed property is.

## 6. Authorization and revocation freshness

An authorization decision MUST be evaluated against the locally accepted authorization generation and revocation view required by the selected profile.

A cryptographically valid credential MUST NOT override a locally known revocation or superseding lineage state.

A cached authorization or resolver result MAY be reused only when the implementation can establish that its credential generation, authorization generation, revocation epoch, lineage generation, profile/version context, and local invalidation dependencies remain valid.

If a deployment permits disconnected operation, it MUST define the local stale-authorization policy separately. This contract does not invent a universal wall-clock expiry or convergence interval. Until a profile defines and tests such a bound, inability to establish required freshness MUST fail closed rather than silently extending authorization.

## 7. Resumption interaction

Possession of a valid ticket, PSK, or resumption secret MUST NOT bypass lifecycle freshness validation.

Before resumed authorization is admitted, the implementation MUST establish that all lifecycle generations on which the resumed authorization depends remain valid. If that cannot be established, it MUST fall back to a full AUTH path that can safely revalidate the required context or fail closed.

A full AUTH path is not itself permission to mutate trust or erase rollback suspicion.

## 8. Replay-continuity interaction

`spec/replay-continuity.md` remains authoritative for replay-window continuity. This contract composes with it as follows:

- replay state in `CONTINUITY_BROKEN` cannot be made fresh by restoring authorization state alone;
- fresh authorization does not create a fresh replay epoch;
- a trusted replay window does not prove authorization or revocation freshness;
- recovery of one lifecycle component MUST NOT implicitly recover another component with a distinct security meaning.

This separation prevents restart handling from collapsing authentication, authorization, revocation, replay, and trust mutation into one generic generation counter.

## 9. Observable failure boundary

Implementations MUST preserve internal distinction among cryptographic authentication failure, authorization denial, unavailable freshness state, rollback suspicion, and replay-continuity failure.

Remote error signaling MAY coalesce those internal causes where required by the privacy/error contract. Implementations MUST NOT expose a new remotely distinguishable freshness or revocation oracle without the corresponding error/privacy specification and Rust/C response-class evidence.

## 10. Required conformance scenarios

Before a profile claims this contract, shared Rust/C decision evidence SHOULD cover at least:

```text
LPF-01 all required persisted generations restore fresh -> admission may proceed to normal checks
LPF-02 authorization generation missing after restart -> fail closed
LPF-03 revocation view corrupt or unverifiable -> fail closed
LPF-04 older persisted generation / rollback suspicion -> fail closed
LPF-05 valid AUTH proof does not repair missing lifecycle state
LPF-06 transport/session replacement does not reset lifecycle generations
LPF-07 partial multi-component commit does not restore as fully fresh
LPF-08 cached authorization from predecessor generation is rejected
LPF-09 valid resumption secret with stale authorization context is rejected or forced to safe full AUTH
LPF-10 trusted replay state does not override stale authorization/revocation state
LPF-11 fresh authorization state does not override replay CONTINUITY_BROKEN
LPF-12 normal AUTH does not mutate trust while recovery is incomplete
```

Where both implementations claim a concrete lifecycle transition, deterministic fixtures MUST identify the same pre-state, event, post-state or failure class, and durable-commit disposition.

## 11. Evidence boundary

Host tests can establish state-machine and accept/reject semantics. They do not establish flash durability, atomicity under physical power loss, secure-storage integrity, monotonic-counter correctness, rollback resistance, endurance, or restart latency on an MCU.

Physical target evidence for TD-002 MUST identify the target, storage backend, persistence strategy, freshness/rollback source, power-loss assumptions, bytes retained/written, and measured restart/update behavior. Unavailable measurements remain unavailable.

Symbolic formal results MAY analyze an abstraction of lifecycle freshness, but they do not establish storage correctness or physical rollback resistance. The model-to-runtime abstraction gap MUST remain explicit under TD-003.

## 12. Qualification status

This document advances TD-004 by making the restart/freshness composition rule independently reviewable. It does not by itself provide Rust/C implementation parity, physical MCU measurements, formal proof, independent cryptographic review, a bounded disconnected revocation-convergence policy, authorization-aware resumption implementation, or Common Contract qualification.

Accordingly, `iot-core` and `p2p-iot-core` remain non-selectable until their own declared exit evidence exists. ZK-ARCHE remains a project targeting RFC-class engineering quality; this document is not an RFC and does not imply IETF status.
