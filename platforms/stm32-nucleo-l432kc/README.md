# NUCLEO-L432KC constrained stress target

The STM32L432KC remains the deliberately constrained ZK-ARCHE MCU-core target:

```text
Cortex-M4F
80 MHz
64 KB SRAM
256 KB flash
hardware RNG
```

It is **not** the first STM32 bring-up target. The current ZK-ARCHE C lane uses a Ristretto255/libsodium crypto backend, so the correct engineering order is:

1. establish exact vector parity on the larger compact STM32U585 Cortex-M33 target;
2. retain the complete crypto/protocol footprint;
3. port the already-proven embedded integration to L432KC;
4. measure whether the mandatory suite fits 64 KB SRAM / 256 KB flash;
5. optimize footprint without changing wire or security semantics;
6. record a truthful unsupported result if the mandatory floor cannot fit.

Do not create a weaker `L432` suite under the same ZK-ARCHE profile name merely to make the binary fit. The constrained target exists to test the bottom-up doctrine, not to create an exception to it.

When work begins here, reuse the STM32U585 transport/store abstractions where possible and produce a separate target manifest, map file, stack watermark, latency run and vector-parity report.