# Transport Continuation Decision Contract

Status: implementation-linked draft for `zk227`/`zk230`. This contract governs whether an already-authenticated association may survive transport reconnect, route change, or address mobility. It does not define a new transport protocol or resumption credential.

## Security boundary

Transport location is not protocol identity. UDP/TCP addresses, BLE handles, CAN identifiers, serial endpoints, connection IDs, and other adapter metadata MUST NOT become authentication or authorization authority merely because an adapter exposes them.

Continuation MUST remain bound to authenticated peer/profile context, required channel binding, current replay continuity, current bounded-reuse counter continuity, an authenticated authorization-generation binding, current authorization generation, current association invalidation state, and the separately evaluated resumption decision.

`authorization_generation_bound` means the continuing association is tied by authenticated protocol evidence or authenticated local association metadata to a specific authorization generation. An adapter MUST NOT synthesize this fact from an address, connection identifier, route, cached label, or unauthenticated transport metadata.

A route or connection change MAY be accepted when those protocol facts remain current. A route or connection change MUST NOT manufacture generation binding or current lifecycle state.

## Required precedence

Implementations claiming this contract MUST apply the following fail-closed precedence:

1. transport address used as identity -> `REJECT`;
2. transport metadata used as authority -> `REJECT`;
3. association invalidated -> `REJECT`;
4. replay continuity stale -> `REJECT`;
5. bounded-reuse counter continuity stale/unknown -> `REJECT`;
6. authorization generation unbound/unknown -> `FULL_AUTH_REQUIRED`;
7. authorization generation stale -> `FULL_AUTH_REQUIRED`;
8. authenticated peer-context mismatch -> `REJECT`;
9. authenticated profile-context mismatch -> `FULL_AUTH_REQUIRED`;
10. required channel binding invalid -> `REJECT`;
11. resumption not authorized -> `FULL_AUTH_REQUIRED`;
12. continuation not requested -> `FULL_AUTH_REQUIRED`;
13. otherwise -> `CONTINUE`.

The `resumption_authorized` input is a summary of the resumption owner decision; the explicit replay/reuse/generation-binding/generation-freshness checks are defense-in-depth seam checks so an adapter cannot bypass lifecycle freshness through an incorrectly cached continuation decision.

`FULL_AUTH_REQUIRED` is not authorization repair. A new AUTH attempt must independently establish the authenticated peer/profile and authorization context required by the applicable association-admission rules. An old association MUST NOT copy its authorization-generation binding forward merely because its old transport remains reachable.

## LINK / transport ownership

The LINK/transport layer owns byte delivery and transport observations such as route/connection change, MTU, ordering/reliability characteristics, and trustworthy channel-binding material when the transport can actually provide it.

The LINK/transport layer does **not** own protocol identity, authorization-generation allocation or advancement, authorization-generation binding, trust mutation, revocation state, replay-epoch authority, or the resumption authorization decision. Those facts are consumed from their existing protocol/lifecycle owners. This boundary prevents a transport adapter from becoming a duplicate lifecycle authority.

## Mobility invariant

`transport_route_changed` and `transport_connection_changed` are observations, not authority inputs. When the authenticated protocol context, authenticated authorization-generation binding, and lifecycle facts remain current, changing either MUST NOT by itself change ZK-ARCHE identity. Conversely, an unchanged socket/address MUST NOT preserve an association that has been invalidated, lacks an authenticated generation relationship, belongs to an older authorization generation, or lost durable replay/reuse continuity.

## Evidence

The canonical corpus is `rust/test-vectors/state/transport-continuation-v3.txt`, consumed by Rust and C decision tests. It includes successful steady/mobility cases plus negative address-as-identity, metadata-as-authority, invalidation, replay continuity, reuse-counter continuity, authorization-generation binding/freshness, peer/profile, binding, resumption, and non-continuation cases.

This evidence establishes decision semantics only. It does not establish TLS exporter correctness, UDP/TCP mobility interoperability, ticket/PSK wire behavior, physical target behavior, formal proof, external review, or deployment qualification.
