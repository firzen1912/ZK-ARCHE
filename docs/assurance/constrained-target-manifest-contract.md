# Constrained-target evidence manifest contract

Status: assurance scaffold for TD-002; this document does not claim physical-target evidence exists.

## Purpose

TD-002 requires reproducible STM32/ESP32-S3-class execution evidence. Workstation tests, host simulation, cross-compilation, static-size observations, datasheet limits, or inferred board capabilities are useful engineering inputs but MUST NOT be recorded as physical-target measurements.

A constrained-target evidence manifest records the context required to interpret a real measurement. The least-capable conformant peer remains the resource-envelope authority; missing measurements remain explicit gaps rather than guessed values.

The canonical generic manifest template is `evidence/constrained-target/manifest-template.json`, and its fail-closed structural validator is `scripts/check-constrained-target-manifest.py`.

## Schema and states

The current generic manifest schema is:

`ZKARCHE-CONSTRAINED-TARGET/2`

`evidence_status` is one of:

- `unmeasured`: scaffold only. `physical_target_executed` MUST be false; measurement values MUST remain null; sample and warm-up counts MUST remain zero; qualification execution flags MUST remain false; `qualification.result` MUST be `UNMEASURED`; measured-time/raw-evidence provenance MUST remain absent.
- `measured`: evidence captured from the identified physical target. `physical_target_executed` MUST be true and the target, implementation, toolchain, crypto context, lifecycle assumptions, measurement method, measurements, and provenance required by the validator MUST be populated.

A structurally valid manifest is not itself a certification or deployment qualification. For a measured manifest, `qualification.result` records `PASS` or `FAIL`; the structural validator checks that the result and execution flags are well-typed but does not infer a PASS from those flags or promote the record to any broader assurance class.

## Required measured identity and provenance

A `measured` manifest MUST bind observations to concrete execution context, including:

- exact target family, board, board revision, architecture, CPU clock, RAM capacity, available flash, execution environment, and power mode;
- a full lowercase 40-hex implementation commit SHA, lane, protocol operation, profile, toolchain/compiler/build profile, compiler flags, and lowercase 64-hex firmware artifact SHA-256;
- crypto backend/version and accelerator-use state;
- entropy source, DRBG/reseed posture, key-generation mode, key-storage location, secure-boot state, and debug state;
- restart, rollback, clone, reprovision, persistent-state backend, and rollback-protection assumptions;
- instrumentation, timing/counter method, memory method, transport context, warm-up/sample counts, and cold-boot sampling posture;
- non-empty operator, measurement timestamp, and at least one raw-evidence reference.

Changing firmware, compiler flags, crypto backend, board revision, clocking, storage configuration, entropy path, or other execution-defining context requires a new manifest rather than silently rewriting retained evidence.

## Measurement integrity

For `measured` records the validator requires:

- positive target CPU clock, RAM capacity, and available flash;
- a positive sample count;
- request/response/total wire-byte observations with `total = request + response` and positive total bytes;
- non-negative static RAM, peak stack, peak heap, and flash observations, with observed RAM components not exceeding declared RAM capacity and flash not exceeding declared available flash;
- latency and CPU-cycle ranges containing `min`, `median`, `p95`, and `max`, ordered as `min <= median <= p95 <= max`;
- positive median latency;
- boolean qualification execution flags and a `PASS` or `FAIL` recorded result.

An `unmeasured` manifest MUST NOT use zero-valued or guessed observations as substitutes for missing physical evidence. Null observations are the required representation of an unmeasured state.

## Qualification boundaries

The generic manifest separates structural honesty from assurance conclusions. A structurally valid measured record does not by itself establish:

- constant-time behavior;
- entropy/RNG quality or failure safety;
- cryptographic or computational soundness;
- memory safety;
- rollback resistance or durable-state correctness;
- Rust/C interoperability;
- Common Contract conformance;
- RFC-class completeness;
- deployment readiness.

Those properties require their own evidence. Likewise, the presence of a boolean execution flag says only that the manifest records the corresponding test as executed; higher-level qualification must inspect the retained result/raw evidence rather than infer success from structural validity.

Lifecycle/storage-specific evidence uses the separate `ZKARCHE-CONSTRAINED-LIFECYCLE-STORAGE/*` contract and validator. Do not conflate that richer lifecycle evidence path with the generic `/2` manifest or back-port lifecycle/storage claims into this schema without synchronized implementation and validation changes.

## Validation

Run:

```sh
python3 scripts/check-constrained-target-manifest.py path/to/manifest.json
```

The checker fails closed on schema drift, malformed structure, dishonest `unmeasured` observations or provenance, and a `measured` claim missing required physical-target context, measurements, or provenance.

Repository-owned validation availability is distinct from physical evidence: successfully validating the unmeasured repository template confirms only manifest-contract consistency. It MUST NOT be reported as STM32/ESP32-S3 measurement evidence.
