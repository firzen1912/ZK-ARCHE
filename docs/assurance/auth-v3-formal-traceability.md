# AUTH-v3 Formal Traceability Record

Status: **scoped assurance evidence**. This record reconciles the current synchronized draft AUTH-v3 ProVerif model with the repository surfaces that implement, specify, or test the modeled security context. It advances TD-003 traceability only. It does not make AUTH v3 selectable, prove the custom role-membership proof, establish parser equivalence, close TD-003, or establish RFC-class status.

## 1. Exact evidence identity

Current reconciliation base for this traceability record:

```text
repository_commit = 31c1fcb22348a7ea0be78dec9e591b7af26f2526
branch            = dev
latest retained AUTH-v3 formal evidence = bda0e5f65e660ffb1c542305b8a3e9a4ff1eb4b9 / CI #64
```

Current retained formal result:

```text
docs/assurance/formal-runs/2026-08-28-bda0e5f-proverif-auth-v3.md
formal_run_commit = bda0e5f65e660ffb1c542305b8a3e9a4ff1eb4b9
ci_run            = #64 / 33229665049
ci_result         = success
formal_job        = 99040095990 / success
tool              = ProVerif 2.05
model_blob         = 22e4f94f2f02d5ee3a7ef5aa6c2a9f3765f7c219
retained_queries   = 8 queries, all accepted by the fail-closed CI gate
artifact           = formal-proverif-evidence / 9708091599
artifact_digest    = sha256:f88574d0f66d3e417566ed6f5040b751404c3dc6d7c82651a1cac8e4561864d4
```

Previous retained AUTH-v3 evidence remains historical provenance:

```text
docs/assurance/formal-runs/2026-08-28-16de6da-proverif-auth-v3.md
formal_run_commit = 16de6da915a39e8d74dbab665ce1d98385681e8d
retained_results  = 7 queries accepted by the fail-closed CI gate

docs/assurance/formal-runs/2026-08-27-d4994da-proverif-auth-v3.md
formal_run_commit = d4994daaa3dd771cfa87b42485d9032302af82f0
retained_results  = 6 TRUE query results
```

Current synchronized model copies are:

```text
rust/models/proverif/zk_arche_auth_v3_draft.pv
c/models/proverif/zk_arche_auth_v3_draft.pv
Git blob SHA = 22e4f94f2f02d5ee3a7ef5aa6c2a9f3765f7c219
```

The Rust and C paths are synchronized copies, not independent formal implementations. CI #64 explicitly checked synchronization before running ProVerif. A future model edit requires a new exact-model retained run before this result may be cited for the edited model text.

## 2. Attacker and abstraction scope

The draft-v3 model is interpreted under the active-network attacker class corresponding to A0 in `formal-model-contract.md`: the attacker can read, drop, delay, replay, reorder, inject, modify, interleave, and initiate concurrent protocol runs but does not initially know uncompromised long-term secrets.

Important abstractions remain:

- `schnorr_proof` is idealized; computational Schnorr security is not established;
- `role_proof` is idealized; TD-001 independent cryptographic review remains required;
- concrete canonical hashing and byte encoding are abstracted as symbolic bitstrings;
- the replay table is persistent and unbounded;
- restart, rollback, bounded eviction, fresh replay epochs, resumption, revocation convergence, and transport observability are not part of this AUTH-v3 model;
- the FM-07 reflection query reasons over distinct symbolic directional KDF constructors and Finished labels; it is not a computational proof of HMAC collision resistance or implementation correctness;
- constant-time behavior, RNG/DRBG quality, key storage, zeroization, memory safety, side channels, and target resource behavior are outside the symbolic model.

Every successful query below is therefore a scoped model result, not a whole-protocol or deployment claim.

## 3. Property-to-query-to-repository map

