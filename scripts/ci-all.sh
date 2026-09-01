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
  echo "== cross-language conformance lane =="
  bash "$ROOT/scripts/ci-conformance.sh"
} 2>&1 | tee "$LOG"
