#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
mkdir -p "$ROOT/evidence"
LOG="$ROOT/evidence/ci-all.log"

# A qualification record is valid only when validation starts from an exact,
# clean repository HEAD. Development runs against dirty trees still execute,
# but cannot produce per-HEAD evidence.
QUALIFICATION_HEAD=""
if command -v git >/dev/null 2>&1 \
  && git -C "$ROOT" rev-parse --verify HEAD >/dev/null 2>&1 \
  && [ -z "$(git -C "$ROOT" status --porcelain 2>/dev/null)" ]; then
  QUALIFICATION_HEAD="$(git -C "$ROOT" rev-parse --verify HEAD)"
fi

{
  echo "== ZK-ARCHE unified CI =="
  date -u +"timestamp_utc=%Y-%m-%dT%H:%M:%SZ"
  echo "root=$ROOT"
  echo
  echo "== constrained-target evidence contract =="
  python3 "$ROOT/scripts/check-constrained-target-manifest.py" \
    "$ROOT/evidence/constrained-target/manifest-template.json"
  echo
  echo "== constrained lifecycle/storage evidence contract =="
  python3 "$ROOT/scripts/check-constrained-lifecycle-storage.py" \
    "$ROOT/evidence/constrained-target/lifecycle-storage-template.json"
  echo
  echo "== constrained lifecycle/storage negative self-test =="
  python3 "$ROOT/scripts/test-constrained-lifecycle-storage.py"
  echo
  echo "== constrained DATA audit-chain storage evidence contract =="
  python3 "$ROOT/scripts/check-constrained-data-audit-storage.py" \
    "$ROOT/evidence/constrained-target/data-audit-storage-template.json"
  echo
  echo "== fuzz target/corpus provenance =="
  python3 "$ROOT/scripts/check-fuzz-provenance.py"
  echo
  echo "== fuzz harness compile integration =="
  cargo check --manifest-path "$ROOT/rust/fuzz/Cargo.toml" --locked --bins
  echo
  echo "== wire error registry/corpus parity =="
  python3 "$ROOT/scripts/check-error-registry-parity.py"
  echo
  echo "== core version/suite/capability registry parity =="
  python3 "$ROOT/scripts/check-core-registry-parity.py"
  echo
  echo "== AUTH trust-mutation boundary =="
  python3 "$ROOT/scripts/check-auth-trust-boundary.py"
  echo
  echo "== AUTH terminal-flight contract =="
  python3 "$ROOT/scripts/check-auth-terminal-flight-contract.py"
  echo
  echo "== revocation convergence/stale-authorization contract =="
  python3 "$ROOT/scripts/check-revocation-freshness-contract.py"
  echo
  echo "== rust lane =="
  bash "$ROOT/scripts/ci-rust.sh"
  echo
  echo "== c lane =="
  bash "$ROOT/scripts/ci-c.sh"
  echo
  echo "== c vector harness against rust vectors =="
  cd "$ROOT/c"
  if [ -x ./build/tests/test_vectors ]; then
    ./build/tests/test_vectors ../rust/test-vectors/0x0001
  else
    echo "C vector test binary not found after C CI; run make in c/ and retry." >&2
    exit 1
  fi
  echo
  echo "== cross-module lifecycle invariant audit =="
  python3 "$ROOT/scripts/check-cross-module-lifecycle-invariants.py"
  echo
  echo "== lineage replacement dependent-state lifecycle audit =="
  python3 "$ROOT/scripts/check-lineage-replace-lifecycle-invariants.py"
  echo
  echo "== P2P Common Contract qualification corpus =="
  python3 "$ROOT/scripts/check-p2p-common-contract-qualification.py"
  echo
  echo "== P2P cross-class decision composition =="
  python3 "$ROOT/scripts/check-p2p-common-contract-decision.py"
  echo
  echo "== P2P exhaustive decision properties =="
  python3 "$ROOT/scripts/check-p2p-common-contract-properties.py"
  echo
  echo "== P2P Common Contract cross-language qualification =="
  bash "$ROOT/scripts/check-p2p-common-contract-cross-language.sh"
} 2>&1 | tee "$LOG"

if [ -n "$QUALIFICATION_HEAD" ]; then
  bash "$ROOT/scripts/record-qualification.sh" "$QUALIFICATION_HEAD" "$LOG"
else
  echo "qualification record: NOT RECORDED (validation did not start from a clean repository HEAD)"
fi