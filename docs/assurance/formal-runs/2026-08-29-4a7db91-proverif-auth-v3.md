# ProVerif draft AUTH v3 run — `4a7db91597e6df6442200b808acdca62e026ad31`

This artifact retains the exact-head fail-closed ProVerif execution of the non-advertised draft AUTH v3 model after introduction of the scoped FM-01 accepted session-key secrecy query and the explicit symbolic `session_key_v2(...)` derivation boundary. It is TD-003 assurance evidence only. It does **not** claim computational HKDF/Ristretto security, forward secrecy after endpoint compromise, implementation verification, parser/model equivalence, production AUTH-v3 selectability, independent cryptographic review, or RFC-class status.

## Run identity

| Field | Value |
|---|---|
| Repository commit | `4a7db91597e6df6442200b808acdca62e026ad31` |
| Branch | `dev` |
| GitHub Actions workflow | `ZK-ARCHE CI` |
| GitHub Actions run | `33239334561` / run 68 |
| Formal job | `99065843751` |
| Formal job result | PASS |
| Tool | ProVerif `2.05` as installed by the CI lane |
| Canonical draft-v3 model | `rust/models/proverif/zk_arche_auth_v3_draft.pv` |
| Synchronized C mirror | `c/models/proverif/zk_arche_auth_v3_draft.pv` |
| Canonical model Git blob | `a8e2a19c5f1178dacd3e140d722c121e0d556056` |
| Formal artifact | `formal-proverif-evidence`, artifact id `9710892240` |
| Formal artifact digest | `sha256:8dab0e7f9bd0d1edfa5ae952ae89ed6427df114e8917498cc1af49258982d8f9` |
| Formal artifact size | 11448 bytes |
| Rust lane | PASS |
| C lane | PASS |
| Release qualification | PASS |
| Required-lane gate | PASS |

CI #68 completed successfully on this exact commit. The formal lane checked byte-synchronized model copies, reran the retained expected-negative legacy-v2 model, executed draft AUTH v3 with fail-closed result checking, executed replay continuity with the same discipline, and uploaded formal evidence. The AUTH-v3 gate rejects zero-result runs and rejects any `false` or `cannot be proved` result. The exact model at this commit contains nine queries, so successful CI establishes that all nine were accepted under that gate.

## Draft-v3 query set at this commit

| # | Property / correspondence | Exact-head gate state | Evidence boundary |
|---|---|---|---|
| 1 | `ServerAuth1AcceptedV3 ==> ReplayRecordedV3` | PASS | Accepted-message replay-record ordering under persistent/unbounded symbolic replay state. |
| 2 | injective `ServerCompleteV3 ==> ClientAuth3SentV3` | PASS | Scoped authenticated client-finished/server-completion agreement. |
| 3 | injective `ClientCompleteV3 ==> ServerCompleteV3` | PASS | Scoped mutual-completion agreement. |
| 4 | `ClientCompleteV3 ==> ServerAuth2SentV3` | PASS | Scoped server-authentication/context agreement. |
| 5 | `ServerCompleteV3 ==> ClientAuth3SentV3` | PASS | Scoped client-finished acceptance evidence. |
| 6 | `ServerCompleteV3 ==> TrustedRecordPresent(client)` | PASS | Scoped NO-LEARNING relation to pre-existing modeled trust. |
| 7 | `ServerCompleteV3 ==> AuthorizationContextAdmittedV3(client, server, session, secctx)` | PASS | Completion is downstream of the modeled authorization-context admission boundary; parser correctness and authorization policy remain separate. |
| 8 | `event(SessionKeyEstablishedV3(..., key)) && attacker(key)` | PASS | Scoped FM-01 secrecy: under the A0 model without endpoint compromise, ProVerif found no accepted modeled session key reachable by the active network attacker. |
| 9 | `FinishedDirectionsDerivedV3(..., tag, tag) ==> false` | PASS | Scoped FM-07 Finished-direction reflection resistance. |

