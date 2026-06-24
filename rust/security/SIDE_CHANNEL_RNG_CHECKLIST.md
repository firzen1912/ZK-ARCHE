# Side-Channel and RNG Checklist

Use this checklist for each deployment target.

## RNG
- Identify the RNG source used by the platform.
- Verify it is cryptographically secure and seeded before use.
- Run a boot-time sanity check that fails closed if RNG is unavailable.
- Never reuse Schnorr nonces or ephemeral Diffie-Hellman secrets.

## Secret handling
- Zeroize private scalars, nonce material, blinding values, and session keys after use.
- Avoid logging secrets, scalars, nonces, MAC keys, or derived session keys.
- Disable core dumps on production Linux targets.
- Protect flash/NVS storage using hardware-backed protection where available.

## Timing and power
- Use constant-time crypto libraries for scalar arithmetic and MAC verification.
- Avoid branching on secret data in protocol glue code.
- On MCU targets, evaluate power/EM leakage if physical attackers are in scope.

## Debug interfaces
- Disable JTAG/SWD/UART debug access for production devices.
- Disable verbose protocol tracing in production firmware.
- Use secure boot and signed firmware where possible.

## Platform notes
- Raspberry Pi/Linux gateway: use OS RNG, file permissions, no core dumps, service sandboxing.
- ESP32: use esp_random/mbedTLS or libsodium-compatible RNG, NVS encryption, flash encryption, secure boot.
- STM32/FreeRTOS: use hardware RNG peripheral, flash readout protection, explicit zeroization, network-stack fuzzing.
- Zephyr: use sys_csrand_get, settings/NVS storage, stack canaries, MPU where available.
