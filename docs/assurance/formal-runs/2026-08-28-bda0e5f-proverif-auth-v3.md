# ProVerif draft AUTH v3 run — `bda0e5f65e660ffb1c542305b8a3e9a4ff1eb4b9`

This artifact retains the exact-head fail-closed ProVerif execution of the non-advertised draft AUTH v3 model after introduction of the scoped FM-07 Finished-direction reflection-resistance query. It is TD-003 assurance evidence only. It does **not** claim computational HMAC collision resistance, whole-protocol unknown-key-share resistance, parser/model equivalence, production AUTH-v3 selectability, independent cryptographic review, formal verification of the implementation, or RFC-class status.

## Run identity

| Field | Value |
|---|---|
| Repository commit | `bda0e5f65e660ffb1c542305b8a3e9a4ff1eb4b9` |
| Branch | `dev` |
| GitHub Actions workflow | `ZK-ARCHE CI` |
| GitHub Actions run | `33229665049` / run 64 |
| Formal job | `99040095990` |
| Formal job result | PASS |
| Tool | ProVerif `2.05` as installed by the CI lane |
| Canonical draft-v3 model | `rust/models/proverif/zk_arche_auth_v3_draft.pv` |
| Synchronized C mirror | `c/models/proverif/zk_arche_auth_v3_draft.pv` |
| Canonical model Git blob | `22e4f94f2f02d5ee3a7ef5aa6c2a9f3765f7c219` |
| Formal artifact | `formal-proverif-evidence`, artifact id `9708091599` |
| Formal artifact digest | `sha256:f88574d0f66d3e417566ed6f5040b751404c3dc6d7c82651a1cac8e4561864d4` |
| Formal artifact size | 11280 bytes |
| Rust lane | PASS |
| C lane | PASS |
| Release qualification | PASS |
| Required-lane gate | PASS |

CI #64 completed successfully on this exact commit. The formal lane checked byte-synchronized model copies, reran the retained expected-negative legacy-v2 model, executed draft AUTH v3 with fail-closed result checking, executed replay continuity with the same discipline, and uploaded formal evidence. The AUTH-v3 gate rejects zero-result runs and rejects any `false` or `cannot be proved` result. The exact model at this commit contains eight queries, so successful CI establishes that all eight were accepted under that gate.

## Draft-v3 query set at this commit

| # | Property / correspondence | Exact-head gate state | Evidence boundary |
|---|---|---|---|
| 1 | `ServerAuth1AcceptedV3 ==> ReplayRecordedV3` | PASS | Accepted-message replay-record ordering under persistent/unbounded symbolic replay state. |
| 2 | injective `ServerCompleteV3 ==> ClientAuth3SentV3` | PASS | Scoped authenticated client-finished/server-completion agreement. |
| 3 | injective `ClientCompleteV3 ==> ServerCompleteV3` | PASS | Scoped mutual-completion agreement. |
| 4 | `ClientCompleteV3 ==> ServerAuth2SentV3` | PASS | Scoped server-authentication/context agreement. |
| 5 | `ServerCompleteV3 ==> ClientAuth3SentV3` | PASS | Scoped client-finished acceptance evidence. |
| 6 | `ServerCompleteV3 ==> TrustedRecordPresent(client)` | PASS | Scoped NO-LEARNING relation to pre-existing modeled trust. |
| 7 | `ServerCompleteV3 ==> AuthorizationContextAdmittedV3(client, server, session, secctx)` | PASS | Completion is downstream of the modeled authorization-context admission boundary; concrete parser correctness and authorization policy remain separate. |
| 8 | `FinishedDirectionsDerivedV3(..., tag, tag) ==> false` | PASS | Scoped FM-07 reflection resistance at the symbolic Finished interface: the modeled server-to-client and client-to-server Finished values cannot be identical in one derived security/key-confirmation context. |

The eighth query is the only new AUTH-v3 query relative to the retained run at `16de6da915a39e8d74dbab665ce1d98385681e8d`.

## FM-07 concrete-to-symbolic traceability

