# Checkpoint: LINEAGE_REPLACE pair reconciliation — 2026-08-30

## Scope

This checkpoint records promotion of the previously duplicated asymmetric durable pair-decision logic into shared Rust/C implementation surfaces for `zk213`.

Affected specification: `spec/lineage-replace-asymmetric-durable-composition.md`.
Affected implementation: Rust/C `lineage_replace_reconciliation` classifiers.
Affected negative/qualification evidence: `lineage-replace-asymmetric-durable-v1.txt` and its Rust/C consumers.

## Security invariants retained

- normal AUTH remains NO-LEARNING;
- no trust-store mutation occurs in reconciliation classification;
- missing or broken continuity never becomes an availability success;
- durable successor state does not reconstruct missing peer confirmation;
- two different stable successors never receive an implicit winner;
- pair-level successor readiness requires the same successor plus bilateral converged replacement attempts;
- target rollback resistance remains an external storage-property claim and is not inferred from this classifier.

## Rust/C decision parity

The shared classifier has five outcomes in both languages:

- `PAIR_SUCCESSOR_READY`;
- `PAIR_PREDECESSOR_READY`;
- `RECONCILIATION_REQUIRED`;
- `CONTINUITY_BROKEN`;
- `SUCCESSOR_DIVERGENCE`.

The existing AC-01 through AC-14 corpus now calls the production classifier directly rather than maintaining an independent test-local copy of pair semantics. C additionally treats a null fact set as `CONTINUITY_BROKEN`.

## Validation retained in this run

The executable environment provided GCC but not Cargo/rustfmt, ProVerif, cppcheck, or a usable libsodium pkg-config installation. The reconciliation classifier and its exact dependent lineage-attempt/recovery/freshness semantics were compiled with strict warnings as errors and the 14-case asymmetric durable corpus passed.

Unavailable lanes are not treated as passed. No hosted GitHub Actions result is expected or required on `dev` under the current branch policy.

## Evidence boundary

This checkpoint does not establish cryptographic key-confirmation soundness, transport loss/reorder liveness, target crash-safe storage, malicious rollback resistance, constant-time behavior, RNG quality, independent interoperability execution, external cryptographic review, RFC-class completion, Common Contract conformance, or deployment qualification. TD-001 through TD-004 remain open where applicable.
