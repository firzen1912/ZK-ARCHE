# Constrained-target evidence manifest contract

Status: assurance scaffold for TD-002; this document does not claim physical target evidence exists.

## Purpose

TD-002 requires reproducible STM32/ESP32-S3-class execution evidence. Workstation, host-simulation, cross-compilation, or static-size observations are useful engineering inputs but MUST NOT be recorded as physical-target measurements.

A constrained-target evidence manifest records the context needed to interpret a real measurement. The least-capable conformant peer remains the resource-envelope authority; missing measurements remain explicit gaps rather than guessed values.

## Manifest states

`evidence_status` is one of:

- `unmeasured`: scaffold only. All measurement fields MUST be null and `physical_target_executed` MUST be false.
- `measured`: evidence captured from the identified physical target. `physical_target_executed` MUST be true and required measurement/context fields MUST be populated.

The checker intentionally has no `passed`, `qualified`, or `production` state. A valid manifest means only that the evidence record is structurally honest.

## Required identity and provenance

Every manifest MUST record:

- `schema`: `ZKARCHE-CONSTRAINED-TARGET/1`;
- `evidence_status`;
- `physical_target_executed`;
- target family, board, board revision, architecture, and execution environment;
- implementation commit SHA and implementation lane;
- compiler/toolchain identifier and build profile;
- crypto backend and whether an accelerator was used;
- entropy source, DRBG/reseed posture, key-generation mode, key-storage location, secure-boot/debug posture;
- restart, rollback, clone, and reprovision assumptions;
- measured wire bytes, static RAM, peak stack, peak heap, flash/text+rodata footprint, and latency/CPU observations when `measured`.

Null values are mandatory for unavailable measurements in an `unmeasured` manifest. Placeholder numbers such as `0`, guessed board specifications, desktop measurements, or datasheet maxima MUST NOT be substituted for missing physical observations.

## Evidence boundaries

A structurally valid measured manifest does not by itself establish constant-time behavior, RNG quality, cryptographic soundness, memory safety, rollback resistance, interoperability, Common Contract conformance, or deployment readiness. Those properties require their own evidence.

Physical-target measurements MUST be attributable to the implementation commit recorded in the manifest. If firmware, compiler flags, crypto backend, board revision, clocking, storage configuration, or entropy path changes, retain a new manifest instead of silently rewriting old evidence.

## Validation

Run:

```sh
python3 scripts/check-constrained-target-manifest.py path/to/manifest.json
```

The checker fails closed on malformed structure, dishonest `unmeasured` numeric placeholders, or a `measured` claim missing physical-target/context/measurement evidence.
