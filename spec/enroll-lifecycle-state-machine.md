# ZK-ARCHE ENROLL Lifecycle State Machine

Status: **draft normative work**. This document specifies the wire-neutral state transitions and durable decision boundary for explicit enrollment. It does not define an ENROLL packet encoding, promote any profile, or claim RFC/IETF status.

Normative keywords **MUST**, **MUST NOT**, **SHOULD**, and **MAY** are used in the BCP 14 sense where behavior is testable.

## 1. Scope and authority boundary

ENROLL is an explicit trust/authorization mutation. It is not AUTH. Normal AUTH remains NO-LEARNING and MUST NOT enter this state machine, issue a grant, consume an enrollment operation identifier, or alter trusted lineage as a side effect.

This state machine consumes the verified decision inputs defined by `enrollment-grant-issuance.md`. Cryptographic verification, grant encoding, commissioner credential format, and transport framing remain separate concerns.

## 2. Required state

An implementation claiming this contract MUST distinguish at least:

```text
IDLE
ENROLL_REQUESTED
COMMISSIONER_VALIDATED
SUBJECT_VALIDATED
ISSUANCE_AUTHORIZED
COMMITTING
COMMITTED
DENIED
RECOVERY_REQUIRED
```

`DENIED` is terminal for the enrollment attempt. `COMMITTED` is terminal for that operation identifier. A new enrollment attempt requires a new operation identifier and fresh policy evaluation.

`RECOVERY_REQUIRED` means durable outcome cannot be established after interruption. The implementation MUST fail closed for the affected trust mutation until recovery establishes either the predecessor state or the complete committed successor state.

## 3. Transition requirements

### 3.1 IDLE -> ENROLL_REQUESTED

The implementation MUST enter `ENROLL_REQUESTED` only for an explicit ENROLL operation. A normal AUTH message, successful AUTH result, transport connection, or possession proof alone MUST NOT cause this transition.

The implementation MUST bind the attempt to an enrollment operation identifier/nonce before commissioner authorization can lead to issuance.

### 3.2 ENROLL_REQUESTED -> COMMISSIONER_VALIDATED

The commissioner MUST be authenticated, locally authorized for enrollment, sufficiently fresh under the selected policy, and not locally revoked. The requested scope MUST NOT exceed commissioner authority.

Commissioner authorization MUST also carry authenticated provenance for the local authorization generation it belongs to, and that authentically bound generation MUST equal the current locally authoritative generation. These are separate requirements:

- missing authenticated generation provenance MUST fail closed as `COMMISSIONER_AUTHORIZATION_GENERATION_UNBOUND`;
- authenticated provenance for an older generation MUST fail closed as `COMMISSIONER_AUTHORIZATION_GENERATION_STALE`.

A transport address, connection identity, gateway assertion, successful AUTH exchange, possession proof, delegation assertion, or caller-provided generation number MUST NOT synthesize authorization-generation provenance or currentness. Generation allocation and advancement remain owned by the existing authorization/lifecycle authority; ENROLL consumes those facts and does not create a parallel generation authority.

Failure of any commissioner prerequisite MUST transition to `DENIED`. A stale commissioner authorization, unbound commissioner authorization generation, or stale bound generation MUST NOT be repaired or refreshed by successful AUTH.

### 3.3 COMMISSIONER_VALIDATED -> SUBJECT_VALIDATED

The subject MUST prove possession using the mechanism required by the selected enrollment policy. Failure MUST transition to `DENIED`.

Subject validation does not itself create trust and MUST NOT mutate persistent authorization state.

### 3.4 SUBJECT_VALIDATED -> ISSUANCE_AUTHORIZED

Before entering `ISSUANCE_AUTHORIZED`, the implementation MUST apply the complete fail-closed issuance decision from `enrollment-grant-issuance.md`, including:

- commissioner authorization freshness;
- authenticated commissioner authorization-generation provenance;
- currentness of the authentically bound commissioner authorization generation;
- unused enrollment operation identifier under the locally enforced replay domain;
- bounded scope, audience, deployment, validity, and delegation depth;
- current policy epoch, revocation view, and lineage;
- absence of rollback suspicion.

Any failed prerequisite MUST transition to `DENIED`.

### 3.5 ISSUANCE_AUTHORIZED -> COMMITTING

`ISSUANCE_AUTHORIZED` is permission to attempt the mutation, not evidence that it has occurred.

Before the successor grant can become usable, the implementation MUST begin a durable transition that composes the successor trust/authorization state with consumption of the one-time enrollment operation identifier and any required lineage/revocation generation updates.

The implementation MUST NOT expose the successor as committed while the operation identifier can still be recovered as unused.

