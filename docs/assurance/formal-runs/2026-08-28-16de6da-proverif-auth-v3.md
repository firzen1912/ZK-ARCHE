# ProVerif draft AUTH v3 run — `16de6da915a39e8d74dbab665ce1d98385681e8d`

This artifact retains the exact-head fail-closed ProVerif execution of the non-advertised draft AUTH v3 model after introduction of the explicit `AuthorizationContextAdmittedV3` abstraction boundary. It is scoped TD-003 assurance evidence only. It does **not** claim that the raw byte parser is verified by ProVerif, that authorization policy is proven, that the custom role-membership proof is computationally sound, that AUTH v3 is production-selectable, or that ZK-ARCHE is formally verified or RFC-class documented.

## Run identity

| Field | Value |
|---|---|
| Repository commit | `16de6da915a39e8d74dbab665ce1d98385681e8d` |
| Branch | `dev` |
| GitHub Actions workflow | `ZK-ARCHE CI` |
| GitHub Actions run | `33209827503` / run 58 |
| Formal job | `98980037950` |
| Formal job result | PASS |
| Tool | ProVerif `2.05` as installed by the pinned CI lane |
| Canonical draft-v3 model | `rust/models/proverif/zk_arche_auth_v3_draft.pv` |
| Synchronized C mirror | `c/models/proverif/zk_arche_auth_v3_draft.pv` |
| Canonical model Git blob | `0fefe938988feb49086ecf579f2b0ae5df8d16eb` |
| Formal artifact | `formal-proverif-evidence`, artifact id `9701194272` |
| Formal artifact digest | `sha256:46f5ae3bc4902b5c4b407ba1c96bf0933ca5f63e41825fd490215404466f95ec` |
| Formal artifact size | 11188 bytes |
| Rust lane | PASS |
| C lane | PASS |
| Release qualification | PASS |
| Required-lane gate | PASS |

CI #58 completed successfully on this exact commit. The formal job checked synchronized models, reran the retained expected-negative legacy-v2 model, executed the draft AUTH-v3 model with fail-closed result checking, executed the replay-continuity model with the same fail-closed discipline, and uploaded the three formal logs as the retained artifact above.

The AUTH-v3 CI gate refuses success if the model emits no `RESULT` lines or if any result is `false` or `cannot be proved`. Therefore the successful formal job establishes that all seven queries present in the exact model blob above were accepted by ProVerif under that gate. The repository record intentionally relies on the exact commit/model identity and retained CI artifact rather than reconstructing unobserved console text.

## Draft-v3 query set at this commit

The exact model contains seven correspondence queries:

| # | Property / correspondence | Exact-head gate state | Evidence boundary |
|---|---|---|---|
| 1 | `ServerAuth1AcceptedV3 ==> ReplayRecordedV3` | PASS | Accepted-message replay-record ordering under the model's persistent/unbounded replay-table abstraction. |
| 2 | injective `ServerCompleteV3 ==> ClientAuth3SentV3` | PASS | Scoped authenticated client-finished/server-completion agreement. |
| 3 | injective `ClientCompleteV3 ==> ServerCompleteV3` | PASS | Scoped mutual-completion agreement. |
| 4 | `ClientCompleteV3 ==> ServerAuth2SentV3` | PASS | Scoped server-authentication/context agreement. |
| 5 | `ServerCompleteV3 ==> ClientAuth3SentV3` | PASS | Scoped client-finished acceptance evidence. |
| 6 | `ServerCompleteV3 ==> TrustedRecordPresent(client)` | PASS | Scoped NO-LEARNING relation to pre-existing modeled trust. |
| 7 | `ServerCompleteV3 ==> AuthorizationContextAdmittedV3(client, server, session, secctx)` | PASS | Completion can occur only after crossing the modeled authorization-context admission boundary. It does not prove the concrete parser or authorization policy. |

The seventh query is the only new AUTH-v3 correspondence relative to the retained run at `d4994daaa3dd771cfa87b42485d9032302af82f0`.

## Authorization-context abstraction boundary

The model now declares:

```text
AuthorizationContextAdmittedV3(client, server, session, security_context)
```

and emits it on the server path after constructing the modeled security context and before replay/trust/proof acceptance and protocol completion. The corresponding completion query makes this admission point explicit in formal evidence.

The intended concrete-to-symbolic handoff is:

```text
attacker-controlled raw authorization bytes
        ↓
strict ZKCTX parse and structural/profile bounds      Rust/C executable evidence
        ↓
fixed iot-core 7-entry / 148-byte schema validation  Rust/C executable evidence
        ↓
semantic zero/scope validation                       Rust/C executable evidence
        ↓
SHA-256(exact accepted canonical bytes)              Rust/C executable evidence
        ↓
authz_context_hash / authz_core mapping               implementation/traceability boundary
        ↓
AuthorizationContextAdmittedV3                       ProVerif abstraction boundary
        ↓
AUTH-v3 completion correspondences                    scoped symbolic analysis
```

The model does not parse `ZKCTX`, enforce byte lengths or entry IDs, compute SHA-256, prove collision resistance, prove Rust/C parser equivalence, or establish holder/audience/role-policy/revocation authorization. Those remain separate executable/specification/assurance obligations.

## What this run advances

```text
AUTH-V3 MODEL COPIES SYNCHRONIZED                    YES — CI #58 gate
AUTH-V3 EXACT-HEAD MODEL EXECUTION                   RETAINED
AUTH-V3 SEVEN CORRESPONDENCE QUERIES                 PASS UNDER CI FAIL-CLOSED GATE
AUTHZ ADMISSION → COMPLETION CORRESPONDENCE          SCOPED FORMALLY ANALYZED
IOT-CORE RAW PARSER/PROFILE/HASH BEHAVIOR             SEPARATELY TESTED IN RUST + C
PARSER ↔ PROVERIF EQUIVALENCE                        NOT ESTABLISHED
FULL AUTHORIZATION POLICY                            NOT FORMALLY ANALYZED
PRODUCTION AUTH-V3                                   NOT SELECTABLE
FORMALLY VERIFIED                                    NOT CLAIMED
RFC-CLASS DOCUMENTED                                 NOT CLAIMED
COMMON-CONFORMANT                                    NOT CLAIMED
DEPLOYMENT-QUALIFIED                                 NOT CLAIMED
TD-003                                               OPEN
```

## Retained limitations

This run does not establish:

- computational soundness or zero-knowledge of the custom role-membership proof;
- constant-time behavior, RNG/DRBG quality, memory safety, secure storage, zeroization, or side-channel resistance;
- bounded/restart/rollback equivalence for runtime replay state;
- production v2/v3 negotiation or downgrade resistance;
- critical-extension or channel-binding canonical-schema equivalence;
- authorization issuer/authority namespace semantics;
- revocation convergence, resumption authorization, delegation, non-transitive trust, infrastructure independence, or P2P role symmetry as modeled properties;
- constrained-target RAM/flash/CPU sufficiency or deployment readiness;
- independent cryptographic review.

## Next evidence gate

The next TD-003 work should extend formal coverage only where the specification/runtime ownership is sufficiently concrete. The highest-value currently agent-closable candidate is to trace and, where appropriate, model one additional already-defined security boundary without inventing unresolved policy. FM-10 authorization policy remains blocked on authority/provenance and full local policy semantics; critical-extension and channel-binding canonical boundaries remain less concrete than `iot-core` authorization. TD-004 normative completeness therefore remains an adjacent prerequisite for broader formal claims.
