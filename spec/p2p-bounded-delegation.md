# P2P Bounded Delegation Decision Contract

Status: implementation-linked normative decision contract for the Common Contract P2P trust boundary. This document defines local decision semantics only; it does not define a grant wire format or credential signature format.

## 1. Non-transitive trust root

Trust is local and non-transitive by default. A verifier MUST NOT accept a delegation merely because some already-trusted peer claims that the delegation issuer is trustworthy.

For a delegation to be considered, the verifier must have both an accepted trust relationship for the issuer and evidence that the relationship is rooted in the verifier's own local trust state. `issuer_trusted=true` with `issuer_trust_local=false` MUST fail closed with `ISSUER_TRUST_NOT_LOCAL`.

The explicit grant is the bounded authority path from the locally trusted issuer to the authenticated holder. Normal AUTH remains NO-LEARNING; accepting a delegation MUST NOT add the holder or issuer to persistent trust.

## 2. Authorization-generation provenance

Delegated authority is authorization evidence and MUST participate in the same authorization-generation lifecycle as direct authorization.

Before accepting a grant, the verifier MUST establish both:

- `authorization_generation_bound=true`: authenticated grant or authenticated local metadata binds the delegated authority to a specific authorization generation; and
- `authorization_generation_current=true`: that bound generation equals the verifier's current locally authoritative generation.

These are separate facts. A current local generation counter does not prove that a grant belongs to that generation. Missing provenance MUST fail closed with `AUTHORIZATION_GENERATION_UNBOUND`; authenticated provenance for an older generation MUST fail closed with `AUTHORIZATION_GENERATION_STALE`.

A peer assertion, transport address, connection identity, gateway, CA, cloud service, registry lookup, or successful AUTH exchange MUST NOT manufacture this binding. Generation allocation and advancement remain owned by the existing local authorization lifecycle authority; this decision consumes those facts and does not create another authority.

## 3. Required grant bounds

After establishing a local issuer trust root, the verifier MUST require: authenticated holder; present and integrity-valid grant; matching scope, audience and deployment/domain; current validity interval; authenticated and current authorization generation; current policy epoch; current revocation view and no explicit revocation; current authorization lineage; delegation depth within the locally accepted bound; explicit redelegation permission whenever redelegation is requested; and no rollback suspicion in local lifecycle state.

## 4. Fail-closed precedence

The shared Rust/C classifier applies this precedence:

1. rollback suspicion;
2. issuer not trusted;
3. issuer trust not locally rooted;
4. holder not authenticated;
5. missing/invalid grant;
6. scope/audience/deployment mismatch;
7. stale validity interval;
8. authorization generation unbound;
9. authorization generation stale;
10. stale policy epoch;
11. stale revocation view or explicit revocation;
12. stale lineage;
13. depth overflow;
14. forbidden redelegation;
15. otherwise `ACCEPT`.

## 5. Retained authority and lifecycle re-evaluation

`ACCEPT` authorizes only the bounded delegation decision evaluated against the lifecycle facts supplied to that decision. It MUST NOT be interpreted as an indefinitely valid capability merely because the authenticated association, transport connection, session keys, or a cached grant remain present.

Before a retained association uses delegated authority for new security-sensitive application traffic, the owning authorization/association lifecycle MUST re-establish the freshness conditions required by that operation. In particular, authorization-generation currentness, revocation currentness, lineage currentness, and any replay/restart/key-usage continuity required by the association-admission contract MUST remain current.

If re-evaluation fails closed, delegated authority for new protected application use is immediately invalid. An implementation MUST NOT continue to authorize application traffic from the earlier delegation `ACCEPT` solely because cryptographic session material or a transport connection still exists. Reauthentication by itself MUST NOT repair stale authorization generation, revocation, lineage, replay, restart, key-usage, or rollback state.

This contract does not create a second lifecycle authority. The bounded-delegation classifier continues to consume delegation-local facts; association admission and the existing authorization lifecycle own their respective continuity/freshness facts. Implementations MAY retain key material transiently for bounded shutdown or cleanup when another normative profile permits that behavior, but such retention MUST NOT confer application authority.

## 6. Infrastructure independence and constrained floor

The decision consumes already-verified local facts. Evaluating the mandatory decision MUST NOT require CA, DNS, Internet access, manufacturer cloud, blockchain, central registry lookup, or gateway approval when the verifier already has sufficient authorized local state.

Infrastructure MAY distribute or refresh trust, grants, revocation state, authorization-generation state, or policy state outside this decision. Loss of infrastructure does not convert unbound or stale state into current state and does not authorize transitive trust inference. A higher-capability peer MAY perform additional optional work, but it MUST NOT become authorization-generation authority for a constrained peer merely because it has more compute or connectivity.

The same retained-authority rule applies to constrained↔constrained and constrained↔higher-capability peers. A higher-capability peer MUST NOT preserve delegated application authority for a constrained peer by substituting cloud, gateway, transport, or cached assertions for locally required lifecycle freshness.

## 7. Canonical conformance corpora

`rust/test-vectors/p2p/bounded-delegation-v3.txt` is the current canonical bounded-delegation decision corpus and is consumed by both Rust and C implementations. Version 3 adds independent negative decisions for unbound and stale authorization generations. Versions 1 and 2 remain historical evidence for earlier decision surfaces.

The separate `rust/test-vectors/p2p/common-contract-lifecycle-v4.txt` corpus exercises Common Contract association-lifecycle composition across constrained and higher-capability peer classes, including authorization-generation, revocation, lineage, replay, restart, and key-usage continuity. It does not replace the bounded-delegation corpus and MUST NOT be read as proof of a delegation credential wire protocol.

A future executable retained-delegation lifecycle corpus SHOULD demonstrate at least the temporal sequence `delegation accepted -> association established -> required lifecycle fact becomes stale/revoked -> authority invalidated -> subsequent protected application use rejected` in both Rust and C. Until such evidence exists, this section is a normative lifecycle requirement rather than a claim that temporal delegated-authority invalidation is independently qualified.

## 8. Evidence boundary

This contract and the bounded-delegation corpus demonstrate wire-neutral decision semantics and deterministic Rust/C decision parity when their respective tests execute. The Common Contract lifecycle corpus provides separate cross-class decision evidence for association lifecycle prerequisites. Together they define composition requirements; they do not establish a cryptographic delegation-token encoding, temporal retained-delegation qualification, physical constrained-target interoperability, secure storage, formal proof of the composed lifecycle, independent cryptographic review, RFC/IETF status, or deployment qualification.
