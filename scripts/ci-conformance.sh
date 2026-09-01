#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
EVIDENCE="$ROOT/evidence/conformance"
LOG="$EVIDENCE/conformance.log"
JOBS="${JOBS:-$(getconf _NPROCESSORS_ONLN 2>/dev/null || nproc 2>/dev/null || echo 2)}"
CFLAGS="-std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wno-sign-conversion -Werror -O2 -fno-strict-aliasing -fstack-protector-strong -D_FORTIFY_SOURCE=2"
GIT_BIN="${GIT:-git}"
mkdir -p "$EVIDENCE"

run_step() {
  local name="$1"
  shift
  echo
  echo "== $name =="
  echo "+ $*"
  "$@"
}

run_step_in() {
  local name="$1"
  local dir="$2"
  shift 2
  echo
  echo "== $name =="
  echo "+ (cd $dir && $*)"
  (cd "$dir" && "$@")
}

{
  echo "== ZK-ARCHE cross-language conformance =="
  date -u +"timestamp_utc=%Y-%m-%dT%H:%M:%SZ"
  echo "root=$ROOT"

  run_step "formal model mirror synchronization" \
    bash "$ROOT/scripts/sync-formal-models.sh" --check

  run_step "wire error registry/corpus parity" \
    python3 "$ROOT/scripts/check-error-registry-parity.py"

  if [ ! -x "$ROOT/c/build/tests/test_vectors" ]; then
    run_step "build C vector harness" \
      make -C "$ROOT/c" -j"$JOBS" CFLAGS="$CFLAGS" build/tests/test_vectors
  fi

  run_step "C harness against Rust vectors" \
    "$ROOT/c/build/tests/test_vectors" "$ROOT/rust/test-vectors/0x0001"

  run_step_in "Rust vector regeneration" "$ROOT/rust" \
    cargo run --locked \
      --example gen_test_vectors --features test-vectors

  run_step "generated vector drift check" \
    "$GIT_BIN" -C "$ROOT" diff --exit-code -- rust/test-vectors/0x0001

  echo
  echo "cross-language conformance: PASS"
} 2>&1 | tee "$LOG"
