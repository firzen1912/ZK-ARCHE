#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
mkdir -p "$ROOT/evidence"
LOG="$ROOT/evidence/ci-all.log"
{
  echo "== ZK-ARCHE unified CI =="
  date -u +"timestamp_utc=%Y-%m-%dT%H:%M:%SZ"
  echo "root=$ROOT"
  echo
  echo "== rust lane =="
  bash "$ROOT/scripts/ci-rust.sh"
  echo
  echo "== c lane =="
  bash "$ROOT/scripts/ci-c.sh"
  echo
  echo "== python lane =="
  bash "$ROOT/scripts/ci-python.sh"
  echo
  echo "== c vector harness against rust vectors =="
  cd "$ROOT/c"
  if [ -x ./build/tests/test_vectors ]; then
    ./build/tests/test_vectors ../rust/test-vectors/0x0001
  else
    echo "C vector test binary not found after C CI; run make in c/ and retry." >&2
    exit 1
  fi
} 2>&1 | tee "$LOG"
