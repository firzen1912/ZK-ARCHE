# AUTH-v3 Annotated Reference Trace

Status: **draft normative trace of existing non-advertised reference behavior**. This document does not promote AUTH v3, `iot-core`, or any draft registry allocation to production-selectable status. It annotates the existing deterministic reference primitive vector so an independent implementation can identify the security-relevant inputs, intermediate values, state transitions, and fail-closed mutation points without treating source code as the specification.

The authoritative machine-readable fixture for the values below is:

`rust/test-vectors/auth-v3/reference-primitives-v1.json`

The protocol semantics and promotion boundary remain owned by `spec/zk-arche-protocol.md`, `spec/auth-v3-context-encoding.md`, `spec/auth-v3-kc-transcript-encoding.md`, `spec/critical-extension-set-encoding.md`, and `spec/registries.md`.

## 1. Scope and evidence boundary

This trace covers only the draft AUTH-v3 reference primitive path already reproduced by Rust and C fixtures. It is intended to make the existing byte/decision contract reviewable in an RFC-9529-style annotated form.

This trace does **not** establish:

- production advertisement or selection of protocol version `0x03`;
- production promotion of `iot-core` or any other draft profile;
- a complete authorization-context schema;
- a complete negotiation state machine;
- replay persistence across restart/rollback;
- constrained-target resource evidence;
- independent cryptographic review;
- independent third-party interoperability;
- deployment qualification or IETF/RFC status.

The reference fixture's `selected_capabilities = 0x0000000000000005` is a deterministic draft input, not a production-valid AUTH-v3 selected-capability set. `spec/registries.md` requires regeneration before promotion if final selection rules differ.

## 2. Reference authenticated context

The authenticated AUTH-v3 security context is the canonical 125-byte block defined by `spec/zk-arche-protocol.md` and `spec/auth-v3-context-encoding.md`.

| Field | Reference value | Security meaning |
|---|---|---|
| `protocol_version` | `0x03` | selects draft v3 semantics; MUST NOT be reinterpreted as v2 |
| `suite_id` | `0x0001` | binds the selected cryptographic suite |
| `profile_id` | `0x0001` | binds the draft `iot-core` profile identifier |
| `selected_capabilities` | `0x0000000000000005` | deterministic reference input only; binds selected security capabilities |
| `session_id` | `00112233445566778899aabbccddeeff` | binds all flights to one AUTH instance |
| `authz_context_hash` | `11` repeated 32 bytes | placeholder deterministic authorization-context digest |
| `critical_extensions_hash` | `22` repeated 32 bytes | placeholder deterministic critical-extension-set digest |
| `channel_binding_hash` | `33` repeated 32 bytes | placeholder deterministic channel-binding digest |

A conforming implementation of this reference path MUST derive a different transcript hash if any byte of this authenticated context changes. Successful cryptographic verification under a different authorization, extension, channel, profile, suite, capability, or session context is not equivalent AUTH.

## 3. AUTH cryptographic inputs

The reference fixture supplies canonical Ristretto255/scalar encodings for `pid`, client proof values `a_c`/`s_c`, client nonce and ephemeral key, server public key, server proof values `a_s`/`s_s`, server nonce and ephemeral key, plus the established 32-byte `session_key`.

These values are test inputs, not random-generation guidance. Production nonce, ephemeral-key, proof, and session-key generation remains subject to `spec/implementation-requirements.md` and the applicable cryptographic review gates.

## 4. Transcript construction

The transcript domain is exactly:

`zk-arche/kc/v3`

Each labeled field in the reference transcript uses:

`u8 label_len || label || u32_le value_len || value`

Integers are little-endian. Ristretto255 points use 32-byte compressed encodings and scalars use 32-byte canonical encodings.

For the canonical fixture, the resulting transcript length is:

`726` bytes

and:

`TH_v3 = e2b85befd4f3f58b5e880673ce1b27e81de875bf4443d7af6971d811e10439d2`

The transcript MUST commit to the authenticated security context and the AUTH cryptographic inputs in the exact ordering defined by the normative transcript specification. Implementations MUST NOT substitute raw HELLO advertisements, transport addresses, local runtime-profile presets, or unregistered metadata for committed security-context fields.

## 5. Directional key derivation

Using the established `session_key`, `TH_v3`, the selected HKDF-SHA-256 construction, and the exact versioned labels, the reference outputs are:

| Purpose | Reference output |
|---|---|
| `k_s2c_v3` | `622ae55fab76559100ab38020fe72070b2a29bc95b06ef49686d26be280f3cc9` |
| `k_c2s_v3` | `b8f77e03dd68595d4e2ed49f83112373f597955efe32a0ad855d1a1a2ff1cab8` |
| `k_complete_v3` | `ce41b5d3ceb6c54d88292b0fd8299a82bc64aff679a79b7886495d44c0e990ad` |

