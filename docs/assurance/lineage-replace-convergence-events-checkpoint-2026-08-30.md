# Checkpoint — LINEAGE_REPLACE convergence event corpus — 2026-08-30

## Change class

Cross-language lifecycle/state qualification evidence for `zk213`. No cryptographic primitive, wire grammar, registry allocation, profile, normal-AUTH behavior, or trust-establishment rule changes.

## Affected surfaces

- `spec/lineage-replace-convergence-events.md`
- `rust/test-vectors/replay/lineage-replace-convergence-events-v1.txt`
- Rust convergence-event integration test
- C convergence-event integration test
- existing `lineage_replace_convergence` classifier, consumed unchanged

## Falsification targets

The packet is intended to reject or expose:

- duplicate confirmation changing the successor decision;
- confirmation accepted before its successor observation exists;
- one-sided crash/restart being silently treated as completed bilateral confirmation;
- a different successor replacing an already observed candidate within the same attempt;
- missing peer authorization being upgraded to convergence;
- Rust/C disagreement over the same abstract event history.

## Evidence boundary

The event reducer is intentionally test-side and wire-neutral. It demonstrates deterministic accept/reject behavior over a shared abstract event corpus. It does not establish production retransmission semantics, timers, attempt identifiers, real crash persistence, physical rollback resistance, key-confirmation cryptographic binding, distributed consensus/liveness, external review, target measurements, or deployment readiness.

The existing durability and freshness contracts remain the owners of local restart/storage classification. A future production replacement protocol must connect those local facts to authenticated peer confirmation without weakening NO-LEARNING AUTH or allowing two successors to become simultaneously usable.

## Qualification status at publication

A narrow C compilation/falsification of the exact existing convergence classifier was available before this packet. Full clean-checkout Rust/C CI-equivalent, sanitizer, formal, and release-qualification lanes were unavailable in the current execution environment because the repository could not be cloned and no Rust toolchain was installed. Those unavailable lanes must not be reported as passed.
