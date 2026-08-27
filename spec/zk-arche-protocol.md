# ZK-ARCHE Protocol Specification

Status: **draft normative work**. This document is not an IETF RFC and does not claim Internet Standard status. Existing v2 wire behavior remains the deployed compatibility baseline. AUTH v3 in this document is a non-advertised candidate derived from ADR 0001, deterministic Rust/C reference primitives, and retained formal evidence. It MUST NOT be selected in production until the promotion gates below are satisfied.

Normative keyword policy: the key words **MUST**, **MUST NOT**, **REQUIRED**, **SHOULD**, **SHOULD NOT**, and **MAY** are used only where the behavior is sufficiently precise to implement and test. This follows the repository's BCP 14 / RFC-class documentation discipline.

## 1. Scope

ZK-ARCHE defines privacy-preserving peer authentication, scoped authorization context binding, trust/lifecycle operations, and secure-association establishment for heterogeneous IoT and edge peers.

The Common Contract requires that a conformant constrained peer locally verify the mandatory authentication decision. A gateway, CA, cloud service, central registry lookup, DNS service, or other always-online infrastructure MUST NOT become a hidden prerequisite for core AUTH between already-authorized peers that possess sufficiently fresh local trust state.

Authentication, authorization, and trust mutation are distinct operations. Normal AUTH is **NO-LEARNING**: successful AUTH MUST NOT create, expand, or replace trusted state.

## 2. Versioning and compatibility

### 2.1 AUTH v2

Protocol version `0x02` retains its existing packet bytes, transcript domains, key-confirmation behavior, and one-byte ACK semantics. Implementations MUST NOT reinterpret a v2 packet using v3 transcript or completion rules.

Known evidence limitation: retained formal analysis shows that v2 does not support the stronger same-session and authenticated-mutual-completion claims required for AUTH v3. Those limitations are evidence boundaries, not permission to silently change v2.

### 2.2 Draft AUTH v3

Protocol version `0x03` is reserved in this specification for the candidate AUTH-v3 semantics below.

Until explicit registry/ADR promotion:

- implementations MAY implement v3 reference primitives and conformance tests;
- implementations MUST NOT advertise or negotiate v3 in production;
- implementations MUST NOT accept a v3 AUTH flow merely because draft primitive code exists;
- v2 and v3 transcripts, key labels, and completion semantics MUST remain domain-separated.

The existing AUTH packet-type values MAY remain unchanged because the packet header version disambiguates v2 and v3 semantics:

| Message | Existing type |
|---|---:|
| `AUTH_1` | `0x21` |
| `AUTH_2` | `0x22` |
| `AUTH_3` | `0x23` |
| `AUTH_ACK` | `0x24` |

No packet-type value is reassigned by this draft.

## 3. AUTH-v3 authenticated security context

AUTH v3 authenticates one canonical security-context block. Every field below is security-significant and MUST be serialized in this exact order:

```text
protocol_version          u8          (= 0x03)
suite_id                  u16 LE
profile_id                u16 LE
selected_capabilities     u64 LE
session_id                16 bytes
authz_context_hash        32 bytes
critical_extensions_hash  32 bytes
channel_binding_hash      32 bytes
```

The block length is therefore 125 bytes.

### 3.1 Field semantics

`protocol_version` is the negotiated protocol version and MUST equal `0x03` for AUTH v3.

`suite_id` identifies the selected cryptographic suite. A peer MUST fail closed if the suite used by AUTH differs from the selected suite bound into this block.

`profile_id` identifies the selected protocol/security profile. Resource/runtime presets are not profile identifiers and MUST NOT be substituted for this value.

`selected_capabilities` is the mutually selected security-relevant capability set. It is not either peer's raw advertisement. A capability that changes AUTH security semantics MUST be included in this selected value or represented by a critical extension.

`session_id` is the existing 16-byte AUTH-instance identifier. The initiator generates it once for the AUTH instance, all AUTH flights for that instance use the same value, and the value is authenticated by the v3 transcript.

`authz_context_hash` is SHA-256 over the canonical authorization context applicable to this AUTH instance. The underlying authorization context is expected to cover the holder/session binding, role or policy scope, audience/deployment/domain where applicable, validity/epoch information where applicable, and other authorization semantics promoted by the selected profile. Exact sub-encoding remains a TD-004 registry/spec item; until defined, production v3 MUST remain disabled.

`critical_extensions_hash` is SHA-256 over the canonical ordered encoding of all negotiated critical extensions and their security-relevant values. When no critical extension is active, the selected profile MUST define one canonical empty encoding whose SHA-256 value is used.

`channel_binding_hash` is SHA-256 over the normative channel-binding context. A profile with no channel binding MUST define one canonical empty channel-binding encoding. An implementation MUST NOT use an ad-hoc zero value unless the profile specification defines it as the canonical empty representation.

