# LINEAGE_REPLACE storage-transaction checkpoint — 2026-08-31

## Scope

This checkpoint records the storage-neutral execution owner added for the zk213 lineage-replacement path. It does not claim physical storage atomicity, malicious-rollback resistance, power-loss qualification, secure erasure, external review, RFC-class completion, or deployment readiness.

## Evidence added

- Rust semantic owner: `rust/crates/proto/src/lineage_replace_storage_transaction.rs`.
- C semantic owner: `c/src/proto/lineage_replace_storage_transaction.c` with public contract in `c/include/auth/lineage_replace_storage_transaction.h`.
- Canonical shared corpus: `rust/test-vectors/replay/lineage-replace-storage-transaction-v1.txt`.
- Independent Rust/C consumers: `rust/crates/proto/tests/lineage_replace_storage_transaction_corpus.rs` and `c/tests/test_lineage_replace_storage_transaction.c`.
- Normative storage-neutral mapping: `spec/lineage-replace-storage-transaction-contract.md`.

The executor rejects incomplete plans before touching the adapter, persists the pending marker before successor visibility, orders successor activation before predecessor retirement, requires the complete predecessor-bound invalidation set before pending-marker clear, and stops at the first failed durable step. It never clears pending or infers completion after an intermediate failure.

## Negative evidence

TX-02 proves an incomplete replacement plan invokes no storage step. TX-03 through TX-07 inject failure at each ordered logical operation and require execution to stop at that exact cut. These cases compose with the existing write-cut/recovery corpora: any persisted intermediate cut remains subject to fail-closed `CONTINUITY_BROKEN` restart classification rather than optimistic completion or normal-AUTH repair.

## Claim boundary

This checkpoint advances **IMPLEMENTED** storage-neutral lifecycle semantics and shared deterministic Rust/C qualification artifacts. It does not establish **MEASURED**, **FORMALLY ANALYZED**, **EXTERNALLY REVIEWED**, **RFC-CLASS DOCUMENTED**, or **DEPLOYMENT-QUALIFIED** status. `INTEROPERABLE` may be claimed for this packet only when both language consumers are actually executed against the canonical corpus in an available qualification environment.

A production storage adapter still has to show how its real persistence primitives realize these steps, how torn/reordered/corrupt observations fail closed, and—where claimed—how a trusted freshness/high-water mechanism detects restoration of an older valid snapshot. Those remain zk213/TD-002 evidence gaps.
