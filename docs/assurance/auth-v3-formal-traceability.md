# AUTH-v3 Formal Traceability Record

Status: **scoped assurance evidence**. This record reconciles the retained draft AUTH-v3 ProVerif execution with the current repository surfaces that implement, specify, or test the modeled security context. It advances TD-003 traceability only. It does not make AUTH v3 selectable, prove the custom role-membership proof, establish replay persistence, close TD-003, or establish RFC-class status.

## 1. Exact evidence identity

Current repository anchor for this traceability record:

```text
repository_commit = 1ea2f3e938d02fb1a44581bae0bd98643d3ef111
branch            = dev
```

Retained formal result:

```text
docs/assurance/formal-runs/2026-08-27-d4994da-proverif-auth-v3.md
formal_run_commit = d4994daaa3dd771cfa87b42485d9032302af82f0
tool              = ProVerif 2.05
retained_results  = 6 TRUE query results
```

Current synchronized draft-v3 model copies:

```text
rust/models/proverif/zk_arche_auth_v3_draft.pv
c/models/proverif/zk_arche_auth_v3_draft.pv
Git blob SHA      = 4740b0a16a6148382f17fbc756f104222fc6cbd3
```

The Rust and C model paths have the same Git blob SHA at the repository anchor above. They are synchronized copies, not independent formal implementations.

The retained formal run records model SHA-256 `a49b316029d834a84e98aea6cde13371f229c595e75400d3b40207e1fb22deda` at its exact execution commit. A future model edit MUST produce a new retained run before a prior result is described as evidence for the edited model text.

## 2. Attacker and abstraction scope

The retained draft-v3 run is interpreted under the active-network attacker class corresponding to A0 in `formal-model-contract.md`: the attacker can read, drop, delay, replay, reorder, inject, modify, interleave, and initiate concurrent protocol runs but does not initially know uncompromised long-term secrets.

The model additionally makes these important abstractions explicit:

- `schnorr_proof` is idealized; the model does not establish computational Schnorr security;
- `role_proof` is idealized; TD-001 independent cryptographic review remains required;
- canonical hashing and concrete byte encoding are abstracted as symbolic bitstrings;
- the replay table is persistent and unbounded;
- restart, rollback, bounded eviction, fresh replay epochs, resumption, revocation convergence, transport failure observability, and implementation memory behavior are not modeled;
- constant-time behavior, RNG/DRBG quality, key storage, zeroization, memory safety, side channels, and target resource behavior are outside the symbolic model.

Accordingly, every TRUE result below is scoped to the modeled transition system and assumptions rather than promoted to a whole-protocol or deployment claim.

## 3. Property-to-query-to-repository map

| Property | ProVerif event/query evidence | Normative/design owner | Rust implementation/evidence anchor | C implementation/evidence anchor | Current evidence state |
|---|---|---|---|---|---|
| FM-02 client-to-server agreement | injective `ServerCompleteV3 ==> ClientAuth3SentV3` | ADR 0001 + `spec/zk-arche-protocol.md` draft AUTH-v3 state/completion semantics | `rust/crates/proto/src/auth_v3.rs`: `build_kc_transcript_v3`, `derive_kc_keys_v3`, `finished_tag_v3`, `completion_hash_v3`, `completion_tag_v3`; deterministic reference test/vector | `c/include/auth/auth_v3.h`, `c/src/proto/auth_v3.c`, `c/tests/test_auth_v3_reference.c` | scoped TRUE in retained model; production state-machine interoperability not established |
| FM-03 server-to-client / mutual completion | injective `ClientCompleteV3 ==> ServerCompleteV3`; `ClientCompleteV3 ==> ServerAuth2SentV3` | ADR 0001 authenticated AUTH_ACK-v3 design | same Rust v3 reference primitives and vector | same C v3 reference primitives and vector | scoped TRUE in retained model; production dispatch remains disabled |
| FM-04 accepted-message replay ordering | `ServerAuth1AcceptedV3 ==> ReplayRecordedV3` | `spec/iot-profiles.md` + `spec/replay-continuity.md` | bounded runtime replay implementation plus shared FIFO/replay-edge fixtures | bounded runtime replay implementation plus shared FIFO/replay-edge fixtures | scoped TRUE only for persistent/unbounded model replay memory; runtime restart/rollback/epoch equivalence not established |
| FM-05 transcript/security-context integrity | completion/agreement queries carry identical `secctx` and `kcctx` | AUTH-v3 security context + `spec/iot-profiles.md` + authorization-context specification | `AuthV3Context` binds version, suite, profile, selected capabilities, session ID, authz hash, critical-extension hash, channel-binding hash into `KC-TRANSCRIPT-v3` | `auth_v3_context_t` and `auth_v3_build_kc_transcript` expose the same draft fields | scoped TRUE for modeled fields; canonicalization/parser semantics of hashed subcontexts remain a separate TD-004/R-004 boundary |
| FM-09 NO-LEARNING AUTH | `ServerCompleteV3 ==> TrustedRecordPresent(client)` | roadmap NO-LEARNING invariant and future TRUST/AUTH normative text | production trust-store mutation remains separate from draft v3 reference helpers | production trust-store mutation remains separate from draft v3 reference helpers | scoped TRUE in model relative to pre-existing trusted record; not a complete enrollment/trust-state proof |

## 4. Security-context field traceability