### 3.6 COMMITTING -> COMMITTED

The transition to `COMMITTED` requires a recoverably durable state in which:

1. the successor grant/authorization state is present and internally consistent;
2. the enrollment operation identifier is durably consumed for the applicable replay lifetime;
3. required lineage, authorization, and revocation generations are mutually consistent;
4. the committed successor remains bound to the authorization-generation state accepted by the issuance decision rather than silently inheriting a different generation during commit;
5. restart recovery cannot make both the successor usable and the consumed operation identifier reusable.

Only after these conditions hold MAY the implementation report successful enrollment.

### 3.7 COMMITTING -> RECOVERY_REQUIRED

If interruption, write failure, restart, or rollback detection prevents the implementation from establishing the complete committed successor or the intact predecessor, it MUST enter `RECOVERY_REQUIRED` for the affected domain.

Recovery MUST follow `lifecycle-persistence-freshness.md`. Normal AUTH MUST NOT repair this state, clear rollback suspicion, synthesize authorization-generation provenance/currentness, synthesize the successor, or reset the enrollment replay domain.

## 4. Replay consumption semantics

A consumed enrollment operation identifier MUST remain unusable for the replay lifetime required by the selected profile, including across restart where that profile claims restart-safe enrollment.

An implementation MUST NOT consume the identifier merely because a syntactically valid request arrived. Consumption occurs as part of the authorized durable mutation boundary so that denial paths do not become an unauthenticated replay-store exhaustion primitive.

An implementation MUST nevertheless ensure that concurrent attempts using the same identifier cannot both reach `COMMITTED`. The storage/locking mechanism is implementation-specific; the single-commit property is normative.

## 5. Restart and rollback

Restart MUST NOT convert `COMMITTED` to an apparently unused enrollment operation. Transport reconnect, address change, process restart, or a new AUTH session MUST NOT reset enrollment replay state or repair stale/unbound commissioner authorization-generation state.

If the implementation detects an older persistence generation or cannot establish whether the mutation committed, it MUST fail closed as `RECOVERY_REQUIRED` or `ROLLBACK_SUSPECTED` under the lifecycle contract rather than choosing the more permissive state.

A profile claiming physical restart-safe enrollment MUST retain target evidence for its persistence strategy. Host-only tests do not establish flash atomicity or power-loss safety.

## 6. Observable failure discipline

Internally, implementations SHOULD preserve the reason classes from the enrollment issuance decision corpus, including distinct unbound-generation and stale-generation failures. Remote signaling MAY coalesce reasons to avoid creating commissioner, subject, revocation, lineage, replay, or authorization-generation oracles.

A remote peer MUST NOT be able to distinguish additional enrollment failure causes unless the applicable error/privacy specification explicitly permits that distinction.

## 7. Required conformance traces

A conforming implementation of this state machine SHOULD retain deterministic positive/negative traces for at least:

```text
ENR-SM-01 explicit ENROLL + all prerequisites + durable commit -> COMMITTED
ENR-SM-02 normal AUTH path -> DENIED; no trust mutation
ENR-SM-03 stale commissioner authorization -> DENIED
ENR-SM-04 commissioner authorization generation unbound -> DENIED
ENR-SM-05 commissioner authorization generation stale -> DENIED
ENR-SM-06 revoked commissioner -> DENIED
ENR-SM-07 failed subject possession -> DENIED
ENR-SM-08 replayed/consumed operation identifier -> DENIED
ENR-SM-09 requested authority exceeds commissioner authority -> DENIED
ENR-SM-10 stale epoch/revocation/lineage -> DENIED
ENR-SM-11 rollback suspicion -> DENIED or RECOVERY_REQUIRED; never COMMITTED
ENR-SM-12 interruption during COMMITTING -> predecessor restored or RECOVERY_REQUIRED
ENR-SM-13 restart after COMMITTED -> operation identifier remains consumed
ENR-SM-14 concurrent same-identifier attempts -> at most one COMMITTED
```

Where Rust and C claim this lifecycle behavior, they MUST agree on terminal accept/deny disposition and reason precedence for shared deterministic inputs.

## 8. Evidence and promotion boundary

This document advances TD-004 by keeping the ENROLL transition contract synchronized with the implementation-backed issuance decision, including commissioner authorization-generation provenance/currentness. It does not establish an ENROLL wire grammar, grant cryptographic format, physical persistence correctness, bounded disconnected revocation convergence, full Rust/C lifecycle implementation, formal proof, external cryptographic review, or deployment qualification.

`iot-core` and `p2p-iot-core` remain draft/non-selectable until their declared implementation and qualification evidence exists.
