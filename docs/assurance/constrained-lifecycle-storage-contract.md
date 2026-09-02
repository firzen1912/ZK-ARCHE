# Constrained lifecycle-storage evidence contract

Status: TD-002 measurement scaffold. This document does not claim physical-target evidence exists.

## Purpose

Revocation freshness, authorization-generation freshness, restart continuity, rollback detection, stale-authorization handling, and one-time enrollment replay require persistent lifecycle state on constrained peers. The general constrained-target manifest records overall execution context; this companion record captures the storage-specific evidence needed to interpret those lifecycle guarantees on STM32/ESP32-S3-class targets.

A storage backend name or datasheet feature is not evidence that ZK-ARCHE survives restart, detects rollback, preserves revocation freshness, or preserves the current authorization generation. Those claims require execution against the identified physical target and retained observations.

## Evidence states

`evidence_status` is one of:

- `unmeasured`: scaffold only. Every execution flag, including authorization-generation and enrollment replay/power-loss flags, MUST be false and every numeric observation MUST be null.
- `measured`: the identified physical target was exercised. Restart, rollback, authorization-generation, authorization-generation power-loss, enrollment replay, and enrollment power-loss tests MUST have executed; target/implementation/storage context MUST be populated; and required observations MUST contain actual measured values.

There is intentionally no `passed`, `qualified`, or `production` state.

## Required context

A measured `ZKARCHE-CONSTRAINED-LIFECYCLE-STORAGE/4` record MUST identify:

- target family, board, and board revision;
- exact implementation commit and implementation lane;
- persistence backend and medium;
- atomic-update strategy;
- monotonic/freshness source used to distinguish current from stale state;
- rollback-detection mechanism or explicit absence boundary;
- power-loss model used during the test;
- authorization-generation record format, generation source, atomicity, restart policy, and rollback policy;
- enrollment replay record format, scope, atomicity, restart policy, and rollback policy;
- persistent-state, revocation-view, authorization-view, authorization-generation, and enrollment-replay byte footprints;
- update latency, authorization-generation update latency, restart-recovery latency, enrollment consume latency, enrollment nonce-generation latency, and bytes written per lifecycle update.

The record MUST be attributable to one exact implementation commit. If storage backend, persistence medium, atomicity strategy, generation/freshness source, rollback mechanism, board revision, or firmware changes, retain a new record instead of rewriting prior evidence.

## Authorization-generation interpretation

`authorization_generation_state_bytes` measures the target-resident state required to determine whether authorization evidence belongs to the current local generation. `authorization_generation_update_latency_us` measures a real generation advance under the declared storage context.

A measured record MUST execute both the authorization-generation lifecycle test and its declared power-loss path. This requirement exists because an in-memory generation comparison is insufficient evidence that an older commissioner grant, association, resumption credential, transport continuation, or DATA release remains stale after reboot.

The measurement schema does not define the generation counter or create another lifecycle authority. Generation semantics remain owned by the protocol authorization/lifecycle implementation. The manifest records only the physical representation and execution context used by that implementation.

## Lifecycle interpretation

A structurally valid measured record establishes only that the declared observations were captured under the declared context. It does not by itself prove rollback resistance, cryptographic integrity, wear lifetime, atomicity under every power-failure point, Common Contract conformance, memory safety, interoperability, or deployment readiness.

For revocation and authorization generation, physical evidence must remain separate from normative semantics. A peer whose authorization view or generation is stale must still fail closed. Optional cloud/gateway synchronization may improve convergence but is not root authority for an already-authorized peer's local AUTH decision.

## Validation

Run:

```sh
python3 scripts/check-constrained-lifecycle-storage.py evidence/constrained-target/lifecycle-storage-template.json
python3 scripts/test-constrained-lifecycle-storage.py
```

The checker rejects fake numeric placeholders in an unmeasured record and rejects measured claims missing physical execution, restart/rollback execution, authorization-generation execution/power-loss evidence, enrollment replay/power-loss execution, provenance, storage context, or required observations.
