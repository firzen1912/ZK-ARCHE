# LINEAGE_REPLACE authenticated-session binding contract

Status: qualification contract; wire-neutral; AUTH-v3 remains non-advertised.

A lifecycle replacement MUST NOT treat possession of a locally populated authorization record as evidence that the currently active AUTH session is the session that authenticated that record. Before replacement authorization is promoted, the implementation must bind the lifecycle request to an authenticated-completion result and the exact AUTH-v3 security context.

For the current qualification surface, `BOUND` requires all of: verified AUTH completion, expected protocol version, suite and profile, exact session identifier, exact authorization-context hash, and exact channel-binding hash. Any mismatch fails closed with a distinct decision. Evaluation order is completion, version, suite, profile, session id, authorization context, channel binding.

`BOUND` does not itself prove current-credential control, successor-key control, lifecycle authorization, predecessor freshness, privilege preservation, durable commit, or peer convergence. Those remain separate lineage-replacement gates. Normal AUTH remains NO-LEARNING.

This contract allocates no wire field and does not make AUTH-v3 selectable. Production wire allocation remains gated on the RFC evolution plan and independent Rust/C qualification.
