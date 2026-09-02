# Constrained authorization-generation instrumentation checkpoint — 2026-09-02

## Scope

This Slot-4 packet extends the constrained-target lifecycle evidence contract to the authorization-generation freshness semantics now consumed by CORE association admission, resumption, transport continuation, DATA release, P2P qualification, and ENROLL commissioner authorization.

It does not add an authorization-generation authority and does not claim STM32/ESP32-S3 measurements.

## Schema v4 additions

`ZKARCHE-CONSTRAINED-LIFECYCLE-STORAGE/4` adds:

- `authorization_generation_test_executed` and `authorization_generation_power_loss_test_executed`;
- storage context for authorization-generation record format, source, atomicity, restart policy, and rollback policy;
- `authorization_generation_state_bytes`;
- `authorization_generation_update_latency_us`.

An `unmeasured` manifest keeps the new flags false and observations null. A `measured` manifest must execute both generation and generation-power-loss paths on the identified physical target and provide actual non-zero generation footprint/update-latency observations.

## Why this matters

The protocol classifiers receive facts such as `commissioner_authorization_generation_current`; that decision is only as durable as the persistent state supplying it. A constrained peer must not reboot into an older authorization generation and thereby make an obsolete commissioner grant, association, resumption credential, or release authority appear current.

The evidence contract therefore records the physical generation representation and recovery context while leaving authorization-generation semantics with the existing lifecycle authority.

## Qualification result for this packet

Repository-owned schema/self-test validation exercises only the evidence checker and unmeasured template. No physical target is executed by this packet, and no generated number is a hardware measurement.

## Claim boundary

This packet advances TD-002 instrumentation only. It does not close TD-002 or establish rollback resistance, flash atomicity, secure monotonic storage, physical resource budgets, RNG quality, constant-time behavior, interoperability, external review, or deployment qualification.
