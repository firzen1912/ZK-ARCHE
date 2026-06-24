# ZK-ARCHE C Lane

This directory contains the C11/libsodium implementation lane for the ZK-ARCHE v2 IoT authentication protocol. It is maintained inside the unified repository and is validated against the Rust test-vector corpus.

## Layout

```text
c/
|-- include/              Public headers
|-- src/                  Crypto, wire, protocol, transport, and store code
|-- bin/                  Client and server binaries
|-- tests/                Unit, end-to-end, wire, and vector tests
|-- fuzz/                 libFuzzer harnesses
|-- models/proverif/      Symbolic model skeleton
|-- scripts/ci-c.sh       C validation script
`-- Makefile
```

## Dependencies

The C lane requires libsodium, pkg-config, make, and a C11 compiler.

```bash
sudo apt-get install libsodium-dev pkg-config build-essential
```

## Build and Test

From the unified repository root:

```bash
./scripts/ci-c.sh
```

From this directory:

```bash
make
make test
./build/tests/test_vectors ../rust/test-vectors/0x0001
```

The vector harness checks byte-level agreement with the Rust vectors for transcript generation, PID derivation, Schnorr auth proof generation, rerandomization, role set-membership, KDF, and key-confirmation tags.

## Run Locally

```bash
./build/server --bind 127.0.0.1:4000 --transport both --state-dir ./server-state
./build/client --server 127.0.0.1:4000 --transport udp --state-dir ./client-state --setup --allow-tofu-setup
./build/client --server 127.0.0.1:4000 --transport udp --state-dir ./client-state
```

Runtime state directories are ignored by default. Use the shared assurance document for validation, fuzzing, replay, and review guidance:

```text
../docs/assurance-and-validation.md
```

## Embedded Design Notes

- Hot protocol paths use caller-provided or fixed-size buffers where practical.
- Packet sizes are capped by `AUTH_MAX_DATAGRAM` unless a build overrides it.
- The library API returns structured `auth_err_t` errors.
- MAC comparisons use libsodium constant-time comparison.
- Platform ports should replace filesystem storage and host transports with target-specific backends while preserving wire and transcript compatibility.

## License

Apache-2.0.
