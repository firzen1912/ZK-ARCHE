#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)

# Run both host-side Common Contract qualification lanes. Each language lane
# remains independently executable and consumes the repository-owned property
# checker plus the canonical cross-class lifecycle corpus.
#
# dev intentionally has no GitHub Actions. A successful invocation is local
# executable evidence only; it is not physical MCU, external-review, or
# deployment evidence.
"$ROOT/scripts/check-p2p-common-contract-rust-qualification.sh"
"$ROOT/scripts/check-p2p-common-contract-c-qualification.sh"

printf '%s\n' 'p2p paired qualification: PASS lanes=rust,c'
