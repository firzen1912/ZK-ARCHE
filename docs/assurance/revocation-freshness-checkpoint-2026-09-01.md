# Revocation convergence / stale-authorization checkpoint — 2026-09-01

## Scope

This checkpoint records a bounded zk214 advancement: the repository now owns deterministic local authorization-freshness decision semantics for offline-capable peers.

The packet adds:

- `spec/revocation-convergence-and-stale-authorization.md`;
- `rust/test-vectors/state/revocation-freshness-v1.txt`;
- `scripts/check-revocation-freshness-contract.py`;
- release-qualification integration for the checker.

## Security decision

Successful authentication is not sufficient authorization when local revocation/authorization state is outside the selected profile freshness bound. Revocation is treated as versioned convergence, not a local delete. A peer may operate offline only while its locally retained view is within the declared freshness bound and otherwise valid. Beyond the bound, absence of infrastructure does not make stale authorization current.

Decision precedence is intentionally fail closed: invalid view integrity, old authority epoch, stale view age, explicit revocation, stale lineage, and scope denial all prevent authorization.

This preserves the Common Contract: no CA/cloud/gateway becomes mandatory for the root authentication decision, normal AUTH stays NO-LEARNING, trust remains local/non-transitive, and explicit bounded delegation remains subject to revocation freshness.

## Evidence boundary

This packet is `DEFINED + DETERMINISTIC-CORPUS + STATIC-QUALIFICATION-GATED` only.

It does NOT establish:

- implemented/authenticated revocation distribution;
- Rust/C runtime consumption of the corpus;
- instantaneous revocation across disconnected peers;
- full/differential update reconciliation;
- restart-safe or rollback-resistant persistent revocation storage;
- session/resumption/key/delegation invalidation in production code;
- formal proof of convergence or freshness;
- physical constrained-target evidence;
- independent cryptographic review;
- deployment qualification.

TD-001 through TD-004 remain open. zk214 does not cross a 25-point scoring boundary from this packet alone.
