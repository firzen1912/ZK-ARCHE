# LINEAGE_REPLACE Authorization Contract

This contract is wire-neutral. A lifecycle replacement request MUST NOT be accepted from a generic authority bit alone. Before the normalized replacement predicate is entered, the implementation MUST establish all of the following: control of the currently authorized credential; control of the proposed successor key; an authenticated current session; authorization of that session for lifecycle replacement; binding to the current authenticated/authorized context; binding to the current predecessor lineage; and a successor authorization scope that is no broader than the currently authorized scope.

The shared decision order is deterministic and fail-closed: current-credential control, successor-key control, session authentication, session authorization, context binding, predecessor binding, then privilege expansion. Only `AUTHORIZED_REPLACEMENT` may be mapped to the normalized lifecycle authority input. All other decisions map to `REJECT_AUTHORITY` and cannot produce a replacement commit plan.

The lower-level `lineage_replace_evaluate` / `evaluate_lineage_replace` predicates remain normalized decision-corpus surfaces. Lifecycle request handlers MUST derive authority through `lineage_replace_classify_authorization` / `classify_lineage_replace_authorization` and enter through the authorized evaluation entrypoint. Normal AUTH remains NO-LEARNING and MUST NOT invoke this lifecycle path as implicit trust mutation.

The AZ-01..AZ-10 canonical corpus in `rust/test-vectors/replay/lineage-replace-authorization-v1.txt` is consumed by Rust and C. It includes negatives for missing current-credential proof, missing successor-key proof, unauthenticated or unauthorized session, context mismatch, stale/wrong predecessor binding, and privilege expansion.

This contract does not define proof encodings, packet grammar, attempt identifiers, retransmission, durable storage, cryptographic verification algorithms, or trust-store mutation. Those remain separately gated by the RFC evolution plan and lifecycle qualification evidence.
