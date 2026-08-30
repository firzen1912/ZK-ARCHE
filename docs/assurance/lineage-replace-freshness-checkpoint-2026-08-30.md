# LINEAGE_REPLACE freshness checkpoint — 2026-08-30

## Scope

This checkpoint records the bounded zk213 addition of a storage-neutral durable-generation/freshness classifier shared by Rust and C.

The change does not modify the `LINEAGE_REPLACE` wire format, cryptographic primitives, AUTH behavior, trust-establishment rules, or the previously qualified structural restart classifier. It adds a stricter adapter-facing gate for deployments that require trusted freshness evidence at restart.

## Affected surfaces

- `spec/lineage-replace-freshness-contract.md`
- `rust/crates/proto/src/lineage_replace_freshness.rs`
- `rust/crates/proto/tests/lineage_replace_freshness_corpus.rs`
- `rust/test-vectors/replay/lineage-replace-freshness-v1.txt`
- `c/include/auth/lineage_replace_freshness.h`
- `c/src/proto/lineage_replace_freshness.c`
- `c/tests/test_lineage_replace_freshness.c`
- `rust/crates/proto/src/lib.rs`

## Decision invariants

The shared classifier accepts a candidate lineage record as `CURRENT` only when all of the following hold:

```text
trusted anchor is available
trusted anchor integrity is valid
anchor is bound to the correct local/security domain
record_generation == trusted_high_water_generation
```

Every unavailable, invalid, misbound, older, or unexpectedly newer generation is non-current and causes the combined restart path to return `CONTINUITY_BROKEN`.

Freshness cannot rescue a structurally invalid or pending recovery observation. The existing structural classifier remains the final state classifier after freshness succeeds.

## Canonical negative coverage

The shared FR-01..FR-10 corpus covers:

- current predecessor;
- current fully committed successor;
- older predecessor snapshot;
- older successor snapshot after a later generation;
- record generation ahead of the trusted high-water value;
- unavailable freshness anchor;
- invalid anchor integrity;
- freshness-anchor binding mismatch;
- current freshness paired with structurally partial lineage state;
- maximum `u64` generation equality without increment/wraparound assumptions.

## Local validation

A narrow standalone C falsification build of the new classifier API was executed with:

```text
gcc -std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Werror
```

The current-generation, rollback, generation-ahead, and null-input fail-closed paths passed in that isolated harness.

A clean repository checkout and repository-owned Rust/C/full qualification were unavailable in the execution environment because direct `github.com` DNS resolution was unavailable and no local Rust toolchain was present. Hosted exact-head Actions therefore remains authoritative after publication.

## Claim boundary

This checkpoint establishes IMPLEMENTED cross-language normalized semantics only until exact-head CI qualifies the committed packet.

It does not establish:

- physical rollback resistance;
- secure-element or monotonic-counter correctness;
- flash/filesystem crash atomicity;
- power-loss behavior on a real target;
- secure erasure;
- clone/reprovision resistance;
- authenticated distributed convergence or key confirmation;
- constrained-target measurements;
- new formal proof results;
- independent cryptographic review;
- Common Contract conformance;
- RFC-class completion;
- deployment or certification status.

TD-001 through TD-004 remain open. In particular, TD-002 still owns target-class storage/resource evidence and TD-004 still owns complete normative lifecycle specification.
