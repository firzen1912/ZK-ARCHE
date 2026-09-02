# P2P Bounded Delegation Decision Contract

Status: implementation-linked normative decision contract for the Common Contract P2P trust boundary. This document defines local decision semantics only; it does not define a grant wire format or credential signature format.

## 1. Non-transitive trust root

Trust is local and non-transitive by default. A verifier MUST NOT accept a delegation merely because some already-trusted peer claims that the delegation issuer is trustworthy.

For a delegation to be considered, the verifier must have both:

- an accepted trust relationship for the issuer; and
- evidence that the issuer relationship is rooted in the verifier's own local trust state rather than inferred transitively from an unrelated peer relationship.

Accordingly, `issuer_trusted=true` with `issuer_trust_local=false` MUST fail closed with `ISSUER_TRUST_NOT_LOCAL`.

This rule does not prohibit explicit delegation. It prevents delegation from silently turning local trust into transitive trust. The explicit grant is the bounded authority path from the locally trusted issuer to the authenticated holder.

## 2. Required grant bounds

After establishing a local issuer trust root, the verifier MUST require all of the following before accepting a delegation:

- authenticated holder;
- present and integrity-valid grant;
- matching scope, audience, and deployment/domain;
- current validity interval and policy epoch;
- current revocation view and no explicit revocation;
- current authorization lineage;
- delegation depth within the locally accepted bound;
- explicit redelegation permission whenever redelegation is requested;
- no rollback suspicion in the local lifecycle state.

Normal AUTH remains NO-LEARNING. Acceptance of a delegation MUST NOT add the holder or issuer to persistent trust merely because authentication or delegation verification succeeded.

## 3. Fail-closed precedence

The shared Rust/C classifier applies this precedence:

1. rollback suspicion;
2. issuer not trusted;
3. issuer trust not locally rooted;
4. holder not authenticated;
5. missing/invalid grant;
6. scope/audience/deployment mismatch;
7. stale validity or epoch;
8. stale revocation view or explicit revocation;
9. stale lineage;
10. depth overflow;
11. forbidden redelegation;
12. otherwise `ACCEPT`.

## 4. Infrastructure independence

The decision consumes already-verified local facts. Evaluating the mandatory decision MUST NOT require CA, DNS, Internet access, manufacturer cloud, blockchain, central registry lookup, or gateway approval when the verifier already has sufficient authorized local state.

Infrastructure MAY distribute or refresh trust, grants, revocation state, or policy state outside the decision. Loss of that infrastructure does not convert stale state into current state and does not authorize transitive trust inference.

## 5. Canonical conformance corpus

`rust/test-vectors/p2p/bounded-delegation-v2.txt` is the current canonical decision corpus. Version 2 adds an explicit negative case for transitively inferred issuer trust and is consumed by both Rust and C implementations.

Version 1 remains historical evidence for the earlier decision surface.

## 6. Evidence boundary

This contract and corpus demonstrate wire-neutral decision parity and explicit non-transitivity semantics. They do not establish a cryptographic delegation-token encoding, physical constrained-target interoperability, secure storage, formal proof, independent cryptographic review, RFC/IETF status, or deployment qualification.
