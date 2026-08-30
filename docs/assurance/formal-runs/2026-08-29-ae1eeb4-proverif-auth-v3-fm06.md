# ProVerif draft AUTH v3 FM-06 run — `ae1eeb47b830996470beb489fe3875e5fc2635a2`

This artifact retains the exact-head fail-closed ProVerif execution that first includes the scoped FM-06 identity-attribution correspondence. It is TD-003 assurance evidence only. It does **not** claim complete unknown-key-share resistance, computational credential/key binding, resolver implementation verification, parser/model equivalence, independent cryptographic review, RFC-class status, Common Contract completion, or deployment qualification.

## Run identity

| Field | Value |
|---|---|
| Repository commit | `ae1eeb47b830996470beb489fe3875e5fc2635a2` |
| Branch | `dev` |
| GitHub Actions workflow | `ZK-ARCHE CI` |
| GitHub Actions run | `33281604442` / run 86 |
| Formal job | `99177588145` |
| Formal job result | PASS |
| Tool | ProVerif `2.05` as installed by the CI lane |
| Canonical draft-v3 model | `rust/models/proverif/zk_arche_auth_v3_draft.pv` |
| Synchronized C mirror | `c/models/proverif/zk_arche_auth_v3_draft.pv` |
| Canonical model Git blob | `2f3817b5fb847ef948e4effab4b7d9871adc2e14` |
| Formal artifact | `formal-proverif-evidence`, artifact id `9723169047` |
| Formal artifact digest | `sha256:fb4a385a50a41b87fc911ac43ddca92f511601c1b05bd22b9b1c332dec46a8a4` |
| Formal artifact size | 11638 bytes |
| Rust lane | PASS |
| C lane | PASS |
| Release qualification | PASS |
| Required-lane gate | PASS |

CI #86 completed successfully on this exact commit. The formal lane checked byte-synchronized Rust/C model copies, reran the retained expected-negative legacy-v2 model, executed draft AUTH v3 with fail-closed result checking, executed replay continuity with the same discipline, and uploaded formal evidence. The AUTH-v3 model produced ten query results and all ten were `true` under the repository gate.

## FM-06 property added at this commit

The model distinguishes peer identity from authentication public key and records the local attribution chain with:

```text
TrustedAttributionPresent(public_key, peer_identity, role, authz_context)
IdentityAttributionResolvedV3(public_key, peer_identity, role, session, security_context)
ServerAttributedCompleteV3(public_key, peer_identity, role, session, security_context, kc_context)
```

The retained scoped correspondence is:

```text
ServerAttributedCompleteV3(key, identity, role, sid, secctx, kcctx)
    ==>
IdentityAttributionResolvedV3(key, identity, role, sid, secctx)
```

Exact hosted result:

```text
RESULT event(ServerAttributedCompleteV3(cpk_3,identity,allowed_3,sid_3,secctx_2,kcctx_2))
==> event(IdentityAttributionResolvedV3(cpk_3,identity,allowed_3,sid_3,secctx_2)) is true.
```

This is intentionally narrower than a blanket unknown-key-share theorem. It establishes only that modeled attributed completion is downstream of modeled exact local identity attribution for the same key / identity / role / session / security-context tuple under the A0 active-network abstraction.

## Ten-query AUTH-v3 result set

The preceding nine scoped properties remained green: replay-record-before-acceptance, client/server completion correspondences, trusted-record presence, authorization-context admission, accepted session-key secrecy under A0/no endpoint compromise, and Finished-direction separation. FM-06 is the tenth query and did not invalidate the retained preceding set.

## Model-to-spec-to-code-to-test anchors

Normative owner:

- `spec/implementation-requirements.md` §4.1 identity-attribution resolver requirements.

Rust implementation:

- `rust/crates/proto/src/auth_v3_iot_core_authz.rs`
- `IotCoreAttributionRecordV1`
- `resolve_iot_core_attribution(...)`

C implementation:

- `c/include/auth/auth_v3_iot_core_authz.h`
- `c/src/proto/auth_v3_iot_core_authz.c`
- `auth_v3_iot_core_attribution_resolve(...)`

Shared executable decision evidence:

- `rust/test-vectors/auth-v3/iot-core-attribution-decisions-v1.txt`
- `rust/crates/proto/tests/auth_v3_iot_core_attribution_corpus.rs`
- `c/tests/test_auth_v3_iot_core_authz.c`

The shared corpus covers exact-local acceptance plus missing, ambiguous, wrong-identity, stale-generation, wrong-role/policy, wrong-audience, and same-holder/different-identity substitution cases. Those executable decisions are complementary evidence, not proof of model/runtime equivalence.

## Evidence-state advancement

```text
FM-06 resolver semantics                          NORMATIVELY OWNED, scoped
FM-06 Rust resolver                               IMPLEMENTED + TESTED
FM-06 C resolver                                  IMPLEMENTED + TESTED
FM-06 shared Rust/C decision corpus               TESTED in green exact-head CI
FM-06 synchronized ProVerif correspondence        MODELED
FM-06 exact-model ProVerif run                    RETAINED — CI #86
FM-06 identity-attribution correspondence         SCOPED FORMALLY ANALYZED under A0
FORMALLY VERIFIED IMPLEMENTATION                  NOT CLAIMED
EXTERNALLY REVIEWED                               NOT CLAIMED
RFC-CLASS DOCUMENTED                              NOT CLAIMED
COMMON-CONFORMANT                                 NOT CLAIMED
DEPLOYMENT-QUALIFIED                              NOT CLAIMED
TD-003                                            OPEN
TD-004                                            OPEN
```

## Explicit limitations

This run does not establish:

- uniqueness or ambiguity detection inside the symbolic resolver;
- correctness of any future wire-visible credential/reference format;
- computational binding between credentials, commitments, public keys, and identities;
- exhaustive unknown-key-share resistance;
- enrollment, delegation, revocation convergence, resumption, or trust-mutation semantics;
- authorization-policy correctness beyond the modeled exact `authz_core` equality boundary;
- parser/model equivalence or Rust/C implementation verification;
- constant-time behavior, RNG quality, memory safety, secure storage, side-channel resistance, or physical MCU resource sufficiency;
- TD-001 independent cryptographic review;
- complete TD-003 attacker/property coverage;
- TD-004 RFC-class completion;
- Common Contract completion or deployment qualification.

The model remains an A0 active-network abstraction with idealized proof primitives. Alias ambiguity, conflicting reference paths, stale resolver caches, authority/provenance namespaces, dynamic compromise, rekey/revocation lifecycle, resumption, privacy properties, and stronger attacker models remain separate evidence obligations.

## Next evidence gate

The primary formal assurance contract still contains stale pre-FM-06 query counts and dependency text. A subsequent documentation-only reconciliation should update that long-lived summary to the ten-query exact-model result and mark FM-06 scoped `FORMALLY ANALYZED`, while preserving all non-claims above. No new theorem should be introduced merely to raise coverage counts; the next formal-model expansion should remain downstream of repository-owned normative/runtime semantics.