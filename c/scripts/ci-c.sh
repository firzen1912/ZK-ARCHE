#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p evidence

JOBS="${JOBS:-$(getconf _NPROCESSORS_ONLN 2>/dev/null || nproc 2>/dev/null || echo 2)}"
CC="${CC:-cc}"
BASE_CFLAGS="-std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wno-sign-conversion -Werror -O2 -fno-strict-aliasing -fstack-protector-strong -D_FORTIFY_SOURCE=2"
TMPDIR="${TMPDIR:-.tmp}"
mkdir -p "$TMPDIR"
case "$TMPDIR" in
  /*) TMPDIR_ABS="$TMPDIR" ;;
  *) TMPDIR_ABS="$PWD/$TMPDIR" ;;
esac
export TMPDIR="$TMPDIR_ABS"
if command -v cygpath >/dev/null 2>&1; then
  TMP_WIN="$(cygpath -w "$TMPDIR_ABS")"
  export TMP="$TMP_WIN"
  export TEMP="$TMP_WIN"
else
  export TMP="$TMPDIR_ABS"
  export TEMP="$TMPDIR_ABS"
fi

compiler_has_runtime() {
  local runtime="$1"
  local resolved
  resolved="$("$CC" -print-file-name="$runtime" 2>/dev/null || true)"
  [ -n "$resolved" ] && [ "$resolved" != "$runtime" ]
}

require_or_skip_runtime() {
  local label="$1"
  local runtime="$2"
  if compiler_has_runtime "$runtime"; then
    return 0
  fi
  if [ "${REQUIRE_SANITIZERS:-0}" = "1" ]; then
    echo "$label runtime ($runtime) not found and REQUIRE_SANITIZERS=1" >&2
    exit 1
  fi
  echo "$label runtime ($runtime) not found; skipping $label build/tests"
  return 1
}

{
  echo "== ZK-ARCHE C CI =="
  date -u +"timestamp_utc=%Y-%m-%dT%H:%M:%SZ"
  echo "cc=$CC"
  "$CC" --version
  echo "tmpdir=$TMPDIR"
  echo "== dependency check =="
  command -v make >/dev/null 2>&1
  command -v pkg-config >/dev/null 2>&1
  command -v cppcheck >/dev/null 2>&1
  pkg-config --exists libsodium
  pkg-config --modversion libsodium
  echo "== static analysis (cppcheck) =="
  cppcheck \
    --std=c11 \
    --enable=warning,performance,portability \
    --error-exitcode=1 \
    --inline-suppr \
    --suppress=missingIncludeSystem \
    -Iinclude \
    src include bin tests
  echo "== clean build =="
  make clean
  make -j"$JOBS" CFLAGS="$BASE_CFLAGS" all
  echo "== unit tests =="
  make CFLAGS="$BASE_CFLAGS" test
  if command -v clang >/dev/null 2>&1; then
    echo "== clang build/tests =="
    make clean
    make -j"$JOBS" CC=clang CFLAGS="$BASE_CFLAGS" all
    make CC=clang CFLAGS="$BASE_CFLAGS" test
  else
    echo "clang not installed; skipping secondary clang build"
  fi
  echo "== ASan build/tests =="
  if require_or_skip_runtime "ASan" "libasan.a"; then
    make clean
    ASAN_OPTIONS=abort_on_error=1:detect_leaks=1:strict_init_order=1:strict_string_checks=1 \
      make CFLAGS="$BASE_CFLAGS" asan
  fi
  echo "== UBSan build/tests =="
  if require_or_skip_runtime "UBSan" "libubsan.a"; then
    make clean
    UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 \
      make CFLAGS="$BASE_CFLAGS" ubsan
  fi
} 2>&1 | tee evidence/c-ci.log
