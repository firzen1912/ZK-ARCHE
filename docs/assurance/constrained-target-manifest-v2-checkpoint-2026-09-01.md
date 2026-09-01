# Constrained target evidence schema v2 checkpoint — 2026-09-01

## Scope

This packet hardens the existing TD-002 lifecycle-storage evidence path. It does not add a second benchmark system. The existing validator/template are extended so future STM32/ESP32-S3-class evidence cannot be accepted from resource numbers detached from their execution context.

## Schema v2

A measured manifest now requires:

- exact target family, board, and revision;
- exact implementation commit/lane and compiler/build profile;
- cryptographic library/version, accelerator path, and software fallback posture;
- entropy source, health-test posture, DRBG, and reseed policy;
- key representation, storage location, and zeroization posture;
- secure-boot/debug state;
- persistent-storage backend, atomic update strategy, monotonic freshness source, rollback detection, and power-loss model;
- transport kind, MTU, and reliability context;
- physical execution of restart, rollback, entropy-path, and key-storage-path tests;
- wire bytes; stack high-water, heap peak, static RAM, flash; persistent authorization/revocation state; AUTH/update/restart latency; and bytes written per update.

The unmeasured repository template is deliberately valid only when physical-target execution/test flags are false and every numeric observation remains null.

## Negative qualification

`scripts/test-constrained-lifecycle-storage.py` verifies that:

1. the repository's unmeasured template passes honestly;
2. inserting even one observation into an unmeasured manifest is rejected;
3. flipping a context-free template to `measured` is rejected even when execution flags are set.

The release qualification already invokes `scripts/check-constrained-lifecycle-storage.py` against the repository template, so schema drift remains repository-gated without requiring hosted Actions on `dev`.

## Evidence boundary

This is evidence-scaffolding and fail-closed claim validation. It is not STM32/ESP32-S3 measurement evidence. No wire/RAM/flash/CPU/latency number, entropy quality, secure-storage property, restart/rollback result, constant-time property, or deployment qualification is inferred from this checkpoint.

TD-002 remains open until reproducible physical-target manifests and raw measurements actually exist.
