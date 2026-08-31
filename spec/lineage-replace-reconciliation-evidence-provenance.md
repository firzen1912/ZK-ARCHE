# Lineage Replacement Reconciliation Evidence Provenance Contract

Status: qualification contract for zk213. This document is wire-neutral and does not allocate a production message, field, identifier encoding, retransmission rule, or cryptographic primitive.

## Purpose

`fresh_authenticated_attempt_evidence` in the reconciliation transition guard is not a durable-state bit and MUST NOT be inferred merely because a successor record is present, integrity-valid, or freshness-current. It represents current-attempt evidence reconstructed from the authenticated replacement lifecycle.

For this qualification model, an attempt identifier is symbolic. Fresh evidence exists only when both peers currently observe the same non-zero attempt identifier and each side has confirmation bound to that same current attempt. Observing a different attempt clears that side's prior confirmation. A restart that loses volatile attempt state clears the affected attempt and confirmation. An explicit clean retry clears both sides before a new attempt is established.

Confirmation for a retired, replaced, unknown, or mismatched attempt MUST NOT satisfy fresh evidence for the current attempt. Duplicate confirmation for the current attempt is idempotent. Late confirmation for a retired attempt is ignored and MUST NOT erase already-established current-attempt confirmation.

The derived fresh-evidence predicate feeds the existing reconciliation transition guard. Durable successor readiness plus stale confirmation is therefore insufficient to activate a successor. Successor activation after `RECONCILIATION_REQUIRED` or an explicit retry from `SUCCESSOR_DIVERGENCE` requires fresh evidence derived from the current attempt lifecycle.

## Evidence

The canonical corpus is `rust/test-vectors/replay/lineage-replace-reconciliation-provenance-v1.txt`. Rust and C consumers replay the same histories and require the same transition result.

The corpus covers current-attempt convergence, stale confirmation after attempt replacement, restart loss and reconfirmation, explicit clean retry, mismatched simultaneous attempts, duplicate current confirmation, late retired-attempt traffic, predecessor recovery, divergence, and continuity-broken stickiness.

## Claim boundary

This model does not prove cryptographic confirmation security, network liveness, durable attempt persistence, rollback-resistant storage, packet ordering behavior, or deployment safety. It only constrains how attempt provenance may justify the existing semantic `fresh_authenticated_attempt_evidence` input.
