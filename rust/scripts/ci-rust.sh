#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p evidence
{
  echo "== rust fmt =="
  cargo fmt --all -- --check
  echo "== rust check =="
  cargo check --workspace --locked --all-targets
  echo "== rust test =="
  cargo test --workspace --locked --all-features
  echo "== rust clippy =="
  cargo clippy --workspace --all-targets --all-features -- -D warnings
} 2>&1 | tee evidence/rust-ci.log
