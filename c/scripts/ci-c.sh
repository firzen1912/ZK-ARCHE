#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p evidence

JOBS="${JOBS:-$(getconf _NPROCESSORS_ONLN 2>/dev/null || nproc 2>/dev/null || echo 2)}"
CC="${CC:-cc}"
BASE_CFLAGS="-std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wno-sign-conversion -Werror -O2 -fno-strict-aliasing -fstack-protector-strong -D_FORTIFY_SOURCE=2"

{
  echo "== ZK-ARCHE C CI =="
  date -u +"timestamp_utc=%Y-%m-%dT%H:%M:%SZ"
  echo "cc=$CC"
  "$CC" --version
  echo "== dependency check =="
  pkg-config --exists libsodium
  pkg-config --modversion libsodium
  echo "== static analysis (cppcheck) =="
  if command -v cppcheck >/dev/null 2>&1; then
    cppcheck \
      --std=c11 \
      --enable=warning,portability \
      --error-exitcode=1 \
      --inline-suppr \
      --suppress=missingIncludeSystem \
      -Iinclude \
      src include bin tests
  else
    echo "cppcheck not installed; skipping static analysis"
  fi
  echo "== clean build =="
  make clean
  make -j"$JOBS" CFLAGS="$BASE_CFLAGS" all
  echo "== unit tests =="
  make CFLAGS="$BASE_CFLAGS" test
  echo "== ASan build/tests =="
  make clean
  ASAN_OPTIONS=abort_on_error=1:detect_leaks=1:strict_init_order=1:strict_string_checks=1 \
    make CFLAGS="$BASE_CFLAGS" asan
  echo "== UBSan build/tests =="
  make clean
  UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 \
    make CFLAGS="$BASE_CFLAGS" ubsan
} 2>&1 | tee evidence/c-ci.log
