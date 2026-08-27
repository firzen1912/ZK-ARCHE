# `iot-core` AUTH-v3 Authorization Context v1

Status: **draft normative work**. This document defines the first concrete authorization-context schema for draft profile `iot-core` (`profile_id = 0x0001`). It does not make AUTH v3 selectable, stabilize `iot-core`, or replace local authorization policy.

The schema uses the canonical `ZKCTX` v1 `AUTHORIZATION` envelope from `spec/auth-v3-context-encoding.md`. Its purpose is to make the authorization bytes and validation rules independently implementable across constrained and edge peers while preserving ZK-ARCHE's separation of authentication, authorization, and trust mutation.

## 1. Design constraints

`iot-core` authorization v1 follows these rules:

- normal AUTH remains NO-LEARNING;
- the context is evaluated against pre-existing locally trusted/verifiable state;
- the context MUST bind to the authenticated holder without exposing transport identity;
- the context MUST NOT reveal the holder's hidden role merely to authorize a secure association;
- the role field therefore identifies the **role policy / allowed-set definition**, not the secret role member proved by the CDS-OR proof;
- a trusted wall clock is not mandatory for this constrained baseline; lifecycle freshness is represented with monotonic authorization/policy/revocation generations;
- every field is mandatory in v1 and unknown authorization field IDs are forbidden;
- production policy validity is stricter than canonical encoding validity.

## 2. Exact field schema

The canonical authorization context contains exactly seven entries in ascending ID order:

| ID | Name | Length | Encoding | v1 semantics |
|---:|---|---:|---|---|
| `0x0001` | `holder_binding` | 32 | opaque bytes | SHA-256 holder binding for the credential/key commitment authenticated by this AUTH flow |
| `0x0002` | `audience_id` | 32 | opaque bytes | deployment/audience identifier provisioned by local policy |
| `0x0003` | `role_policy_id` | 8 | `u64 LE` | identifier/version of the role policy or allowed-role set whose proof is accepted; **not the hidden role value** |
| `0x0004` | `scope_bits` | 8 | `u64 LE` | authorized operation scope; v1 requires exactly `0x0000000000000001` (`SECURE_ASSOCIATION`) |
| `0x0005` | `authorization_generation` | 8 | `u64 LE` | authorization-lineage generation; zero is invalid |
| `0x0006` | `policy_epoch` | 8 | `u64 LE` | policy generation/epoch; zero is invalid |
| `0x0007` | `revocation_epoch` | 8 | `u64 LE` | minimum revocation-view epoch required by the authorization; zero is invalid |

The total canonical encoding length is exactly **148 bytes**:

```text
9-byte ZKCTX envelope
+ 7 × 5-byte entry headers
+ 104 value bytes
= 148 bytes
```

No additional authorization entry is legal in this schema version. Adding or changing a field requires a versioned profile/schema change and new Rust/C vectors; a stable profile ID MUST NOT be silently reinterpreted.

## 3. Holder binding

`holder_binding` binds authorization to the credential/key commitment authenticated by AUTH rather than to an IP address, socket, BLE address, CAN identifier, device-model string, or other transport metadata.

For suite `0x0001`, the intended production construction is:

```text
holder_binding = SHA-256(
    "zk-arche/authz-holder/v1"
    || canonical_authenticated_holder_commitment
)
```

`canonical_authenticated_holder_commitment` is the suite-defined canonical 32-byte commitment/public-key representation associated with the pre-existing trusted record used for the AUTH decision.

The all-zero 32-byte value is invalid. A peer MUST compare the holder binding against the holder/credential relationship accepted by the local trust decision; possession of a syntactically valid context is not authorization.

The AUTH-v3 `session_id` remains separately authenticated in `security_context_v3`. It is intentionally not duplicated in this authorization context. Mutual use of the same `authz_context_hash` and authenticated `session_id` binds the authorization semantics to that AUTH instance without making a session identifier part of long-lived authorization lineage.

## 4. Audience

`audience_id` is a 32-byte opaque identifier for the deployment, resource group, administrative domain, or other audience under which the authorization is valid.

