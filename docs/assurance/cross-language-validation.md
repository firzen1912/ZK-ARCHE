# Cross-Language Validation Notes

The unified repository keeps Rust and C in separate implementation lanes and validates them as one protocol project.

## Primary interop anchor

```text
rust/test-vectors/0x0001/
```

The Rust vector corpus is the canonical deterministic byte-level interop source. The C test-vector harness should be run against this path:

```bash
cd c
make
./build/tests/test_vectors ../rust/test-vectors/0x0001
```

## Baseline command inventory

Release qualification, intended for local release checks and the required CI gate:

```bash
bash ./scripts/ci-release-qualification.sh
```

On Windows with Git Bash installed:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\ci-release-qualification.ps1
```

This command runs:

- Rust fmt/check/test/clippy/audit.
- C cppcheck, build/tests, and configured sanitizer checks.
- The C vector harness against the canonical Rust vector corpus.
- Rust deterministic vector regeneration followed by a git drift check.

Rust:

```bash
./scripts/ci-rust.sh
```

C:

```bash
./scripts/ci-c.sh
```

Combined:

```bash
./scripts/ci-all.sh
```

When the combined command starts from a clean Git HEAD, it verifies that the
same HEAD and clean worktree remain after validation, then writes:

```text
evidence/qualification/<head>-qualification-manifest.tsv
evidence/qualification/<head>-ci-all.log
```

A dirty development run still executes the validation lanes but does not
produce an exact-HEAD qualification record.

Bounded Rust parser fuzzing is an explicit opt-in lane because it requires a
nightly toolchain and `cargo-fuzz` and consumes a configured time budget per
target:

```bash
ZK_ARCHE_FUZZ_SECONDS=60 ./scripts/ci-fuzz.sh
```

Its minimized seed inputs are retained in `rust/fuzz/corpus/`. Any crash
reproducer under `rust/fuzz/artifacts/` makes the lane fail. The bounded fuzz
lane disables LeakSanitizer because ptrace-based runners cannot execute it;
repository sanitizer lanes remain responsible for leak qualification.

## Evidence policy

A passing vector test demonstrates agreement for the checked vector cases only. It does not prove complete cryptographic security, side-channel resistance, complete replay resistance, or production readiness.

The release-qualification evidence log is written to:

```text
evidence/release-qualification/release-qualification.log
```

The security regression coverage is aligned with the safe categories in the historical `firzen1912/zk-arche-compare` security guide: transcript/message mutation, invalid encodings, session uniqueness, packet parser strictness, replay-cache rejection, and RNG/proof negative cases. DoS and long-running fuzzing remain explicit opt-in tests because they need a running target or extended runtime.
