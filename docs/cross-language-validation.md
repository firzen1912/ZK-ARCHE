# Cross-Language Validation Notes

The combined repository keeps Rust and C in separate lanes but validates them as one protocol project.

## Primary interop anchor

```text
rust/test-vectors/0x0001/
```

The C test-vector harness should be run against this path:

```bash
cd c
make
./build/tests/test_vectors ../rust/test-vectors/0x0001
```

## Baseline command inventory

Rust:

```bash
cd rust
./scripts/ci-rust.sh
```

C:

```bash
cd c
./scripts/ci-c.sh
```

Combined:

```bash
./scripts/ci-all.sh
```

## Evidence policy

A passing vector test demonstrates agreement for the checked vector cases only. It does not prove complete cryptographic security, side-channel resistance, complete replay resistance, or production readiness.
