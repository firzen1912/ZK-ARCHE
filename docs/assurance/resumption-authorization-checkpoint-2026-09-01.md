# Authorization-aware resumption checkpoint — 2026-09-01

Scope: `zk221` decision semantics only.

## Evidence added

- Rust and C now expose the same wire-neutral resumption authorization classifier.
- Both implementations consume the canonical 16-case corpus at `rust/test-vectors/state/resumption-authorization-v1.txt`.
- The decision distinguishes `RESUME`, `FULL_AUTH_REQUIRED`, and fail-closed `REJECT`.
- Stale revocation state, explicit revocation, stale lineage, and rollback suspicion cannot be overridden by possession of a resumption credential.
- Missing/stale cached authorization context and changed peer/deployment/audience/profile force a fresh full AUTH rather than silently preserving old privilege.
- Reuse is bounded; a zero limit disables resumption and `usage_count >= usage_limit` forces full AUTH.

## Evidence boundary

This checkpoint does not claim a ticket/PSK wire format, ticket issuance/protection, live resumption handshake, persistent anti-rollback storage, 0-RTT, physical-target evidence, formal proof, independent cryptographic review, or deployment qualification. Full AUTH after fallback remains independently subject to current authorization/revocation/freshness checks and NO-LEARNING trust semantics.

The packet should therefore be read as IMPLEMENTED + cross-language deterministic decision evidence for one `zk221` sub-surface, not complete resumption qualification.
