# Lineage Replacement Multi-Attempt Event Contract

Status: qualification contract for zk213. This document is wire-neutral and does not allocate protocol messages, fields, timers, retry intervals, attempt encodings, or cryptographic primitives.

## 1. Purpose

This contract composes the existing replacement-attempt binding decision with deterministic event histories. Its purpose is to make stale-attempt traffic, duplicate confirmation, asymmetric restart, simultaneous different attempts, and explicit clean retry reproducible across Rust and C before any production wire grammar is allocated.

Normal AUTH remains NO-LEARNING. The event model does not enroll a peer, expand authorization, or mutate a trust store.

## 2. Abstract attempts

The canonical corpus uses three symbolic attempts only for test discrimination:

- `A1 = (attempt=1, predecessor_generation=10, successor=20, context=30)`;
- `A2 = (attempt=2, predecessor_generation=10, successor=21, context=30)`;
- `A3 = (attempt=3, predecessor_generation=11, successor=22, context=31)`.

These numbers are test symbols, not wire values or registry allocations.

## 3. Event semantics

The shared corpus uses the following abstract events:

- `LA` / `PA`: local or peer replacement authority has been accepted for the explicit replacement flow;
- `Ln` / `Pn`: the local or peer side observes symbolic attempt `An` as its current attempt;
- `LCn` / `PCn`: local or peer confirmation for `An` is accepted only when that side currently observes `An`;
- `RL` / `RP`: the indicated side loses volatile attempt and confirmation state at a restart boundary while authorization state is left unchanged by this test model;
- `X`: both sides explicitly abandon current volatile attempt/confirmation state before a clean retry.

Observing a different current attempt clears that side's prior confirmation binding. Observing the same attempt is idempotent. Confirmation for an attempt that is not current is ignored and MUST NOT become confirmation for the current attempt.

## 4. Decision composition

If either side has no current attempt, the event layer returns `INCOMPLETE_ATTEMPT`. This is a test-layer state, not a new protocol error allocation.

When both current attempts exist, the event layer constructs the normalized facts from `lineage-replace-attempt-binding-contract.md` and delegates to the existing Rust/C attempt classifier. In particular:

- different current attempt identities fail as `ATTEMPT_ID_MISMATCH`;
- current attempts with missing bilateral confirmation remain `AWAITING_CONFIRMATION`;
- only one matching current attempt with both confirmations bound to that current attempt can become `CONVERGED`.

## 5. Required invariants

- A confirmation from an old attempt MUST NOT authorize a newer attempt.
- Duplicate current-attempt confirmation MUST be idempotent.
- Old confirmation arriving after current confirmation MUST NOT revoke or replace the valid current binding.
- Restart loss of volatile attempt state MUST NOT be interpreted as convergence.
- Re-observing an attempt after restart MUST require re-establishment of any confirmation evidence lost at that boundary.
- Simultaneous different attempts MUST remain fail closed; this contract does not choose a winner.
- A clean retry after conflict MUST be explicit and MUST establish a fresh matching current attempt on both sides before convergence.
- Transport addresses MUST NOT be used as attempt identity.

## 6. Canonical coverage

`MA-01` through `MA-14` cover normal convergence, stale confirmation on a new attempt, duplicate confirmation, irrelevant old confirmation after convergence, asymmetric restart, re-observation without reconfirmation, reconfirmation, simultaneous different attempts, explicit clean retry, stale peer state after restart, bilateral state loss, old confirmation after a newer attempt, successful confirmation of the newer attempt, and one-sided attempt advancement.

## 7. Evidence and non-claims

This contract supplies deterministic semantic qualification evidence only. It does not establish a cryptographic confirmation construction, attempt-ID wire encoding, retransmission/liveness policy, durable attempt persistence, malicious rollback resistance, crash-safe trust-store commit, secure erasure, formal proof, physical-target behavior, Common Contract conformance, RFC-class completion, or deployment qualification.
