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

echo "[p2p-common-contract] decision matrix and normative-boundary qualification"
python3 "$ROOT/scripts/check-p2p-common-contract-decision.py"

python3 "$ROOT/scripts/check-p2p-common-contract-mutations.py"

# The Common Contract is a composition boundary, not an isolated admission
# predicate. Require the repository-wide lifecycle invariant audit in the same
# lane so bounded delegation cannot repair stale authorization/revocation/
# lineage state, infrastructure availability cannot become authority, and the
# P2P decision remains aligned with enrollment, resumption, transport, and DATA
# release lifecycle semantics.
echo "[p2p-common-contract] cross-module lifecycle invariant qualification"
python3 "$ROOT/scripts/check-cross-module-lifecycle-invariants.py"

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

# Offline Common Contract establishment depends on a locally incorporated,
# authority-scoped revocation view. Require the deterministic FULL/DIFF
# reconciliation oracle in the same qualification lane so stale, gapped,
# conflicting, rollback, authority-substituted, or unauthenticated updates
# cannot drift independently from cross-class P2P qualification.
echo "[p2p-common-contract] revocation-view reconciliation qualification"
python3 "$ROOT/scripts/check-revocation-view-reconciliation.py"

# A Common Contract association is not sufficient authority for indefinite
# protected application use. Qualify the DATA release boundary in the same
# repository-owned lane so cross-class/offline establishment cannot be called
# complete while release authorization would survive revocation, generation,
# lineage, replay/restart/usage continuity, binding, or rollback loss.
echo "[p2p-common-contract] protected DATA retained-authority qualification"
"$ROOT/scripts/check-data-release-retained-authority.sh"

echo "p2p-common-contract-cross-language: PASS corpus=common-contract-lifecycle-v4 decision_matrix=pass mutations=pass cross_module_lifecycle=pass retained_authority_loss=pass revocation_reconciliation=pass protected_data_release=pass C=pass Rust=pass"
