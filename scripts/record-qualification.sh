#!/usr/bin/env bash
# Record that a repository-owned qualification gate passed against an exact
# repository HEAD.
#
# A qualification claim is bound to a resolvable commit, the gate that
# produced it, the toolchain, and retained evidence. Without such a record,
# "exact-current dev health" is an assumption rather than evidence (TD-005).
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
EXPECTED_HEAD="${1:-}"
LOG_PATH="${2:-}"
GATE_REL="${3:-scripts/ci-all.sh}"
RECORD_KIND="${4:-qualification}"
EVIDENCE="${ZK_ARCHE_QUALIFICATION_EVIDENCE_DIR:-$ROOT/evidence/qualification}"

if [ -z "$EXPECTED_HEAD" ]; then
  echo "usage: $0 EXPECTED_HEAD [LOG] [GATE_REL] [RECORD_KIND]" >&2
  exit 2
fi

if [[ ! "$RECORD_KIND" =~ ^[A-Za-z0-9._-]+$ ]]; then
  echo "qualification record: invalid record kind: $RECORD_KIND" >&2
  exit 2
fi

if [[ "$GATE_REL" = /* || "$GATE_REL" == *".."* ]]; then
  echo "qualification record: invalid repository-relative gate path: $GATE_REL" >&2
  exit 2
fi

if ! command -v git >/dev/null 2>&1; then
  echo "qualification record: UNAVAILABLE (git is required to bind evidence to an exact repository state)" >&2
  exit 125
fi

hash_file() {
  local path="$1"
  if command -v sha256sum >/dev/null 2>&1; then
    sha256sum "$path" | awk '{print $1}'
  elif command -v shasum >/dev/null 2>&1; then
    shasum -a 256 "$path" | awk '{print $1}'
  else
    echo "qualification record: UNAVAILABLE (sha256sum or shasum is required to bind retained evidence)" >&2
    return 125
  fi
}

if ! HEAD_OID="$(git -C "$ROOT" rev-parse --verify HEAD 2>/dev/null)"; then
  echo "qualification record: UNAVAILABLE (cannot resolve repository HEAD)" >&2
  exit 125
fi

if [ "$HEAD_OID" != "$EXPECTED_HEAD" ]; then
  echo "qualification record: NOT RECORDED (HEAD changed during validation: expected ${EXPECTED_HEAD:0:12}, found ${HEAD_OID:0:12})" >&2
  exit 1
fi

# A dirty worktree means the validated bytes are no longer the exact HEAD
# bytes, so no record may be written. Generated evidence is repository-ignored.
if [ -n "$(git -C "$ROOT" status --porcelain 2>/dev/null)" ]; then
  echo "qualification record: NOT RECORDED (worktree changed during validation of HEAD ${HEAD_OID:0:12})" >&2
  exit 1
fi

REPO_SHORT="${HEAD_OID:0:12}"
BRANCH="$(git -C "$ROOT" rev-parse --abbrev-ref HEAD 2>/dev/null || echo detached)"
mkdir -p "$EVIDENCE"
MANIFEST="$EVIDENCE/${REPO_SHORT}-${RECORD_KIND}-manifest.tsv"

rust_version="$( (cd "$ROOT/rust" && rustc --version) 2>/dev/null || echo unknown)"
cargo_version="$( (cd "$ROOT/rust" && cargo --version) 2>/dev/null || echo unknown)"
cc_version="$(${CC:-cc} --version 2>/dev/null | head -n 1 || echo unknown)"
gate_path="$ROOT/$GATE_REL"
if [ ! -f "$gate_path" ]; then
  echo "qualification record: NOT RECORDED (qualification gate missing: $GATE_REL)" >&2
  exit 1
fi
gate_sha256="$(hash_file "$gate_path")"

retained_log=""
retained_log_sha256="none"
if [ -n "$LOG_PATH" ]; then
  if [ ! -f "$LOG_PATH" ]; then
    echo "qualification record: NOT RECORDED (requested qualification log does not exist: $LOG_PATH)" >&2
    exit 1
  fi
  if [ "$RECORD_KIND" = "qualification" ] && [ "$GATE_REL" = "scripts/ci-all.sh" ]; then
    # Preserve the historical ci-all evidence filename.
    retained_log="$EVIDENCE/${REPO_SHORT}-ci-all.log"
  else
    retained_log="$EVIDENCE/${REPO_SHORT}-${RECORD_KIND}.log"
  fi
  cp "$LOG_PATH" "$retained_log"
  retained_log_sha256="$(hash_file "$retained_log")"
  retained_log="${retained_log#"$ROOT/"}"
fi

printf 'repository_head\tbranch\ttimestamp_utc\tgate\tgate_sha256\trust\tcargo\tcc\tresult\tlog\tlog_sha256\n' > "$MANIFEST"
printf '%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n' \
  "$HEAD_OID" \
  "$BRANCH" \
  "$(date -u +%Y-%m-%dT%H:%M:%SZ)" \
  "$GATE_REL" \
  "$gate_sha256" \
  "$rust_version" \
  "$cargo_version" \
  "$cc_version" \
  "PASS" \
  "${retained_log:-none}" \
  "$retained_log_sha256" >> "$MANIFEST"

echo "qualification record: PASS ($REPO_SHORT; kind=$RECORD_KIND; evidence=${MANIFEST#"$ROOT/"}; gate=$GATE_REL; gate_sha256=$gate_sha256; log_sha256=$retained_log_sha256)"
