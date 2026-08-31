#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

EVIDENCE="evidence/release-qualification"
LOG="$EVIDENCE/release-qualification.log"
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
  echo "== ZK-ARCHE release qualification =="
  date -u +"timestamp_utc=%Y-%m-%dT%H:%M:%SZ"
  echo "root=$ROOT"

  GIT_BIN="${GIT:-git}"

  echo
  echo "== tool inventory =="
  for tool in cargo rustc make pkg-config cppcheck proverif; do
    if command -v "$tool" >/dev/null 2>&1; then
      echo "$tool=$(command -v "$tool")"
    else
      echo "$tool=missing"
    fi
  done
  if command -v "$GIT_BIN" >/dev/null 2>&1; then
    echo "git=$(command -v "$GIT_BIN")"
  else
    echo "git=missing"
  fi
  if command -v gcc >/dev/null 2>&1; then echo "gcc=$(command -v gcc)"; fi
  if command -v clang >/dev/null 2>&1; then echo "clang=$(command -v clang)"; fi

  run_step "formal qualification" \
    bash "$ROOT/scripts/ci-formal.sh"

  run_step "Rust lane" bash "$ROOT/scripts/ci-rust.sh"
  run_step "C lane" bash "$ROOT/scripts/ci-c.sh"

  run_step_in "C harness against Rust vectors" "$ROOT/c" \
    ./build/tests/test_vectors ../rust/test-vectors/0x0001

  run_step_in "Rust vector regeneration" "$ROOT/rust" \
    cargo run --example gen_test_vectors --features test-vectors
  run_step "generated vector drift check" \
    "$GIT_BIN" -C "$ROOT" diff --exit-code -- rust/test-vectors/0x0001

  echo
  echo "release qualification: PASS"
} 2>&1 | tee "$LOG"
