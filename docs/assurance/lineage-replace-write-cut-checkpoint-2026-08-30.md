# LINEAGE_REPLACE logical write-cut checkpoint — 2026-08-30

## Scope

This checkpoint covers only the storage-neutral logical write ordering and deterministic fault-injection model for zk213. It does not change wire behavior, cryptographic primitives, authorization semantics, trust authority, or normal AUTH behavior.

## Affected surfaces

- `spec/lineage-replace-storage-transaction-contract.md`
- `rust/crates/proto/src/lineage_replace_faults.rs`
- `rust/crates/proto/tests/lineage_replace_fault_corpus.rs`
- `c/include/auth/lineage_replace_faults.h`
- `c/src/proto/lineage_replace_faults.c`
- `c/tests/test_lineage_replace_faults.c`
- `rust/test-vectors/replay/lineage-replace-write-cuts-v1.txt`

## Security invariants exercised

- only stable predecessor and fully committed successor observations may recover operably;
- every intermediate logical write cut fails closed through the existing recovery classifier;
- restart cannot silently complete a trust mutation;
- normal AUTH remains NO-LEARNING;
- unknown C cut identifiers produce invalid-integrity facts and therefore fail closed;
- Rust and C consume one canonical expected-state corpus.

## Evidence boundary

This packet is deterministic model evidence only. It does not measure or prove real flash/filesystem ordering, power-loss behavior, journaling, anti-rollback properties, secure erasure, or deployment readiness. No TD-002 physical measurement is claimed. No new TD-003 formal result or TD-001 external review is claimed. RFC-class completion remains blocked by the broader requirements in `docs/roadmaps/rfc-evolution-plan.md`.

Hosted exact-head CI remains authoritative for Rust formatting/check/tests/clippy/audit, C build/tests/static analysis/sanitizers, formal gates, and Rust/C release qualification after publication.
