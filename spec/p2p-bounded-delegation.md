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

## 5. Infrastructure independence and constrained floor

The decision consumes already-verified local facts. Evaluating the mandatory decision MUST NOT require CA, DNS, Internet access, manufacturer cloud, blockchain, central registry lookup, or gateway approval when the verifier already has sufficient authorized local state.

Infrastructure MAY distribute or refresh trust, grants, revocation state, authorization-generation state, or policy state outside this decision. Loss of infrastructure does not convert unbound or stale state into current state and does not authorize transitive trust inference. A higher-capability peer MAY perform additional optional work, but it MUST NOT become authorization-generation authority for a constrained peer merely because it has more compute or connectivity.

## 6. Canonical conformance corpus

`rust/test-vectors/p2p/bounded-delegation-v3.txt` is the current canonical decision corpus and is consumed by both Rust and C implementations. Version 3 adds independent negative decisions for unbound and stale authorization generations. Versions 1 and 2 remain historical evidence for earlier decision surfaces.

## 7. Evidence boundary

This contract and corpus demonstrate wire-neutral decision semantics and deterministic Rust/C decision parity when their respective tests execute. They do not establish a cryptographic delegation-token encoding, physical constrained-target interoperability, secure storage, formal proof, independent cryptographic review, RFC/IETF status, or deployment qualification.