The executable Rust and C negative fixtures already establish the implementation-side scenario used to justify this formal packet. Both lanes derive separate server-to-client and client-to-server Finished keys from the same deterministic AUTH-v3 session/transcript vector, use distinct `server finished v3` and `client finished v3` labels, reproduce their expected deterministic tags, and reject cross-direction substitution. They also show that changing only the label while retaining the source-direction key does not reproduce the target-direction expected tag.

The model represents the corresponding boundary as:

```text
shared AUTH-v3 security/key-confirmation context
        ↓
k_s2c_v3(shared, kcctx)              symbolic directional KDF constructor
k_c2s_v3(shared, kcctx)              symbolic directional KDF constructor
        ↓
hmac(k_s2c, (server_finished_v3_label, kcctx))
hmac(k_c2s, (client_finished_v3_label, kcctx))
        ↓
FinishedDirectionsDerivedV3(..., server_tag, client_tag)
        ↓
query FinishedDirectionsDerivedV3(..., tag, tag) ==> false
```

Allowed claim:

> Under the draft AUTH-v3 symbolic model's distinct directional KDF constructors and Finished labels, ProVerif 2.05 accepted the non-reachability property that one normally derived server-direction Finished value and client-direction Finished value are not the same symbolic term in the same modeled context. Independent Rust/C deterministic negative fixtures separately exercise the concrete directional key/label separation.

Disallowed inference:

- HMAC collision resistance is computationally proven;
- HKDF/HMAC implementation correctness is formally verified;
- arbitrary protocol messages cannot be reflected across every state or transport path;
- FM-06 unknown-key-share resistance is independently proven;
- peer-role symmetry, negotiation downgrade resistance, authorization policy, resumption, rekey, or revocation properties follow from this query;
- the synchronized ProVerif copies are independent Rust/C formal implementations.

## Evidence-state advancement

```text
FM-07 RUST NEGATIVE FIXTURE                       TESTED at green exact-head CI
FM-07 C NEGATIVE FIXTURE                          TESTED at green exact-head CI
FM-07 SYNCHRONIZED PROVERIF QUERY                 MODELED
FM-07 EXACT-MODEL PROVERIF RUN                    RETAINED — CI #64
FM-07 FINISHED-DIRECTION REFLECTION PROPERTY      SCOPED FORMALLY ANALYZED
FM-06 UKS                                         NOT independently established
FM-08 DOWNGRADE                                   BLOCKED-NORMATIVE
FM-10 AUTHORIZATION POLICY                        BLOCKED-NORMATIVE
FORMALLY VERIFIED                                 NOT CLAIMED
RFC-CLASS DOCUMENTED                              NOT CLAIMED
COMMON-CONFORMANT                                 NOT CLAIMED
DEPLOYMENT-QUALIFIED                              NOT CLAIMED
TD-003                                            OPEN
```

## Retained limitations

This run does not establish:

- computational security, constant-time behavior, RNG quality, memory safety, secure storage, zeroization, or side-channel resistance;
- cryptographic soundness or zero-knowledge of the custom role-membership proof;
- parser/model equivalence or authorization-policy correctness;
- bounded/restart/rollback equivalence for runtime replay state or an authenticated fresh replay epoch;
- production AUTH-v3 negotiation, downgrade resistance, or selectability;
- critical-extension or channel-binding canonical-schema equivalence;
- authorization authority/provenance namespace semantics;
- revocation convergence, resumption authorization, delegation, non-transitive trust, infrastructure independence, or P2P role symmetry;
- constrained-target RAM/flash/CPU sufficiency, physical-target evidence, or field readiness;
- independent cryptographic review.

## Next evidence gate

The long-lived assurance records should next be reconciled to this eight-query retained result so FM-07 is no longer listed merely as `DEFINED`. After that reconciliation, additional TD-003 work should not invent properties whose normative/runtime semantics are incomplete. FM-06 should advance only if a non-redundant peer-identity mismatch scenario can be defined beyond existing agreement correspondences; FM-08 and FM-10 remain blocked on TD-004 normative ownership.
