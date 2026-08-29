# FM-06 Identity-Attribution Formal Traceability — 2026-08-29

Status: **model promotion pending exact-head hosted formal qualification**.

This note records the bounded promotion of FM-06 from a defined resolver requirement into the synchronized AUTH-v3 symbolic model. It does not claim that the edited model has passed ProVerif until the exact resulting `dev` commit completes the hosted formal lane successfully.

## Scope promoted

The repository already owns a local, read-only identity-attribution resolver contract in `spec/implementation-requirements.md`, matching Rust and C implementations, and a shared Rust/C attribution decision corpus. Those concrete surfaces require exact local attribution across credential/reference selection, peer identity, holder binding, audience, role/policy, scope, authorization generation, policy epoch, and revocation epoch while preserving normal AUTH as NO-LEARNING.

The synchronized AUTH-v3 model now introduces a distinct symbolic `peerid` type so peer identity is not collapsed into the authentication public key. It records:

```text
TrustedAttributionPresent(public_key, peer_identity, role, authz_context)
IdentityAttributionResolvedV3(public_key, peer_identity, role, session, security_context)
ServerAttributedCompleteV3(public_key, peer_identity, role, session, security_context, kc_context)
```

The server path requires the active modeled authorization context to equal the pre-authorized `authz_core` tuple before emitting `IdentityAttributionResolvedV3`. At authenticated completion it emits `ServerAttributedCompleteV3` with the same public-key / peer-identity / role / security-context tuple.

The added correspondence is:

```text
ServerAttributedCompleteV3(key, identity, role, sid, secctx, kcctx)
    ==>
IdentityAttributionResolvedV3(key, identity, role, sid, secctx)
```

This is intentionally narrower than a blanket unknown-key-share claim. It checks that the modeled attributed completion is downstream of exact local identity attribution rather than treating successful possession of a public key as sufficient identity attribution by itself.

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

Shared decision evidence:

- `rust/test-vectors/auth-v3/iot-core-attribution-decisions-v1.txt`
- `rust/crates/proto/tests/auth_v3_iot_core_attribution_corpus.rs`
- `c/tests/test_auth_v3_iot_core_authz.c`

The shared corpus covers exact-local acceptance plus missing, ambiguous, wrong-identity, stale-generation, wrong-role/policy, wrong-audience, and same-holder/different-identity substitution cases. These executable decisions remain separate evidence from the symbolic correspondence.

## Explicit non-claims and remaining gaps

This model promotion does **not** establish:

- uniqueness or ambiguity detection inside the symbolic resolver;
- correctness of a future wire-visible credential/reference format;
- enrollment, delegation, revocation convergence, resumption, or trust mutation semantics;
- computational binding between credentials, commitments, public keys, and identities;
- parser/model equivalence;
- Rust/C implementation verification;
- authorization-policy correctness beyond the modeled exact `authz_core` equality boundary;
- protection against every unknown-key-share construction;
- TD-001 independent cryptographic review;
- Common Contract completion, RFC-class status, or deployment qualification.

The model remains an A0 active-network abstraction with idealized proof primitives. Alias ambiguity, conflicting reference paths, stale resolver caches, and authority/provenance namespaces remain executable/specification concerns unless and until a stronger symbolic state relation is justified.

## Qualification rule

The model blob written by this packet is `2f3817b5fb847ef948e4effab4b7d9871adc2e14` and must be byte-identical at both:

```text
rust/models/proverif/zk_arche_auth_v3_draft.pv
c/models/proverif/zk_arche_auth_v3_draft.pv
```

This note may be upgraded from **model promotion pending exact-head hosted formal qualification** to retained scoped `FORMALLY ANALYZED` evidence only after the exact resulting commit passes:

1. `scripts/sync-formal-models.sh --check`;
2. ProVerif 2.05 AUTH-v3 fail-closed execution with no false/unproved result;
3. the repository's complete required hosted CI gate.

Until then, the prior nine-query retained AUTH-v3 run remains the latest retained formal result.