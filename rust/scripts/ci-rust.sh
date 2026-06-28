#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p evidence
{
  echo "== ZK-ARCHE Rust CI =="
  date -u +"timestamp_utc=%Y-%m-%dT%H:%M:%SZ"
  rustc --version
  cargo --version
  echo "== rust fmt =="
  cargo fmt --all -- --check
  echo "== rust check =="
  cargo check --workspace --locked --all-targets --all-features
  echo "== rust test =="
  cargo test --workspace --locked --all-features
  echo "== rust clippy =="
  cargo clippy --workspace --all-targets --all-features -- -D warnings
  echo "== rust dependency audit =="
  if command -v cargo-audit >/dev/null 2>&1; then
    cargo audit
  else
    echo "cargo-audit not installed; skipping dependency advisory audit"
  fi
} 2>&1 | tee evidence/rust-ci.log
