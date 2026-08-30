# LINEAGE_REPLACE convergence event model

Status: qualification-supporting, wire-neutral model for `zk213`.

This document defines the deterministic event semantics exercised by `lineage-replace-convergence-events-v1.txt`. It does not allocate protocol messages, packet fields, timers, retransmission rules, or a persistence format.

## Scope

The model exists to falsify three unsafe assumptions around distributed lineage replacement:

1. duplicate observations may not create a second successor or change a completed decision;
2. reordered confirmation may not be accepted before the corresponding successor has been observed locally;
3. a crash/restart boundary may not preserve an unqualified volatile confirmation merely to recover availability.

The model reuses the existing convergence classifier. `CONVERGED` still requires local and peer authorization, the same successor lineage, and confirmation from both sides. Conflicting successor observations are sticky within one modeled attempt and classify as `SUCCESSOR_CONFLICT`.

## Event vocabulary

- `LA`, `PA`: local/peer replacement authority is established.
- `LSn`, `PSn`: local/peer side observes successor identifier `n` for this modeled attempt.
- `LCn`, `PCn`: local/peer confirmation for successor `n` is observed. A confirmation arriving before that side has observed successor `n` is ignored by this reducer; a future wire protocol must retransmit or otherwise re-establish the fact rather than treating unaffiliated reordered input as sufficient confirmation.
- `CL`, `CP`: local/peer crash/restart boundary clears that side's volatile confirmation in this logical model.

Repeated authorization, successor, or confirmation observations for the same value are idempotent. Observing a different successor after one has already been bound makes the conflict sticky and cannot be repaired by later confirmation.

## Security boundaries

This is not a crash-safe storage implementation and does not claim that authorization, successor identity, or confirmation survives real power loss. `CL`/`CP` deliberately model only loss of volatile confirmation so the corpus can prove that availability is not recovered by inventing confirmation after restart. Real restart behavior remains governed by the lineage durability/freshness contracts and target-specific storage evidence.

This model also does not establish cryptographic key confirmation. A production confirmation must eventually be authenticated and bound to the predecessor lineage, successor lineage, replacement attempt/context, transcript/security context, and any generation/epoch semantics required by the normative lifecycle design.

Normal AUTH remains NO-LEARNING. These events describe an explicit authorized trust-mutation workflow and cannot be inferred from ordinary authentication traffic.

## Corpus obligations

The shared Rust/C corpus covers normal convergence, duplicate idempotence, confirmation-before-successor reordering, asymmetric crash after one-sided confirmation, reconfirmation after restart, conflicting successor observations, stale/different successor retry, missing peer authorization, and harmless order variation of independent observations.

Passing this corpus is evidence of deterministic decision compatibility for this abstract event reducer only. It is not evidence of network liveness, durable atomic commit, malicious rollback resistance, distributed consensus, cryptographic confirmation soundness, target behavior, RFC-class completion, or deployment qualification.
