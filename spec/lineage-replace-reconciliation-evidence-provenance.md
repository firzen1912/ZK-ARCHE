# Lineage Replacement Reconciliation Evidence Provenance Contract

Status: qualification contract for zk213. This document is wire-neutral and does not allocate a production message, field, identifier encoding, retransmission rule, or cryptographic primitive.

## Purpose

Successor activation after reconciliation requires evidence that both peers currently refer to the same replacement attempt and that each side has confirmation bound to that current attempt. That evidence is not a durable-state bit and MUST NOT be inferred merely because a successor record is present, integrity-valid, or freshness-current.

Rust and C therefore share one pure attempt-evidence classifier. The reconciliation transition consumes the classifier decision directly instead of accepting a caller-derived boolean.

## Normalized attempt-evidence facts

The classifier consumes four wire-neutral normalized values:

- local current attempt identifier;
- peer current attempt identifier;
- local confirmation's attempt identifier;
- peer confirmation's attempt identifier.

In C, `0` is the unset sentinel for this normalized interface. Rust represents the same absence as `None`. This convention does not allocate, encode, reserve, or constrain any future wire attempt-identifier format.

The decision precedence is normative for the qualification surface:

1. if either current attempt is absent, return `MISSING_CURRENT_ATTEMPT`;
2. if the two current attempts differ, return `ATTEMPT_MISMATCH`;
3. if local confirmation is not bound to the local current attempt, return `LOCAL_CONFIRMATION_MISSING`;
4. if peer confirmation is not bound to the peer current attempt, return `PEER_CONFIRMATION_MISSING`;
5. otherwise return `FRESH_CURRENT_ATTEMPT`.

Only `FRESH_CURRENT_ATTEMPT` may satisfy the reconciliation transition's current-attempt evidence requirement. All other outcomes fail closed for successor activation.

## Provenance lifecycle

For the qualification event model, observing a different attempt clears that side's prior confirmation. A restart that loses volatile attempt state clears the affected attempt and confirmation. An explicit clean retry clears both sides before a new attempt is established.

Confirmation for a retired, replaced, unknown, or mismatched attempt MUST NOT establish `FRESH_CURRENT_ATTEMPT`. Duplicate confirmation for the current attempt is idempotent. Late confirmation for a retired attempt is ignored and MUST NOT erase already-established current-attempt confirmation.

Durable successor readiness plus stale confirmation is therefore insufficient to activate a successor. Successor activation after `RECONCILIATION_REQUIRED` or an explicit retry from `SUCCESSOR_DIVERGENCE` requires the shared classifier to return `FRESH_CURRENT_ATTEMPT`.

## Evidence

The direct canonical decision corpus is `rust/test-vectors/replay/lineage-replace-attempt-evidence-v1.txt`; Rust and C consumers require the same normalized decision, including missing attempts, mismatched attempts, stale/missing local confirmation, stale/missing peer confirmation, and a valid maximum-width `u32` symbolic identifier.

The composed history corpus remains `rust/test-vectors/replay/lineage-replace-reconciliation-provenance-v1.txt`. Its Rust and C consumers replay attempt/restart/confirmation histories, invoke the shared attempt-evidence classifier, and feed that decision to the reconciliation transition guard.

## Claim boundary

This model does not prove cryptographic confirmation security, network liveness, durable attempt persistence, rollback-resistant storage, packet ordering behavior, wire interoperability, or deployment safety. `FRESH_CURRENT_ATTEMPT` means only that the normalized semantic facts agree on current attempt identity and bilateral confirmation binding. Authentication of those facts and any future wire representation remain separate requirements.
