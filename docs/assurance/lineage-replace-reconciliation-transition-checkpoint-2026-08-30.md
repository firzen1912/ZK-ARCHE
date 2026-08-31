# Checkpoint: lineage replacement reconciliation transition

Date: 2026-08-30
Roadmap owner: zk213
Evidence class: implementation + deterministic negative/transition corpus

## Change

Promoted a wire-neutral transition guard for pair reconciliation into shared Rust/C implementation surfaces.

The guard prevents `RECONCILIATION_REQUIRED` from becoming successor-ready solely because durable state later appears structurally valid/current. Successor activation requires both the pair classifier's `PAIR_SUCCESSOR_READY` result and fresh authenticated attempt evidence for the current replacement attempt.

The shared RT-01..RT-12 corpus covers fresh authenticated reconciliation to successor readiness; persisted/current successor state without fresh attempt evidence; safe predecessor resumption after an interrupted attempt; unresolved reconciliation remaining held; current successor divergence; prior divergence without clean retry; explicit clean retry with fresh successor evidence; clean retry back to a clean predecessor; continuity-broken stickiness; and idempotent already-ready successor/predecessor states.

## Claim boundary

This checkpoint does not establish production rekey/re-registration, wire confirmation, target rollback resistance, physical power-loss safety, interoperability execution across both languages, formal verification, independent cryptographic review, RFC-class completeness, or deployment qualification.

No roadmap/debt item is closed by this change.
