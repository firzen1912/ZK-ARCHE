# AUTH No-Learning / Trust-Mutation Boundary Checkpoint — 2026-09-01

## Scope

This checkpoint records a bounded Slot-0 lifecycle qualification packet for the current Rust/C production server dispatch paths. It does not claim complete authorization semantics, formal verification, independent cryptographic review, constrained-target evidence, or deployment qualification.

## Evidence advanced

The repository now has:

- `spec/auth-trust-mutation-boundary.md`, which owns the narrow rule that normal AUTH cannot silently become enrollment or persistent trust mutation;
- `rust/test-vectors/state/auth-trust-boundary-v1.txt`, a deterministic five-case trust-effect corpus spanning accepted/rejected AUTH and the explicit SETUP mutation control case; and
- `scripts/check-auth-trust-boundary.py`, a fail-closed structural gate tying that corpus and normative text to the Rust and C production dispatch ownership boundaries.

The checker requires Rust AUTH to retain immutable registry access and rejects mutable registry ownership inside normal AUTH. It requires C AUTH candidate scanning/terminal handling to remain free of `auth_registry_put` / `auth_registry_save` while retaining `SETUP_3` as the explicit persistent registry-mutation control path.

## Validation boundary

The checker is a structural conformance/drift gate. It does not prove that every callee is side-effect free, that storage is rollback-resistant, that application authorization is complete, or that future trust-management operations are correct. Runtime and independent cross-language qualification remain separately required.

During this automation environment, a clean exact repository checkout was unavailable because shell Git access could not resolve `github.com`. The checker mechanics were therefore falsified against a reconstructed fixture matching the exact-current production markers: the canonical fixture passed and an injected `auth_registry_put` in C AUTH_3 failed as intended. Full Rust/C repository-owned qualification is not claimed.

## Claim posture

- IMPLEMENTED: existing production trust-mutation ownership is now normatively captured and mechanically guarded.
- TESTED: checker mechanics only; no new full exact-head Rust/C runtime PASS claimed.
- INTEROPERABLE: unchanged.
- COMMON-CONFORMANT: unchanged.
- MEASURED: unchanged.
- FORMALLY ANALYZED: unchanged.
- EXTERNALLY REVIEWED: unchanged; TD-001 remains open.
- RFC-CLASS DOCUMENTED: advanced within the lifecycle portion of TD-004, but not complete.
- DEPLOYMENT-QUALIFIED: unchanged.

TD-001 through TD-004 remain open.