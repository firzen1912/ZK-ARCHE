# ERROR registry parity gate checkpoint — 2026-08-31

## Scope

This checkpoint records a bounded reproducible-truth improvement for the shared Rust/C ERROR registry and `rust/test-vectors/wire/error-code-normalization-v1.txt` corpus.

The protocol already had a canonical normalization corpus and both implementations consumed its semantics, but release qualification did not mechanically prove that the Rust allocation table, the C allocation table, and the corpus still described the same registered wire set.

## Change

`scripts/check-error-registry-parity.py` now fails closed when any of the following occurs:

- Rust and C expose different wire ERROR numeric allocations;
- either language duplicates a wire allocation;
- registered `0x7fff` `UNSPECIFIED` disappears;
- a registered wire ERROR lacks exactly one identity-normalization corpus case;
- a registered value normalizes to any value other than itself;
- an unregistered corpus value normalizes to anything other than `0x7fff`;
- corpus case identifiers are duplicated or malformed;
- required registry/corpus inputs are missing.

C-local status values `0x0000` through `0x0005` remain outside the wire allocation set. If they appear as received wire values in the corpus, they are therefore governed by the unregistered-value rule and must normalize to `UNSPECIFIED`.

`scripts/ci-release-qualification.sh` now executes this parity gate before the Rust and C lanes. A release-qualification PASS can no longer coexist with silent drift among the two language registries and the canonical normalization corpus.

## Validation performed in this run

The new checker was executed in an isolated reconstructed repository fixture with matching Rust/C registries, registered identity cases, and unknown/local-only fallback cases. The gate returned PASS. This validates the checker mechanics only.

The available execution environment could not establish a clean exact-current repository checkout, so the complete exact-head Rust, C, formal, interoperability, sanitizer, and release-qualification lanes were not executed here. They remain unavailable rather than inferred passing.

## Claim boundary

This packet advances **IMPLEMENTED** repository-owned conformance governance and provides narrow **TESTED** evidence for the checker mechanics.

It does not by itself establish Rust/C interoperability, cryptographic review, formal security, constrained-target measurements, RFC-class completion, or deployment qualification. The existing canonical corpus and language-specific consumers still require execution under the full repository-owned qualification lanes before interoperability claims advance.
