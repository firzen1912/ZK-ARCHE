# Constrained enrollment replay instrumentation checkpoint — 2026-09-02

## Scope

This Slot-4 packet strengthens the existing constrained-target lifecycle evidence contract for the newly explicit one-time ENROLL replay decision. It does not add a second replay authority and does not claim physical STM32/ESP32-S3 measurements.

The enrollment classifier still owns the security decision. This checkpoint only defines what a physical-target evidence manifest must record before repository claims may say that the persistent replay/RNG/storage path was actually exercised.

## Schema v3 additions

`ZKARCHE-CONSTRAINED-LIFECYCLE-STORAGE/3` adds:

- explicit `enrollment_replay_test_executed` and `enrollment_power_loss_test_executed` gates;
- a bounded RNG-adapter description with interface name and maximum request size;
- storage context for enrollment replay record format, scope, atomicity, restart policy, and rollback rejection policy;
- target observations for replay-state bytes, replay-entry capacity, consume latency, and nonce-generation latency.

An `unmeasured` manifest must keep all execution flags false and all observations null. A `measured` manifest must provide real target/toolchain/crypto/RNG/storage/transport context, execute both enrollment replay and power-loss tests, and provide non-zero replay footprint/capacity/latency observations.

## Why this matters

A one-time ENROLL nonce is not replay-safe merely because the in-memory classifier receives `enrollment_nonce_unused=true`. A constrained-target maturity claim needs evidence that consumed state survives restart, that rollback/power-loss behavior is bounded and fail-closed, and that the RNG/storage adapters fit the selected MCU profile.

The validator remains deliberately evidence-only: it does not assert that a named storage backend is atomic, that a monotonic source is rollback-resistant, or that an RNG is cryptographically sound. Those properties require the physical target execution and review represented by the manifest.

## Claim boundary

This packet advances benchmark/qualification scaffolding only. It is not physical hardware evidence, does not close TD-002, and does not promote `iot-core` or `p2p-iot-core`. It also does not establish RNG quality, key-storage security, constant-time behavior, flash endurance, power-failure correctness, or field readiness.
