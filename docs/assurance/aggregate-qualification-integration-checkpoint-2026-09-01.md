# Aggregate qualification integration checkpoint — 2026-09-01

## Scope

Slot 11 integration audit found that `scripts/ci-release-qualification.sh` had not yet incorporated two repository-owned gates added by recent roadmap work:

- `scripts/test-constrained-lifecycle-storage.py`, which verifies the constrained-target evidence manifest fails closed against fabricated observations and context-free measured claims; and
- `scripts/check-p2p-common-contract-cross-language.sh`, which requires the shared P2P lifecycle corpus to pass through both the C and Rust association-admission implementations.

The aggregate release qualifier now invokes both gates. This keeps the local/repository-owned qualification path aligned with current evidence surfaces without adding GitHub Actions to `dev`.

## Why this matters

A standalone test that is never reached by the aggregate qualification path can silently drift or be skipped during later release preparation. The constrained-manifest self-test protects evidence honesty, while the cross-language P2P runner prevents a C-only lifecycle pass from being presented as cross-language Common Contract evidence.

The cross-language runner remains fail closed: if Cargo/Rust execution is unavailable, it reports `UNAVAILABLE` and exits non-zero rather than treating the C result as sufficient.

## Evidence boundary

This checkpoint strengthens integration and qualification coverage only. It does not add new cryptographic behavior, create physical constrained-target measurements, establish formal proof, complete external review, make `p2p-iot-core` selectable, or establish deployment qualification.

TD-001, TD-002, TD-003, and TD-004 remain open under their existing declared evidence requirements.
