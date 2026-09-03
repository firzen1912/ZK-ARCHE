# ZK-ARCHE hardware deployment platforms

This directory turns the protocol implementations into board/deployment targets without changing ZK-ARCHE wire or cryptographic semantics.

## Target matrix

| Target | Class | Initial role | Transport | Status |
|---|---|---|---|---|
| Raspberry Pi 5 | Linux-edge | client/server/commissioner harness | UDP/TCP | deployable from the existing C lane |
| NVIDIA Jetson Orin Nano | accelerated-edge | client/server/commissioner harness | UDP/TCP | deployable from the existing C lane |
| Seeed Studio XIAO ESP32-S3 Plus | MCU-plus | constrained client/peer | Wi-Fi UDP first; BLE later | ESP-IDF client port; on-board/vector validation still required |
| WeAct STM32U585CIU6 Core Mini | MCU-core bring-up | constrained client/peer | UART/USB first | primary compact STM32 port target; enough memory to preserve the current suite while measuring it |
| ST NUCLEO-L432KC | MCU-core stress target | minimum-resource client/peer | UART/USB | later 64-KB-RAM/256-KB-flash qualification target; current crypto footprint must be proven before claiming support |

## Why two STM32 targets

The first STM32 port must establish correct byte-level behavior before optimizing for the smallest memory envelope. The compact STM32U585 board gives the current Ristretto255/libsodium-based implementation enough headroom for a faithful Cortex-M33 port while also exposing a true RNG, TrustZone and modern security controls. Once the port reproduces canonical vectors, the NUCLEO-L432KC becomes a deliberate stress target. Failure to fit the L432KC is evidence that drives footprint work; it is not permission to weaken the mandatory security suite.

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
 libsodium/POSIX     ESP libsodium     ARM libsodium / later reviewed backend
 UDP/TCP/files       NVS/Wi-Fi UDP     RNG/flash/UART framing
        |                |                |
    Pi / Orin       XIAO ESP32-S3+   STM32U585 -> L432 stress target
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
4. Build/flash the XIAO ESP32-S3 Plus client, then reproduce canonical vectors before accepting live SETUP/AUTH evidence.
5. Bring up the compact STM32U585 with hardware RNG, bounded flash persistence and UART/USB byte transport.
6. Cross-compile the same Ristretto255 suite; retain exact flash/RAM/stack/latency measurements and canonical vector parity.
7. Run STM32U585 <-> ESP32/Pi/Orin interoperability in both initiator/responder directions as implementations become available.
8. Port the proven MCU path to NUCLEO-L432KC and measure whether the mandatory suite fits its 64 KB RAM / 256 KB flash envelope.
9. If L432KC does not fit, record the result and optimize implementation footprint without changing mandatory protocol semantics.
10. Run the no-infrastructure matrix in both directions:

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
- `platforms/stm32-u585-core-mini/` — primary compact STM32 Cortex-M33 port.
- `platforms/stm32-nucleo-l432kc/` — later minimum-resource STM32 stress/qualification target.

These files are integration surfaces. Security- or wire-relevant behavior remains owned by the canonical spec, C/Rust implementations, vectors and assurance gates.