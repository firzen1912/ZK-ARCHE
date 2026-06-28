# ZK-ARCHE Python Lane

Python reference implementation lane for the ZK-ARCHE v2 protocol, designed to match the Rust and C wire format and protocol capabilities:

- Native packet header, TLV, HELLO, SETUP, AUTH, and ACK payload codecs.
- Ristretto255/SHA-256/HKDF-SHA256/HMAC-SHA256 suite `0x0001`.
- Setup enrollment with pinned/TOFU server key policy and optional pairing token.
- Online authentication with PID derivation, Schnorr proofs, role commitment re-randomization, CDS-OR role-set proof, ECDH-style session key derivation, and key confirmation.
- UDP and TCP transport bindings. TCP uses the same `u32_le length || packet` framing as the Rust/C implementations.
- Filesystem credential, registry, server-key, and in-memory replay-cache stores compatible with the Rust filesystem byte layout.
- Test-vector validation against the Rust `test-vectors/0x0001` set.

## Runtime requirement

This implementation uses Python `ctypes` against system `libsodium` for Ristretto255. No PyPI crypto wrapper is required, but `libsodium` must expose the `crypto_core_ristretto255_*` and `crypto_scalarmult_ristretto255*` APIs.

On Debian/Ubuntu:

```bash
sudo apt-get install libsodium23
```

## Install and test standalone

```bash
python -m pip install -e .[dev]
pytest
```

## CLI examples

Start a UDP server:

```bash
zk-arche-server --bind 127.0.0.1:4000 --transport udp --state-dir ./server-state
```

Enroll a client with TOFU enabled for lab use:

```bash
zk-arche-client --server 127.0.0.1:4000 --transport udp --state-dir ./client-state --setup --allow-tofu-setup
```

Authenticate:

```bash
zk-arche-client --server 127.0.0.1:4000 --transport udp --state-dir ./client-state
```

Generate deterministic device identity from a 32-byte root hex string:

```bash
zk-arche-client --derive-from-root 000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f
```

## Notes

The role-membership proof is a direct port of the Rust/C CDS-OR proof and should still be treated as security-sensitive custom cryptography requiring external review before production use. The server lookup during `AUTH_1` remains `O(n)` over the registry because the PID hides the device identity.


## Parent-repository validation

From the unified repository root:

```bash
./scripts/ci-python.sh
```

The Python lane is intended as a readable reference and interop aid. It is not the constrained-device implementation target; STM32/ESP32-S3-class constraints are governed by the C lane and the `iot-core` profile in `../docs/improvement-roadmap.md`.
