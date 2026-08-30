# LINEAGE_REPLACE restart classifier checkpoint — 2026-08-30

## Scope

This checkpoint covers the storage-neutral restart classifier added downstream of the existing zk213 `LINEAGE_REPLACE` decision predicate, commit planner, and logical state machine.

## Affected normative surface

- `spec/lineage-replace-durability-contract.md`
- `spec/lineage-replace-state-machine.md` remains unchanged and still defines the in-memory logical transition.

## Implementation and executable evidence

- Rust: `rust/crates/proto/src/lineage_replace_recovery.rs`
- C: `c/src/proto/lineage_replace_recovery.c`
- Canonical shared corpus: `rust/test-vectors/replay/lineage-replace-recovery-v1.txt`
- Rust corpus test: `rust/crates/proto/tests/lineage_replace_recovery_corpus.rs`
- C corpus test: `c/tests/test_lineage_replace_recovery.c`

The classifier accepts only clean predecessor or fully committed successor observations. Pending, mixed, partial, corrupt, empty, and contradictory observations fail closed to `CONTINUITY_BROKEN`.

## Narrow falsification performed in this run

A standalone C build of the new classifier and corpus consumer was compiled with:

```text
gcc -std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Werror
```

The 12-case shared corpus and null-input fail-closed behavior passed in that narrow harness.

A clean repository checkout and local Rust execution were unavailable because the runtime could not resolve `github.com`. Hosted exact-head CI therefore remains authoritative for Rust formatting/check/tests/clippy/audit, full C build/tests/static analysis/sanitizers, formal gates, and Rust/C release qualification.

## Security review boundary

This packet does not allocate a wire message, mutate a production trust store, complete an interrupted replacement, or claim durable transaction correctness. It intentionally refuses to reconstruct `REPLACEMENT_PENDING` after restart.

No claim is made for physical rollback resistance, power-loss survival, secure erasure, external cryptographic review, Common Contract conformance, RFC-class status, or deployment readiness.

## Roadmap effect

This advances zk213 evidence within its current rubric band by making restart/partial-state interpretation reproducible and cross-language-testable. It does not close the phase because actual authenticated rekey/re-registration, durable atomic persistence, real storage-adapter evidence, rollback handling on target hardware, and production transition integration remain absent.
