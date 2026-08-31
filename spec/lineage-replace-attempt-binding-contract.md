# Lineage Replacement Attempt and Confirmation Binding Contract

Status: implementation contract for zk213 qualification. This document is wire-neutral and does not allocate protocol messages, fields, identifiers, timers, or cryptographic primitives.

## 1. Purpose

A distributed lineage replacement confirmation MUST refer to one authenticated replacement attempt. A confirmation from an older, different, or partially observed attempt cannot authorize a newer transition merely because the peers, predecessor, or successor overlap.

This contract narrows the decision boundary required before a future wire grammar can be reviewed. It preserves normal AUTH as NO-LEARNING and does not itself mutate trust.

## 2. Normalized facts

The classifier consumes only normalized facts supplied by a future authenticated exchange:

- `local_authorized` and `peer_authorized`: both peers have independently accepted the authority for this explicit replacement flow;
- `same_attempt`: both peers refer to the same bounded replacement-attempt identity;
- `same_predecessor_generation`: both peers bind the attempt to the same predecessor generation/epoch;
- `same_successor`: both peers bind the attempt to the same successor identity/lineage;
- `same_context`: both peers bind the attempt to the same authenticated deployment/protocol context;
- `local_confirmation_bound` and `peer_confirmation_bound`: each side's confirmation has been verified as belonging to that complete attempt tuple.

These booleans do not define how an attempt identity, successor identity, or context is encoded. They are an implementation-independent semantic seam.

## 3. Decision precedence

Implementations MUST classify in this order:

1. missing local or peer authorization -> `UNAUTHORIZED`;
2. different attempt identity -> `ATTEMPT_ID_MISMATCH`;
3. different predecessor generation -> `PREDECESSOR_MISMATCH`;
4. different successor -> `SUCCESSOR_MISMATCH`;
5. different authenticated context -> `CONTEXT_MISMATCH`;
6. either confirmation not bound to the complete tuple -> `AWAITING_CONFIRMATION`;
7. otherwise -> `CONVERGED`.

A stale confirmation from a previous attempt therefore remains non-operative even if all current tuple fields except the confirmation binding are valid. An old attempt identifier observed after a newer attempt is not upgraded by later confirmation traffic.

## 4. Fail-closed requirements

- A transport address MUST NOT substitute for attempt identity, predecessor identity, successor identity, or authenticated context.
- Confirmation MUST NOT be inferred merely from receipt of any message after local commit.
- Reordered or duplicated traffic MUST NOT change a mismatch decision into convergence without fresh evidence bound to the current complete tuple.
- A crash/restart that loses confirmation-binding evidence MUST return to an unconfirmed/fail-closed state until that evidence is re-established.
- This contract does not choose a simultaneous-attempt winner. Existing conflict handling remains fail closed unless a later reviewed convergence policy explicitly defines otherwise.

## 5. Non-claims

This contract does not establish a cryptographic key-confirmation construction, wire encoding, retransmission/liveness policy, crash-safe durable peer commit, malicious rollback resistance, secure erasure, formal proof, physical-target behavior, or deployment qualification. Those remain separately evidence-gated.
