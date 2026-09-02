#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
TMP="${TMPDIR:-/tmp}/zk-arche-p2p-c-qualification-$$"
trap 'rm -rf "$TMP"' EXIT INT TERM
mkdir -p "$TMP"
CC_BIN="${CC:-cc}"
"$CC_BIN" -std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Werror \
  -I"$ROOT/c/include" \
  "$ROOT/c/src/proto/association_admission.c" \
  "$ROOT/c/tests/test_p2p_common_contract_lifecycle.c" \
  -o "$TMP/p2p-c-qualification"
(cd "$ROOT/c" && "$TMP/p2p-c-qualification")
