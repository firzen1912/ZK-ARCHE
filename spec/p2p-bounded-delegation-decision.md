# Bounded P2P Delegation Decision

Status: implementation-backed draft contract. This document does not promote `p2p-iot-core`, claim deployment qualification, or create an external trust dependency.

## 1. Purpose and boundary

This contract defines the decision boundary already implemented by the Rust and C P2P delegation classifiers and exercised by `rust/test-vectors/p2p/bounded-delegation-v3.txt`.

Delegation is authorization, not authentication and not trust mutation. A conformant verifier MUST authenticate the holder independently and MUST establish issuer trust from its own local trust state before evaluating a delegation. A delegation MUST NOT make an otherwise-untrusted issuer trusted, MUST NOT make third-party trust transitive, and MUST NOT turn transport addressing, cloud reachability, registry reachability, or gateway approval into protocol identity or authority.

The mandatory decision is wire-neutral. Higher-capability peers MAY use richer storage, indexing, revocation distribution, or policy tooling, but those facilities MUST NOT weaken the decision accepted by the least-capable conformant peer.

## 2. Required input facts

The classifier consumes already-verified facts. Cryptographic grant parsing and signature/proof verification occur before this boundary.

A verifier MUST bind its decision to all of the following facts represented by the current Rust/C implementations:

- issuer is trusted by the verifier;
- issuer trust is local to the verifier rather than inferred transitively;
- holder authentication is complete;
- delegation grant is present and its integrity is valid;
- requested scope, audience, and deployment match the grant;
- grant validity interval is current;
- authorization generation is bound and current;
- protocol epoch is current;
- revocation knowledge is current and the grant is not explicitly revoked;
- delegation lineage is current;
- delegation depth is within the configured bound;
- requested redelegation is explicitly permitted; and
- no rollback is suspected.

A caller MUST NOT treat `ACCEPT` as evidence that any omitted fact was checked elsewhere unless that fact is outside this contract by construction.

## 3. Fail-closed decision order

The current implementation uses deterministic precedence so compound faults produce a stable cross-language decision. The following order is normative for the implementation-backed v3 contract:

1. inconsistent local-trust facts (`issuer_trust_local && !issuer_trusted`) -> `DENY / INVALID_FACTS`;
2. rollback suspected -> `DENY / ROLLBACK_SUSPECTED`;
3. issuer not trusted -> `DENY / ISSUER_UNTRUSTED`;
4. issuer trust not local -> `DENY / ISSUER_TRUST_NOT_LOCAL`;
5. holder unauthenticated -> `DENY / HOLDER_UNAUTHENTICATED`;
6. grant missing or invalid -> `DENY / GRANT_MISSING` or `GRANT_INVALID`;
7. scope, audience, deployment, or validity mismatch -> the corresponding deny reason;
8. authorization generation unbound or stale -> the corresponding deny reason;
9. stale epoch -> `DENY / EPOCH_STALE`;
10. stale revocation knowledge -> `DENY / REVOCATION_STALE`;
11. explicit revocation -> `DENY / REVOKED`;
12. stale lineage -> `DENY / LINEAGE_STALE`;
13. excessive delegation depth -> `DENY / DEPTH_EXCEEDED`;
14. requested but unauthorized redelegation -> `DENY / REDELEGATION_FORBIDDEN`;
15. otherwise -> `ACCEPT / CURRENT`.

Implementations claiming this contract MUST preserve the same action and reason for the canonical v3 corpus. A future change to fields or precedence requires a new corpus version and synchronized Rust/C consumers; silently changing v3 semantics is not permitted.

## 4. Local and non-transitive trust invariant

`issuer_trusted` and `issuer_trust_local` are deliberately distinct. `issuer_trusted=true` is insufficient when `issuer_trust_local=false`. This prevents a valid delegation from being interpreted as a trust-establishment path.

For peers A, B, and C, A trusting B and B trusting C MUST NOT cause A to trust C. A MAY accept a grant issued by C only after A independently establishes C as a locally trusted issuer through an explicit trust-management/enrollment operation outside normal AUTH and outside this delegation decision.

Normal AUTH remains NO-LEARNING: successful authentication or presentation of a valid delegation MUST NOT silently mutate the verifier's trust store.

## 5. Bounded redelegation

Redelegation is never implicit. If a holder asks to redelegate, the parent grant MUST explicitly permit it and the resulting chain MUST remain within the verifier's configured depth bound. A higher-capability peer MUST NOT accept a deeper chain merely because it has more compute or storage than a constrained peer operating under the same profile.

Every retained or derived delegation MUST be re-evaluated when authorization generation, epoch, revocation state, lineage, local issuer trust, or rollback state changes. Cached acceptance is not durable authority.

## 6. Offline/common-contract behavior

For an already-authorized peer, evaluation of this decision MUST NOT require a CA, cloud service, central registry, DNS, Internet connectivity, blockchain, manufacturer service, or gateway approval. Implementations MAY receive revocation/epoch updates through such systems when available, but absence of infrastructure MUST NOT be converted into fresh authority. If required freshness cannot be established under the selected profile, the verifier fails closed according to the relevant stale-state reason.

## 7. Evidence and claim discipline

Current implementation evidence consists of the Rust classifier in `rust/crates/proto/src/p2p_delegation.rs`, the C classifier in `c/src/proto/p2p_delegation.c`, the C public decision types in `c/include/auth/p2p_delegation.h`, and the shared deterministic v3 corpus consumed by both languages.

Those artifacts support an IMPLEMENTED/TESTABLE decision contract. They do not by themselves establish physical-MCU qualification, constrained resource measurements, complete cross-class interoperability, external cryptographic review, formal proof, or deployment qualification. The top-level Common Contract qualification matrix remains the authority for those stronger claims.

## 8. Required negative qualification

At minimum, qualification MUST demonstrate rejection for: non-local issuer trust; unauthenticated holder; missing/invalid grant; scope/audience/deployment mismatch; invalid validity window; unbound/stale authorization generation; stale epoch; stale revocation state; explicit revocation; stale lineage; excessive depth; forbidden redelegation; rollback suspicion; and inconsistent local-trust facts.

Compound-negative tests MUST preserve the precedence in section 3. Cross-language qualification MUST compare both action and reason, not only accept/deny. Any future executable Common Contract qualification that promotes this surface MUST bind its retained evidence to an exact repository HEAD.