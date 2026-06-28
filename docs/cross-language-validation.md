# Cross-Language Validation Notes

The unified repository keeps Rust, C, and Python in separate lanes but validates them as one protocol project.

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

The Python lane currently carries mirrored fixtures under:

```text
python/test-vectors/0x0001/
```

Before vector semantics change, add or run a sync check that compares the Python fixture set against the Rust vector corpus.

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

Python:

```bash
./scripts/ci-python.sh
```

Combined:

```bash
./scripts/ci-all.sh
```

## Evidence policy

A passing vector test demonstrates agreement for the checked vector cases only across the lanes where that vector is implemented. It does not prove complete cryptographic security, side-channel resistance, complete replay resistance, or production readiness.
