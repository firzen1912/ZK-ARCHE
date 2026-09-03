# Authorization-Aware Resumption Decision Contract

Status: implementation-linked draft for `zk221`. This document defines the wire-neutral local decision that precedes any future resumption handshake. It does not define a ticket, PSK, 0-RTT, or wire encoding.

## 1. Security boundary

A resumption secret is not authorization. A peer MUST resume only when the locally available credential and authorization context remain valid for the same authenticated holder, deployment/domain, audience, selected profile, authorization lineage, current revocation state, current restart/replay continuity, current bounded-reuse counter continuity, current authorization generation, and current credential epoch.

A credential used for resumption MUST carry an authenticated authorization-generation binding, directly or through authenticated local metadata. A caller-provided statement that the local generation is current is insufficient when the credential itself cannot be related to that generation.

General-purpose state-changing 0-RTT remains out of scope for the constrained baseline. An explicitly invalidated session MUST NOT be resurrected by possession of an otherwise valid resumption credential.

## 2. Inputs

The decision consumes local facts representing at least credential presence/integrity, binding, expiry and bounded reuse, persisted reuse-counter continuity, cached authorization context presence/freshness, authenticated authorization-generation binding and equality with the current local generation, revocation and lineage state, restart/replay continuity, credential epoch freshness, explicit invalidation, peer/deployment/audience/profile equality, and rollback suspicion.

The classifier is intentionally storage- and wire-neutral. TRUST/LINK/storage/profile specifications own the mechanisms that authenticate these facts.

## 3. Actions

`RESUME` permits only the future resumption flow to continue; it does not itself establish a new secure association.

`FULL_AUTH_REQUIRED` means the fast path MUST stop, but fresh normal AUTH may be attempted. Full AUTH remains subject to current authorization/revocation/freshness checks and MUST NOT repair trust state, invent an authorization-generation binding, or synthesize current authorization.

`REJECT` means local lifecycle state is unsafe for resumption. Recovery or authority refresh, where separately permitted, is not part of the failed resumption.

## 4. Required precedence

Implementations MUST apply this fail-closed precedence:

1. rollback suspicion -> `REJECT`;
2. stale restart/replay continuity -> `REJECT`;
3. stale/unknown bounded-reuse counter continuity -> `REJECT`;
4. explicit session invalidation -> `REJECT`;
5. stale credential epoch -> `FULL_AUTH_REQUIRED`;
6. missing/invalid credential, binding mismatch, expiry, or reuse exhaustion -> `FULL_AUTH_REQUIRED`;
7. missing/stale cached authorization context -> `FULL_AUTH_REQUIRED`;
8. missing authenticated authorization-generation binding -> `FULL_AUTH_REQUIRED`;
9. stale credential-bound authorization generation -> `FULL_AUTH_REQUIRED`;
10. stale revocation state -> `REJECT`;
11. explicit revocation -> `REJECT`;
12. stale authorization lineage -> `REJECT`;
13. changed peer, deployment/domain, audience, or profile -> `FULL_AUTH_REQUIRED`;
14. otherwise -> `RESUME`.

A zero usage limit disables resumption. `usage_count >= usage_limit` requires full AUTH. A peer MUST NOT reset or reconstruct a lower usage count because process or volatile transport state restarted.

## 5. Lifecycle invariants

Possession of a ticket/PSK MUST NOT override revocation, stale lineage, invalidation, stale restart continuity, stale reuse-counter continuity, absent/stale authorization-generation binding, or rollback detection.

Authorization-generation advancement invalidates credentials bound to predecessor authorization state even when cryptographically intact and unexpired. A credential with no authenticated generation binding is not eligible for resumption under the current contract.

Fresh AUTH MAY establish a new association only under the current authorization decision. It MUST NOT reinterpret the failed credential as generation-bound, mutate trust as a side effect, or reset replay/reuse state.

Restart or transport reconnection MUST NOT manufacture fresh authorization, revocation, replay, reuse-counter, epoch, generation, or lineage state. Explicit session invalidation remains terminal for that session instance. Transport addresses are not protocol identity.

## 6. Conformance evidence

The canonical decision corpus is `rust/test-vectors/state/resumption-authorization-v4.txt` and is consumed by both Rust and C tests.

The v4 corpus covers current-state success plus rollback, stale restart continuity, stale reuse-counter continuity, stale credential epoch, explicit invalidation, missing/invalid credential, binding mismatch, expiry, reuse exhaustion, missing/stale authorization context, missing authorization-generation binding, stale authorization generation, stale revocation state, explicit revocation, stale lineage, and peer/deployment/audience/profile changes.

Versions 1–3 remain historical evidence. New implementations claiming the current contract MUST use version 4.

This evidence demonstrates decision parity only. It does not claim ticket issuance/protection, atomic persistent invalidation, anti-rollback storage, resumption wire interoperability, formal proof, physical MCU measurements, independent review, or deployment qualification.
