# AUTH_3 terminal-session disposition checkpoint — 2026-08-31

## Scope

This checkpoint records a bounded TD-004 / Rust-C state-conformance repair for failed `AUTH_3` handling.

## Starting evidence

At starting `dev`, Rust retained a pending AUTH session after failed terminal verification while C released the pending slot after every processed `AUTH_3`. The observable-failure matrix correctly classified this as a state-machine divergence and prohibited an interoperability claim for failed-AUTH_3 retry semantics.

## Decision

The normative rule is now fail-closed terminal consumption:

- locating a pending AUTH session for `AUTH_3` consumes that pending state before terminal cryptographic/context verification;
- success and failure both leave no pending AUTH state for that `session_id`;
- failed terminal verification does not permit corrected same-session `AUTH_3` continuation;
- a retry begins from `AUTH_1` with a fresh `session_id`;
- exact duplicate cached-response replay may return a cached packet but does not recreate pending state or re-run terminal verification.

The rationale is bounded state lifetime and avoidance of repeated terminal-verification attempts against retained ephemeral state. This does not redefine replay identity, wire encoding, proof/KDF/MAC inputs, authorization, or trust mutation.

## Implementation alignment

C already consumed the pending AUTH slot after `auth_server_handle_auth3` regardless of result.

Rust now removes the pending AUTH session before calling `handle_auth_3`, using a small generic helper whose unit test verifies consume-once semantics. This aligns Rust with the normative rule and the existing C production dispatcher.

## Validation boundary

The available execution environment does not contain Cargo, rustc, rustfmt, or ProVerif, and therefore cannot execute the Rust repository-owned test/format/clippy/formal lanes. Full clean-checkout release qualification is not claimed.

The source change is intentionally small and ownership-local: the Rust dispatcher changes from lookup-then-remove-on-success to remove-before-terminal-verification, and a unit test exercises the exact state-container transition independently of cryptographic construction.

A future exact-head qualification run must execute the Rust test plus complete repository-owned Rust/C interoperability/release lanes before this surface is reported as TESTED and INTEROPERABLE end-to-end.

## Claim boundary

Advanced: normative state-machine ownership and Rust/C implementation alignment for pending AUTH disposition.

Not advanced by this checkpoint alone: cryptographic review, physical measurements, formal proof, complete interoperability qualification, privacy equivalence, RFC-class completion, Common Contract conformance, or deployment qualification.

TD-001 through TD-004 remain open.