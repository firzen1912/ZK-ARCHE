# Lineage replacement storage-capability checkpoint — 2026-08-31

## Scope

This checkpoint reviews the new shared Rust/C storage-capability classifier used to make the rollback-resistant LINEAGE_REPLACE storage boundary explicit. The change is wire-neutral and does not alter cryptographic primitives, transcript construction, authorization semantics, trust mutation, packet formats, registries, or transport identity.

## Security claim

The classifier prevents the repository from collapsing several distinct target properties into a generic `secure_storage=true` or `durable=true` assertion. Rust and C now make durability, power-loss recovery, record integrity, replay protection, freshness-anchor availability, freshness-anchor integrity, and lineage binding separate deterministic gates.

Only `QUALIFIED` means that the declared adapter metadata contains every property required by this strict qualification surface. This is a metadata-semantic result, not target evidence.

## Negative evidence

The canonical SC-01…SC-10 corpus covers the positive path, each missing property independently, deterministic precedence under multiple failures, and the missing-capability fail-closed case.

Available local execution during creation:

- C11 strict warnings-as-errors compilation: PASS for the new classifier/corpus;
- C UBSan execution: PASS for the new classifier/corpus;
- Rust fmt/check/test/clippy/audit: unavailable in the execution environment;
- complete repository C build/tests/ASan/UBSan/cppcheck: unavailable because a clean repository checkout and libsodium are unavailable;
- ProVerif/formal synchronization: unavailable;
- full Rust/C interoperability/release qualification: unavailable.

No unavailable lane is counted as passing.

## Claim boundary

This checkpoint advances IMPLEMENTED and locally executable C TESTED evidence for the storage-capability semantic surface. It does not advance MEASURED, FORMALLY ANALYZED, EXTERNALLY REVIEWED, RFC-CLASS DOCUMENTED, or DEPLOYMENT-QUALIFIED status.

TD-002 remains open: no physical ESP32-S3, STM32, PSA/TF-M, secure-element, filesystem, or flash measurement is created here. TD-003 and TD-004 remain open. TD-001 remains open and unaffected.

The implementation deliberately reflects the 2026-08-31 research finding that PSA/TF-M service selection, vendor NVS durability, and hardware anti-rollback mechanisms have materially different semantics. In particular, a storage technology name is not accepted as proof of per-lineage freshness.
