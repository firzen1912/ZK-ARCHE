# AUTH-v3 Formal Traceability Record

Status: **scoped assurance evidence**. This record reconciles the current synchronized draft AUTH-v3 ProVerif model with the repository surfaces that implement, specify, or test the modeled security context. It advances TD-003 traceability only. It does not make AUTH v3 selectable, prove the custom role-membership proof, establish parser equivalence, close TD-003, or establish RFC-class status.

## 1. Exact evidence identity

Current repository anchor for this traceability record:

```text
repository_commit = 16de6da915a39e8d74dbab665ce1d98385681e8d
branch            = dev
ci_run            = #58 / 33209827503
ci_result         = success
formal_job        = 98980037950 / success
```

Current retained formal result:

```text
docs/assurance/formal-runs/2026-08-28-16de6da-proverif-auth-v3.md
formal_run_commit = 16de6da915a39e8d74dbab665ce1d98385681e8d
tool              = ProVerif 2.05
model_blob         = 0fefe938988feb49086ecf579f2b0ae5df8d16eb
retained_queries   = 7 correspondence queries, all accepted by the fail-closed CI gate
artifact           = formal-proverif-evidence / 9701194272
artifact_digest    = sha256:46f5ae3bc4902b5c4b407ba1c96bf0933ca5f63e41825fd490215404466f95ec
```

Previous retained AUTH-v3 evidence remains historical provenance:

```text
docs/assurance/formal-runs/2026-08-27-d4994da-proverif-auth-v3.md
formal_run_commit = d4994daaa3dd771cfa87b42485d9032302af82f0
retained_results  = 6 TRUE query results
```

Current synchronized model copies are:

```text
rust/models/proverif/zk_arche_auth_v3_draft.pv
c/models/proverif/zk_arche_auth_v3_draft.pv
Git blob SHA = 0fefe938988feb49086ecf579f2b0ae5df8d16eb
```

The Rust and C paths are synchronized copies, not independent formal implementations. CI #58 explicitly checked synchronization before running ProVerif. A future model edit requires a new exact-model retained run before this result may be cited for the edited model text.

## 2. Attacker and abstraction scope

The draft-v3 model is interpreted under the active-network attacker class corresponding to A0 in `formal-model-contract.md`: the attacker can read, drop, delay, replay, reorder, inject, modify, interleave, and initiate concurrent protocol runs but does not initially know uncompromised long-term secrets.

Important abstractions remain:

- `schnorr_proof` is idealized; computational Schnorr security is not established;
- `role_proof` is idealized; TD-001 independent cryptographic review remains required;
- concrete canonical hashing and byte encoding are abstracted as symbolic bitstrings;
- the replay table is persistent and unbounded;
- restart, rollback, bounded eviction, fresh replay epochs, resumption, revocation convergence, and transport observability are not part of this AUTH-v3 model;
- constant-time behavior, RNG/DRBG quality, key storage, zeroization, memory safety, side channels, and target resource behavior are outside the symbolic model.

Every successful query below is therefore a scoped model result, not a whole-protocol or deployment claim.

## 3. Property-to-query-to-repository map

| Property | ProVerif correspondence | Normative/design owner | Concrete anchors | Evidence state |
|---|---|---|---|---|
| FM-02 client-to-server agreement | injective `ServerCompleteV3 ==> ClientAuth3SentV3` | ADR 0001 + draft AUTH-v3 state/completion semantics | Rust/C `auth_v3` reference primitives + deterministic reference vectors | scoped formally analyzed |
| FM-03 mutual completion | injective `ClientCompleteV3 ==> ServerCompleteV3`; `ClientCompleteV3 ==> ServerAuth2SentV3` | ADR 0001 authenticated AUTH_ACK-v3 design | same Rust/C draft-v3 reference primitives/vectors | scoped formally analyzed |
| FM-04 accepted-message replay ordering | `ServerAuth1AcceptedV3 ==> ReplayRecordedV3` | `spec/iot-profiles.md` + `spec/replay-continuity.md` | bounded runtime replay + shared replay fixtures; separate replay-continuity model | scoped formally analyzed only under persistent/unbounded AUTH-v3 replay abstraction |
| FM-05 transcript/security-context integrity | completion/agreement queries carry identical `secctx`/`kcctx` | AUTH-v3 security context + `spec/auth-v3-context-encoding.md` + `spec/iot-core-authorization-context.md` | Rust/C `AuthV3Context`/`auth_v3_context_t`; `iot-core` raw receive/decode/hash paths | scoped formally analyzed for modeled fields; concrete byte validation remains separate executable evidence |
| FM-05 authorization-context admission handoff | `ServerCompleteV3 ==> AuthorizationContextAdmittedV3(client, server, session, secctx)` | this traceability boundary + concrete `iot-core` authorization spec | Rust/C strict canonical parser, 7-entry/148-byte `iot-core` validation, exact accepted-byte hash | **scoped formally analyzed at the symbolic admission boundary in CI #58**; parser equivalence not established |
| FM-09 NO-LEARNING AUTH | `ServerCompleteV3 ==> TrustedRecordPresent(client)` | roadmap NO-LEARNING invariant and future TRUST/AUTH normative text | production trust-store mutation remains separate from draft-v3 reference helpers | scoped formally analyzed relative to pre-existing modeled trust |

