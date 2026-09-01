#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

GIT_DIR="$(git rev-parse --absolute-git-dir)"
HOOKS_DIR="$GIT_DIR/zk-arche-hooks"
mkdir -p "$HOOKS_DIR"
cp .githooks/pre-commit .githooks/pre-push "$HOOKS_DIR/"
chmod +x "$HOOKS_DIR/pre-commit" "$HOOKS_DIR/pre-push"
git config core.hooksPath "$HOOKS_DIR"

configured="$(git config --get core.hooksPath)"
if [ "$configured" != "$HOOKS_DIR" ]; then
  echo "failed to configure core.hooksPath" >&2
  exit 1
fi

echo "Git hooks installed in $HOOKS_DIR"
echo "  pre-commit: formatting and contract consistency"
echo "  pre-push dev: core Rust/C/conformance qualification"
echo "  pre-push main: full release qualification, including ProVerif"