The model constructor is:

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

The Rust draft context exposes the corresponding fields as:

```text
AuthV3Context::protocol_version
AuthV3Context::suite_id
AuthV3Context::profile_id
AuthV3Context::selected_capabilities
AuthV3Context::session_id
AuthV3Context::authz_context_hash
AuthV3Context::critical_extensions_hash
AuthV3Context::channel_binding_hash
```

The C draft context exposes the same semantic set through `auth_v3_context_t`.

This mapping is semantic, not a proof that parser inputs are canonical. The model treats the three hashed subcontexts as already-formed bitstrings. Therefore duplicate handling, deterministic empty representations, field ordering, multiplicity, unknown critical-value processing, and alternate encodings remain outside the formal result until the concrete parse/validate/canonicalize/hash contract is itself covered by executable evidence or a richer model.

## 5. Replay traceability after the current replay work

The current repository has materially advanced runtime replay semantics beyond the state present when the retained v3 run was first produced:

```text
minimum iot-core replay window = 64 accepted AUTH_1 identifiers
retention                      = deterministic FIFO
restart rule                   = restore trusted state or CONTINUITY_BROKEN
replay epoch rule              = unresolved
profile selectable             = 0
```

The runtime replay-continuity state machine and shared Rust/C decision corpus therefore provide executable evidence for fail-closed restart behavior, but they do not change what the retained ProVerif run proved. The symbolic model still has no `RESTORING`, `CONTINUITY_BROKEN`, restart, rollback, bounded eviction, or authenticated fresh-epoch transition.

The following claim is the strongest allowed reconciliation:

> The retained draft-v3 model establishes accepted-message replay-record ordering under persistent/unbounded symbolic replay memory. Current Rust/C evidence separately establishes bounded FIFO decisions and fail-closed replay-continuity state transitions. These evidence lanes are complementary and are not yet equivalent.

A future TD-003 replay-model packet should either model the promoted restart/epoch semantics or keep the stronger symbolic replay assumption explicit in every retained run.

## 6. Retained counterexample lineage

The legacy AUTH-v2 formal run remains part of the evidence chain. Its two material correspondence failures were not erased:

1. outer `session_id` was not authenticated by the v2 key-confirmation context;
2. the public one-byte AUTH_ACK could be synthesized by an active attacker, so client completion did not establish server acceptance of AUTH_3.

The draft-v3 model addresses those specific modeled failures by binding `session_id` and the wider security context into `KC-TRANSCRIPT-v3` and by authenticating server completion with a dedicated completion key/MAC. The six retained TRUE draft-v3 results are therefore regression evidence for those modeled fixes, not proof that all AUTH-v3 protocol properties are complete.

## 7. Explicitly open formal properties

This record does not upgrade the following FM rows to analyzed:

```text
FM-06 unknown-key-share resistance beyond current bound identities/context
FM-07 explicit reflection resistance
FM-08 downgrade resistance and production negotiation
FM-10 full authorization-scope semantics
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

Those rows remain open until their state, attacker assumptions, queries/lemmas, retained results, and model-to-code mappings exist.

## 8. Evidence-state update

At the repository anchor for this record:

```text
DRAFT AUTH-v3 MODEL COPIES SYNCHRONIZED       YES
DRAFT AUTH-v3 RETAINED PROVERIF RUN           YES
FM-02/FM-03 DRAFT-V3 AGREEMENT                SCOPED FORMALLY ANALYZED
FM-04 REPLAY-RECORD ORDERING                   SCOPED FORMALLY ANALYZED UNDER PERSISTENT/UNBOUNDED MODEL STATE
FM-05 MODELED SECURITY-CONTEXT INTEGRITY       SCOPED FORMALLY ANALYZED FOR MODELED FIELDS
FM-09 PRE-EXISTING-TRUST RELATION              SCOPED FORMALLY ANALYZED
RUST/C DRAFT-V3 REFERENCE PRIMITIVE PARITY     TESTED BY EXISTING CI/VECTORS
PRODUCTION AUTH-V3                             NOT IMPLEMENTED / NOT SELECTABLE
REPLAY EPOCH RECOVERY                          UNRESOLVED
TD-003                                         OPEN
TD-004                                         OPEN
TD-002                                         OPEN
TD-001 EXTERNAL CRYPTO REVIEW                  OPEN
FORMALLY VERIFIED                              NOT CLAIMED
RFC-CLASS DOCUMENTED                           NOT CLAIMED
DEPLOYMENT-QUALIFIED                           NOT CLAIMED
```

## 9. Next dependency-ready TD-003 packet

The next formal packet should not invent the unresolved replay-epoch cryptography. The highest-value dependency-ready work is to model or mechanically trace the **already-specified fail-closed replay-continuity states** (`TRUSTED`, `RESTORING`, `CONTINUITY_BROKEN`) and prove only properties that do not require a fresh-epoch recovery design, for example:

- AUTH admission is impossible while `RESTORING`;
- missing/corrupt/stale/rollback-suspected restored state cannot transition directly to AUTH acceptance;
- empty-cache reset and fresh outer-session identifiers cannot escape `CONTINUITY_BROKEN`;
- no transition out of `CONTINUITY_BROKEN` exists in the current model until a separately reviewed authenticated fresh-epoch mechanism is added.

That packet can reduce the formal/runtime restart gap without prematurely defining the cryptographic recovery transition.
