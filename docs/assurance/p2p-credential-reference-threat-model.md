# P2P credential/reference mapping threat model

Status: **draft assurance contract**

Scope: `zk239` decentralized trust semantics and the `p2p-iot-core` Common Contract.

This document defines the security boundary between a locally authorized peer reference and the credential/proof material presented during P2P authentication. It does not create a new trust engine, credential format, revocation engine, or transport identity. Existing AUTH, authorization, lineage, revocation, association-admission, and delegation decisions remain authoritative.

## 1. Security objective

A conformant peer MUST accept a P2P authentication result as belonging to a locally authorized peer only when the presented cryptographic credential/proof is mapped to the intended local trust record under the currently selected deployment, profile, suite, authorization generation, credential epoch, lineage, and binding context.

The mapping MUST NOT be inferred from:

- IP/MAC/Bluetooth/serial/USB or other transport address;
- DNS/SNI/hostname or discovery label;
- gateway, relay, proxy, cloud, CA, manufacturer service, or registry assertion by itself;
- human-readable peer name;
- stale cached authorization state outside the profile freshness bound;
- a credential identifier that is valid in a different deployment, audience, lineage, profile, suite, or authorization generation.

Normal AUTH is **NO-LEARNING**: successful authentication can prove possession of already-authorized credential material, but MUST NOT create, widen, replace, or transitively import a local trust record.

## 2. Assets and authorities

Protected assets are:

1. the local trust-record namespace;
2. the binding from a local peer reference to acceptable credential/proof material;
3. authorization scope and generation;
4. credential epoch and lineage/replacement state;
5. delegation depth and provenance;
6. selected Common Contract profile and mandatory security floor;
7. session/association authority derived from successful AUTH;
8. revocation and stale-state decisions.

Authority remains local and non-transitive unless an explicit, bounded, scoped delegation artifact is verified. A high-capability peer does not gain additional trust authority merely because it can consult more infrastructure.

## 3. Mapping model

A local implementation SHOULD model an authorized reference as a bounded record equivalent to:

```text
LocalPeerReference {
    local_reference_id,
    deployment_id,
    allowed_credential_reference_or_commitment,
    allowed_profile_set,
    allowed_suite_set,
    authorization_scope,
    authorization_generation,
    credential_epoch,
    lineage_id,
    delegation_constraints,
    revocation_state,
    freshness_policy
}
```

The exact storage representation is implementation-specific. The security semantics are not.

For an AUTH attempt, the implementation derives or verifies a presented credential reference/commitment from the authenticated proof transcript. Acceptance requires that the authenticated reference resolves to exactly one currently acceptable local trust record for the active security context.

Ambiguous, missing, multiply matching, stale, revoked, rollback-suspect, or context-incompatible mappings MUST fail closed. Implementations MUST NOT use first-match, nearest-match, prefix-match, case-folded textual match, transport-address fallback, or network-discovered alias fallback to repair ambiguity.

## 4. Threats and required behavior

### TM-P2P-01 credential substitution

**Attack:** an adversary presents a valid credential/proof for peer B while claiming or being routed as peer A.

**Required behavior:** the authenticated credential reference/commitment must map to A's authorized local record. A valid proof for B MUST NOT satisfy A's mapping.

### TM-P2P-02 cross-deployment reference reuse

**Attack:** credential material authorized in deployment X is replayed in deployment Y.

**Required behavior:** deployment context is part of the mapping/admission decision. Cross-deployment reuse MUST fail closed unless an explicit authorization artifact grants that deployment scope.

### TM-P2P-03 stale credential replacement

**Attack:** a previously valid credential/reference is used after an authorized replacement or lineage advance.

**Required behavior:** current lineage and credential epoch govern acceptance. Predecessor credentials MUST NOT regain authority merely because their cryptographic proof is still valid.

### TM-P2P-04 authorization-generation rollback

**Attack:** an old mapping snapshot is restored after authorization policy has advanced.

**Required behavior:** the mapping is valid only under the current authorization generation and rollback/restart continuity policy. A generation mismatch MUST invalidate dependent association/resumption/release authority according to the owning lifecycle contracts.

### TM-P2P-05 revocation bypass by alternate reference

**Attack:** a revoked peer presents an alias, alternate identifier, prior reference, or transport address to reach the same credential authority.

