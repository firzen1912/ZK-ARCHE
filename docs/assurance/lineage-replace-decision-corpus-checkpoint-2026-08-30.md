# LINEAGE_REPLACE Decision Corpus Checkpoint — 2026-08-30

Scope: zk213 / priority-1 Rust/C reproducible truth for the wire-neutral lineage-replacement decision surface.

## Evidence advanced

This checkpoint adds one canonical RE-01..RE-20 semantic corpus plus independent Rust and C pure decision evaluators. The corpus is consumed directly by both language-specific tests, preserving one expected-decision source of truth.

Affected surfaces:

```text
spec/lineage-replace-decision-contract.md
rust/test-vectors/replay/lineage-replace-decisions-v1.txt
rust/crates/proto/src/lineage_replace.rs
rust/crates/proto/tests/lineage_replace_corpus.rs
c/include/auth/lineage_replace.h
c/src/proto/lineage_replace.c
c/tests/test_lineage_replace.c
```

## Security review boundary

The evaluators are deliberately pure. They do not parse a network message, verify cryptographic authority evidence, mutate trust, activate an epoch, retire a predecessor, write durable state, or make `iot-core` selectable. Normal AUTH cannot implicitly create a successor lineage because `AUTH`, restart, and transport-change triggers are explicit negative corpus cases.

The packet covers the decision classes required by the current state-owner specification: authority, predecessor/successor binding, context/downgrade/domain binding, freshness/revocation, replay, concurrent replacement, rollback, storage ambiguity, and dependent-state revalidation.

## Validation

A standalone C build of the evaluator and the same 20-case mutation/decision matrix passed locally with `gcc -std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Werror`. A clean repository checkout was unavailable in the execution environment because DNS resolution for `github.com` failed, and `rustc` was not installed there. Therefore no local Rust or full repository CI-equivalent lane is claimed. Hosted exact-head CI after publication remains authoritative.

## Non-claims retained

This checkpoint does not establish atomic predecessor replacement, crash-consistent storage, rollback-proof hardware, cryptographic recovery authority, successor proof-of-possession, formal verification of the implementation, PFS, PCS, KCI resistance, TD-001 external review, TD-002 physical measurements, TD-003 completion, TD-004 completion, Common Contract conformance, RFC-class status, or deployment qualification.
