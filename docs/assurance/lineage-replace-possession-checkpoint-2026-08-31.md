# LINEAGE_REPLACE possession checkpoint — 2026-08-31

## Scope

This checkpoint removes the untyped possession-boolean boundary from the lifecycle qualification model by introducing one Rust/C classifier for verified proof results bound to the current session and requested predecessor/successor references.

## Evidence

- Rust semantic owner: `rust/crates/proto/src/lineage_replace_possession.rs`.
- C semantic owner: `c/src/proto/lineage_replace_possession.c`.
- Canonical shared corpus: `rust/test-vectors/replay/lineage-replace-possession-v1.txt` (`LP-01` through `LP-10`).
- Rust consumer: `rust/crates/proto/tests/lineage_replace_possession_corpus.rs`.
- C consumer: `c/tests/test_lineage_replace_possession.c`.

The corpus includes absent/unverified proof results, stale session binding, wrong predecessor/successor references, and deterministic current-credential-before-successor rejection precedence.

## Claim boundary

This checkpoint does not claim production cryptographic possession verification, production wire integration, complete Rust/C interoperability execution, target power-loss/rollback evidence, external review, formal proof, RFC status, or deployment qualification. The proof-result objects are an integration contract for a later real lifecycle handler, not substitutes for that handler.
