# ZK-ARCHE Rust Lane

This directory contains the Rust reference implementation lane for the ZK-ARCHE v2 IoT authentication protocol. It owns the deterministic test-vector corpus used for cross-language validation with the C lane.

## Layout

```text
rust/
|-- Cargo.toml                 Workspace manifest
|-- crates/proto/              Protocol library
|-- crates/client/             Client binary
|-- crates/server/             Server binary
|-- fuzz/                      cargo-fuzz targets
|-- models/proverif/           Symbolic model skeleton
|-- scripts/ci-rust.sh         Rust validation script
|-- spec/                      Wire-spec generator and generated docx
`-- test-vectors/0x0001/       Deterministic JSON vectors
```

## Build and Test

From the unified repository root:

```bash
./scripts/ci-rust.sh
```

From this directory:

```bash
cargo fmt --all -- --check
cargo check --workspace --locked --all-targets
cargo test --workspace --locked --all-features
cargo clippy --workspace --all-targets --all-features -- -D warnings
```

## Regenerate Test Vectors

```bash
cargo run --example gen_test_vectors --features test-vectors
```

The generated vectors are written under:

```text
test-vectors/0x0001/
```

The C lane validates against those files from the repository root layout:

```bash
cd ../c
make
./build/tests/test_vectors ../rust/test-vectors/0x0001
```

## Run Locally

```bash
cargo run -p server -- --bind 0.0.0.0:4000 --state-dir ./server-state
cargo run -p client -- --server 127.0.0.1:4000 --transport udp --state-dir ./client-state --setup --allow-tofu-setup
cargo run -p client -- --server 127.0.0.1:4000 --transport udp --state-dir ./client-state
```

Runtime state directories are ignored by default. Use the shared assurance document for validation, fuzzing, replay, and review guidance:

```text
../docs/assurance-and-validation.md
```

## License

Apache-2.0.
