# Constrained lifecycle-storage evidence contract

Status: TD-002 measurement scaffold. This document does not claim physical-target evidence exists.

## Purpose

Revocation freshness, restart continuity, rollback detection, and stale-authorization handling require persistent lifecycle state on constrained peers. The general constrained-target manifest records overall execution context; this companion record captures the storage-specific evidence needed to interpret those lifecycle guarantees on STM32/ESP32-S3-class targets.

A storage backend name or datasheet feature is not evidence that ZK-ARCHE survives restart, detects rollback, or preserves revocation freshness. Those claims require execution against the identified physical target and retained observations.

## Evidence states

`evidence_status` is one of:

- `unmeasured`: scaffold only. `physical_target_executed`, `restart_test_executed`, and `rollback_test_executed` MUST be false and every numeric observation MUST be null.
- `measured`: the identified physical target was exercised. Restart and rollback tests MUST have executed, target/implementation/storage context MUST be populated, and required observations MUST contain actual measured values.

There is intentionally no `passed`, `qualified`, or `production` state.

## Required context

A measured record MUST identify:

- target family, board, and board revision;
- exact implementation commit and implementation lane;
- persistence backend and medium;
- atomic-update strategy;
- monotonic/freshness source used to distinguish current from stale state;
- rollback-detection mechanism or explicit absence boundary;
- power-loss model used during the test;
- persistent-state, revocation-view, and authorization-view byte footprints;
- update latency, restart-recovery latency, and bytes written per lifecycle update.

The record MUST be attributable to one exact implementation commit. If storage backend, persistence medium, atomicity strategy, freshness source, rollback mechanism, board revision, or firmware changes, retain a new record instead of rewriting prior evidence.

## Lifecycle interpretation

A structurally valid measured record establishes only that the declared observations were captured under the declared context. It does not by itself prove rollback resistance, cryptographic integrity, wear lifetime, atomicity under every power-failure point, Common Contract conformance, memory safety, interoperability, or deployment readiness.

For revocation, physical evidence must remain separate from normative semantics. A peer whose authorization view exceeds the selected profile freshness bound must still fail closed; storage measurements cannot widen that bound. Optional cloud/gateway synchronization may improve convergence but is not root authority for an already-authorized peer's local AUTH decision.

## Validation

Run:

```sh
python3 scripts/check-constrained-lifecycle-storage.py evidence/constrained-target/lifecycle-storage-template.json
```

The checker rejects fake numeric placeholders in an unmeasured record and rejects measured claims missing physical execution, restart/rollback execution, provenance, storage context, or required observations.
