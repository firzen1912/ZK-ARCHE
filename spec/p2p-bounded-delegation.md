# P2P bounded delegation contract

Status: implementation-backed draft for `zk239`; not an RFC or deployment claim.

## Security boundary

ZK-ARCHE trust is local and non-transitive by default. `A trusts B` and `B trusts C` MUST NOT imply that A accepts C. A MAY accept C only when A locally validates an explicit delegation grant issued by an entity A already trusts and every bound below is satisfied.

Normal AUTH remains NO-LEARNING. Successful AUTH authenticates the holder; it does not create a delegation, enlarge scope, or mutate the local trust store.

## Required local facts

A bounded delegation acceptance requires all of the following to be true: the issuer is already trusted by the local verifier; the holder is authenticated; an explicit grant exists and its integrity has already been verified; scope, audience, and deployment match; validity and policy epoch are current; revocation state is current and the grant/holder is not revoked; lineage is current; delegation depth is within the locally configured maximum; and any requested redelegation is explicitly permitted. Rollback suspicion fails closed before ordinary evaluation.

The decision MUST be local. CA, cloud identity, DNS, Internet, blockchain, manufacturer cloud, gateway approval, or an online central registry MUST NOT be required when the verifier has sufficient current local state.

## Depth and redelegation

Delegation depth is an explicit bound, not inferred graph reachability. A grant that exceeds the local maximum MUST fail closed. Redelegation is disabled unless the grant explicitly permits it; possession of an accepted delegated authorization does not itself confer authority to delegate again.

## Revocation and freshness

A stale revocation view, stale lineage, stale epoch, expired/not-yet-valid grant, or rollback suspicion MUST fail closed. Infrastructure loss does not automatically revoke a grant, but a peer MUST stop accepting it once its locally retained state exceeds the applicable freshness bound.

## Evidence boundary

The current Rust/C classifier consumes already-verified Boolean facts and does not define a wire encoding, signature/MAC format, persistent grant store, commissioner flow, or cryptographic grant-verification algorithm. Those remain separate prerequisites for full `zk239`/`zk240` conformance. The canonical decision corpus is `rust/test-vectors/p2p/bounded-delegation-v1.txt`.
