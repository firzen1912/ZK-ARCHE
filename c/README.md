# auth-c — C port of the ZK-ARCHE v2 IoT authentication protocol

A production-quality C implementation of the protocol, wire-compatible
with the Rust reference. Single registry, shared state, interoperable
at the byte level in every direction: C ↔ C, C ↔ Rust, either way
over UDP or TCP.

## Directory layout

```
auth-c/
├── include/auth/          public headers
│   ├── iot_auth.h             error codes, sizes, init
│   ├── auth_crypto.h      RNG, points, scalars, HKDF, HMAC, Blake2b
│   ├── auth_transcript.h  canonical transcript builder, PID
│   ├── auth_proofs.h      Schnorr/rerand/CDS-OR prove + verify
│   ├── auth_wire.h        24-byte header, TLV codec, Hello, negotiate
│   ├── auth_payloads.h    SETUP_1/2/3, AUTH_1/2/3, ACK codecs
│   ├── auth_proto.h       setup + auth state machines
│   ├── auth_transport.h   UDP + TCP bindings
│   └── auth_store.h       filesystem-backed stores
├── src/
│   ├── crypto/   auth_crypto.c, auth_proofs.c, auth_transcript.c
│   ├── wire/     auth_wire.c, auth_payloads.c
│   ├── proto/    auth_proto.c
│   ├── transport/auth_transport_{addr,udp,tcp}.c
│   └── store/    auth_store_fs.c
├── bin/          auth_client.c, auth_server.c
├── tests/        test_vectors.c, test_wire.c, test_e2e.c
└── Makefile
```

## Dependencies

Just libsodium (≥ 1.0.18) and a C11 compiler.

```bash
sudo apt install libsodium-dev pkg-config build-essential
```

## Build + test

```bash
make
./build/tests/test_vectors ../iot-auth-refactor/test-vectors/0x0001
./build/tests/test_wire
./build/tests/test_e2e
```

All three test binaries pass with zero failures. The `test_vectors`
harness verifies byte-for-byte match against the Rust reference
test vectors (`transcript`, `pid`, `schnorr_auth_client`,
`rerandomization`, `role_set_membership`, `kdf_kc`) across all 6 DRBG-
seeded fixtures.

## Run

### Standalone C server + C client

```bash
# Start a C server (UDP + TCP concurrently)
./build/auth_server --bind 127.0.0.1:4000 --transport both \
    --state-dir ./server-state

# C client enrolls on first run
./build/auth_client --server 127.0.0.1:4000 --transport udp \
    --state-dir ./client-state --setup --allow-tofu-setup

# Subsequent authentications (UDP or TCP, same credentials)
./build/auth_client --server 127.0.0.1:4000 --transport udp \
    --state-dir ./client-state
./build/auth_client --server 127.0.0.1:4000 --transport tcp \
    --state-dir ./client-state
```

### Cross-language: C client against Rust server (or vice versa)

Fully interoperable. A C-implemented device can enroll with the Rust
server and authenticate over either transport; the reverse works too.

## Design for embedded targets

The library is written to be portable to a constrained IoT node:

- **No heap in hot paths.** All buffers are caller-provided or fixed-
  size on the stack. The single small `malloc` is in the test-vector
  JSON parser.
- **Fixed-size structures.** Points are 32 B, scalars are 32 B,
  sessions are 16 B, packets are capped at `AUTH_MAX_DATAGRAM`
  (2048 B by default, adjustable at compile time).
- **No C++ runtime, no pthreads in the library itself.** Threads only
  appear in the server binary (`bin/auth_server.c`), which uses
  one thread per TCP connection. A single-threaded build is a trivial
  specialization.
- **Structured errors.** Every function returns `auth_err_t`, a
  wire-stable u16. The wire-transmittable range `0x0100..0x07FF` is
  reserved for peer-observable conditions; `0x0001..0x00FF` are
  local-only.
- **Strict warnings.** Compiles clean under `-Wall -Wextra -Wpedantic
  -Wshadow -Wconversion` on GCC and Clang.
- **Constant-time equality** for MAC tag comparison via
  `sodium_memcmp`.

For a true bare-metal port, swap `src/transport/*.c` for lwIP or
Zephyr-net bindings, and `src/store/auth_store_fs.c` for a
flash-sector implementation. The `include/` headers and `src/crypto/`
and `src/wire/` and `src/proto/` translate unchanged.

## License

Apache-2.0.


If omitted, state defaults to `./server-state` for the server and `./client-state` for the client.
