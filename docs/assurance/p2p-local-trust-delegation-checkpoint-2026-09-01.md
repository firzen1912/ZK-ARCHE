# P2P local-trust / delegation checkpoint — 2026-09-01

## Scope

This packet advances zk239–zk241 by converting the roadmap's local/non-transitive trust doctrine and bounded-delegation requirement into an explicit Common Contract plus deterministic negative-case governance.

It does not promote `p2p-iot-core`, create a delegation wire format, or claim runtime delegation support.

## Evidence added

- `spec/p2p-local-trust-and-delegation.md` owns local trust, non-transitivity, delegation bounds, NO-LEARNING interaction, and authentication-versus-authorization separation.
- `rust/test-vectors/p2p/local-trust-delegation-v1.txt` records eight canonical cases.
- `scripts/check-p2p-common-contract-qualification.py` now fails closed if the new contract or corpus drifts.

The corpus distinguishes work that is already structurally meaningful (`direct-local-trust`, `transitive-only`) from cases blocked on actual delegation implementation or unresolved lifecycle freshness/revocation semantics.

## Evidence boundary

This checkpoint is specification and qualification-governance evidence. It does not establish:

- executable delegation;
- MCU↔MCU or MCU↔edge runtime interoperability;
- revocation convergence or a stale-authorization bound;
- physical constrained-target measurements;
- independent cryptographic review;
- complete formal traceability;
- RFC-class completion; or
- deployment qualification.

A valid future delegation may only make a subject eligible for local authorization evaluation. It must not imply automatic persistent trust or make normal AUTH a trust-learning operation.
