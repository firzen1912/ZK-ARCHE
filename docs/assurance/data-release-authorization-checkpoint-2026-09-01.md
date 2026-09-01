# Data release authorization checkpoint — 2026-09-01

This checkpoint records the first executable mandatory-floor policy decision for zk231/zk233/zk234.

Implemented:
- Rust and C wire-neutral release classifiers;
- deterministic policy/freshness decision precedence;
- holder, audience, purpose, data-type, policy, epoch, revocation, lineage, rollback, and channel-binding gates;
- canonical negative corpus under `rust/test-vectors/state/data-release-authorization-v1.txt`.

Evidence boundary:
- IMPLEMENTED: yes, classifier only.
- TESTED: narrow local C classifier falsification in this run; full repository qualification unavailable.
- INTEROPERABLE: decision semantics are mirrored; no DATA wire interoperability claim.
- COMMON-CONFORMANT: not complete.
- MEASURED: no constrained-target physical evidence.
- FORMALLY ANALYZED: no new formal result.
- EXTERNALLY REVIEWED: no.
- RFC-CLASS DOCUMENTED: partial normative contract only.
- DEPLOYMENT-QUALIFIED: no.

Still open: DATA_COMMIT/RELEASE_REQUEST/RELEASE_PROOF/RELEASE_KEY/AUDIT_APPEND wire ownership, token/proof verification, protected storage/key wrapping, replay behavior, audit continuity, physical target budgets, formal properties, and independent review.
