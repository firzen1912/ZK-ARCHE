# Transport Continuation Decision Contract

Status: implementation-linked draft for `zk227`/`zk230`. This contract governs whether an already-authenticated association may survive transport reconnect, route change, or address mobility. It does not define a new transport protocol or resumption credential.

## Security boundary

Transport location is not protocol identity. UDP/TCP addresses, BLE handles, CAN identifiers, serial endpoints, connection IDs, and other adapter metadata MUST NOT become authentication or authorization authority merely because an adapter exposes them.

Continuation MUST remain bound to authenticated peer/profile context, required channel binding, current replay continuity, current bounded-reuse counter continuity, current authorization generation, current association invalidation state, and the separately evaluated resumption decision.

A route or connection change MAY be accepted when those protocol facts remain current. A route or connection change MUST NOT manufacture current lifecycle state.

## Required precedence

Implementations claiming this contract MUST apply the following fail-closed precedence:

1. transport address used as identity -> `REJECT`;
2. transport metadata used as authority -> `REJECT`;
3. association invalidated -> `REJECT`;
4. replay continuity stale -> `REJECT`;
5. bounded-reuse counter continuity stale/unknown -> `REJECT`;
6. authorization generation stale -> `FULL_AUTH_REQUIRED`;
7. authenticated peer-context mismatch -> `REJECT`;
8. authenticated profile-context mismatch -> `FULL_AUTH_REQUIRED`;
9. required channel binding invalid -> `REJECT`;
10. resumption not authorized -> `FULL_AUTH_REQUIRED`;
11. continuation not requested -> `FULL_AUTH_REQUIRED`;
12. otherwise -> `CONTINUE`.

The `resumption_authorized` input is a summary of the resumption owner decision; the explicit replay/reuse/generation checks are defense-in-depth seam checks so an adapter cannot bypass lifecycle freshness through an incorrectly cached continuation decision.

## Mobility invariant

`transport_route_changed` and `transport_connection_changed` are observations, not authority inputs. When the authenticated protocol context and lifecycle facts remain current, changing either MUST NOT by itself change ZK-ARCHE identity. Conversely, an unchanged socket/address MUST NOT preserve an association that has been invalidated, revoked through the authorization generation, or lost durable replay/reuse continuity.

## Evidence

The canonical corpus is `rust/test-vectors/state/transport-continuation-v2.txt`, consumed by Rust and C decision tests. It includes successful steady/mobility cases plus negative address-as-identity, metadata-as-authority, invalidation, replay continuity, reuse-counter continuity, authorization-generation, peer/profile, binding, resumption, and non-continuation cases.

This evidence establishes decision semantics only. It does not establish TLS exporter correctness, UDP/TCP mobility interoperability, ticket/PSK wire behavior, physical target behavior, formal proof, external review, or deployment qualification.
