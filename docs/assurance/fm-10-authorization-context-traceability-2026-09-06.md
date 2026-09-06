# FM-10 authorization-context traceability checkpoint — 2026-09-06

Status: **implementation-traceable / not newly formally analyzed**.

This checkpoint narrows TD-003 for FM-10 (authentication/authorization separation) after commit `e82468eb053bd0c8cb55d8fbbcfb31c67410bf6d` made the `iot-core` local attribution resolver reject semantically invalid authorization contexts before any credential-reference lookup can produce an authorization decision.

It does **not** claim a new ProVerif result, implementation verification, computational proof, independent review, RFC status, or deployment qualification. The current AUTH-v3 symbolic model still treats concrete authorization admission as an abstraction boundary, so a fresh retained formal result would be required before any edited model text could be called `FORMALLY ANALYZED`.

## 1. Property statement

FM-10 requires that successful authentication or possession proof alone must not imply authorization outside the locally accepted holder, audience, policy, scope, lineage, and freshness context.

For draft `iot-core` authorization context v1, the concrete fail-closed rule is:

```text
successful attribution / authorization acceptance
        requires
semantic authorization-context validity
        AND
one unambiguous pre-existing credential reference
        AND
expected peer identity match
        AND
exact local holder/audience/role-policy/scope/generation/epoch match
```

Normal AUTH remains NO-LEARNING. A failed or successful resolver call must not create aliases, repair trust records, enroll a credential, expand authorization, or mutate trusted state.

## 2. Normative specification correspondence

Primary specification:

- `spec/iot-core-authorization-context.md`

The v1 schema requires:

- non-zero holder binding;
- non-zero audience;
- non-zero role-policy identifier;
- `scope_bits == SECURE_ASSOCIATION`;
- non-zero authorization generation;
- non-zero policy epoch;
- non-zero revocation epoch;
- local authorization evaluation beyond canonical encoding validity;
- exact local evaluation of holder, audience, role-policy/proof, lineage, revocation, and deployment restrictions.

This preserves the repository-wide rule that authentication, authorization, and trust mutation are separate semantics.

## 3. Rust implementation correspondence

Primary implementation:

- `rust/crates/proto/src/auth_v3_iot_core_authz.rs`

Relevant boundary:

```text
validate_iot_core_authorization_context(context)
        ↓ success only
resolve_iot_core_attribution(...)
        ↓
unique credential-reference selection
        ↓
expected peer identity equality
        ↓
exact authorization tuple equality
        ↓
record returned
```

As of `e82468e`, `resolve_iot_core_attribution` invokes `validate_iot_core_authorization_context` before candidate lookup. Any semantic validation failure maps to `IotCoreAttributionError::AuthorizationMismatch`.

This matters because equality against a malformed local tuple is not sufficient authorization evidence: semantic validity is a prerequisite to attribution.

## 4. C implementation correspondence

Primary implementation:

- `c/src/proto/auth_v3_iot_core_authz.c`

The C resolver now mirrors the Rust decision order:

```text
auth_v3_iot_core_authz_validate(context)
        ↓ AUTH_V3_IOT_CORE_AUTHZ_OK only
credential-reference resolution
        ↓
identity comparison
        ↓
authorization tuple comparison
```

Invalid semantic context state maps to `AUTH_V3_IOT_CORE_ATTRIBUTION_AUTHORIZATION_MISMATCH`, preserving cross-language decision vocabulary without introducing a new wire error.

## 5. Symbolic-model correspondence

Current synchronized AUTH-v3 model pair:

- `rust/models/proverif/zk_arche_auth_v3_draft.pv`
- `c/models/proverif/zk_arche_auth_v3_draft.pv`

The model explicitly documents `AuthorizationContextAdmittedV3` as an abstraction boundary for concrete Rust/C parsing, profile validation, semantic checks, and exact-byte hashing.

The server path emits `AuthorizationContextAdmittedV3` before the modeled local attribution relation and requires `authz_in = authz_core` before `IdentityAttributionResolvedV3`.

Therefore the current correspondence is:

```text
concrete semantic validator + exact local tuple resolver
        ↓ abstracted by
AuthorizationContextAdmittedV3 + authz_in = authz_core
        ↓
IdentityAttributionResolvedV3
        ↓
ServerAttributedCompleteV3
```

This is an **abstraction correspondence**, not proof that every malformed concrete context is impossible in the symbolic model. The symbolic term `authz_core` does not encode the seven v1 semantic field predicates individually.

## 6. Claim boundary

Evidence status after this checkpoint:

```text
FM-10 property intent                 = DEFINED
normative iot-core authorization rule = IMPLEMENTED in draft spec
Rust semantic admission boundary      = IMPLEMENTED
C semantic admission boundary         = IMPLEMENTED
Rust/C decision ordering              = IMPLEMENTATION-TRACEABLE
AUTH-v3 model abstraction boundary    = MODELED
field-by-field authz semantics         = NOT MODELED
fresh exact-model formal run           = NOT EXECUTED by this checkpoint
FM-10 formally analyzed end-to-end     = NOT CLAIMED
```

The current retained AUTH-v3 formal evidence may continue to support the previously modeled abstraction, but it must not be extended to claim formal proof of the new concrete semantic-validator ordering without a model change and retained exact-model rerun.

## 7. Required next evidence

The smallest dependency-ready next steps are:

1. add explicit Rust/C negative tests proving that each semantically invalid context fails attribution even when a local record contains the same malformed values;
2. decide whether FM-10 should remain an admission abstraction or whether the canonical symbolic model should represent the individual v1 semantic predicates;
3. if the model changes, synchronize both model copies and run the repository formal lane at the exact resulting HEAD;
4. retain model blob, tool/version, repository HEAD, query inventory, result, and abstraction gaps;
5. keep runtime semantic tests distinct from symbolic analysis claims.

A future model expansion should prefer a small predicate such as `authorization_context_valid_v1(...)` or mechanically generated field predicates rather than reproducing parser byte logic in ProVerif. The objective is to prove authorization semantics, not to pretend the symbolic model verifies parser implementation details.