The value is provisioned or derived by the deployment's authorization policy. This specification does not impose DNS, PKI, Internet, cloud, or central-registry semantics on it.

The all-zero audience is invalid. A peer MUST reject an authorization whose audience does not match a locally accepted audience for the requested operation.

## 5. Role-policy privacy boundary

`role_policy_id` identifies the role-policy definition / allowed-role set used to interpret the role-membership proof. It MUST NOT contain the holder's hidden role value.

A non-zero value is required. A peer MUST verify that:

1. the role-membership proof is valid for the role policy/set identified by `role_policy_id`; and
2. that policy is authorized for the bound holder, audience, scope, and lifecycle state.

This preserves the distinction between:

```text
proof: "the hidden role is a member of the allowed set"

and

authorization context: "this AUTH instance is evaluated under role-policy set X"
```

A context that encodes the secret role member directly is not conformant to this `iot-core` schema.

## 6. Scope

`scope_bits` is a 64-bit little-endian bitmap.

Draft `iot-core` authorization v1 defines only:

| Bit | Name | Meaning |
|---:|---|---|
| 0 | `SECURE_ASSOCIATION` | peer may establish/use the authenticated secure association subject to local policy |

All other bits are reserved and MUST be zero. Therefore the only valid v1 value is `1`.

Trust mutation, enrollment, commissioner authority, rekey authority, DATA release, and other higher-level privileges are **not** granted by this scope. They require their own explicit lifecycle/profile semantics.

## 7. Authorization generation and epochs

All three lifecycle integers are one-based; zero is invalid.

### 7.1 `authorization_generation`

This value identifies the authorization lineage generation for the holder/audience/policy relationship. The constrained v1 rule is fail closed: the received generation MUST equal the generation accepted by current local trusted authorization state unless a future versioned replacement rule explicitly defines successor handling.

### 7.2 `policy_epoch`

This value identifies the policy generation used to interpret `role_policy_id`, `scope_bits`, and other local authorization conditions. The peer MUST evaluate against a policy state compatible with this epoch. Silent fallback to an older policy interpretation is forbidden.

### 7.3 `revocation_epoch`

This value states the minimum issuer/authority revocation-view epoch required by the authorization. The local revocation view MUST be at least this epoch and MUST also satisfy the profile's separately defined freshness policy. A higher local epoch does not automatically authorize the peer; revocation/lineage checks still apply.

This field does not solve restart, rollback, or stale-view policy by itself. Those remain part of the broader replay/revocation lifecycle contract.

## 8. Canonical validation rules

A conformant `iot-core` v1 encoder/validator MUST reject:

- entry count other than seven;
- missing, duplicate, reordered, or unknown IDs;
- non-zero ZKCTX reserved entry flags;
- holder binding length other than 32 or an all-zero holder binding;
- audience length other than 32 or an all-zero audience;
- `role_policy_id = 0`;
- any `scope_bits` value other than `1`;
- zero authorization generation;
- zero policy epoch;
- zero revocation epoch;
- trailing bytes or non-canonical ZKCTX encoding.

A canonical context can still be unauthorized. Local authorization evaluation MUST additionally verify holder, audience, role-policy/proof, lineage, revocation, and any deployment restrictions.

## 9. Deterministic vector

The shared draft vector is:

```text
rust/test-vectors/auth-v3/iot-core-authorization-v1.txt
```

Its canonical encoding is 148 bytes and its SHA-256 digest is the draft `authz_context_hash` value for that fixture.

Both Rust and C MUST reproduce the same encoding/hash and reject semantic zero/scope violations before this schema can be considered interoperable.

## 10. Common Contract and promotion boundary

This schema is intentionally small and fixed so that a constrained MCU can validate it without a heap, certificate-chain parser, policy language runtime, or network authority lookup.

It does not complete `iot-core` promotion. Remaining blockers include at least:

- immutable profile-definition manifest/conformance fingerprint;
- production-valid selected-capability rules;
- replay lifetime/epoch semantics;
- critical-extension value schemas where used;
- channel-binding policy;
- production AUTH-v3 state machines and interop;
- constrained target measurements;
- security/privacy considerations and retained negative evidence.

The draft profile remains non-advertised and non-selectable.