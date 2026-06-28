#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p evidence

PYTHON="${PYTHON:-python}"
command -v "$PYTHON" >/dev/null 2>&1 || PYTHON=python3

{
  echo "== ZK-ARCHE Python CI =="
  date -u +"timestamp_utc=%Y-%m-%dT%H:%M:%SZ"
  "$PYTHON" -V
  "$PYTHON" -m pip --version
  echo "== install (editable, with dev extras) =="
  "$PYTHON" -m pip install -e .[dev]
  echo "== pytest =="
  "$PYTHON" -m pytest -q
} 2>&1 | tee evidence/python-ci.log
