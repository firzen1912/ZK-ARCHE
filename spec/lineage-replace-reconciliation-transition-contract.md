# Lineage Replacement Reconciliation Transition Contract

Status: zk213 qualification contract; wire-neutral and non-cryptographic.

## Purpose

This contract defines the bounded transition out of pair-level lineage replacement reconciliation. It does not allocate a packet, field, timer, retry algorithm, or cryptographic primitive.

The pair classifier in `lineage_replace_reconciliation` describes the currently observed durable state. This transition guard answers a different question: whether a previously unresolved/divergent pair may become AUTH-usable on a successor or safely resume the predecessor.

## Inputs

- `prior`: previous pair reconciliation decision.
- `current`: decision after newly observed durable/freshness/attempt facts.
- `fresh_authenticated_attempt_evidence`: true only when fresh authenticated replacement-attempt/confirmation evidence was re-established for the current attempt. Durable storage alone MUST NOT set this fact.
- `explicit_clean_retry`: true only for a new explicitly authorized lifecycle retry after successor divergence.

## Normative rules

1. `PAIR_CONTINUITY_BROKEN` is fail-closed within this transition surface. A clean retry does not repair lost continuity.
2. A current `SUCCESSOR_DIVERGENCE` MUST reject successor activation.
3. A prior divergence MUST remain rejected unless an explicit clean retry is established.
4. Transition from `RECONCILIATION_REQUIRED` to `PAIR_SUCCESSOR_READY` MUST also present fresh authenticated attempt evidence. Persisted successor state, record integrity, or freshness-current storage alone is insufficient.
5. A clean retry after divergence may activate a successor only when the current pair classifier reports `PAIR_SUCCESSOR_READY` and fresh authenticated attempt evidence is present.
6. A freshness-current clean `PAIR_PREDECESSOR_READY` observation may resume the predecessor from `RECONCILIATION_REQUIRED`; this resumes the predecessor only and does not continue or infer confirmation for the interrupted attempt.
7. An already ready successor/predecessor may remain ready when the current classifier still reports the same ready state.
8. All other combinations MUST remain `HOLD`.

## Security boundary

This contract performs no trust mutation and does not make normal AUTH a learning flow. It assumes that the underlying pair decision already reflects normalized durable-recovery and freshness results. It does not prove authenticity, freshness-anchor quality, crash-safe persistence, liveness, or distributed consensus.

The shared RT-01..RT-12 corpus is authoritative for Rust/C decision parity.
