#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
mkdir -p "$ROOT/evidence"
LOG="$ROOT/evidence/python-ci.log"
{
  echo "== ZK-ARCHE Python CI =="
  date -u +"timestamp_utc=%Y-%m-%dT%H:%M:%SZ"
  echo "root=$ROOT"
  echo
  cd "$ROOT/python"
  python -V
  python -m pip --version
  python -m pip install -e .[dev]
  python -m pytest -q
} 2>&1 | tee "$LOG"
