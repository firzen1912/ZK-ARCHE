# Seeed Studio XIAO ESP32-S3 Plus

This ESP-IDF project adapts the canonical ZK-ARCHE C client to the XIAO ESP32-S3 Plus without changing the protocol suite. It uses:

- Espressif's ESP-IDF `libsodium` component pinned to 1.0.22;
- the repository's existing `c/src/crypto`, `c/src/wire`, and `c/src/proto` sources;
- Wi-Fi + UDP for the first interoperability transport;
- NVS for the device root, pinned server key, role commitment/blind and role code;
- GPIO21 user LED for a simple result signal.

A fresh board creates a device root once and persists it in NVS. If enrollment state is missing, firmware executes explicit SETUP; after successful enrollment it immediately executes AUTH. Subsequent boots reuse the persisted credentials and execute AUTH.

## Prerequisites

Install a supported ESP-IDF toolchain, activate its environment, and verify `idf.py` is available.

## Configure

From the repository root:

```bash
cd platforms/esp32-xiao-s3-plus
idf.py set-target esp32s3
idf.py menuconfig
```

Under **ZK-ARCHE XIAO ESP32-S3 Plus**, set:

- Wi-Fi SSID/password;
- Raspberry Pi or Jetson IPv4 address;
- UDP port (default 4040);
- requested role;
- allowed peer roles;
- optional pairing token.

`Allow first-boot TOFU enrollment` defaults OFF. Enable it only for an intentionally controlled bring-up environment when you understand the trust consequence. Normal AUTH never learns new trust.

## Build

```bash
idf.py build
```

The ESP-IDF component manager resolves the pinned `espressif/libsodium` dependency. The build deliberately keeps ZK-ARCHE's first qualification profile in internal RAM instead of making PSRAM a hidden requirement.

## Flash and monitor

Linux/macOS example:

```bash
idf.py -p /dev/ttyACM0 flash monitor
```

Windows example:

```powershell
idf.py -p COM5 flash monitor
```

Use the actual port for your board.

Successful first boot should progress through:

```text
crypto init
NVS credential load/create
Wi-Fi association
SETUP (only when enrollment state is missing)
credential persistence
AUTH
3 short user-LED pulses
```

A failure produces an error log and 10 short pulses.

## Pi/Orin responder

On the Linux peer, configure the service to bind an isolated LAN interface or run directly:

```bash
./c/build/auth_server \
  --bind 0.0.0.0:4040 \
  --transport udp \
  --state-dir ./server-state
```

If the server requires a pairing token, configure the same token in the ESP-IDF project.

## Security and qualification notes

- The XIAO firmware does not log device-root, private scalar, role blind, or session keys.
- NVS persistence is a functional bring-up backend, not yet a claim of secure-at-rest key storage.
- Flash encryption, secure boot, debug lockdown and production provisioning must be evaluated separately and recorded in the target manifest.
- `CONFIG_LIBSODIUM_USE_MBEDTLS_SHA=n` is retained defensively because a 2026 libsodium/ESP-IDF hardware-SHA integration regression caused silent hash corruption on ESP32-family targets. Canonical vector parity must be established before changing this setting.
- A successful live AUTH is not enough to clear TD-002. Retain flash/RAM/stack/latency/restart/RNG/storage/transport evidence using the repository templates.

## Immediate validation gate

Before treating this firmware as a conformant hardware port, run the canonical C/Rust vector material through an ESP32 test harness and compare the exact Ristretto points, transcript hashes, packet bytes, key-confirmation values and decisions. The live SETUP/AUTH path must not substitute for byte-level vector parity.