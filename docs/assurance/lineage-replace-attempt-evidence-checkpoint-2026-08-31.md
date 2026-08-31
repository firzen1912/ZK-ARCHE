# Lineage Replacement Attempt-Evidence Checkpoint — 2026-08-31

## Scope

This checkpoint advances zk213's reproducible Rust/C lifecycle truth by replacing the reconciliation transition's caller-supplied freshness boolean with a shared pure attempt-evidence decision surface.

Affected semantic surfaces:

- `c/include/auth/lineage_replace_attempt_evidence.h`
- `c/src/proto/lineage_replace_attempt_evidence.c`
- `rust/crates/proto/src/lineage_replace_attempt_evidence.rs`
- `c/include/auth/lineage_replace_reconciliation_transition.h`
- `c/src/proto/lineage_replace_reconciliation_transition.c`
- `rust/crates/proto/src/lineage_replace_reconciliation_transition.rs`
- `spec/lineage-replace-reconciliation-evidence-provenance.md`

Affected executable evidence:

- `rust/test-vectors/replay/lineage-replace-attempt-evidence-v1.txt`
- direct Rust/C attempt-evidence corpus consumers;
- existing reconciliation-transition corpus consumers, now adapted to the decision enum;
- existing RP-01..RP-14 provenance histories, now deriving the transition input through the shared classifier rather than test-local `fresh()` logic.

## Security property advanced

Durable state and historical confirmation cannot be promoted into successor-activation authority through caller-specific boolean derivation. Only a shared `FRESH_CURRENT_ATTEMPT` decision—same current symbolic attempt plus bilateral confirmation bound to that attempt—satisfies the reconciliation transition's evidence gate.

Missing attempts, attempt mismatch, missing/stale local confirmation, and missing/stale peer confirmation remain distinct fail-closed decisions. They are decision-compatible across the Rust/C source surfaces and canonical vector vocabulary.

## Local validation available in this execution environment

The C attempt-evidence corpus, reconciliation-transition corpus, and reconciliation-provenance corpus were compiled from the packet with GCC using `-std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Werror`; all passed. The same three binaries also passed locally under `-fsanitize=address,undefined -fno-omit-frame-pointer`.

Cargo/rustfmt, ProVerif, cppcheck, and a usable libsodium pkg-config installation were unavailable in the execution environment. Full Rust execution, full libsodium-linked C CI, formal execution, cppcheck, and whole-repository release qualification are therefore not claimed by this checkpoint.

## Claim boundary

This checkpoint does not allocate a wire attempt identifier, authenticate the symbolic identifier, define retransmission/liveness, establish crash-safe attempt persistence, prove rollback resistance, supply physical MCU measurements, close TD-001 through TD-004, establish independent interoperability, establish formal proof, or make ZK-ARCHE RFC-class/deployment-qualified.
