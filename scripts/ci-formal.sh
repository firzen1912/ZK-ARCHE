#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
EVIDENCE="${ZK_ARCHE_FORMAL_EVIDENCE_DIR:-$ROOT/evidence/formal}"
mkdir -p "$EVIDENCE"

if ! command -v proverif >/dev/null 2>&1; then
  echo "formal qualification: UNAVAILABLE (proverif is not installed)" >&2
  exit 125
fi

bash "$ROOT/scripts/sync-formal-models.sh" --check

run_model() {
  local name="$1"
  local model="$2"
  local expected_true="$3"
  local log="$EVIDENCE/${name}.log"
  local status=0

  echo
  echo "== ProVerif $name =="
  echo "model=$model"
  echo "expected_true_queries=$expected_true"

  set +e
  proverif "$model" 2>&1 | tee "$log"
  status=${PIPESTATUS[0]}
  set -e

  if [ "$status" -ne 0 ]; then
    echo "formal qualification: FAIL ($name proverif exit=$status)" >&2
    return 1
  fi

  if grep -Eiq '^RESULT .* (is false\.|cannot be proved|cannot be proved\.)' "$log"; then
    echo "formal qualification: FAIL ($name contains an unproved/false RESULT)" >&2
    return 1
  fi

  local true_count
  true_count="$(grep -Ec '^RESULT .* is true\.$' "$log" || true)"
  if [ "$true_count" -ne "$expected_true" ]; then
    echo "formal qualification: FAIL ($name expected $expected_true true RESULT lines, found $true_count)" >&2
    return 1
  fi

  echo "formal qualification: PASS ($name, $true_count/$expected_true queries true)"
}

run_model \
  "auth-v3" \
  "$ROOT/rust/models/proverif/zk_arche_auth_v3_draft.pv" \
  9

run_model \
  "replay-continuity" \
  "$ROOT/rust/models/proverif/zk_arche_replay_continuity_draft.pv" \
  9

run_model \
  "lineage-replace-commit" \
  "$ROOT/rust/models/proverif/zk_arche_lineage_replace_commit_draft.pv" \
  6

echo
echo "formal qualification: PASS"