**Required behavior:** aliases and transport metadata cannot bypass credential/reference revocation. Revocation applies according to the canonical credential/lineage authority defined by the owning revocation contract.

### TM-P2P-06 ambiguous local mapping

**Attack:** two local records accept the same credential/reference or an attacker crafts a value matching more than one record.

**Required behavior:** ambiguity MUST fail closed. There is no "best" or first-match resolution in the mandatory Common Contract.

### TM-P2P-07 identifier truncation or prefix collision

**Attack:** a constrained parser compares only a prefix/truncated identifier and accepts a collision.

**Required behavior:** the profile defines the complete authenticated reference/commitment length. Comparison MUST cover the complete canonical value. Resource constraints may bound identifier size but may not weaken comparison semantics.

### TM-P2P-08 transport-identity confusion

**Attack:** a peer is accepted because it arrives from a known address/interface despite presenting the wrong authenticated credential reference.

**Required behavior:** transport address is routing metadata, not protocol identity. Address continuity cannot repair a credential/reference mismatch.

### TM-P2P-09 infrastructure authority injection

**Attack:** DNS, cloud, gateway, CA, manufacturer registry, or discovery infrastructure supplies a different peer-to-credential mapping during AUTH.

**Required behavior:** auxiliary infrastructure may help discovery or synchronization, but MUST NOT replace the locally authorized mapping for already-authorized Common Contract AUTH. If required local state is unavailable or stale beyond policy, fail closed/restricted according to local policy rather than outsourcing the root decision.

### TM-P2P-10 transitive trust import

**Attack:** trusted peer A asserts that B should be trusted, and B is accepted without a valid bounded delegation artifact.

**Required behavior:** local trust is non-transitive. Only explicit delegation satisfying scope, depth, provenance, freshness, revocation, lineage, and authorization-generation requirements can create or extend authorization.

### TM-P2P-11 delegation widening

**Attack:** a valid delegated reference is reused outside its audience, deployment, scope, depth, profile, or validity interval.

**Required behavior:** delegated mapping MUST remain bounded by all delegation constraints. Delegation cannot repair an independently failing lifecycle fact.

### TM-P2P-12 profile/suite confusion

**Attack:** the same reference is used to negotiate a weaker profile or suite than the local record permits.

**Required behavior:** a matching credential reference is necessary but not sufficient. Profile/suite compatibility and downgrade resistance remain independently mandatory.

### TM-P2P-13 reference oracle / NO-LEARNING violation

**Attack:** unauthenticated or failed AUTH attempts are used to enumerate whether candidate references exist in the local trust store.

**Required behavior:** normal AUTH error behavior SHOULD avoid distinguishable responses that reveal trust-record membership beyond what the protocol necessarily exposes. Failed authentication MUST NOT mutate the trust store or create a learned mapping.

### TM-P2P-14 constrained-store exhaustion

**Attack:** an adversary induces unbounded alias/reference insertion or trust-store growth.

**Required behavior:** mandatory constrained profiles use bounded local trust representation. Network input cannot allocate permanent trust records during normal AUTH. Enrollment/delegation paths must enforce their own bounded capacity and authority policy.

### TM-P2P-15 restart/persistence divergence

**Attack:** credential mapping state survives restart while revocation/generation/lineage state rolls back, or vice versa.

**Required behavior:** restart continuity is a lifecycle fact. If the implementation cannot establish mutually consistent persisted mapping and lifecycle state, dependent authority fails closed until repaired through the owning authenticated recovery/enrollment mechanism.

### TM-P2P-16 privacy-linkability amplification

**Attack:** a stable credential reference is unnecessarily exposed on the wire and used to track a peer across sessions or transports.

**Required behavior:** implementations SHOULD use the protocol's authenticated reference/commitment construction and avoid exposing local reference identifiers directly. Privacy-preserving representation must not weaken exact local mapping or create ambiguous acceptance. Any repeated-identifier behavior remains subject to the resumption/privacy contracts.

## 5. Common Contract invariants

The following invariants apply across constrained and high-capability peers:

