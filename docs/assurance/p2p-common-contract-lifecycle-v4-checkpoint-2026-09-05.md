# P2P Common Contract lifecycle v4 qualification checkpoint — 2026-09-05

## Scope

This checkpoint advances executable Common Contract qualification without promoting `p2p-iot-core` or claiming physical-target, field, external-review, or complete interoperability evidence.

The canonical lifecycle corpus is `rust/test-vectors/p2p/common-contract-lifecycle-v4.txt`. Both Rust and C consumers use the same corpus.

## What v4 adds

Version 4 preserves every v3 fact and adds key-usage/counter continuity (`usage_counter_continuity_current`) as a mandatory Common Contract prerequisite.

The fact was already a mandatory reject condition on the resumption and transport-continuation surfaces, and CORE association admission gained it in `18aa670`. Until this corpus version the P2P surface did not model it, so both P2P harnesses had to supply it as a constant. They now read it from the corpus.

Three negative cases cover it in every peer-class direction, mirroring how restart continuity is covered:

| Case | Direction | Condition |
|---|---|---|
| `XC4-022` | `mcu-core` → `linux-edge` | key-usage continuity stale |
| `XC4-023` | `linux-edge` → `mcu-core` | key-usage continuity stale |
| `XC4-024` | `mcu-core` → `mcu-core` | key-usage continuity stale |

Covering all three directions is the point: resource asymmetry may reduce optional functionality, but it MUST NOT reduce the mandatory authentication floor. A high-capability peer receives no weaker decision path than a constrained one, and a constrained peer must reach the same fail-closed decision about a high-capability peer.

Losing key-usage continuity is a nonce/key-reuse hazard rather than a replay-window question, so it fails closed independently of `replay_continuity_current` and `restart_continuity_current`.

## Evidence obtained in this run

Corpus shape:

```text
cases=24 columns=21 establish=6 fail_closed=18 offline_establish=5 cross_class=20 same_class=4
```

The C consumer was compiled and executed locally with `cc (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0` and:

```text
-std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Werror
```

Observed results:

```text
p2p common-contract C lifecycle qualification v4: ok cases=24 establish=6 fail_closed=18 offline_establish=5
p2p-common-contract-mutations: PASS canonical=24 positive=6 mutations=92 dimensions=16
p2p-common-contract-cross-language: PASS corpus=common-contract-lifecycle-v4 mutations=pass C=pass Rust=pass
cross-module-lifecycle-invariants: PASS surfaces=7 authz_generation=12 revocation=6 lineage=6
  replay_restart=7 usage_counter=6 transport_non_authority=2 infrastructure_non_authority=1
  delegation_non_repair=7
```

Unlike the v2 checkpoint, Rust execution was available in this run and is reported as PASS. The cross-language gate exercised the C consumer, the mutation qualification, and the Rust consumer against the same corpus.

Mutation breadth rose from 86 to 92 across 16 dimensions, because `usage_counter_continuity_current` is now a mutable fail-closed dimension applied to all six establishment baselines.

The corpus was checked against a negative control: with the CORE classifier's usage-counter check disabled, the C harness aborts on `XC4-022`. The new cases are load-bearing rather than decorative.

## Claim boundary

This evidence supports a stronger **TESTED local decision contract** for cross-class Common Contract lifecycle behavior. It does not by itself establish:

- on-wire MCU↔MCU or MCU↔edge interoperability;
- physical STM32/ESP32-S3 resource or persistence evidence;
- that key-usage counters are actually persisted or mirrored correctly on a real target — the corpus models the decision given the fact, not the storage physics that produce it;
- complete restart/rollback storage correctness;
- complete revocation-convergence timing evidence;
- formal proof of the full Common Contract;
- independent cryptographic review;
- production-selectable `p2p-iot-core` status;
- deployment qualification.

TD-001 through TD-004 remain open. `zk239`–`zk241` retain their current roadmap score thresholds until their declared exit evidence exists.
