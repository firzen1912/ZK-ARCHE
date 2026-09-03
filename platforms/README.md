# ZK-ARCHE hardware deployment platforms

This directory turns the protocol implementations into board/deployment targets without changing ZK-ARCHE wire or cryptographic semantics.

## Target matrix

| Target | Class | Initial role | Transport | Status |
|---|---|---|---|---|
| Raspberry Pi 5 | Linux-edge | client/server/commissioner harness | UDP/TCP | deployable from the existing C lane |
| NVIDIA Jetson Orin Nano | accelerated-edge | client/server/commissioner harness | UDP/TCP | deployable from the existing C lane |
| Seeed Studio XIAO ESP32-S3 Plus | MCU-plus | constrained client/peer | Wi-Fi UDP first; BLE later | port scaffold; requires a full Ristretto255-capable libsodium component and on-board validation |
| ST NUCLEO-L432KC | MCU-core | constrained client/peer | UART/USB first | qualification scaffold; crypto footprint must be proven before claiming a flashable AUTH profile |

## Non-negotiable porting rule

The active C implementation uses the ZK-ARCHE suite through `c/include/auth/crypto.h`, with the current host backend implemented by libsodium/Ristretto255. Board ports MUST NOT silently replace Ristretto255, transcript hashing, HKDF, HMAC, packet formats, domain separation, replay behavior, or credential semantics merely because a platform library exposes a different primitive.

A target is considered a ZK-ARCHE protocol port only when it preserves the same byte-level and decision-level semantics as the canonical vectors. A board-specific firmware that only proves boot, RNG, storage, or transport is a bring-up artifact, not protocol-conformance evidence.

## Port layering

```text
                ZK-ARCHE normative behavior
                         |
              c/include/auth/*.h
                         |
       c/src/proto + c/src/wire + crypto API
                         |
        +----------------+----------------+
        |                |                |
   Linux adapter     ESP-IDF adapter   STM32 adapter
        |                |                |
 libsodium/POSIX     full sodium      reviewed embedded
 UDP/TCP/files       RNG/NVS/UDP      RNG/storage/UART
        |                |                |
    Pi / Orin       XIAO ESP32-S3+   NUCLEO-L432KC
```

The protocol core remains transport/storage agnostic. A port should adapt:

1. cryptographic backend implementing the existing `auth/crypto.h` contract;
2. entropy/RNG integration;
3. persistent credential/trust storage;
4. transport send/receive and MTU behavior;
5. monotonic/replay persistence where required;
6. timing/resource measurement hooks.

## Qualification order

1. Build and run the existing host tests on x86/Linux.
2. Deploy the C server/client on Raspberry Pi 5 and Jetson Orin Nano.
3. Retain Linux-edge and accelerated-edge manifests.
4. Bring up XIAO ESP32-S3 Plus RNG, NVS and UDP.
5. Compile the unmodified protocol/wire core against a full Ristretto255-capable embedded crypto backend.
6. Reproduce canonical vectors on ESP32-S3 before doing live SETUP/AUTH.
7. Bring up NUCLEO-L432KC with hardware RNG, bounded persistent storage, UART/USB transport and measured memory map.
8. Prove that the selected crypto backend fits the L432KC. If it does not, record that result rather than weakening the mandatory suite.
9. Run the no-infrastructure matrix in both directions:

```text
STM32 <-> STM32
STM32 <-> ESP32-S3
STM32 <-> Raspberry Pi 5
STM32 <-> Jetson Orin Nano
```

## Evidence required per board

Use the existing constrained-target evidence contract and templates under `docs/assurance/` and `evidence/constrained-target/`. At minimum retain board/revision, toolchain, source commit, crypto backend/version, accelerator path, RNG/DRBG posture, key-storage location, boot/debug state, wire bytes, flash/static RAM/heap/stack, latency/CPU measurements, restart/replay behavior, transport/MTU and dependency inventory.

Do not use a successful build or a successful single handshake as a substitute for this evidence.

## Directory ownership

- `platforms/linux/` — Raspberry Pi 5 and Jetson Orin Nano deployment helpers.
- `platforms/esp32-xiao-s3-plus/` — ESP-IDF integration contract and firmware port.
- `platforms/stm32-nucleo-l432kc/` — STM32Cube/embedded integration contract and constrained-target bring-up.

These files are integration surfaces. Security- or wire-relevant behavior remains owned by the canonical spec, C/Rust implementations, vectors and assurance gates.