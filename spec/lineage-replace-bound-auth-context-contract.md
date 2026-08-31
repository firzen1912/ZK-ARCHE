# LINEAGE_REPLACE session-bound authorization composition contract

Status: qualification contract; wire-neutral; AUTH-v3 remains non-advertised.

The recommended `iot-core` lifecycle authorization entrypoint MUST NOT accept a caller-supplied assertion that the current session is authenticated. It MUST derive that fact from the explicit AUTH-session binding classifier and then compose the resulting fact with the existing `iot-core` authorization/attribution binding.

The composition order preserves the existing authorization precedence. Missing current-credential control and successor-key control remain distinguishable from session-authentication failure. Session authentication is true only when `lineage_replace_classify_session_binding` / `classify_lineage_replace_session_binding` returns `SESSION_BOUND` / `Bound`. Any missing AUTH context, missing expectation, failed completion, version/suite/profile mismatch, session-id mismatch, authorization-context mismatch, or channel-binding mismatch therefore maps to `REJECT_SESSION_AUTHENTICATION` once earlier possession gates have passed.

A session-bound decision does not by itself authorize replacement. The existing `iot-core` authorization context and attribution record MUST still agree, the peer and predecessor bindings MUST be current, and requested successor scope MUST remain within the current authorized scope. Normal AUTH remains NO-LEARNING; this composition is lifecycle-only and performs no trust mutation.

This contract introduces no packet, registry value, cryptographic primitive, retry behavior, transport identity, persistence mechanism, or infrastructure dependency. It does not make AUTH-v3 selectable and does not establish durable atomic commit, rollback resistance, independent interoperability, external review, RFC status, or deployment qualification.
