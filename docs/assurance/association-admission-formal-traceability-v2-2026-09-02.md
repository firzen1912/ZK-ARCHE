# Secure-association admission formal traceability v2 — 2026-09-02

Status: **MODELED + IMPLEMENTATION-TRACEABLE; NEW MODEL NOT YET FORMALLY ANALYZED**.

This checkpoint reconciles the secure-association symbolic model with the current v2 CORE admission classifier. It adds the two lifecycle prerequisites introduced after the original association model: current authorization generation and current restart continuity.

## Added properties

For a single admission evaluation:

- `AssociationEstablished(e) ==> AuthorizationGenerationCurrent(e)`;
- `AssociationEstablished(e) ==> RestartContinuityCurrent(e)`.

These correspond directly to `authorization_generation_current` and `restart_continuity_current` in the Rust/C association-admission facts and to the canonical `association-admission-v2.txt` negative cases.

The model retains the existing prerequisites for completed AUTH, pre-existing trust, authorization presence/freshness, revocation freshness/non-revocation, lineage, replay continuity, required binding, and rollback-clear state. It also retains the forbidden establishment combinations for normal-AUTH trust mutation and explicit revocation.

## Claim boundary

This is a synchronized symbolic decision-composition model. The two new events are authoritative facts supplied by owning lifecycle subsystems; the model does not prove that persistence, revocation distribution, generation management, restart recovery, or concrete parsers derive those facts correctly.

No prior ProVerif result is inherited by the edited model. `scripts/ci-formal.sh` now expects 14 true association-admission queries. A fresh exact-head ProVerif run is required before these two added properties can be reported `FORMALLY ANALYZED`.

In the current automation environment, `proverif` is unavailable. Therefore the present evidence state is:

`IMPLEMENTED(runtime) + TESTED(decision corpus) + MODELED + IMPLEMENTATION-TRACEABLE != FORMALLY ANALYZED`

This checkpoint does not establish constant-time behavior, RNG quality, memory safety, rollback-resistant storage, computational soundness of the custom proof, independent cryptographic review, RFC-class completion, physical-target evidence, or deployment qualification.