The three outputs are purpose-separated. A conforming implementation MUST NOT substitute one directional/completion key for another or reuse v2 key-confirmation labels.

## 6. Flight-by-flight annotated completion

### 6.1 AUTH_1 / context establishment

Before the responder treats the AUTH instance as eligible for v3 processing, the selected version/suite/profile/capabilities and canonical authorization, critical-extension, channel-binding, and session contexts must be the same context later committed by `TH_v3`.

Receipt or parsing of draft v3 bytes alone does not authorize v3 processing. Production v3 remains disabled until its promotion gate is satisfied.

### 6.2 AUTH_2 / server Finished

The server Finished reference value is:

`tag_s = 453edbad5976c5c08e720e6bffb0e111eb117501f26afb0f55b54cc719e38096`

The client MUST verify `tag_s` under `k_s2c_v3` and the same `TH_v3` before emitting AUTH_3. Failure terminates this AUTH instance; it MUST NOT be repaired by falling back to v2 semantics, changing the selected profile, or changing committed context.

### 6.3 AUTH_3 / client Finished

The client Finished reference value is:

`tag_c = 6c748a2e93e297095be36ba757c66bf886ec11fb9833ffdab7cbdcf5aa75d819`

The server MUST verify `tag_c` under `k_c2s_v3` and the same `TH_v3` before entering its completed state or generating the authenticated v3 completion response.

Possession of a session key before this verification is insufficient to claim mutually completed AUTH.

### 6.4 AUTH_ACK-v3 / authenticated server completion

The reference completion hash is:

`0291e9e9e586c2d49e35c849ee8147dc263f6f51c0553c571806dfd38ced91ea`

and the authenticated completion tag is:

`tag_ack = 5766a1ebf0af40b4da5f8226ec104b78fa47e16c60ad2f46b9a0a95822d39954`

The client reaches `MUTUALLY_COMPLETE` only after verifying this 32-byte v3 completion under `k_complete_v3` for the same transcript and client Finished. The one-byte v2 ACK is not a valid v3 completion.

## 7. Mandatory mutation/falsification matrix

An implementation claiming conformance to this reference trace MUST fail closed when a test independently mutates any of the following while retaining the other reference inputs:

| Mutation | Required consequence |
|---|---|
| `protocol_version` changed or v2 semantics used after v3 selection | reject; no cross-version fallback |
| `suite_id` changed | transcript/context mismatch; reject |
| `profile_id` changed | transcript/context mismatch; reject |
| `selected_capabilities` changed | transcript/context mismatch; reject |
| `session_id` changed | cross-session mismatch; reject |
| `authz_context_hash` changed | authorization-context mismatch; reject |
| `critical_extensions_hash` changed | critical-context mismatch; reject |
| `channel_binding_hash` changed | channel-context mismatch; reject |
| v2 transcript/KDF label substituted | domain mismatch; reject |
| `tag_s` changed | client MUST NOT emit AUTH_3 |
| `tag_c` changed | server MUST NOT complete or emit valid AUTH_ACK-v3 |
| v2 one-byte ACK substituted | client MUST NOT report mutual completion |
| valid `tag_ack` replayed into another session/context | reject |
| unknown/non-canonicalizable critical context | reject before AUTH completion |

These are security-semantic requirements, not an assertion that every row has independent executed evidence at this exact repository head. The conformance package must retain executable Rust/C positive and negative evidence before promotion claims advance.

## 8. State/claim boundary

The reference trace corresponds to the semantic progression:

`START → AUTH1_* → KEYS_DERIVED → SERVER_FINISHED_VERIFIED → CLIENT_FINISHED_SENT → SERVER_CLIENT_FINISHED_VERIFIED → MUTUALLY_COMPLETE`

No earlier state may be exposed as mutually completed AUTH. Normal AUTH remains NO-LEARNING throughout this trace: successful proof/Finished/completion verification MUST NOT create or expand trust, delegation, enrollment, or authorization lineage.

Authentication completion also does not by itself grant arbitrary authorization. The authorization decision remains scoped to the authenticated authorization context and local lifecycle state.

## 9. Promotion evidence still required

This annotated trace reduces TD-004 documentation debt for the existing reference primitive, but it does not close TD-004. Production promotion still requires the complete normative negotiation and profile contract, complete state/error behavior, final authorization-context semantics, replay/restart/rekey/resumption lifecycle, registry promotion/change control, canonical positive/negative vectors, executable Rust/C conformance, constrained-target evidence where claimed, formal-analysis scope/result mapping, and independent review required by the roadmap.
