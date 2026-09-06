#!/usr/bin/env bash
# Bounded, reproducible fuzz run over the retained corpus.
#
# The retained corpus under rust/fuzz/corpus/<target>/ is the seed. A run that
# finds a new input adds to that corpus; a run that finds a crash leaves a
# reproducer under rust/fuzz/artifacts/<target>/ and fails this gate.
#
# This lane is deliberately time-bounded so it can run in development. It is
# not a substitute for long-running or continuous fuzzing, and a clean bounded
# run is not evidence that the parsers are free of defects. Leak detection is
# disabled because LeakSanitizer cannot run under ptrace-based sandboxes; the
# repository's sanitizer lanes remain responsible for leak qualification.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TARGETS=("wire_parse" "auth_payloads")
MAX_TOTAL_TIME="${ZK_ARCHE_FUZZ_SECONDS:-60}"
EVIDENCE="${ZK_ARCHE_FUZZ_EVIDENCE_DIR:-$ROOT/evidence/fuzz}"

if ! command -v cargo >/dev/null 2>&1; then
  echo "fuzz qualification: UNAVAILABLE (cargo is not installed)" >&2
  exit 125
fi
if ! cargo +nightly --version >/dev/null 2>&1; then
  echo "fuzz qualification: UNAVAILABLE (a nightly toolchain is required by cargo-fuzz)" >&2
  exit 125
fi
if ! cargo fuzz --version >/dev/null 2>&1; then
  echo "fuzz qualification: UNAVAILABLE (cargo-fuzz is not installed)" >&2
  exit 125
fi

mkdir -p "$EVIDENCE"
LOG="$EVIDENCE/fuzz.log"
status=0

{
  echo "== ZK-ARCHE fuzz lane =="
  date -u +"timestamp_utc=%Y-%m-%dT%H:%M:%SZ"
  cargo +nightly --version
  cargo fuzz --version
  echo "max_total_time_per_target=${MAX_TOTAL_TIME}s"

  for target in "${TARGETS[@]}"; do
    echo
    echo "== fuzz target: $target =="
    corpus="$ROOT/rust/fuzz/corpus/$target"
    artifacts="$ROOT/rust/fuzz/artifacts/$target"
    mkdir -p "$corpus"
    echo "seed_corpus_inputs=$(find "$corpus" -type f | wc -l | tr -d ' ')"

    if ! (cd "$ROOT/rust" && \
          ASAN_OPTIONS="${ASAN_OPTIONS:-detect_leaks=0}" \
          cargo +nightly fuzz run "$target" -- \
          -max_total_time="$MAX_TOTAL_TIME" -print_final_stats=1); then
      echo "fuzz qualification: FAIL ($target exited non-zero)" >&2
      status=1
    fi

    if [ -d "$artifacts" ] && [ -n "$(find "$artifacts" -type f -print -quit)" ]; then
      echo "fuzz qualification: FAIL ($target left crash reproducers:)" >&2
      find "$artifacts" -type f >&2
      status=1
    fi

    echo "final_corpus_inputs=$(find "$corpus" -type f | wc -l | tr -d ' ')"
  done

  echo
  if [ "$status" -eq 0 ]; then
    echo "fuzz qualification: PASS (no crash reproducers; corpus retained under rust/fuzz/corpus/)"
  else
    echo "fuzz qualification: FAIL"
  fi
  exit "$status"
} 2>&1 | tee "$LOG"
