# Authorization-Aware Resumption Decision Contract

Status: implementation-linked draft for `zk221`. This document defines the wire-neutral local decision that precedes any future resumption handshake. It does not define a ticket, PSK, 0-RTT, or wire encoding.

## 1. Security boundary

A resumption secret is not authorization. A peer MUST resume only when the locally available credential and authorization context remain valid for the same authenticated holder, deployment/domain, audience, selected profile, authorization lineage, current revocation state, current restart/replay continuity, and current credential epoch.

General-purpose state-changing 0-RTT remains out of scope for the constrained baseline.

An explicitly invalidated session MUST NOT be resurrected by possession of an otherwise valid resumption credential.

## 2. Inputs

The decision consumes local facts representing at least:

- credential presence and integrity;
- binding to the intended resumption context;
- expiry and bounded reuse count;
- presence and freshness of the cached authorization context;
- current revocation view and explicit revocation status;
- current authorization lineage;
- restart/replay continuity state;
- credential epoch freshness;
- explicit session invalidation state;
- peer/holder, deployment/domain, audience, and profile equality;
- rollback suspicion from persistent lifecycle state.

The classifier is intentionally storage- and wire-neutral. The mechanisms that authenticate these facts remain separately owned by TRUST/LINK/storage/profile specifications.

## 3. Actions

`RESUME` means the local facts are sufficient to permit a future resumption flow to continue. It does not by itself establish a new secure association.

`FULL_AUTH_REQUIRED` means the resumption fast path MUST NOT continue, but a fresh normal AUTH may be attempted. Full AUTH remains subject to current authorization/revocation/freshness checks and MUST NOT repair trust state as a side effect.

`REJECT` means local lifecycle state is unsafe for resumption. A caller may separately recover or refresh authority/replay state where the applicable lifecycle specification permits it, but MUST NOT treat the failed resumption itself as recovery.

## 4. Required precedence

The implementations MUST apply the following fail-closed precedence:

1. rollback suspicion -> `REJECT`;
2. stale restart/replay continuity -> `REJECT`;
3. explicit session invalidation -> `REJECT`;
4. stale credential epoch -> `FULL_AUTH_REQUIRED`;
5. missing/invalid credential, binding mismatch, expiry, or reuse exhaustion -> `FULL_AUTH_REQUIRED`;
6. missing/stale cached authorization context -> `FULL_AUTH_REQUIRED`;
7. stale revocation state -> `REJECT`;
8. explicit revocation -> `REJECT`;
9. stale authorization lineage -> `REJECT`;
10. changed peer, deployment/domain, audience, or profile -> `FULL_AUTH_REQUIRED`;
11. otherwise -> `RESUME`.

A zero usage limit disables resumption. `usage_count >= usage_limit` requires full AUTH.

## 5. Lifecycle invariants

Possession of a ticket/PSK MUST NOT override revocation, stale lineage, invalidation, stale restart continuity, or rollback detection.

Successful prior AUTH MUST NOT be treated as indefinitely fresh authorization.

Restart or transport reconnection MUST NOT manufacture fresh authorization, revocation, replay, epoch, or lineage state.

Credential epoch advancement invalidates the resumption fast path for credentials from older epochs; the caller must perform fresh AUTH under current policy before any later credential issuance.

Explicit session invalidation is terminal for that session instance. A caller MUST NOT map invalidation to mere reconnect/resume.

Normal AUTH remains NO-LEARNING and MUST NOT create persistent trust merely because a resumption attempt fell back to full AUTH.

Transport address changes are not protocol identity changes; identity/context matching is defined over authenticated protocol context, not raw socket address.

## 6. Conformance evidence

The canonical decision corpus is `rust/test-vectors/state/resumption-authorization-v2.txt` and is consumed by both Rust and C tests.

The corpus covers current-state success plus rollback, stale restart continuity, stale credential epoch, explicit invalidation, missing/invalid credential, binding mismatch, expiry, reuse exhaustion, missing/stale authorization context, stale revocation state, explicit revocation, stale lineage, and peer/deployment/audience/profile changes.

Version 1 remains historical evidence for the earlier decision surface; new implementations claiming the current contract MUST use version 2.

This evidence demonstrates decision parity only. It does not claim ticket issuance, cryptographic ticket protection, persistent anti-rollback storage, resumption wire interoperability, formal proof, physical MCU measurements, independent review, or deployment qualification.
