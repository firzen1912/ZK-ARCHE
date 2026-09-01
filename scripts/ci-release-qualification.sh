#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

EVIDENCE="evidence/release-qualification"
LOG="$EVIDENCE/release-qualification.log"
mkdir -p "$EVIDENCE"

export REQUIRE_CARGO_AUDIT="${REQUIRE_CARGO_AUDIT:-1}"
export REQUIRE_SANITIZERS="${REQUIRE_SANITIZERS:-1}"

if ! git -C "$ROOT" diff --quiet --ignore-submodules --; then
  echo "release qualification: FAIL (tracked working-tree changes must be committed first)" >&2
  exit 1
fi
if ! git -C "$ROOT" diff --cached --quiet --ignore-submodules --; then
  echo "release qualification: FAIL (staged changes must be committed first)" >&2
  exit 1
fi

run_step() {
  local name="$1"
  shift
  echo
  echo "== $name =="
  echo "+ $*"
  "$@"
}

{
  echo "== ZK-ARCHE release qualification =="
  date -u +"timestamp_utc=%Y-%m-%dT%H:%M:%SZ"
  echo "root=$ROOT"

  GIT_BIN="${GIT:-git}"

  echo
  echo "== tool inventory =="
  for tool in cargo rustc make pkg-config cppcheck proverif python3; do
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
  run_step "cross-language conformance" \
    bash "$ROOT/scripts/ci-conformance.sh"

  echo
  echo "release qualification: PASS"
} 2>&1 | tee "$LOG"
