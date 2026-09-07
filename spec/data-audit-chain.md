# ZK-ARCHE local DATA audit chain v1

This document defines a wire-neutral local evidence primitive for `zk235`. It does not authorize DATA release and does not replace current authentication, device-local release authority, revocation, lineage, policy, or channel-binding checks.

## Entry commitment

For each locally retained audit event, compute:

```text
SHA-256(
    "ZKARCHE-DATA-AUDIT-v1" ||
    previous_head[32] ||
    sequence_u64_be ||
    event_kind_u8 ||
    release_context_hash[32] ||
    authorization_generation_u64_be ||
    revocation_epoch_u64_be ||
    policy_epoch_u64_be
)
```

`sequence` starts at 1. A zero sequence is invalid. Implementations MUST fail closed rather than wrap sequence state; the current bounded API rejects append when `next_sequence == UINT64_MAX`.

`event_kind` is presently:

- `1` — release granted;
- `2` — release denied.

`release_context_hash` is a commitment to the canonical release context. Protected plaintext MUST NOT be inserted into this audit primitive.

## Continuity and recovery boundary

The local state is `(head[32], next_sequence)`, with genesis `head = 32 zero bytes` and `next_sequence = 1`. An implementation verifies a retained transition by recomputing the next head from the prior head, exact sequence, and exact event fields.

A mismatch proves only that the presented transition is inconsistent with the supplied predecessor state. Detecting storage rollback across restart requires an independently protected persistence/rollback anchor and remains a separate deployment-evidence requirement.

The audit chain MUST NOT be used as a source of authorization or trust mutation. Missing, corrupt, rolled-back, or unavailable audit state may affect evidence retention or a deployment's local fail-closed policy, but it MUST NOT silently manufacture release authority.

## Deterministic fixture

With zero previous head, sequence `1`, event kind `1`, release-context bytes `00..1f`, authorization generation `7`, revocation epoch `11`, and policy epoch `13`, the resulting head is:

```text
9df36e00b55684fcd1af8297cb832f616b62cde5181c0ce9f2cd8e997be58098
```

Rust and C tests consume this same semantic fixture. Physical persistence, power-loss recovery, transparency publication, and external anchoring are not established by this vector.
