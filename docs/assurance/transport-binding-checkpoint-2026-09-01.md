# Transport Binding / Adapter Authority Checkpoint — 2026-09-01

## Scope

This checkpoint records the Slot 7 interoperability/transport packet that establishes a shared Rust/C `ZK-ARCHE-BIND` decision boundary. It is evidence of implementation and deterministic decision parity scaffolding; it is not a claim that a secure channel exporter or complete transport profile is deployed.

## Implemented evidence

- Rust classifier: `rust/crates/proto/src/transport/binding.rs`.
- C classifier: `c/include/auth/transport_binding.h` and `c/src/proto/transport_binding.c`.
- Canonical cross-language corpus: `rust/test-vectors/state/transport-binding-v1.txt`.
- C corpus consumer: `c/tests/test_transport_binding.c`; Rust consumes the same corpus from its module test.
- Normative ownership and fail-closed rules: `spec/transport-binding-and-adapter-authority.md`.

The classifier distinguishes `BOUND`, `UNBOUND_ALLOWED`, and `REJECT`. It explicitly rejects attempts to use a transport address as protocol identity or unauthenticated transport metadata as protocol authority. A required missing binding fails closed, and a supplied invalid/stale/mismatched binding cannot be silently downgraded to unbound success.

The corpus also retains the opposite mobility invariant: `address_changed_current_binding` remains `BOUND` because address equality is intentionally not part of the cryptographic binding decision.

## Validation performed in this run

The C classifier and canonical 12-case corpus were compiled in isolation with C11 plus `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Werror`; execution returned `transport binding corpus: ok`.

A clean exact-head repository checkout, Rust toolchain, full libsodium-backed C suite, sanitizers, formal tooling, and aggregate release qualification were not available in this execution environment. Those lanes are therefore UNAVAILABLE, not inferred PASS.

## Evidence boundary

This packet advances IMPLEMENTED behavior and shared decision semantics. It does not establish:

- a TLS/DTLS/QUIC/OSCORE/EDHOC exporter construction;
- secure transport metadata authentication;
- full Rust/C transport interoperability;
- resumption wire-protocol integration;
- physical constrained-target measurements;
- formal analysis of the binding classifier;
- independent cryptographic review; or
- deployment qualification.

`TD-001` through `TD-004` remain open. In particular, RFC-class completion still requires independently implementable adapter/profile text plus executable interoperability evidence, and TD-002 still requires actual constrained-target measurements.
