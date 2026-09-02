# P2P Common Contract lifecycle v2 qualification checkpoint — 2026-09-02

## Scope

This checkpoint advances executable Common Contract qualification without promoting `p2p-iot-core` or claiming physical-target, field, external-review, or complete interoperability evidence.

The canonical lifecycle corpus is `rust/test-vectors/p2p/common-contract-lifecycle-v2.txt`. Both Rust and C consumers use the same corpus.

## What v2 adds

Version 2 preserves the existing CORE association-admission facts and adds qualification-level prerequisites for:

- current authorization generation;
- restart continuity;
- mandatory constrained-floor compatibility.

The corpus exercises MCU↔MCU, MCU→edge, and edge→MCU decisions; offline success with sufficient local state; authorization freshness and generation; revocation and explicit revocation; lineage; replay and restart continuity; required binding; mandatory-floor mismatch; and rejection of trust mutation during normal AUTH.

A high-capability peer receives no weaker decision path. Infrastructure availability is deliberately not an authority input: otherwise-valid local state can establish while infrastructure is absent, and infrastructure presence cannot repair stale authorization, revocation, lineage, replay/restart continuity, binding, or mandatory-floor failures.

## Evidence obtained in this run

The C consumer was compiled and executed locally with:

```text
gcc -std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Werror
```

Observed result:

```text
p2p common-contract C lifecycle qualification v2: ok cases=20 establish=6 fail_closed=14 offline_establish=5
```

The cross-language gate remains fail-closed when Cargo is unavailable. Rust source consumes the same v2 corpus, but Rust execution was unavailable in this run and is therefore not reported as PASS.

## Claim boundary

This evidence supports a stronger **TESTED local decision contract** for cross-class Common Contract lifecycle behavior. It does not by itself establish:

- on-wire MCU↔MCU or MCU↔edge interoperability;
- physical STM32/ESP32-S3 resource or persistence evidence;
- complete restart/rollback storage correctness;
- complete revocation-convergence timing evidence;
- formal proof of the full Common Contract;
- independent cryptographic review;
- production-selectable `p2p-iot-core` status;
- deployment qualification.

`zk239`–`zk241` therefore retain their current roadmap score thresholds until their declared exit evidence exists.