## 4. KC-TRANSCRIPT-v3

The v3 key-confirmation transcript uses a new domain and MUST NOT reuse the v2 transcript domain.

Conceptually:

```text
KC-TRANSCRIPT-v3 =
    domain("zk-arche/kc/v3")
    || security_context_v3
    || pid
    || a_c
    || s_c
    || nonce_c
    || eph_c
    || server_pub
    || a_s
    || s_s
    || nonce_s
    || eph_s
```

The concrete reference serialization is defined by the interoperable Rust/C draft primitives and canonical vector at:

```text
rust/test-vectors/auth-v3/reference-primitives-v1.json
```

Before v3 promotion, the exact field labels, length-prefix rules, point/scalar encodings, and canonical empty-context encodings MUST be fully reproduced in specification text and negative vectors. Source code alone is not normative.

Define:

```text
TH_v3 = SHA-256(KC-TRANSCRIPT-v3)
```

Any change to a security-context field or AUTH cryptographic field listed above MUST change `TH_v3` except where two byte-identical canonical representations are intentionally the same value.

Outer transport/record sequence metadata is not automatically part of `KC-TRANSCRIPT-v3`. Sequence and retransmission behavior MUST instead be enforced by the state machine unless later versioned evidence promotes a field into the authenticated context.

## 5. AUTH-v3 key schedule and directional confirmation

Given the established AUTH session key and `TH_v3`, v3 derives three independent 32-byte purpose keys using the repository's selected HKDF-SHA-256 construction and the exact versioned labels below:

```text
k_s2c_v3       = HKDF-Expand(..., "kc s2c v3", 32)
k_c2s_v3       = HKDF-Expand(..., "kc c2s v3", 32)
k_complete_v3  = HKDF-Expand(..., "kc complete s2c v3", 32)
```

The v2 labels MUST NOT be reused for these purposes.

Directional Finished tags are:

```text
tag_s = HMAC(k_s2c_v3, "server finished v3" || TH_v3)
tag_c = HMAC(k_c2s_v3, "client finished v3" || TH_v3)
```

`AUTH_2` carries `tag_s`. The client MUST verify `tag_s` before emitting `AUTH_3`.

`AUTH_3` carries `tag_c`. The server MUST verify `tag_c` before entering server-complete state or producing `AUTH_ACK-v3`.

Comparisons of Finished/completion authenticators MUST use a constant-time equality function in production implementations.

## 6. Authenticated server completion

The v2 one-byte public ACK is invalid for v3.

After successful verification of the matching `AUTH_3.tag_c`, define:

```text
completion_hash = SHA-256(
    "zk-arche/auth-complete/v3"
    || TH_v3
    || tag_c
)

tag_ack = HMAC(
    k_complete_v3,
    "server complete v3"
    || completion_hash
)
```

`AUTH_ACK-v3` carries exactly the 32-byte `tag_ack`.

The server MUST NOT create `tag_ack` before successful verification of the matching `AUTH_3` for the same authenticated `session_id` and security context.

The client MUST NOT report mutual AUTH completion until it verifies `tag_ack` under the same v3 context. A public v2 ACK, all-zero value, random value, completion from another session, completion from another selected profile/suite/context, or completion generated before server verification of `AUTH_3` MUST fail.

## 7. AUTH-v3 state machine

The minimum semantic states are:

```text
START
  -> AUTH1_SENT / AUTH1_RECEIVED
  -> KEYS_DERIVED
  -> SERVER_FINISHED_VERIFIED
  -> CLIENT_FINISHED_SENT
  -> SERVER_CLIENT_FINISHED_VERIFIED
  -> MUTUALLY_COMPLETE
```

Client transition to `MUTUALLY_COMPLETE` requires successful `tag_ack` verification.

Server transition to completed state requires successful `tag_c` verification and generation of the authenticated completion response.

Possession of a derived session key before `MUTUALLY_COMPLETE` does not by itself establish mutually completed AUTH. Application-facing APIs that claim mutual completion MUST NOT expose that stronger state early.

Wrong message type, wrong legal sequence, cross-session message, context mismatch, unsupported critical context, invalid Finished tag, invalid completion tag, and incompatible negotiated security context MUST fail closed.

## 8. Replay and retransmission boundary

AUTH replay handling remains a separate lifecycle obligation from v3 context binding.

Current Rust and C implementations use bounded volatile replay memory. The formal draft-v3 model uses a persistent/unbounded replay table, which is a stronger abstraction than runtime behavior.

Therefore this specification does not yet define a complete Common Contract replay lifetime/epoch policy. Production v3 promotion requires a normative decision for:

- minimum or profile-specific replay retention capacity;
- deterministic eviction behavior;
- process restart/state-loss behavior;
- rollback behavior;
- stale replay-state behavior;
- the authenticated event, if any, that legally starts a fresh replay epoch.