## 4. Security-context and authorization admission traceability

The modeled constructor is:

```text
security_context_v3(
  protocol_version,
  suite,
  profile,
  selected_capabilities,
  session_id,
  authz_context,
  critical_extensions,
  channel_binding)
```

Rust exposes the corresponding draft fields through `AuthV3Context`; C exposes the same semantic set through `auth_v3_context_t`.

The model treats authorization, critical-extension, and channel-binding subcontexts as already-formed symbolic inputs. It does not parse `ZKCTX`, count entries, validate IDs/lengths, detect duplicates, enforce reserved flags, evaluate profile limits, or compute SHA-256.

### 4.1 Concrete `iot-core` authorization boundary

The current `iot-core` authorization receive contract is independently executable in Rust and C:

```text
profile_id                 = 0x0001
context kind               = AUTHORIZATION
canonical byte length      = 148
entry count                = 7
entry IDs                  = 0x0001 .. 0x0007 in ascending order
entry value lengths        = 32,32,8,8,8,8,8
scope_bits                 = 1 only
lifecycle integers         = non-zero
unknown/additional fields  = forbidden in schema v1
```

Rust anchors:

```text
rust/crates/proto/src/auth_v3_context_parser.rs
rust/crates/proto/src/auth_v3_iot_core_authz.rs
  decode_iot_core_authorization_context_bytes(...)
  hash_iot_core_authorization_context_bytes(...)
```

C anchors:

```text
c/src/proto/auth_v3_context_parser.c
c/src/proto/auth_v3_iot_core_authz.c
  auth_v3_iot_core_authz_decode_bytes(...)
  auth_v3_iot_core_authz_hash_bytes(...)
```

The Rust path checks the exact 148-byte profile length before generic parsing, applies the seven-entry bound before entry-vector allocation, validates the exact schema and semantic zero/scope rules, then hashes the exact accepted bytes. The C path enforces the same profile length and schema using fixed caller-owned seven-entry storage and likewise hashes the exact accepted bytes.

Existing tests cover the canonical vector and profile-bound failures including 147/149-byte inputs, encoded entry count eight, wrong context kind, wrong schema ID, and the broader shared malformed `ZKCTX` corpus. CI #58 reran the complete Rust and C lanes successfully.

### 4.2 Explicit symbolic handoff added at `16de6da`

The current model now declares:

```text
AuthorizationContextAdmittedV3(client, server, session, security_context)
```

and emits it on the server path after construction of the modeled security context, with the explicit comment that concrete Rust/C parsing, profile validation, semantic checks, and exact-byte hashing have already succeeded.

CI #58 retained a seventh correspondence:

```text
ServerCompleteV3(client, server, session, secctx, kcctx)
    ==>
AuthorizationContextAdmittedV3(client, server, session, secctx)
```

The model/runtime boundary is therefore:

```text
attacker-controlled raw authorization bytes
        ↓
strict ZKCTX parse + structural/profile bounds       executable Rust/C evidence
        ↓
exact seven-field iot-core schema validation         executable Rust/C evidence
        ↓
semantic zero/scope validation                       executable Rust/C evidence
        ↓
SHA-256(exact accepted canonical bytes)              executable Rust/C evidence
        ↓
authz_context_hash / authz_core mapping              implementation traceability boundary
        ↓
AuthorizationContextAdmittedV3                       ProVerif abstraction boundary
        ↓
security_context_v3 / kc_context_v3 / completion     scoped formal analysis
```

The new correspondence proves only a property of the modeled transition system: modeled server completion is downstream of the modeled admission event for the same client/server/session/security context. It does **not** prove that the concrete parser is correct, that SHA-256 is collision-resistant, that the parser and symbolic model are equivalent, or that holder/audience/role/revocation policy authorizes the requested operation.

Critical-extension and channel-binding canonical schemas remain less concrete than the `iot-core` authorization boundary and must remain separate FM-05 assumptions until equivalent normative and executable evidence exists.

## 5. Replay traceability

The current repository has bounded runtime replay behavior plus a separate replay-continuity state machine/model. The AUTH-v3 model still uses persistent/unbounded symbolic replay memory.

The strongest allowed reconciliation remains:

> The draft-v3 model establishes accepted-message replay-record ordering under persistent/unbounded symbolic replay memory. Current Rust/C evidence separately establishes bounded FIFO decisions and fail-closed replay-continuity transitions, while the dedicated replay-continuity model provides scoped symbolic analysis of those states. These evidence lanes are complementary, not one complete implementation proof.

The authenticated fresh replay-epoch transition remains unresolved.

## 6. Retained counterexample lineage

The legacy AUTH-v2 formal evidence remains part of the assurance chain. Its material correspondence failures were not erased:

1. the outer `session_id` was not authenticated by the v2 key-confirmation context;
2. the public one-byte AUTH_ACK could be synthesized by an active attacker, so client completion did not establish server acceptance of AUTH_3.

Draft-v3 binds `session_id` and the wider security context into its KC context and authenticates server completion with a dedicated completion key/MAC. The current seven successful draft-v3 correspondences are regression evidence for those modeled fixes plus the explicit authorization-admission handoff; they are not proof that all AUTH-v3 properties are complete.

## 7. Explicitly open formal properties

The following remain open or not comprehensively analyzed by this record:

```text
FM-06 unknown-key-share resistance beyond currently bound identities/context
FM-07 explicit reflection resistance
FM-08 downgrade resistance and production negotiation
FM-10 full authorization-scope / authority-provenance semantics
FM-11 non-transitive trust
FM-12 bounded delegation
FM-13 revocation freshness/convergence
FM-14/FM-15 resumption authorization and reuse bounds
FM-16 P2P role symmetry
FM-17 infrastructure independence as an explicit modeled property
FM-18 credential/reference binding
FM-19..FM-21 privacy and observable-failure properties
FM-22 compromise/recovery boundaries
```

The new admission correspondence does not upgrade FM-10. The concrete `iot-core` validator establishes syntax, fixed schema, basic semantic invariants, and exact-byte hashing; it does not prove local authority/provenance, holder/audience/role-policy lineage, revocation freshness, or operation authorization. The August 28 research specifically keeps authority/provenance namespace semantics unresolved.

## 8. Evidence-state update

```text
DRAFT AUTH-v3 MODEL COPIES SYNCHRONIZED             YES — CI #58
DRAFT AUTH-v3 EXACT-HEAD PROVERIF RUN               RETAINED — 16de6da / CI #58
FM-02/FM-03 AGREEMENT                               SCOPED FORMALLY ANALYZED
FM-04 REPLAY-RECORD ORDERING                        SCOPED FORMALLY ANALYZED UNDER PERSISTENT/UNBOUNDED MODEL STATE
FM-05 MODELED SECURITY-CONTEXT INTEGRITY            SCOPED FORMALLY ANALYZED FOR MODELED FIELDS
AUTHZ ADMISSION → COMPLETION CORRESPONDENCE         SCOPED FORMALLY ANALYZED
IOT-CORE AUTHZ RAW PARSE/PROFILE BOUNDS             TESTED IN RUST + C
IOT-CORE AUTHZ EXACT-BYTE HASH PATH                 TESTED IN RUST + C
IOT-CORE AUTHZ PARSER ↔ PROVERIF EQUIVALENCE        NOT ESTABLISHED; EXPLICIT ABSTRACTION BOUNDARY
CRITICAL-EXTENSION CANONICAL SCHEMA                 NOT YET EQUIVALENTLY TRACED
CHANNEL-BINDING CANONICAL SCHEMA                    NOT YET EQUIVALENTLY TRACED
FM-09 PRE-EXISTING-TRUST RELATION                   SCOPED FORMALLY ANALYZED
PRODUCTION AUTH-v3                                  NOT IMPLEMENTED / NOT SELECTABLE
REPLAY EPOCH RECOVERY                               UNRESOLVED
TD-003                                              OPEN
TD-004                                              OPEN
TD-002                                              OPEN
TD-001 EXTERNAL CRYPTO REVIEW                       OPEN
FORMALLY VERIFIED                                   NOT CLAIMED
RFC-CLASS DOCUMENTED                                NOT CLAIMED
COMMON-CONFORMANT                                   NOT CLAIMED
DEPLOYMENT-QUALIFIED                                NOT CLAIMED
```

## 9. Next dependency-ready packet

With the authorization-admission handoff now retained at an exact green commit, broader formal work should not outrun normative/runtime ownership. The next priority remains TD-003 only where another already-defined boundary can be mapped without inventing policy; otherwise TD-004 specification work becomes the prerequisite.

The strongest immediate candidate is to audit the existing AUTH-v3 property/attacker matrix and identify the next property whose runtime/spec semantics are already concrete enough for a narrow correspondence and negative-test mapping. FM-10 should **not** be promoted until authority/provenance and full authorization decision semantics are defined. Critical-extension and channel-binding canonical boundaries should likewise remain explicit assumptions until their schemas and executable validation are comparably concrete.