| Property | ProVerif correspondence | Normative/design owner | Concrete anchors | Evidence state |
|---|---|---|---|---|
| FM-02 client-to-server agreement | injective `ServerCompleteV3 ==> ClientAuth3SentV3` | ADR 0001 + draft AUTH-v3 state/completion semantics | Rust/C `auth_v3` reference primitives + deterministic reference vectors | scoped formally analyzed |
| FM-03 mutual completion | injective `ClientCompleteV3 ==> ServerCompleteV3`; `ClientCompleteV3 ==> ServerAuth2SentV3` | ADR 0001 authenticated AUTH_ACK-v3 design | same Rust/C draft-v3 reference primitives/vectors | scoped formally analyzed |
| FM-04 accepted-message replay ordering | `ServerAuth1AcceptedV3 ==> ReplayRecordedV3` | `spec/iot-profiles.md` + `spec/replay-continuity.md` | bounded runtime replay + shared replay fixtures; separate replay-continuity model | scoped formally analyzed only under persistent/unbounded AUTH-v3 replay abstraction |
| FM-05 transcript/security-context integrity | completion/agreement queries carry identical `secctx`/`kcctx` | AUTH-v3 security context + `spec/auth-v3-context-encoding.md` + `spec/iot-core-authorization-context.md` | Rust/C `AuthV3Context`/`auth_v3_context_t`; `iot-core` raw receive/decode/hash paths | scoped formally analyzed for modeled fields; concrete byte validation remains separate executable evidence |
| FM-05 authorization-context admission handoff | `ServerCompleteV3 ==> AuthorizationContextAdmittedV3(client, server, session, secctx)` | this traceability boundary + concrete `iot-core` authorization spec | Rust/C strict canonical parser, 7-entry/148-byte `iot-core` validation, exact accepted-byte hash | scoped formally analyzed at the symbolic admission boundary; parser equivalence not established |
| FM-07 Finished-direction reflection resistance | `FinishedDirectionsDerivedV3(..., tag, tag) ==> false` | draft AUTH-v3 directional key/Finished separation | Rust/C `auth_v3_reflection` negative fixtures + AUTH-v3 Finished/KDF helpers | **scoped formally analyzed in CI #64**; computational HMAC/HKDF correctness not established |
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

Existing tests cover the canonical vector and profile-bound failures including 147/149-byte inputs, encoded entry count eight, wrong context kind, wrong schema ID, and the broader shared malformed `ZKCTX` corpus. The exact-head implementation lanes associated with the current FM-07 formal evidence were green in CI #64.

### 4.2 Explicit symbolic authorization handoff

The current model declares:

```text
AuthorizationContextAdmittedV3(client, server, session, security_context)
```

and emits it on the server path after construction of the modeled security context, with the explicit comment that concrete Rust/C parsing, profile validation, semantic checks, and exact-byte hashing have already succeeded.

The retained correspondence is:

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

The correspondence proves only a property of the modeled transition system: modeled server completion is downstream of the modeled admission event for the same client/server/session/security context. It does **not** prove that the concrete parser is correct, that SHA-256 is collision-resistant, that the parser and symbolic model are equivalent, or that holder/audience/role/revocation policy authorizes the requested operation.

Critical-extension and channel-binding canonical schemas remain less concrete than the `iot-core` authorization boundary and must remain separate FM-05 assumptions until equivalent normative and executable evidence exists.

### 4.3 Finished-direction reflection traceability

The current model records both normally derived directional Finished values under one authenticated security/key-confirmation context:

```text
k_s2c_v3(shared, kcctx)
k_c2s_v3(shared, kcctx)
        ↓
hmac(k_s2c, (server_finished_v3_label, kcctx))
hmac(k_c2s, (client_finished_v3_label, kcctx))
        ↓
FinishedDirectionsDerivedV3(..., server_tag, client_tag)
```

The retained FM-07 non-reachability query is:

```text
FinishedDirectionsDerivedV3(..., tag, tag) ==> false
```

Rust and C independently exercise the corresponding deterministic negative scenario: server-direction and client-direction Finished values differ; verbatim cross-direction substitution is rejected by comparison against the target expected value; and changing only the label while retaining the source-direction key does not reproduce the target-direction tag.

Allowed claim:

> Under the draft AUTH-v3 symbolic model's distinct directional KDF constructors and Finished labels, ProVerif 2.05 accepted the non-reachability property that the normally derived server-direction and client-direction Finished values are not the same symbolic term in the same modeled context. Independent Rust/C deterministic negative fixtures separately exercise the concrete directional key/label separation.

Disallowed inference:

