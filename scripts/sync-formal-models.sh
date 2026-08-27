#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PAIRS=(
  "rust/models/proverif/zk_arche_auth_skeleton.pv:c/models/proverif/zk_arche_auth_skeleton.pv"
  "rust/models/proverif/zk_arche_auth_v3_draft.pv:c/models/proverif/zk_arche_auth_v3_draft.pv"
)

mode="${1:---check}"

case "$mode" in
  --check)
    ;;
  --write)
    for pair in "${PAIRS[@]}"; do
      canonical="${pair%%:*}"
      mirror="${pair#*:}"
      cp "$ROOT/$canonical" "$ROOT/$mirror"
    done
    ;;
  *)
    echo "usage: $0 [--check|--write]" >&2
    exit 2
    ;;
esac

for pair in "${PAIRS[@]}"; do
  canonical="${pair%%:*}"
  mirror="${pair#*:}"
  if ! cmp -s "$ROOT/$canonical" "$ROOT/$mirror"; then
    echo "formal model mirror drift: $mirror" >&2
    diff -u "$ROOT/$canonical" "$ROOT/$mirror" || true
    echo "run: scripts/sync-formal-models.sh --write" >&2
    exit 1
  fi
  echo "formal model mirrors match $canonical"
done
