# Lineage replacement authenticated-session binding checkpoint — 2026-08-31

Scope: zk213 qualification only.

This checkpoint adds one shared Rust/C semantic owner for binding a lifecycle replacement attempt to the exact AUTH-v3 security context. The classifier requires verified authenticated completion and exact version/suite/profile/session/authz-context/channel-binding agreement before returning `BOUND`.

Evidence added: C implementation/header and SB corpus consumer; Rust implementation and corpus consumer; canonical SB-01..SB-09 vectors; normative wire-neutral contract.

Negative coverage: missing completion, stale/wrong version, suite/profile mismatch, wrong session identifier, authorization-context mismatch, channel-binding mismatch, and missing input fail closed.

Claim boundary: AUTH-v3 is still non-advertised. This does not establish cryptographic review, formal proof, constrained-target measurements, durable atomic commit, Rust/C executable interoperability in environments where Rust cannot run, external review, RFC status, or deployment qualification.
