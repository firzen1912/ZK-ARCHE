# DATA release authorization traceability

Status: assurance traceability record; this document does not create new wire or policy semantics.

## Purpose

This record binds the existing policy-bound DATA release decision surface to the data-sovereignty roadmap without promoting optional high-end mechanisms into the constrained mandatory floor.

The authoritative behavior remains the normative DATA-sovereignty contract and the Rust/C implementations plus their canonical deterministic corpus. Authentication is necessary when required by policy but is never sufficient to authorize protected-data release. Device-local release authority remains final.

## Required decision invariants

A release decision is permitted only when all mandatory local facts are current and compatible:

1. the requester is authenticated;
2. explicit device-local release authority exists and is current;
3. protected data remains encrypted before authorized release;
4. the release key is scoped to the requested release;
5. authorization exists, is fresh, and is bound to the current authorization generation;
6. revocation state is current and the authority is not explicitly revoked;
7. lineage is current;
8. holder, audience, purpose, data type, policy, and epoch match;
9. required channel binding is valid;
10. the release operation is unused; and
11. rollback is not suspected.

Failure is deterministic and fail-closed. An unauthenticated request may require fresh AUTH; it does not acquire release authority through AUTH. All other invalid authority/lifecycle conditions deny release.

## Traceability matrix

| Property | Rust owner | C owner | Canonical evidence | Remaining qualification gap |
|---|---|---|---|---|
| AUTH != DATA authorization | `rust/crates/proto/src/data_release_authorization.rs` | `c/src/proto/data_release_authorization.c` | `rust/test-vectors/state/data-release-authorization-v4.txt` | End-to-end AUTH→RELEASE qualification across both implementations |
| Device-local authority is final | same | same | same | Persistent authority-store and restart/rollback evidence |
| Authorization generation freshness | same | same | same | Temporal mutation corpus and durable generation continuity |
| Revocation / lineage invalidation | same | same | same | Disconnected convergence and stale-state bounds |
| Holder/audience/purpose/data-type/policy/epoch scoping | same | same | same | Wire-level RELEASE_REQUEST / RELEASE_PROOF binding evidence |
| Channel-bound release | same | same | same | Transport-specific exporter/context interop negatives |
| One-use release / replay rejection | same | same | same | Durable replay continuity across restart/power loss |
| Rollback fail-closed behavior | same | same | same | Rollback-resistant physical persistence evidence |

## Evidence boundary

The classifier and deterministic corpus can establish decision compatibility only when executed. Their existence alone is not a fresh `TESTED` or `INTEROPERABLE` result.

They do not establish:

- cryptographic correctness of a future `RELEASE_PROOF` or `RELEASE_KEY` construction;
- durable atomic persistence, secure erasure, power-loss recovery, or rollback resistance;
- physical STM32/ESP32-S3 CPU, RAM, flash, latency, entropy, or key-storage behavior;
- transparency publication or external audit-service correctness;
- field/deployment readiness;
- independent cryptographic review; or
- RFC/IETF status.

Heavyweight credentials, general-purpose ZK circuits, remote policy engines, cloud services, and transparency infrastructure remain optional extensions. A conformant constrained peer must not require them merely to evaluate a bounded release profile it supports.

## Next executable evidence

The next dependency-ready DATA qualification packet should add a shared temporal corpus consumed independently by Rust and C that starts from an accepted release and then mutates exactly one authoritative lifecycle fact: authorization generation, revocation, lineage, epoch, channel binding, replay/use state, or rollback state. The subsequent release must deterministically fail with the same action/reason in both implementations.

That temporal corpus must not claim persistence or physical-target evidence unless the relevant store/target lane is actually executed and retained.
