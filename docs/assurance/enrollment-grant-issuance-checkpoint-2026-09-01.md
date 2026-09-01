# Enrollment grant issuance checkpoint — 2026-09-01

## Scope

This packet adds a wire-neutral Rust/C decision boundary for explicit commissioner-authorized enrollment/grant issuance. It closes the semantic seam between NO-LEARNING AUTH and trust/authorization mutation without claiming a complete ENROLL protocol.

## Evidence

- Rust decision implementation: `rust/crates/proto/src/enrollment_grant.rs`
- C decision implementation: `c/src/proto/enrollment_grant.c`
- Shared canonical corpus: `rust/test-vectors/state/enrollment-grant-v1.txt`
- C corpus consumer: `c/tests/test_enrollment_grant.c`
- Normative contract: `spec/enrollment-grant-issuance.md`

The corpus independently falsifies normal-AUTH issuance, absent explicit ENROLL, commissioner authentication/authorization/revocation failures, missing subject possession, authority escalation, unbounded scope/audience/deployment/validity, stale epoch/revocation/lineage, delegation-depth overflow, and rollback suspicion.

## Claim boundary

This is IMPLEMENTED decision semantics plus narrow C-executed qualification when that test is run. It is not evidence of a grant wire format, cryptographic grant verification, durable atomic persistence, revocation propagation, physical MCU measurements, formal proof, independent cryptographic review, RFC status, or deployment qualification.
