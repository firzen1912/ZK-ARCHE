# LINEAGE_REPLACE asymmetric durable checkpoint — 2026-08-30

## Scope

Checkpoint for the `zk213` asymmetric durable-commit qualification packet. It composes existing replacement-attempt, durable-recovery, and freshness semantics across two peers without changing cryptography, wire grammar, production state machines, or trust-mutation ownership.

## Affected surfaces

- normative qualification contract: `spec/lineage-replace-asymmetric-durable-composition.md`
- canonical shared corpus: `rust/test-vectors/replay/lineage-replace-asymmetric-durable-v1.txt`
- Rust consumer: `rust/crates/proto/tests/lineage_replace_asymmetric_durable_corpus.rs`
- C consumer: `c/tests/test_lineage_replace_asymmetric_durable.c`
- unchanged dependencies: replacement-attempt classifier, durable recovery classifier, freshness classifier

## Negative evidence exercised

The AC-01..AC-14 corpus covers:

- local durable successor commit while the peer remains on a predecessor after missing confirmation;
- peer restart/predecessor state after the local side has a stable successor;
- old valid successor snapshot restored below a newer trusted high-water generation;
- predecessor snapshot below the trusted high-water generation;
- partial durable successor state;
- attempt-ID mismatch combined with asymmetric stable state;
- both peers storing the same successor while one lacks converged confirmation;
- both peers storing different stable successors;
- clean predecessor recovery with no converged replacement;
- clean same-successor convergence after explicit reconciliation.

## Review conclusions

The packet is intentionally conservative. `PAIR_SUCCESSOR_READY` requires same-successor stable recovery plus bilateral converged attempt state. `RECONCILIATION_REQUIRED`, `SUCCESSOR_DIVERGENCE`, and `CONTINUITY_BROKEN` are not AUTH-readiness outcomes and cannot be upgraded by storage integrity alone.

No winner rule, wire identifier, timeout, retry policy, retransmission behavior, cryptographic primitive, target storage primitive, or availability override is introduced.

## Evidence claims

Implemented: yes, as Rust/C qualification surfaces and canonical vectors.

Locally tested: the C corpus can be compiled against the existing C attempt/freshness/recovery classifiers when a C compiler is available.

Not established by this checkpoint: full Rust/C clean-checkout qualification, target power-loss behavior, target rollback resistance, Common Contract conformance, formal proof, external cryptographic review, RFC-class completion, or deployment qualification.
