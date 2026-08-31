# LINEAGE_REPLACE Authorization Contract

This contract is wire-neutral. A lifecycle replacement request MUST NOT be accepted from a generic authority bit alone. Before the normalized replacement predicate is entered, the implementation MUST establish all of the following: control of the currently authorized credential; control of the proposed successor key; an authenticated current session; authorization of that session for lifecycle replacement; binding to the current authenticated/authorized context; binding to the current predecessor lineage; and a successor authorization scope that is no broader than the currently authorized scope.

The shared decision order is deterministic and fail-closed: current-credential control, successor-key control, session authentication, session authorization, context binding, predecessor binding, then privilege expansion. Only `AUTHORIZED_REPLACEMENT` may be mapped to the normalized lifecycle authority input. All other decisions map to `REJECT_AUTHORITY` and cannot produce a replacement commit plan.

The lower-level `lineage_replace_evaluate` / `evaluate_lineage_replace` predicates remain normalized decision-corpus surfaces. Lifecycle request handlers MUST derive authority through `lineage_replace_classify_authorization` / `classify_lineage_replace_authorization` and enter through the authorized evaluation entrypoint. Normal AUTH remains NO-LEARNING and MUST NOT invoke this lifecycle path as implicit trust mutation.

## Binding to the concrete `iot-core` AUTH authorization context

For an `iot-core` lifecycle caller, session authorization, authenticated context binding, predecessor lineage binding, and privilege-preservation MUST be derived from the same locally authoritative authorization context and attribution record used by the current AUTH decision. Callers MUST NOT independently manufacture those four facts.

`lineage_replace_authorization_from_iot_core` / `lineage_replace_authorization_from_iot_core()` is the shared Rust/C wire-neutral adapter for this rule. Its inputs are:

- the current `IotCoreAuthorizationContextV1` / `auth_v3_iot_core_authorization_context_v1_t`;
- the locally resolved `IotCoreAttributionRecordV1` / `auth_v3_iot_core_attribution_record_v1_t` for the authenticated peer;
- lifecycle evidence for current-credential control, proposed-successor-key control, current-session authentication, expected peer identity, current predecessor credential reference, and requested successor scope.

The adapter fails closed unless the authorization context has a valid constrained `iot-core` shape: non-zero holder binding and audience, non-zero role/policy identifier, exactly the `SECURE_ASSOCIATION` scope, and non-zero authorization generation, policy epoch, and revocation epoch. It then requires the attribution record to match the holder, audience, role/policy, scope, authorization generation, policy epoch, and revocation epoch of that context. A peer-identity mismatch is a context-binding failure. A credential-reference mismatch is a predecessor-binding failure. Requested successor scope MUST be non-zero and a subset of the current authorized scope; otherwise the result is `REJECT_PRIVILEGE_EXPANSION`.

This adapter does not itself verify the cryptographic proofs that establish current-credential control, successor-key control, or session authentication. Those booleans remain outputs of the cryptographic/session layer and are intentionally kept distinct from authorization and trust mutation. A later protocol-integrated lifecycle path MUST feed only verified results into this adapter.

The AZ-01..AZ-10 canonical corpus in `rust/test-vectors/replay/lineage-replace-authorization-v1.txt` covers the normalized authorization decision surface. The AX-01..AX-12 corpus in `rust/test-vectors/replay/lineage-replace-auth-context-v1.txt` covers the concrete `iot-core` binding adapter and is consumed by Rust and C. Its negative cases cover invalid authorization-context shape, attribution/context mismatch, wrong peer binding, stale/wrong predecessor binding, privilege expansion, and deterministic rejection precedence.

This contract does not define proof encodings, packet grammar, attempt identifiers, retransmission, durable storage, cryptographic verification algorithms, or trust-store mutation. Those remain separately gated by the RFC evolution plan and lifecycle qualification evidence.
