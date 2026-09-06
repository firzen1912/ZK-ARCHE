# P2P credential/reference mapping traceability — 2026-09-06

Status: assurance checkpoint for `zk239`; no profile promotion or new maturity claim.

## Threat boundary

A credential/reference value is an opaque lookup key into pre-existing local trust state. It is not protocol identity, authorization, or a trust-mutation primitive. For already-authorized Common Contract AUTH, the mandatory decision remains local and must not depend on CA, cloud, DNS, manufacturer service, gateway approval, or a central registry lookup.

A successful mapping requires a unique pre-existing local record, an exact match to the peer identity expected by the authenticated protocol context, a semantically valid authorization context, and exact locally accepted holder/audience/role-policy/scope/generation/policy/revocation state. Revocation, lineage, replay/restart continuity, usage continuity, rollback, and required binding checks remain independent fail-closed gates.

Normal AUTH is NO-LEARNING. Missing or ambiguous references, mismatched identity, stale authorization state, malformed authorization contexts, or transport-only identity hints MUST NOT create, replace, merge, or broaden trust.

## Attacker cases

The mapping boundary must fail closed against:

- zero matches and duplicate-reference ambiguity;
- one credential reference resolving to a peer other than the authenticated/expected peer;
- a syntactically present but semantically invalid authorization context, including matching malformed local values;
- stale authorization generation, policy epoch, or revocation epoch;
- alternate aliases or transport addresses used to bypass revocation or lineage replacement;
- trust imported transitively from another peer without explicit bounded delegation accepted locally;
- infrastructure-provided identity mappings that disagree with or replace local authorization state;
- persisted mapping state that survives while associated lifecycle authority rolls back or is revoked.

Transport address, socket identity, BLE/MAC/CAN identifiers, DNS names, and discovery labels are routing/discovery metadata, not Common Contract protocol identity.

## Current repository traceability

Concrete local mapping evidence already exists in both implementation lanes:

- Rust resolver: `rust/crates/proto/src/auth_v3_iot_core_authz.rs`
- C resolver: `c/src/proto/auth_v3_iot_core_authz.c`
- shared decision corpus: `rust/test-vectors/auth-v3/iot-core-attribution-decisions-v1.txt`
- Rust corpus consumer: `rust/crates/proto/tests/auth_v3_iot_core_attribution_corpus.rs`
- C corpus consumer: `c/tests/test_auth_v3_iot_core_authz.c`
- normative authorization schema: `spec/iot-core-authorization-context.md`

The shared corpus covers exact local resolution, missing reference, duplicate-reference ambiguity, wrong peer, stale generation, wrong role policy, wrong audience, and same-holder/different-peer reference cases. The resolver boundary separately validates the authorization context before attribution, so matching malformed values do not become authorization.

The P2P Common Contract lifecycle qualification composes successful local authority with fail-closed generation, revocation, lineage, replay/restart, usage-continuity, binding, and protected-DATA retained-authority checks. Mapping success is therefore not durable authority by itself.

## Qualification gaps retained

This checkpoint closes the *documented threat-model/traceability* portion of the `zk239` credential/reference-mapping exit evidence, but it does not justify a score increase by itself. Remaining evidence includes at least:

- explicit shared Rust/C negatives for every mapping threat above, including transport non-authority and malformed-context/malformed-local-record combinations;
- revocation/epoch convergence across disconnected peers (`zk214` dependency);
- broader formal/model traceability for P2P trust and authorization decisions;
- constrained bounded-store/target evidence where `zk240`/`zk241` require it;
- independent review where TD-001 applies.

No new `TESTED`, `FORMALLY ANALYZED`, `COMMON-CONFORMANT`, `MEASURED`, `EXTERNALLY REVIEWED`, or `DEPLOYMENT-QUALIFIED` claim is made by this document.
