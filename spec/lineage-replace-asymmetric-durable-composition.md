# LINEAGE_REPLACE asymmetric durable composition

Status: qualification contract for `zk213`; wire-neutral and storage-neutral.

## Purpose

This contract composes the existing replacement-attempt, durable-recovery, and freshness decisions across two peers. It does not allocate a wire message or make a physical-storage claim. Its purpose is to prevent asymmetric commit/crash state from silently creating two pair-usable lineages or accepting a stale authenticated snapshot.

## Normalized pair outcomes

A pair is `PAIR_SUCCESSOR_READY` only when both peers recover the same freshness-current stable successor and both replacement-attempt decisions are `CONVERGED`.

A pair is `PAIR_PREDECESSOR_READY` only when both peers recover freshness-current stable predecessor state and neither peer claims a converged replacement attempt.

A pair is `RECONCILIATION_REQUIRED` when individually stable observations disagree about predecessor/successor position or confirmation state. This outcome is fail-closed for pair-level successor readiness; it does not authorize a second lineage or invent missing confirmation.

A pair is `SUCCESSOR_DIVERGENCE` when both peers recover stable successor state but the successors differ. No winner is inferred.

If either peer's freshness/recovery classifier returns `CONTINUITY_BROKEN`, the pair outcome is `CONTINUITY_BROKEN`.

## Shared implementation contract

Rust and C expose the pair decision as `lineage_replace_reconciliation`. The classifier consumes only already-normalized replacement-attempt decisions, already-normalized durable recovery state, and the explicit `same_successor` comparison result. It must not parse transport metadata, infer protocol identity from an address, reconstruct lost confirmation from storage, or mutate trust.

The ordering of decisions is intentional:

1. any local or peer `CONTINUITY_BROKEN` state dominates every availability outcome;
2. two stable successors with different successor bindings produce `SUCCESSOR_DIVERGENCE` and never an implicit winner;
3. the same stable successor is pair-ready only when both attempt decisions are `CONVERGED`;
4. two stable predecessors are pair-ready only when neither side claims a converged replacement;
5. every other non-broken combination remains `RECONCILIATION_REQUIRED`.

A null/absent reconciliation fact set in C maps to `CONTINUITY_BROKEN`; callers must not treat missing pair evidence as permission to continue.

## Invariants

- Stable local storage is not sufficient for pair-level successor readiness.
- A local durable successor committed before peer confirmation does not make the pair successor-ready.
- Loss of volatile confirmation at a peer restart must be re-established by the explicit replacement lifecycle; persisted successor structure alone does not reconstruct confirmation.
- A record generation below or above the trusted high-water generation remains fail-closed even when its record integrity is valid.
- Different stable successors never become pair-ready through availability preference or implicit tie breaking.
- Recovery of a clean predecessor after an interrupted attempt does not resurrect the interrupted attempt.
- Normal AUTH remains NO-LEARNING. This qualification model performs no trust mutation.

## Canonical corpus

`rust/test-vectors/replay/lineage-replace-asymmetric-durable-v1.txt` is consumed independently by Rust and C tests. It includes local durable successor / peer predecessor asymmetry, stale old-successor replay after a later high-water generation, peer predecessor rollback, partial durable state, confirmation loss, successor divergence, and clean converged endpoints.

The corpus tests the shared Rust/C reconciliation classifier directly. Test-local copies of the pair decision logic are not authoritative.

## Evidence boundary

This contract does not establish network liveness, retransmission behavior, actual crash-safe writes, malicious rollback resistance of a target device, secure erasure, monotonic-counter quality, cryptographic soundness of key confirmation, external review, RFC status, Common Contract conformance, or deployment qualification. Target-specific rollback resistance still requires TD-002 evidence, and RFC-class lifecycle/wire semantics remain under TD-004.
