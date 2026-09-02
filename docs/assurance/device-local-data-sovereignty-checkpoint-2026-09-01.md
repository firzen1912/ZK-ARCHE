# Device-local DATA sovereignty checkpoint — 2026-09-01

## Scope

This packet advances the mandatory constrained DATA sovereignty floor without introducing optional heavyweight credentials or proof systems.

The existing release classifier already covered authentication separation, authorization freshness, revocation, lineage, holder/audience/purpose/data-type/policy/epoch matching, channel binding, and rollback. Version 2 adds three missing per-device sovereignty guards: explicit current device-local release authority, confirmation that protected data remains encrypted, and release-key scope equality with the protected object/data class.

## Evidence map

- Rust semantics: `rust/crates/proto/src/data_release_authorization.rs`
- C ABI: `c/include/auth/data_release_authorization.h`
- C semantics: `c/src/proto/data_release_authorization.c`
- canonical current corpus: `rust/test-vectors/state/data-release-authorization-v2.txt`
- Rust corpus consumer: inline unit test
- C corpus consumer: `c/tests/test_data_release_authorization.c`
- normative decision contract: `spec/data-release-authorization-decision.md`

The 19-case corpus independently exercises current release, unauthenticated fallback, missing/stale device authority, plaintext rejection, key-scope mismatch, missing/stale authorization, stale revocation, explicit revocation, stale lineage, holder/audience/purpose/data-type/policy/epoch mismatch, required-binding failure, and rollback suspicion.

## Claim boundary

This is IMPLEMENTED decision semantics plus narrow executable corpus evidence where tests actually run. It does not claim a DATA wire protocol, cryptographic release-token verification, data-at-rest encryption implementation, key wrapping, audit/hash-chain implementation, physical target measurements, formal proof, independent cryptographic review, RFC/IETF status, or deployment qualification.
