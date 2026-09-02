#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${TMPDIR:-/tmp}/zk-arche-p2p-common-contract-$$"
trap 'rm -rf "$BUILD_DIR"' EXIT
mkdir -p "$BUILD_DIR"

cc -std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Werror \
  -I"$ROOT/c/include" \
  "$ROOT/c/src/proto/association_admission.c" \
  "$ROOT/c/tests/test_p2p_common_contract_lifecycle.c" \
  -o "$BUILD_DIR/p2p-common-contract-c"
(
  cd "$ROOT/c"
  "$BUILD_DIR/p2p-common-contract-c"
)

if ! command -v cargo >/dev/null 2>&1; then
  echo "p2p-common-contract-cross-language: UNAVAILABLE: cargo not found" >&2
  exit 2
fi

(
  cd "$ROOT/rust"
  cargo test -p proto --test p2p_common_contract_lifecycle -- --exact canonical_p2p_common_contract_lifecycle_corpus
)

echo "p2p-common-contract-cross-language: PASS corpus=common-contract-lifecycle-v2 C=pass Rust=pass"
