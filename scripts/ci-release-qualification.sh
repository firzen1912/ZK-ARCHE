#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

EVIDENCE="evidence/release-qualification"
LOG="$EVIDENCE/release-qualification.log"
mkdir -p "$EVIDENCE"

run_step() {
  local name="$1"
  shift
  echo
  echo "== $name =="
  echo "+ $*"
  "$@"
}

run_step_in() {
  local name="$1"
  local dir="$2"
  shift 2
  echo
  echo "== $name =="
  echo "+ (cd $dir && $*)"
  (cd "$dir" && "$@")
}

{
  echo "== ZK-ARCHE release qualification =="
  date -u +"timestamp_utc=%Y-%m-%dT%H:%M:%SZ"
  echo "root=$ROOT"

  GIT_BIN="${GIT:-git}"

  echo
  echo "== tool inventory =="
  for tool in cargo rustc make pkg-config cppcheck proverif python3; do
    if command -v "$tool" >/dev/null 2>&1; then
      echo "$tool=$(command -v "$tool")"
    else
      echo "$tool=missing"
    fi
  done
  if command -v "$GIT_BIN" >/dev/null 2>&1; then
    echo "git=$(command -v "$GIT_BIN")"
  else
    echo "git=missing"
    exit 1
  fi
  if command -v gcc >/dev/null 2>&1; then echo "gcc=$(command -v gcc)"; fi
  if command -v clang >/dev/null 2>&1; then echo "clang=$(command -v clang)"; fi

  QUALIFICATION_HEAD="$("$GIT_BIN" -C "$ROOT" rev-parse --verify HEAD)"
  echo "qualification_head=$QUALIFICATION_HEAD"

  exact_head_gate() {
    local phase="$1"
    local current_head current_tree current_branch status manifest
    current_head="$("$GIT_BIN" -C "$ROOT" rev-parse --verify HEAD)"
    current_tree="$("$GIT_BIN" -C "$ROOT" rev-parse 'HEAD^{tree}')"
    current_branch="$("$GIT_BIN" -C "$ROOT" symbolic-ref --quiet --short HEAD 2>/dev/null || printf 'DETACHED')"

    if [[ "$current_head" != "$QUALIFICATION_HEAD" ]]; then
      echo "exact-head-provenance: FAIL: HEAD moved expected=$QUALIFICATION_HEAD observed=$current_head" >&2
      return 1
    fi
    if "$GIT_BIN" -C "$ROOT" rev-parse --verify MERGE_HEAD >/dev/null 2>&1; then
      echo "exact-head-provenance: FAIL: merge in progress" >&2
      return 1
    fi
    status="$("$GIT_BIN" -C "$ROOT" status --porcelain=v1 --untracked-files=all)"
    if [[ -n "$status" ]]; then
      echo "exact-head-provenance: FAIL: working tree is not clean" >&2
      printf '%s\n' "$status" >&2
      return 1
    fi

    manifest="$EVIDENCE/exact-head-${phase}.tsv"
    printf 'schema\thead\ttree\tbranch\tclean\n' > "$manifest"
    printf 'ZKARCHE-EXACT-HEAD/1\t%s\t%s\t%s\ttrue\n' \
      "$current_head" "$current_tree" "$current_branch" >> "$manifest"
    echo "exact-head-provenance: PASS phase=$phase head=$current_head tree=$current_tree branch=$current_branch clean=true"
  }

  run_step "exact-head clean preflight" exact_head_gate preflight

  run_step "formal qualification" \
    bash "$ROOT/scripts/ci-formal.sh"

  run_step "wire error registry/corpus parity" \
    python3 "$ROOT/scripts/check-error-registry-parity.py"

  run_step "AUTH terminal-flight contract/corpus parity" \
    python3 "$ROOT/scripts/check-auth-terminal-flight-contract.py"

  run_step "AUTH no-learning/trust-mutation boundary" \
    python3 "$ROOT/scripts/check-auth-trust-boundary.py"

  run_step "revocation convergence/stale-authorization contract" \
    python3 "$ROOT/scripts/check-revocation-freshness-contract.py"

  run_step "cross-module lifecycle invariant audit" \
    python3 "$ROOT/scripts/check-cross-module-lifecycle-invariants.py"

  run_step "constrained lifecycle-storage evidence honesty" \
    python3 "$ROOT/scripts/check-constrained-lifecycle-storage.py" \
      "$ROOT/evidence/constrained-target/lifecycle-storage-template.json"

  run_step "constrained lifecycle-storage negative self-test" \
    python3 "$ROOT/scripts/test-constrained-lifecycle-storage.py"

  run_step "P2P Common Contract qualification corpus" \
    python3 "$ROOT/scripts/check-p2p-common-contract-qualification.py"

  run_step "P2P cross-class decision composition" \
    python3 "$ROOT/scripts/check-p2p-common-contract-decision.py"

  run_step "P2P exhaustive decision properties" \
    python3 "$ROOT/scripts/check-p2p-common-contract-properties.py"

  run_step "P2P Rust/C lifecycle qualification" \
    bash "$ROOT/scripts/check-p2p-common-contract-cross-language.sh"

  run_step "Rust lane" bash "$ROOT/scripts/ci-rust.sh"
  run_step "C lane" bash "$ROOT/scripts/ci-c.sh"

  run_step_in "C harness against Rust vectors" "$ROOT/c" \
    ./build/tests/test_vectors ../rust/test-vectors/0x0001

  run_step_in "Rust vector regeneration" "$ROOT/rust" \
    cargo run --example gen_test_vectors --features test-vectors
  run_step "generated vector drift check" \
    "$GIT_BIN" -C "$ROOT" diff --exit-code -- rust/test-vectors/0x0001

  run_step "exact-head clean postflight" exact_head_gate postflight

  echo
  echo "release qualification: PASS"
} 2>&1 | tee "$LOG"
