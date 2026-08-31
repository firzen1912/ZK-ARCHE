# LINEAGE_REPLACE session-bound authorization checkpoint — 2026-08-31

Scope: zk213 lifecycle qualification, Rust/C decision parity, wire-neutral composition.

This checkpoint removes one caller-controlled security fact from the recommended lifecycle path. `lineage_replace_authorization_from_bound_iot_core` now composes the existing AUTH-session binding classifier with the existing `iot-core` authorization/attribution adapter. Callers provide possession evidence, expected peer/predecessor identity, and requested scope, but cannot directly assert `current_session_authenticated` at this entrypoint.

Canonical BA-01 through BA-12 cover the positive path; current-credential and successor-key rejection precedence; failed AUTH completion; stale/different session identifier; authorization-context mismatch; channel-binding mismatch; stale attribution; wrong predecessor; privilege expansion; and multi-fault precedence. Missing session context/expectation is specified to fail as session authentication once possession gates are satisfied.

Claim boundary: this is implementation/spec/test evidence only. It does not establish that current-credential or successor-key possession is cryptographically derived from production lifecycle messages, that AUTH-v3 is selectable, that replacement is durably atomic or rollback resistant on hardware, that Rust/C interoperability has executed in this environment, or that the protocol is externally reviewed, RFC-class complete, or deployment qualified. TD-001 through TD-004 remain open.
