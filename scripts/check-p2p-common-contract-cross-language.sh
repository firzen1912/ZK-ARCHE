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

python3 "$ROOT/scripts/check-p2p-common-contract-mutations.py"

if ! command -v cargo >/dev/null 2>&1; then
  echo "p2p-common-contract-cross-language: UNAVAILABLE: cargo not found" >&2
  exit 2
fi

(
  cd "$ROOT/rust"
  cargo test -p proto --test p2p_common_contract_lifecycle -- \
    --exact canonical_p2p_common_contract_lifecycle_corpus
  cargo test -p proto --test p2p_common_contract_lifecycle -- \
    --exact retained_cross_class_authority_fails_closed_after_lifecycle_loss
)

# A Common Contract association is not sufficient authority for indefinite
# protected application use. Qualify the DATA release boundary in the same
# repository-owned lane so cross-class/offline establishment cannot be called
# complete while release authorization would survive revocation, generation,
# lineage, replay/restart/usage continuity, binding, or rollback loss.
echo "[p2p-common-contract] protected DATA retained-authority qualification"
"$ROOT/scripts/check-data-release-retained-authority.sh"

echo "p2p-common-contract-cross-language: PASS corpus=common-contract-lifecycle-v4 mutations=pass retained_authority_loss=pass protected_data_release=pass C=pass Rust=pass"
