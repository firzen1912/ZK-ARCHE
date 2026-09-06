#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
mkdir -p "$ROOT/evidence"
LOG="$ROOT/evidence/ci-all.log"

# A qualification record is valid only when validation starts from an exact,
# clean repository HEAD. Development runs against dirty trees still execute,
# but cannot produce per-HEAD evidence.
QUALIFICATION_HEAD=""
if command -v git >/dev/null 2>&1 \
  && git -C "$ROOT" rev-parse --verify HEAD >/dev/null 2>&1 \
  && [ -z "$(git -C "$ROOT" status --porcelain 2>/dev/null)" ]; then
  QUALIFICATION_HEAD="$(git -C "$ROOT" rev-parse --verify HEAD)"
fi

{
  echo "== ZK-ARCHE unified CI =="
  date -u +"timestamp_utc=%Y-%m-%dT%H:%M:%SZ"
  echo "root=$ROOT"
  echo
  echo "== constrained-target evidence contract =="
  python3 "$ROOT/scripts/check-constrained-target-manifest.py" \
    "$ROOT/evidence/constrained-target/manifest-template.json"
  echo
  echo "== rust lane =="
  bash "$ROOT/scripts/ci-rust.sh"
  echo
  echo "== c lane =="
  bash "$ROOT/scripts/ci-c.sh"
  echo
  echo "== c vector harness against rust vectors =="
  cd "$ROOT/c"
  if [ -x ./build/tests/test_vectors ]; then
    ./build/tests/test_vectors ../rust/test-vectors/0x0001
  else
    echo "C vector test binary not found after C CI; run make in c/ and retry." >&2
    exit 1
  fi
  echo
  echo "== P2P Common Contract cross-language qualification =="
  bash "$ROOT/scripts/check-p2p-common-contract-cross-language.sh"
} 2>&1 | tee "$LOG"

if [ -n "$QUALIFICATION_HEAD" ]; then
  bash "$ROOT/scripts/record-qualification.sh" "$QUALIFICATION_HEAD" "$LOG"
else
  echo "qualification record: NOT RECORDED (validation did not start from a clean repository HEAD)"
fi
