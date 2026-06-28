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

Release qualification, intended for local release checks and the required CI
gate:

```bash
bash ./scripts/ci-release-qualification.sh
```

On Windows with Git Bash installed:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\ci-release-qualification.ps1
```

This command runs:

- Rust fmt/check/test/clippy/audit.
- Python install/compile/lint/security audit/pytest.
- C cppcheck, GCC build/tests, clang build/tests when available, ASan, UBSan.
- Rust-vs-Python JSON vector parity.
- The C vector harness against both Rust and Python vector corpora.
- Rust deterministic vector regeneration followed by a git drift check.

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

The release-qualification evidence log is written to:

```text
evidence/release-qualification/release-qualification.log
```

The security regression coverage is aligned with the safe categories in the
`firzen1912/zk-arche-compare` security guide: transcript/message mutation,
invalid encodings, session uniqueness, packet parser strictness, replay cache
rejection, and RNG/proof negative cases. DoS and long-running fuzzing remain
explicit opt-in tests because they need a running target or extended runtime.
