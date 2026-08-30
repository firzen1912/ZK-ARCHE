# FM-06 Identity-Attribution Formal Traceability — 2026-08-29

Status: **retained scoped FORMALLY ANALYZED evidence for exact-head `ae1eeb47b830996470beb489fe3875e5fc2635a2`**.

This note records the bounded promotion of FM-06 from a defined resolver requirement into the synchronized AUTH-v3 symbolic model and the exact hosted qualification result for that model. The result is a scoped symbolic correspondence under the repository's A0 active-network abstraction; it is not implementation verification and does not establish stronger computational, parser, lifecycle, or deployment claims.

## Scope promoted

The repository owns a local, read-only identity-attribution resolver contract in `spec/implementation-requirements.md`, matching Rust and C implementations, and a shared Rust/C attribution decision corpus. Those concrete surfaces require exact local attribution across credential/reference selection, peer identity, holder binding, audience, role/policy, scope, authorization generation, policy epoch, and revocation epoch while preserving normal AUTH as NO-LEARNING.

The synchronized AUTH-v3 model introduces a distinct symbolic `peerid` type so peer identity is not collapsed into the authentication public key. It records:

```text
TrustedAttributionPresent(public_key, peer_identity, role, authz_context)
IdentityAttributionResolvedV3(public_key, peer_identity, role, session, security_context)
ServerAttributedCompleteV3(public_key, peer_identity, role, session, security_context, kc_context)
```

The server path requires the active modeled authorization context to equal the pre-authorized `authz_core` tuple before emitting `IdentityAttributionResolvedV3`. At authenticated completion it emits `ServerAttributedCompleteV3` with the same public-key / peer-identity / role / security-context tuple.

The FM-06 correspondence is:

```text
ServerAttributedCompleteV3(key, identity, role, sid, secctx, kcctx)
    ==>
IdentityAttributionResolvedV3(key, identity, role, sid, secctx)
```

This is intentionally narrower than a blanket unknown-key-share claim. It checks that modeled attributed completion is downstream of exact local identity attribution rather than treating successful possession of a public key as sufficient identity attribution by itself.

## Exact hosted qualification

Exact commit:

```text
ae1eeb47b830996470beb489fe3875e5fc2635a2
```

Hosted workflow:

```text
ZK-ARCHE CI #86
run id: 33281604442
conclusion: success
```

All five applicable jobs completed successfully:

```text
Rust lane — fmt, check, test, clippy, audit
C lane — build, tests, static analysis, sanitizers
Formal lane — legacy v2 + AUTH v3 + replay continuity gates
Release qualification — Rust/C interop & security gate
CI complete — required-lane gate
```

The formal lane ran ProVerif 2.05, passed synchronized-model checking, executed the retained legacy-v2 expected-negative regression, executed the draft AUTH-v3 model fail-closed, and executed the replay-continuity model fail-closed.

The exact AUTH-v3 model produced **10 query results and all 10 were `true`**. The new FM-06 result was:

```text
RESULT event(ServerAttributedCompleteV3(cpk_3,identity,allowed_3,sid_3,secctx_2,kcctx_2))
==> event(IdentityAttributionResolvedV3(cpk_3,identity,allowed_3,sid_3,secctx_2)) is true.
```

The other retained AUTH-v3 results continued to pass, including replay-record-before-acceptance, client/server completion agreement, trusted-record presence, authorization-context admission, scoped session-key secrecy, and finished-direction separation. This exact run therefore extends the retained AUTH-v3 evidence set from nine to ten successful scoped queries without invalidating the prior query set.

Formal artifact:

```text
name: formal-proverif-evidence
artifact id: 9723169047
artifact digest: sha256:fb4a385a50a41b87fc911ac43ddca92f511601c1b05bd22b9b1c332dec46a8a4
```

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

## Exact model identity

The qualified model blob is `2f3817b5fb847ef948e4effab4b7d9871adc2e14` and is byte-identical at:

```text
rust/models/proverif/zk_arche_auth_v3_draft.pv
c/models/proverif/zk_arche_auth_v3_draft.pv
```

The hosted formal lane independently enforced synchronization before executing ProVerif.

## Explicit non-claims and remaining gaps

This retained FM-06 result does **not** establish:

- uniqueness or ambiguity detection inside the symbolic resolver;
- correctness of a future wire-visible credential/reference format;
- enrollment, delegation, revocation convergence, resumption, or trust mutation semantics;
- computational binding between credentials, commitments, public keys, and identities;
- parser/model equivalence;
- Rust/C implementation verification;
- authorization-policy correctness beyond the modeled exact `authz_core` equality boundary;
- protection against every unknown-key-share construction;
- constant-time behavior, RNG quality, or memory safety;
- TD-001 independent cryptographic review;
- TD-002 physical constrained-target measurements;
- complete TD-003 property/attacker coverage or model-to-runtime equivalence;
- TD-004 RFC-class completion;
- Common Contract completion, RFC-class status, or deployment qualification.

The model remains an A0 active-network abstraction with idealized proof primitives. Alias ambiguity, conflicting reference paths, stale resolver caches, authority/provenance namespaces, dynamic compromise, rekey/revocation lifecycle, resumption, privacy properties, and stronger attacker models remain separate evidence obligations.

## Evidence state

For this exact model and exact hosted run, FM-06 may now be described as **FORMALLY ANALYZED within the stated symbolic scope**. It must not be described as formally verified implementation behavior, computationally proven secure, externally reviewed, Common Contract complete, RFC-class, or deployment-qualified.
