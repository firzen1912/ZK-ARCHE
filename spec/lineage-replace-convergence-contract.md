# LINEAGE_REPLACE distributed convergence contract

Status: implementation-facing draft; wire-neutral.

This contract defines the minimum distributed decision semantics required before a future authenticated rekey/re-registration exchange may treat a successor lineage as converged. It intentionally assigns no packet type, field, retry message, transport behavior, timeout, or cryptographic construction.

## Required decision facts

Each peer evaluates five logical facts: local replacement authorization is valid; peer replacement authorization is valid; both sides refer to the same successor lineage; local key confirmation is complete; and peer key confirmation is complete.

A successor is **CONVERGED** only when both authorization facts are valid, both sides identify the same successor, and both confirmations are complete. When authorization and successor identity agree but either confirmation is absent, the state is **AWAITING_CONFIRMATION** and MUST NOT authorize use of the successor as a completed replacement. Missing authorization is **UNAUTHORIZED**. Two authenticated but different successor candidates are **SUCCESSOR_CONFLICT** and MUST fail closed rather than selecting a winner implicitly.

Duplicate or reordered observations may repeat an existing decision but MUST NOT turn `UNAUTHORIZED`, `SUCCESSOR_CONFLICT`, or `AWAITING_CONFIRMATION` into `CONVERGED` unless the complete required fact set becomes true. Normal AUTH remains NO-LEARNING and cannot create these replacement facts as a side effect.

## Evidence boundary

The shared CV-01 through CV-08 corpus tests Rust/C decision parity for the logical classifier. This does not establish a wire protocol, liveness, distributed consensus, crash-safe remote commit, channel-binding construction, key-confirmation cryptography, real network reordering behavior, target persistence, rollback resistance, or field deployment evidence. Those remain separate qualification obligations.
