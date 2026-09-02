# Transport association continuation checkpoint — 2026-09-01

## Scope

This packet closes a transport/decomposition seam between the existing transport adapter rule, BIND decision, replay continuity, secure-association admission, and authorization-aware resumption surfaces.

It adds no transport-specific protocol and does not let transport identity become protocol authority.

## Evidence map

- Rust semantics: `rust/crates/proto/src/transport/continuation.rs`
- Rust integration: `rust/crates/proto/src/transport/mod.rs`
- C ABI: `c/include/auth/transport_continuation.h`
- C semantics: `c/src/proto/transport_continuation.c`
- canonical corpus: `rust/test-vectors/state/transport-continuation-v1.txt`
- C corpus consumer: `c/tests/test_transport_continuation.c`
- normative decision contract: `spec/transport-association-continuation.md`

## Properties qualified by the corpus

- a route change does not become identity;
- a connection replacement does not become authorization;
- address-as-identity and metadata-as-authority fail closed;
- explicit association invalidation cannot be undone by reconnect;
- stale replay/restart continuity cannot be repaired by reconnect;
- authenticated peer mismatch fails closed;
- profile mismatch requires fresh AUTH;
- required binding failure fails closed;
- unauthorized continuation requires fresh AUTH;
- route/connection change alone does not change a valid continuation decision.

## Evidence boundary

This packet is IMPLEMENTED decision semantics plus narrow Rust/C corpus evidence only when those tests execute successfully. It is not byte-level network interoperability, exporter/channel-binding proof, transport security, ticket/PSK protection, persistent anti-rollback evidence, formal proof, physical MCU evidence, external review, RFC status, or deployment qualification.
