# Enrollment authorization freshness and replay checkpoint — 2026-09-02

## Scope

This packet extends the existing Rust/C enrollment-grant issuance boundary with two lifecycle facts required by the canonical roadmap but previously implicit: commissioner authorization freshness and one-time enrollment replay state.

It does not create another trust engine, replay store, or commissioner authority. The classifier consumes facts owned by the existing authorization, revocation, lineage, and replay lifecycle authorities.

## Evidence map

- Rust semantics: `rust/crates/proto/src/enrollment_grant.rs`
- C ABI: `c/include/auth/enrollment_grant.h`
- C semantics: `c/src/proto/enrollment_grant.c`
- current canonical corpus: `rust/test-vectors/state/enrollment-grant-v2.txt`
- Rust corpus consumer: inline unit test in `enrollment_grant.rs`
- C corpus consumer: `c/tests/test_enrollment_grant.c`
- normative decision contract: `spec/enrollment-grant-issuance.md`

The 19-case v2 corpus independently exercises valid issuance, normal-AUTH NO-LEARNING rejection, absent explicit ENROLL, commissioner authentication/authorization failure, **stale commissioner authorization**, commissioner revocation, **replayed enrollment nonce**, missing subject possession, authority escalation, unbounded scope/audience/deployment/validity, stale epoch/revocation/lineage, delegation-depth overflow, and rollback suspicion.

## Security implications

Successful AUTH cannot refresh commissioner authorization, consume or reset enrollment replay state, or restore revoked/stale authority. An otherwise authorized commissioner whose authorization is stale cannot issue new authority. An already-consumed enrollment operation cannot be made reusable by successful possession proof or current credentials.

## Claim boundary

This packet is IMPLEMENTED decision semantics plus narrow executable C corpus evidence where that test actually runs. It is not evidence of a complete ENROLL wire protocol, cryptographic grant verification, persistent replay-store correctness across restart, revocation convergence latency, Rust execution in environments without Rust tooling, physical MCU measurements, formal proof, independent cryptographic review, RFC status, or deployment qualification.
