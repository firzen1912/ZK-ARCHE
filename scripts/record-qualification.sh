#!/usr/bin/env bash
# Record that scripts/ci-all.sh passed against an exact repository HEAD.
#
# This mirrors the per-HEAD evidence contract already used by scripts/ci-formal.sh:
# a qualification claim is bound to a resolvable commit, the toolchain that
# produced it, and a retained log. Without such a record, "exact-current dev
# health" is an assumption rather than evidence (TD-005).
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
EXPECTED_HEAD="${1:-}"
LOG_PATH="${2:-}"
EVIDENCE="${ZK_ARCHE_QUALIFICATION_EVIDENCE_DIR:-$ROOT/evidence/qualification}"

if [ -z "$EXPECTED_HEAD" ]; then
  echo "usage: $0 EXPECTED_HEAD [CI_ALL_LOG]" >&2
  exit 2
fi

if ! command -v git >/dev/null 2>&1; then
  echo "qualification record: UNAVAILABLE (git is required to bind evidence to an exact repository state)" >&2
  exit 125
fi

if ! HEAD_OID="$(git -C "$ROOT" rev-parse --verify HEAD 2>/dev/null)"; then
  echo "qualification record: UNAVAILABLE (cannot resolve repository HEAD)" >&2
  exit 125
fi

if [ "$HEAD_OID" != "$EXPECTED_HEAD" ]; then
  echo "qualification record: NOT RECORDED (HEAD changed during validation: expected ${EXPECTED_HEAD:0:12}, found ${HEAD_OID:0:12})" >&2
  exit 1
fi

# A dirty worktree means the validated bytes are no longer the bytes that were
# checked at the start of the run, so no record may be written.
if [ -n "$(git -C "$ROOT" status --porcelain 2>/dev/null)" ]; then
  echo "qualification record: NOT RECORDED (worktree changed during validation of HEAD ${HEAD_OID:0:12})" >&2
  exit 1
fi

REPO_SHORT="${HEAD_OID:0:12}"
BRANCH="$(git -C "$ROOT" rev-parse --abbrev-ref HEAD 2>/dev/null || echo detached)"
mkdir -p "$EVIDENCE"
MANIFEST="$EVIDENCE/${REPO_SHORT}-qualification-manifest.tsv"

rust_version="$( (cd "$ROOT/rust" && rustc --version) 2>/dev/null || echo unknown)"
cargo_version="$( (cd "$ROOT/rust" && cargo --version) 2>/dev/null || echo unknown)"
cc_version="$(${CC:-cc} --version 2>/dev/null | head -n 1 || echo unknown)"

retained_log=""
if [ -n "$LOG_PATH" ] && [ -f "$LOG_PATH" ]; then
  retained_log="$EVIDENCE/${REPO_SHORT}-ci-all.log"
  cp "$LOG_PATH" "$retained_log"
  retained_log="${retained_log#"$ROOT/"}"
fi

printf 'repository_head\tbranch\ttimestamp_utc\tgate\trust\tcargo\tcc\tresult\tlog\n' > "$MANIFEST"
printf '%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n' \
  "$HEAD_OID" \
  "$BRANCH" \
  "$(date -u +%Y-%m-%dT%H:%M:%SZ)" \
  "scripts/ci-all.sh" \
  "$rust_version" \
  "$cargo_version" \
  "$cc_version" \
  "PASS" \
  "${retained_log:-none}" >> "$MANIFEST"

echo "qualification record: PASS ($REPO_SHORT; evidence=${MANIFEST#"$ROOT/"})"
