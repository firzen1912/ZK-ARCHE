# Transport-Independent Association Continuation

Status: implementation-linked decision contract for transport/reconnect composition. This document does not define a transport protocol, ticket format, or new wire message.

## 1. Authority boundary

A transport address, MAC address, socket tuple, connection identifier, radio endpoint, URI, or adapter metadata is a routing/context value only. It MUST NOT become protocol identity, authorization authority, trust authority, enrollment authority, or revocation authority merely because it is stable or supplied by the operating system.

A transport change therefore has two separate effects:

1. routing/connection state may change; and
2. authenticated protocol context must remain valid or be re-established.

The first does not imply the second.

## 2. Inputs

Association continuation consumes local facts representing:

- whether continuation/resumption is actually requested;
- equality of authenticated peer context;
- equality of authenticated profile/security context;
- whether the selected profile requires channel/exporter binding;
- whether that required binding remains valid for the continued association;
- whether the resumption/lifecycle authority permits continuation;
- whether replay/restart continuity remains current;
- whether the association has been explicitly invalidated;
- whether the route or underlying transport connection changed;
- whether an implementation attempted to use transport address as identity;
- whether an implementation attempted to use transport metadata as protocol authority.

The classifier does not authenticate these facts. AUTH, BIND, TRUST, replay continuity, and resumption/lifecycle components own their respective evidence.

## 3. Decision

A conformant implementation MUST apply the following precedence:

1. attempted transport-address identity -> `REJECT`;
2. attempted transport-metadata authority -> `REJECT`;
3. explicit association invalidation -> `REJECT`;
4. stale replay/restart continuity -> `REJECT`;
5. authenticated peer-context mismatch -> `REJECT`;
6. authenticated profile-context mismatch -> `FULL_AUTH_REQUIRED`;
7. required binding invalid -> `REJECT`;
8. resumption/continuation not authorized -> `FULL_AUTH_REQUIRED`;
9. continuation not requested -> `FULL_AUTH_REQUIRED`;
10. otherwise -> `CONTINUE`.

A route change or connection replacement MUST NOT, by itself, alter the decision once all authenticated protocol/lifecycle facts above remain valid.

## 4. Reconnect and binding semantics

When a selected profile requires transport/channel/exporter binding, a reconnect MAY continue only when the binding evidence used by the association remains valid under the binding specification. A new socket or route identifier is not a substitute for such evidence.

When binding is not mandatory for the selected profile, a route/connection change still MUST NOT bypass authenticated peer/profile matching, replay continuity, invalidation, or resumption authorization.

A caller receiving `FULL_AUTH_REQUIRED` MAY attempt a fresh normal AUTH. Normal AUTH remains NO-LEARNING and does not inherit trust merely from the transport connection.

`REJECT` is fail-closed for the continuation attempt. Recovery, re-enrollment, authority refresh, or replay-state repair belongs to their owning lifecycle mechanisms.

## 5. Conformance

The canonical decision corpus is `rust/test-vectors/state/transport-continuation-v1.txt`. Rust and C implementations claiming this contract must reproduce its action/reason outputs.

The corpus demonstrates that changed routes and connections are accepted only when authenticated context remains valid, and independently rejects address-as-identity, metadata-as-authority, invalidation, replay discontinuity, peer/profile mismatch, required-binding failure, and unauthorized continuation.

This decision evidence is not proof of TCP/UDP/CoAP/BLE interoperability, channel-exporter correctness, ticket protection, persistent anti-rollback state, formal soundness, MCU measurements, independent review, or deployment qualification.
