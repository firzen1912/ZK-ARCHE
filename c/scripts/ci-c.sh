#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p evidence
{
  echo "== dependency check =="
  pkg-config --exists libsodium
  pkg-config --modversion libsodium
  echo "== clean build =="
  make clean
  make -j"$(nproc)" all
  echo "== unit tests =="
  make test
  echo "== ASan build/tests =="
  make clean
  make asan
  echo "== UBSan build/tests =="
  make clean
  make ubsan
} 2>&1 | tee evidence/c-ci.log
