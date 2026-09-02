# Resumption lifecycle continuity checkpoint — 2026-09-01

## Scope

This packet extends the existing authorization-aware resumption classifier rather than adding another lifecycle authority. It adds the three dependency-ready facts missing from the previous decision surface: restart/replay continuity, credential epoch freshness, and explicit session invalidation.

## Evidence map

- Rust semantics: `rust/crates/proto/src/resumption_authorization.rs`
- C semantics: `c/src/proto/resumption_authorization.c`
- C ABI: `c/include/auth/resumption_authorization.h`
- canonical current corpus: `rust/test-vectors/state/resumption-authorization-v2.txt`
- Rust corpus consumer: inline unit test in `resumption_authorization.rs`
- C corpus consumer: `c/tests/test_resumption_authorization.c`
- normative decision contract: `spec/resumption-authorization-decision.md`

The pre-existing bounded `usage_count/usage_limit` rule continues to prevent indefinite credential reuse. The new rules additionally prevent restart discontinuity or explicit invalidation from being treated as reconnectable success, and require fresh AUTH when a credential belongs to an older policy epoch.

## Claim boundary

This is IMPLEMENTED decision semantics plus narrow executable corpus evidence where the Rust/C tests are actually run. It is not a resumption wire protocol, ticket issuance/protection mechanism, persistent anti-rollback implementation, session-key erasure proof, formal proof, physical MCU evidence, independent cryptographic review, RFC status, or deployment qualification.
