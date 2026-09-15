#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)

# Keep peer class and optional infrastructure outside protocol authority before
# exercising the Rust implementation against the canonical cross-class
# lifecycle corpus. The corpus includes MCU<->MCU and MCU<->edge directions,
# offline success, authorization-generation/revocation/restart continuity,
# mandatory-floor downgrade rejection, binding failure, and NO-LEARNING trust
# mutation rejection.
python3 "$ROOT/scripts/check-p2p-common-contract-properties.py"

cd "$ROOT/rust"
cargo test -p proto --test p2p_common_contract_lifecycle --locked
