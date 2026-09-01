# AUTH No-Learning and Trust-Mutation Boundary

Status: **draft normative implementation contract** for the currently implemented AUTH/SETUP boundary. This document uses BCP 14 keywords only for behavior that is directly testable against the Rust and C production dispatch paths. It does not claim complete authorization semantics, external review, Common Contract conformance, RFC status, or deployment qualification.

## 1. Scope

This contract owns one narrow lifecycle invariant: normal authentication must not silently become enrollment or persistent trust mutation.

It applies to the currently implemented `AUTH_1` / `AUTH_3` server path and uses the current `SETUP_3` enrollment path as the explicit mutation control case. It does not define the complete future ENROLL, LINEAGE_REPLACE, rekey, delegation, revocation, or application authorization protocols.

## 2. Normal AUTH is NO-LEARNING

Normal `AUTH_1` and `AUTH_3` processing **MUST NOT create, replace, delete, or otherwise mutate persistent trust/enrollment registry state**.

Authentication may read locally provisioned enrollment material, evaluate proofs, consult replay state, and create or consume bounded ephemeral session state. Those operations are not persistent trust learning.

A successful AUTH result **MUST NOT be interpreted as an implicit enrollment, trust grant, or application authorization grant**.

A failed AUTH result likewise **MUST NOT** repair, create, promote, or otherwise learn a trust binding from attacker-controlled input.

## 3. Explicit trust-mutation owner

Persistent trust mutation **MUST occur only through an explicit trust-management operation** whose authorization, inputs, persistence behavior, and lifecycle semantics are separately owned.

For the currently implemented v2 server path, successful `SETUP_3` is the explicit enrollment control case: it may persist the already-established pending setup identity material into the registry. Normal AUTH is not permitted to call that mutation path.

Future enrollment, replacement, revocation, rekey, commissioner/grant, or lineage operations do not inherit permission from this document. Each requires its own normative mutation authority and failure semantics.

## 4. Authentication is not general authorization

The current AUTH path may apply profile admission constraints such as an allowed-role set while deciding whether the authentication exchange may proceed. That admission check does not by itself define general application authorization.

An authenticated peer **MUST NOT** be treated as authorized for arbitrary application operations merely because AUTH completed. Application or secure-association authorization remains scoped by its separately owned audience, role/policy, scope, generation, policy-epoch, revocation-epoch, deployment, and freshness rules where those rules are implemented.

This document therefore preserves the semantic split:

```text
authentication result != application authorization != persistent trust mutation
```

## 5. Replay, retry, and failure behavior

Replay rejection, terminal-flight consumption, retransmission handling, retry, and cache behavior **MUST NOT** create a persistent trust-learning side effect.

A fresh AUTH retry after failure remains a normal AUTH operation and is subject to the same NO-LEARNING rule. The retry cannot be used as an implicit enrollment or trust-repair path.

This section does not redefine the replay identity, response-cache policy, terminal-flight disposition, or restart continuity rules owned by their dedicated specifications.

## 6. Conformance evidence

An implementation claiming this boundary requires evidence that:

1. accepted `AUTH_1` may create bounded pending AUTH state but does not mutate persistent enrollment/trust state;
2. rejected `AUTH_1` does not mutate persistent enrollment/trust state;
3. accepted `AUTH_3` completes authentication without persistent trust mutation;
4. rejected `AUTH_3` consumes or rejects ephemeral state according to the terminal-flight contract without persistent trust mutation; and
5. an explicit enrollment operation such as successful `SETUP_3` remains the separately identifiable registry-mutation control path.

Rust and C must retain equivalent trust-mutation decisions for these cases wherever both implementations claim the same protocol behavior.

## 7. Security rationale

Allowing normal AUTH to learn trust from the peer being authenticated collapses verification and provisioning into one attacker-influenced operation. It also makes replay, retry, parser, and proof failures potential trust-mutation surfaces. The NO-LEARNING rule keeps authentication dependent on locally authoritative state and makes trust changes auditable as explicit lifecycle events.

## 8. Claim boundary

This contract establishes only the current no-learning/trust-mutation separation. It does not prove that every authorization policy is complete, that current registry storage is rollback-resistant, that every future trust-management operation is safe, or that the custom role proof is cryptographically sound.

TD-001 through TD-004 remain independently governed by their required evidence.