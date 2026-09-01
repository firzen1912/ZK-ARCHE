#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
EVIDENCE="${ZK_ARCHE_FORMAL_EVIDENCE_DIR:-$ROOT/evidence/formal}"
mkdir -p "$EVIDENCE"

if ! command -v git >/dev/null 2>&1; then
  echo "formal qualification: UNAVAILABLE (git is required to bind evidence to an exact repository state)" >&2
  exit 125
fi

if ! command -v proverif >/dev/null 2>&1; then
  echo "formal qualification: UNAVAILABLE (proverif is not installed)" >&2
  exit 125
fi

if ! REPO_HEAD="$(git -C "$ROOT" rev-parse --verify HEAD 2>/dev/null)"; then
  echo "formal qualification: UNAVAILABLE (cannot resolve repository HEAD)" >&2
  exit 125
fi
REPO_SHORT="${REPO_HEAD:0:12}"

if ! PROVERIF_VERSION="$(proverif -version 2>&1 | head -n 1)" || [ -z "$PROVERIF_VERSION" ]; then
  echo "formal qualification: UNAVAILABLE (cannot resolve ProVerif version)" >&2
  exit 125
fi

bash "$ROOT/scripts/sync-formal-models.sh" --check

MANIFEST="$EVIDENCE/${REPO_SHORT}-formal-manifest.tsv"
printf 'repository_head\ttool_version\tmodel_name\tmodel_path\tmodel_blob\texpected_true\tobserved_true\tresult\tlog\n' > "$MANIFEST"

run_model() {
  local name="$1"
  local model="$2"
  local expected_true="$3"
  local rel_model="${model#"$ROOT/"}"
  local tracked_blob current_blob
  local log_base="$EVIDENCE/${REPO_SHORT}-${name}"
  local tmp_log="$log_base.tmp.log"
  local final_log
  local status=0
  local true_count=0
  local result="FAIL"

  if ! tracked_blob="$(git -C "$ROOT" rev-parse "HEAD:$rel_model" 2>/dev/null)"; then
    echo "formal qualification: FAIL ($name model is not tracked at HEAD: $rel_model)" >&2
    return 1
  fi
  if ! current_blob="$(git -C "$ROOT" hash-object "$model" 2>/dev/null)"; then
    echo "formal qualification: FAIL ($name cannot hash model: $rel_model)" >&2
    return 1
  fi
  if [ "$tracked_blob" != "$current_blob" ]; then
    echo "formal qualification: FAIL ($name model differs from exact HEAD: $rel_model)" >&2
    return 1
  fi

  rm -f "$tmp_log"

  {
    printf 'repository_head=%s\n' "$REPO_HEAD"
    printf 'tool_version=%s\n' "$PROVERIF_VERSION"
    printf 'model_name=%s\n' "$name"
    printf 'model_path=%s\n' "$rel_model"
    printf 'model_blob=%s\n' "$tracked_blob"
    printf 'expected_true_queries=%s\n' "$expected_true"
    printf '%s\n' '--- proverif output ---'
  } > "$tmp_log"

  echo
  echo "== ProVerif $name =="
  echo "repository_head=$REPO_HEAD"
  echo "tool_version=$PROVERIF_VERSION"
  echo "model=$rel_model"
  echo "model_blob=$tracked_blob"
  echo "expected_true_queries=$expected_true"

  set +e
  proverif "$model" 2>&1 | tee -a "$tmp_log"
  status=${PIPESTATUS[0]}
  set -e

  true_count="$(grep -Ec '^RESULT .* is true\.$' "$tmp_log" || true)"

  if [ "$status" -eq 0 ] \
    && ! grep -Eiq '^RESULT .* (is false\.|cannot be proved|cannot be proved\.)' "$tmp_log" \
    && [ "$true_count" -eq "$expected_true" ]; then
    result="PASS"
    final_log="$log_base.PASS.log"
  else
    final_log="$log_base.FAIL.log"
  fi

  mv "$tmp_log" "$final_log"
  printf '%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n' \
    "$REPO_HEAD" "$PROVERIF_VERSION" "$name" "$rel_model" "$tracked_blob" \
    "$expected_true" "$true_count" "$result" "${final_log#"$ROOT/"}" >> "$MANIFEST"

  if [ "$status" -ne 0 ]; then
    echo "formal qualification: FAIL ($name proverif exit=$status; evidence=$final_log)" >&2
    return 1
  fi
  if grep -Eiq '^RESULT .* (is false\.|cannot be proved|cannot be proved\.)' "$final_log"; then
    echo "formal qualification: FAIL ($name contains an unproved/false RESULT; evidence=$final_log)" >&2
    return 1
  fi
  if [ "$true_count" -ne "$expected_true" ]; then
    echo "formal qualification: FAIL ($name expected $expected_true true RESULT lines, found $true_count; evidence=$final_log)" >&2
    return 1
  fi

  echo "formal qualification: PASS ($name, $true_count/$expected_true queries true; evidence=$final_log)"
}

run_model \
  "auth-v3" \
  "$ROOT/rust/models/proverif/zk_arche_auth_v3_draft.pv" \
  10

run_model \
  "replay-continuity" \
  "$ROOT/rust/models/proverif/zk_arche_replay_continuity_draft.pv" \
  9

run_model \
  "lineage-replace-commit" \
  "$ROOT/rust/models/proverif/zk_arche_lineage_replace_commit_draft.pv" \
  6

run_model \
  "association-admission" \
  "$ROOT/rust/models/proverif/zk_arche_association_admission_draft.pv" \
  12

echo
echo "formal evidence manifest: $MANIFEST"
echo "formal qualification: PASS"
