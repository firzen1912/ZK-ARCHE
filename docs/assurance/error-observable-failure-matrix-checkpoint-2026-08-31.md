# ERROR / Observable-Failure Matrix Checkpoint — 2026-08-31

## Scope

This checkpoint records the TD-004 conformance packet that inventories currently observable error/state behavior without inventing missing normative semantics.

Starting exact `dev` HEAD: `fd11c08ba4770efab20e8bf328ae13e52ebbc963`.

Primary surfaces reviewed:

- `spec/zk-arche-protocol.md`
- `spec/registries.md`
- `spec/privacy-considerations.md`
- `docs/technical-debt/README.md`
- `rust/crates/server/src/main.rs`
- `c/bin/server.c`
- C production-dispatch replay tests
- current Rust/C ERROR normalization corpus and registry-parity gate

## Finding

Exact-current Rust and C differ after a failed `AUTH_3` for an existing pending AUTH session:

- Rust retains the pending AUTH session when the AUTH_3 handler fails.
- C releases the pending AUTH session after the handler invocation even when the handler reports failure.

The protocol requires failed completion/context/authenticator verification to fail closed, but does not currently own whether such a failure consumes the pending session or permits same-flight retry/retransmission. Selecting either implementation as normative would therefore invent semantics.

## Change

Added `spec/error-and-observable-failure-state-matrix.md` with:

- explicit `OWNED`, `IMPLEMENTATION-OBSERVED`, `DIVERGENT`, and `UNRESOLVED` evidence labels;
- current ERROR-versus-silence behavior for bounded dispatcher surfaces;
- state consequences and retry/retransmission ownership boundaries;
- an explicit failed-AUTH_3 Rust/C divergence row;
- privacy/observability boundaries for silence versus protocol ERROR;
- clearing requirements before the divergent row can become conformant.

No protocol wire allocation, cryptographic primitive, parser, formal model, test vector, trust mutation rule, or runtime implementation was changed by this packet.

## Validation boundary

The automation execution environment could not establish a clean exact-head checkout because direct GitHub cloning failed DNS resolution (`Could not resolve host: github.com`). Full repository-owned Rust/C/formal/release qualification was therefore unavailable and is not reported as passing.

The packet was checked structurally against exact-current GitHub source and specification content. No executable theorem, interop, sanitizer, or target measurement result is claimed.

## Evidence state

- IMPLEMENTED: runtime behavior unchanged; conformance/spec inventory advanced.
- TESTED: no exact-head executable qualification upgrade in this environment.
- INTEROPERABLE: **not upgraded**; a bounded failed-AUTH_3 state-lifetime divergence is now explicit.
- COMMON-CONFORMANT: unchanged.
- MEASURED: unchanged.
- FORMALLY ANALYZED: unchanged.
- EXTERNALLY REVIEWED: unchanged; TD-001 remains open.
- RFC-CLASS DOCUMENTED: advanced within TD-004, but not complete.
- DEPLOYMENT-QUALIFIED: unchanged.

## Required next evidence

Before resolving the failed-AUTH_3 row, the repository needs one normative session-disposition/retry rule, deterministic Rust/C failure/retry/replay tests, resource-exhaustion and privacy analysis, affected formal-model alignment, and an exact-head Rust/C qualification run.

This checkpoint does not close TD-004 or any roadmap phase by itself.