The eighth query is the only new AUTH-v3 query relative to the retained run at `bda0e5f65e660ffb1c542305b8a3e9a4ff1eb4b9`.

## FM-01 implementation-to-symbolic boundary

The current runtime AUTH-v3 path consumes the existing session key derived from the ephemeral shared point and transcript inputs before deriving purpose-separated AUTH-v3 key-confirmation/completion keys. The model now represents that existing boundary explicitly:

```text
modeled ephemeral DH shared point
        ↓
session_key_v2(shared, nonce_c, nonce_s, pid, eph_c, eph_s)
        ↓
k_s2c_v3 / k_c2s_v3 / k_complete_v3
        ↓
accepted AUTH-v3 completion
        ↓
SessionKeyEstablishedV3(..., key)
        ↓
query event(SessionKeyEstablishedV3(..., key)) && attacker(key)
```

`session_key_v2(...)` is a symbolic constructor corresponding to the repository's existing HKDF-SHA256 session-key derivation inputs. It abstracts concrete HKDF, Ristretto encoding/group behavior, byte serialization, RNG, storage, and implementation correctness.

Allowed claim:

> Under the draft AUTH-v3 symbolic A0 active-network model, with no modeled endpoint or long-term-secret compromise, ProVerif 2.05 accepted the reachability/secrecy query showing that an accepted modeled `session_key_v2` value is not learned by the attacker. The session-key constructor is explicitly traceable to the existing Rust/C session-key KDF boundary consumed by AUTH v3.

Disallowed inference:

- computational HKDF, SHA-256, or Ristretto security is formally proven;
- the Rust or C implementation is formally verified;
- forward secrecy or post-compromise security is established under selective endpoint compromise;
- RNG/DRBG quality, memory safety, constant-time behavior, secure storage, erasure, or side-channel resistance is established;
- application/DATA traffic-key secrecy follows automatically;
- TD-001 independent cryptographic review is satisfied;
- AUTH v3 is production-selectable or RFC-class documented.

## Evidence-state advancement

```text
FM-01 SESSION-KEY DERIVATION BOUNDARY              IMPLEMENTATION-TRACEABLE, scoped
FM-01 SYNCHRONIZED PROVERIF SECRECY QUERY          MODELED
FM-01 EXACT-MODEL PROVERIF RUN                     RETAINED — CI #68
FM-01 ACCEPTED SESSION-KEY SECRECY                  SCOPED FORMALLY ANALYZED under A0/no-compromise
FM-07 FINISHED-DIRECTION REFLECTION                 REMAINS SCOPED FORMALLY ANALYZED
FM-06 UKS                                           NOT independently established
FM-08 DOWNGRADE                                     BLOCKED-NORMATIVE
FM-10 AUTHORIZATION POLICY                          BLOCKED-NORMATIVE
FORMALLY VERIFIED                                   NOT CLAIMED
RFC-CLASS DOCUMENTED                                NOT CLAIMED
COMMON-CONFORMANT                                   NOT CLAIMED
DEPLOYMENT-QUALIFIED                                NOT CLAIMED
TD-003                                              OPEN
```

## Retained limitations

This run does not establish computational security, constant-time behavior, RNG quality, memory safety, secure storage, zeroization, side-channel resistance, custom role-proof soundness/zero knowledge, parser/model equivalence, authorization-policy correctness, bounded/restart/rollback replay equivalence, an authenticated fresh replay epoch, production negotiation/downgrade behavior, authority/provenance semantics, revocation convergence, resumption authorization, delegation, non-transitive trust, infrastructure independence, P2P role symmetry, constrained-target sufficiency, field readiness, or independent cryptographic review.

## Next evidence gate

The long-lived assurance records should next be reconciled to this nine-query retained result so FM-01 advances from `DEFINED`/`MODELED` to scoped `FORMALLY ANALYZED` with the A0/no-compromise boundary explicit. Additional TD-003 work should continue to avoid inventing semantics that remain blocked on TD-004, especially downgrade, authorization authority/provenance, revocation, resumption, and compromise/recovery behavior.