- HMAC collision resistance is computationally proven;
- HKDF/HMAC implementation correctness is formally verified;
- arbitrary messages across every AUTH state or transport path are reflection-proof;
- FM-06 unknown-key-share resistance follows from FM-07;
- peer-role symmetry, downgrade resistance, authorization, resumption, rekey, or revocation follows from FM-07.

## 5. Replay traceability

The current repository has bounded runtime replay behavior plus a separate replay-continuity state machine/model. The AUTH-v3 model still uses persistent/unbounded symbolic replay memory.

The strongest allowed reconciliation remains:

> The draft-v3 model establishes accepted-message replay-record ordering under persistent/unbounded symbolic replay memory. Current Rust/C evidence separately establishes bounded FIFO decisions and fail-closed replay-continuity transitions, while the dedicated replay-continuity model provides scoped symbolic analysis of those states. These evidence lanes are complementary, not one complete implementation proof.

The authenticated fresh replay-epoch transition remains unresolved.

## 6. Retained counterexample and result lineage

The legacy AUTH-v2 formal evidence remains part of the assurance chain. Its material correspondence failures were not erased:

1. the outer `session_id` was not authenticated by the v2 key-confirmation context;
2. the public one-byte AUTH_ACK could be synthesized by an active attacker, so client completion did not establish server acceptance of AUTH_3.

Draft-v3 binds `session_id` and the wider security context into its KC context and authenticates server completion with a dedicated completion key/MAC. The retained AUTH-v3 lineage progressed from six queries, to seven with the explicit authorization-admission handoff, to the current eight-query result adding the scoped FM-07 Finished-direction reflection property. These results are regression evidence for the modeled fixes and boundaries; they are not proof that all AUTH-v3 properties are complete.

## 7. Explicitly open formal properties

The following remain open or not comprehensively analyzed by this record:

```text
FM-01 explicit session/association-secret secrecy
FM-06 unknown-key-share resistance beyond currently bound identities/context
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

FM-07 is no longer listed as merely open: its narrow Finished-direction property is now scoped formally analyzed. That state must not be generalized into generic reflection resistance for every protocol message/state.

The authorization-admission correspondence does not upgrade FM-10. The concrete `iot-core` validator establishes syntax, fixed schema, basic semantic invariants, and exact-byte hashing; it does not prove local authority/provenance, holder/audience/role-policy lineage, revocation freshness, or operation authorization. The August 28 research specifically keeps authority/provenance namespace semantics unresolved.

## 8. Evidence-state update

```text
DRAFT AUTH-v3 MODEL COPIES SYNCHRONIZED             YES — CI #64
DRAFT AUTH-v3 EXACT-MODEL PROVERIF RUN              RETAINED — bda0e5f / CI #64
AUTH-v3 RETAINED QUERY COUNT                        8
FM-02/FM-03 AGREEMENT                               SCOPED FORMALLY ANALYZED
FM-04 REPLAY-RECORD ORDERING                        SCOPED FORMALLY ANALYZED UNDER PERSISTENT/UNBOUNDED MODEL STATE
FM-05 MODELED SECURITY-CONTEXT INTEGRITY            SCOPED FORMALLY ANALYZED FOR MODELED FIELDS
AUTHZ ADMISSION → COMPLETION CORRESPONDENCE         SCOPED FORMALLY ANALYZED
FM-07 FINISHED-DIRECTION REFLECTION                 TESTED RUST+C + SCOPED FORMALLY ANALYZED
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

The long-lived AUTH-v3 traceability record is now reconciled to the retained eight-query FM-07 result. The next TD-003 packet should reconcile `docs/assurance/formal-model-contract.md` to the same evidence state before adding another theorem.

After that reconciliation, the strongest agent-closable formal candidate is FM-01 only if the exact derived session/association secret and compromise assumptions can be named without inventing missing protocol semantics. FM-06 should advance only if a non-redundant peer-identity mismatch scenario can be specified beyond the identities already carried by the FM-02/FM-03 agreement correspondences. FM-08 and FM-10 remain blocked on TD-004 normative ownership, and critical-extension/channel-binding canonical boundaries remain explicit FM-05 assumptions until their schemas and executable validation are comparably concrete.