A successful draft-v3 formal replay correspondence MUST NOT be interpreted as proof of replay continuity across runtime eviction, restart, or rollback.

## 9. NO-LEARNING and authorization boundary

Normal AUTH v3 operates relative to pre-existing locally trusted/verifiable state.

Successful proof verification MUST NOT create a trusted record, enroll an unknown peer, expand role scope, create delegation, or mutate authorization lineage as a side effect.

A valid possession or role-membership proof does not alone establish authorization. The peer MUST evaluate the bound authorization context under local policy before granting operations represented by that context.

Trust mutation belongs to explicit ENROLL, commissioner/grant, rekey/re-registration, revocation, or other versioned lifecycle operations.

## 10. Downgrade and cross-version behavior

If v3 is eventually negotiated, a peer MUST NOT silently execute v2 AUTH semantics for that session.

Before v3 promotion, negative conformance coverage MUST include at least:

- negotiated/selected v3 followed by v2 AUTH semantics;
- changed suite after transcript establishment;
- changed profile after transcript establishment;
- changed selected capabilities;
- changed authorization-context hash;
- changed critical-extension hash;
- changed channel-binding hash;
- changed `session_id`;
- v2 transcript/KDF domain used in v3;
- v2 one-byte ACK presented in v3;
- valid v3 completion replayed across another session/context;
- unknown or non-canonicalizable critical context.

A peer MUST fail closed when no mutually acceptable mandatory security floor exists.

## 11. Deterministic interoperability evidence

Current non-advertised draft evidence includes:

- Rust reference primitives: `rust/crates/proto/src/auth_v3.rs`;
- C reference primitives: `c/include/auth/auth_v3.h` and `c/src/proto/auth_v3.c`;
- canonical vector: `rust/test-vectors/auth-v3/reference-primitives-v1.json`;
- C independent reproduction test: `c/tests/test_auth_v3_reference.c`;
- synchronized ProVerif draft models: `rust/models/proverif/zk_arche_auth_v3_draft.pv` and `c/models/proverif/zk_arche_auth_v3_draft.pv`;
- retained formal run: `docs/assurance/formal-runs/2026-08-27-d4994da-proverif-auth-v3.md`.

The retained formal run reports scoped TRUE results for modeled draft-v3 agreement, mutual completion, accepted-message replay-record ordering, bound security context, and pre-existing-trust correspondences. Those results are scoped to the exact symbolic model and assumptions; they do not establish production interoperability, constrained-target performance, replay persistence, custom-proof soundness, constant-time behavior, or field readiness.

## 12. AUTH-v3 promotion gate

AUTH v3 MUST remain non-advertised and non-selectable until all of the following exist at one reviewed implementation state:

1. ADR 0001 is accepted or superseded by an explicit reviewed decision.
2. `spec/registries.md` contains versioned protocol/suite/profile/capability/critical-extension allocation rules sufficient to interpret the authenticated context.
3. Canonical encodings for authorization context, critical extensions, and channel binding are normative and have positive/negative vectors.
4. Rust and C production state machines implement v3 side-by-side without changing v2 bytes.
5. Rust/C production interoperability is retained for full `AUTH_1/AUTH_2/AUTH_3/AUTH_ACK-v3` flows.
6. Active-attacker negative tests cover session/context mutation, forged completion, cross-session completion, downgrade, and unsupported critical context.
7. Formal analysis is rerun against the production-corresponding v3 model and retained with exact tool/model/commit identity.
8. Replay lifetime/epoch semantics are defined narrowly enough that Common Contract replay claims match bounded runtime behavior.
9. Constrained resource measurements exist for any constrained profile that claims v3 support.
10. Security/privacy considerations and known limitations are updated to match the implemented behavior.

Meeting these conditions still does not imply IETF standardization, external cryptographic review, field readiness, or certification.

## 13. Remaining specification structure

The following sections remain required for the complete ZK-ARCHE RFC-class package:

1. Introduction and applicability
2. Full terminology and trust roles
3. Complete threat model
4. Suite definitions and mandatory-to-implement floor
5. Full wire grammar and canonical encoding
6. HELLO negotiation and registry behavior
7. SETUP / enrollment
8. AUTH v2 compatibility and AUTH v3 normative promotion
9. Authorization-context semantics
10. Late enrollment and commissioner grants
11. Rekey / re-registration / lineage
12. Revocation convergence
13. Session/key lifecycle and usage limits
14. Session resumption
15. Transport/channel bindings
16. Native datagram behavior
17. Error handling and observable failure classes
18. Complete state machines
19. Test vectors and annotated traces
20. Security considerations
21. Privacy considerations
22. Constrained implementation requirements
23. Registry/extension/change-control policy
24. Conformance and claim language
