# Compact STM32U585 ZK-ARCHE port

Primary bring-up target: **WeAct STM32U585CIU6 Core Mini**.

The objective is a real ZK-ARCHE endpoint on Cortex-M33. The connected Pi/Orin serial bridge forwards opaque packets only; it is not allowed to verify proofs, select roles, pin identities, mutate trust, or derive session keys on behalf of the MCU.

The repository also retains NUCLEO-L432KC as a later minimum-resource stress target after the Cortex-M33 implementation has proven byte-level parity.

## Current port contents

- `Inc/zk_arche_stm32.h` — HAL-facing port contract.
- `Src/zk_arche_sodium_rng.c` — installs STM32 hardware RNG as libsodium's randombytes provider before `auth_init()`.
- `Src/zk_arche_stm32_client.c` — canonical SETUP + AUTH client over a bounded UART packet transport.
- `Inc/zk_arche_ram_store.h` / `Src/zk_arche_ram_store.c` — bring-up-only credential store that survives within one boot but intentionally does not survive reset.
- `Src/integration_example.c` — CubeMX integration example.
- `tools/build-libsodium.sh` — Cortex-M33 static libsodium cross-build helper.

## 1. Generate the STM32 project

Create a CubeMX/CubeIDE project for **STM32U585CIU6** and enable at minimum:

```text
RNG                     enabled
USART1                   asynchronous, 921600 baud, 8-N-1
SWD                      enabled for bring-up/debug
GPIO                     as needed for status LED
```

The UART rate is not security-relevant and may be lowered if signal integrity requires it. Both ends must match.

Generate the project and confirm `huart1` and `hrng` exist. If you choose a different UART, update `integration_example.c` accordingly.

## 2. Build libsodium for Cortex-M33

Use a pinned/reviewed libsodium source release and an Arm GNU bare-metal toolchain:

```bash
./platforms/stm32-u585-core-mini/tools/build-libsodium.sh \
  /absolute/path/to/libsodium \
  /absolute/path/to/zk-sodium-u585
```

Then add to the CubeIDE/CMake project:

```text
/absolute/path/to/zk-sodium-u585/include
/absolute/path/to/zk-sodium-u585/lib/libsodium.a
```

Also add ZK-ARCHE include/source paths:

```text
c/include
c/src/crypto/*.c
c/src/wire/*.c
c/src/proto/*.c
platforms/stm32-u585-core-mini/Inc
platforms/stm32-u585-core-mini/Src/zk_arche_sodium_rng.c
platforms/stm32-u585-core-mini/Src/zk_arche_stm32_client.c
platforms/stm32-u585-core-mini/Src/zk_arche_ram_store.c
```

Do not add the POSIX `c/src/transport` or filesystem `c/src/store/store_fs.c` sources to the bare-metal build.

Recommended first-pass target flags:

```text
-Os
-mcpu=cortex-m33
-mthumb
-ffunction-sections
-fdata-sections
-fno-strict-aliasing
-Wl,--gc-sections
```

Do not use LTO for the first qualification build. Retain the linker `.map` file.

## 3. Integrate after Cube initialization

In the generated `main.c`, after clock/peripheral setup:

```c
MX_GPIO_Init();
MX_USART1_UART_Init();
MX_RNG_Init();

zk_arche_demo_run();
```

Add a declaration for `zk_arche_demo_run()` or expose it through your application header.

The example uses a pairing token and keeps TOFU disabled. Start the Linux responder with the same token:

```bash
zk-arche-server \
  --bind 127.0.0.1:4040 \
  --transport udp \
  --state-dir ./server-state \
  --require-pairing-token zk-arche-lab-token
```

## 4. Connect STM32 UART to the Linux bridge

Use a **3.3 V UART** connection or a 3.3 V USB-UART adapter:

```text
STM32 TX  -> bridge RX
STM32 RX  <- bridge TX
STM32 GND -> bridge GND
```

Do not connect 5 V UART signaling to the MCU.

On Pi/Orin/another Linux host:

```bash
python3 platforms/linux/serial_bridge.py \
  --serial /dev/ttyUSB0 \
  --baud 921600 \
  --server 127.0.0.1:4040
```

The UART format is deliberately tiny:

```text
u16 big-endian packet length
raw ZK-ARCHE packet bytes
```

The bridge forwards exactly those packet bytes to UDP and returns exactly the responder bytes. It has no ZK-ARCHE identity/trust authority.

## 5. Flash/debug

Build and flash with STM32CubeIDE or an ST-Link-compatible SWD probe. USB DFU may also be useful for board recovery where supported by the board/boot configuration, but SWD is preferred during cryptographic bring-up because breakpoints, fault inspection, map analysis and reset-state testing are required.

A successful `zk_arche_client_run()` returns `AUTH_OK`. The supplied integration example breaks into the debugger on failure rather than printing secret state.

## Current storage boundary

The included RAM credential store is intentionally **not persistent**. It is enough to prove in one boot that:

```text
hardware RNG -> canonical crypto -> SETUP -> credential state -> AUTH
```

It is not enough for persistent identity, restart/replay qualification, rollback resistance, secure provisioning, or TD-002 closure.

The production/qualification storage backend must reserve dedicated flash space through the final linker/Cube memory map and implement at minimum:

- versioned credential record;
- erase/program failure detection;
- power-loss-safe update strategy;
- rollback/clone assumptions;
- explicit debug/readout protection posture;
- zeroization/reprovision policy.

Do not hard-code an arbitrary flash page before that memory region is reserved by the actual firmware linker layout.

## Acceptance gate

Do not label the STM32 firmware `p2p-iot-core` conformant until all of the following are retained:

1. successful Cortex-M33 build with exact source/toolchain/dependency versions;
2. exact canonical vector parity with Rust/C host implementations;
3. raw flash/static RAM/stack/heap measurements;
4. RNG execution manifest and failure behavior;
5. SETUP + AUTH against Pi/Orin without external identity infrastructure;
6. negative proof, replay, malformed-packet and timeout cases;
7. persistent-store restart evidence once the flash backend is added;
8. bidirectional interoperability once a responder role is available on the MCU.

This preserves the rule that a transport bridge may assist connectivity but may never become the root authentication decision-maker.