1. **Same mapping semantics:** MCU and edge implementations make the same accept/reject decision from equivalent authenticated facts.
2. **No weaker constrained identity:** bounded storage or shorter implementation code cannot substitute transport identity, partial comparisons, or external authority for cryptographic mapping.
3. **No hidden infrastructure dependency:** already-authorized AUTH can resolve its local mapping without CA/cloud/DNS/central-registry/gateway approval.
4. **No trust mutation during normal AUTH:** authentication proves an already-authorized mapping; it does not enroll one.
5. **No transitive trust:** peer assertions do not import trust without explicit delegation verification.
6. **Lifecycle composition:** mapping success cannot override revocation, stale authorization, generation mismatch, lineage mismatch, restart/rollback failure, replay failure, profile/suite incompatibility, or required binding failure.
7. **Transport independence:** route/address changes do not change protocol identity when authenticated mapping remains valid; route/address stability does not preserve identity when authenticated mapping fails.
8. **Fail-closed ambiguity:** zero or multiple acceptable mappings are rejection conditions.

## 6. Required qualification evidence

Before `zk239` can claim this threat-model leg as executable rather than documentary, shared Rust/C qualification SHOULD include at least:

| Case | Mutation | Required result |
|---|---|---|
| Q-MAP-01 | exact current local mapping | accept if all other lifecycle facts pass |
| Q-MAP-02 | credential B presented as peer A | reject |
| Q-MAP-03 | correct credential, wrong deployment | reject |
| Q-MAP-04 | predecessor credential after lineage replacement | reject |
| Q-MAP-05 | stale authorization generation | fail closed |
| Q-MAP-06 | revoked canonical credential with alternate alias | reject |
| Q-MAP-07 | two local records match one authenticated reference | reject |
| Q-MAP-08 | prefix/truncation collision | reject |
| Q-MAP-09 | trusted transport address, wrong credential | reject |
| Q-MAP-10 | cloud/gateway mapping differs from local mapping | local decision wins or fail closed; no remote authority substitution |
| Q-MAP-11 | peer assertion without delegation artifact | reject trust mutation |
| Q-MAP-12 | delegated mapping outside scope/depth/audience | reject |
| Q-MAP-13 | matching credential with incompatible profile/suite | reject |
| Q-MAP-14 | failed AUTH repeated with unknown references | no trust-store mutation |
| Q-MAP-15 | restart with inconsistent mapping/lifecycle persistence | fail closed |
| Q-MAP-16 | address changes, authenticated mapping unchanged | identity decision remains based on authenticated mapping |

The corpus SHOULD be canonical and consumed by both Rust and C implementations. Tests should assert both the final decision and that normal AUTH does not mutate local trust state.

## 7. Formal-analysis obligations

This document does not claim formal proof. TD-003 remains open.

Future synchronized formal models should distinguish at least:

- authenticated credential possession;
- local authorized mapping;
- delegation authorization;
- current lifecycle authority;
- accepted association.

Useful properties include:

```text
AcceptedAssociation(peer)
    ==> AuthenticatedCredential(peer)
        && LocalAuthorizedMapping(peer)
        && CurrentLifecycleAuthority(peer)

NormalAuthSuccess(peer)
    =/=> TrustRecordCreated(peer)

AcceptedAssociation(peerA)
    =/=> AuthenticatedCredential(peerB)   when peerA != peerB
```

Compromise models must state which local records, credential secrets, delegation authorities, and persistence state the attacker controls; a symbolic mapping result must not be interpreted as constant-time, RNG, storage-integrity, or physical-target evidence.

## 8. Traceability

This threat model composes with, but does not supersede:

- the AUTH transcript/context-binding specification;
- local trust and NO-LEARNING requirements;
- enrollment/commissioner/delegation authority contracts;
- authorization-generation and lineage classifiers;
- revocation convergence/freshness rules;
- CORE association-admission decisions;
- resumption authorization decisions;
- `p2p-iot-core` profile negotiation and downgrade rules.

When two contracts disagree, the safer fail-closed behavior applies until the normative specifications are reconciled. Documentation MUST NOT be used to claim implementation, interoperability, formal analysis, constrained-target measurement, or external review that has not actually occurred.

## 9. Exit-evidence effect

This document closes the **documented threat-model definition** portion of the `zk239` credential/reference-mapping exit requirement. It does **not** by itself raise `zk239` to the next scoring threshold. The phase still requires executable shared mapping qualification, revocation/epoch convergence evidence, broader formal/model traceability, and the remaining Common Contract evidence declared by `docs/roadmaps/improvement-roadmap.md`.
