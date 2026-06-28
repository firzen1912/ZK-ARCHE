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
  "$PYTHON" -m pip install --upgrade pip
  "$PYTHON" -m pip install -e '.[dev]'
  echo "== dependency consistency =="
  "$PYTHON" -m pip check
  echo "== bytecode compile =="
  "$PYTHON" -m compileall -q zk_arche tests fuzz
  echo "== ruff syntax/import lint =="
  "$PYTHON" -m ruff check --select F,E9 zk_arche tests fuzz
  echo "== bandit security scan =="
  "$PYTHON" -m bandit -q -r zk_arche
  echo "== python dependency audit =="
  "$PYTHON" -m pip_audit
  echo "== pytest =="
  "$PYTHON" -m pytest -q --basetemp .pytest-tmp
} 2>&1 | tee evidence/python-ci.log
