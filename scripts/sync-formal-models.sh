#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CANONICAL="$ROOT/rust/models/proverif/zk_arche_auth_skeleton.pv"
MIRRORS=(
  "$ROOT/c/models/proverif/zk_arche_auth_skeleton.pv"
)

mode="${1:---check}"

case "$mode" in
  --check)
    ;;
  --write)
    for mirror in "${MIRRORS[@]}"; do
      cp "$CANONICAL" "$mirror"
    done
    ;;
  *)
    echo "usage: $0 [--check|--write]" >&2
    exit 2
    ;;
esac

for mirror in "${MIRRORS[@]}"; do
  if ! cmp -s "$CANONICAL" "$mirror"; then
    echo "formal model mirror drift: ${mirror#$ROOT/}" >&2
    diff -u "$CANONICAL" "$mirror" || true
    echo "run: scripts/sync-formal-models.sh --write" >&2
    exit 1
  fi
done

echo "formal model mirrors match rust/models/proverif/zk_arche_auth_skeleton.pv"
