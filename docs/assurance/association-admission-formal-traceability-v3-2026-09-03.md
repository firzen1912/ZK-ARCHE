# Secure-association admission formal traceability v3 — 2026-09-03

Status: **MODELED + IMPLEMENTATION-TRACEABLE; UPDATED MODEL NOT YET FORMALLY ANALYZED**.

This checkpoint supersedes the v2 association-admission traceability checkpoint for the current CORE admission surface. The current Rust/C classifier and canonical v3 decision corpus distinguish two separate authorization-generation facts:

1. `authorization_generation_bound` — authenticated provenance binds the authorization evidence to a specific lifecycle generation; and
2. `authorization_generation_current` — that bound generation equals the locally authoritative current generation.

The synchronized Rust/C ProVerif model now represents both facts independently through `AuthorizationGenerationBound(e)` and `AuthorizationGenerationCurrent(e)`.

## Property mapping

For a single admission evaluation, establishment requires both correspondences:

- `AssociationEstablished(e) ==> AuthorizationGenerationBound(e)`;
- `AssociationEstablished(e) ==> AuthorizationGenerationCurrent(e)`.

The model retains the existing establishment prerequisites for completed AUTH, pre-existing trust, authorization presence/freshness, revocation freshness/non-revocation, current lineage, replay continuity, restart continuity, required channel/context binding, and rollback-clear state. It also retains the forbidden establishment combinations for normal-AUTH trust mutation and explicit revocation.

The concrete Rust/C association classifier independently returns `AUTHORIZATION_GENERATION_UNBOUND` before `AUTHORIZATION_GENERATION_STALE`. The canonical `association-admission-v3.txt` corpus therefore falsifies missing provenance separately from stale generation state. The symbolic correspondence tracks that same distinction but does not derive the provenance fact cryptographically.

## Formal gate synchronization

The association-admission model now contains 15 queries. `scripts/ci-formal.sh` is synchronized to require exactly 15 successful `RESULT ... is true.` lines for this model. The previous expected count of 14 belonged to the v2 model and would incorrectly reject the updated model after the provenance correspondence was added.

A fresh exact-head ProVerif execution is still required before the added provenance correspondence can be reported `FORMALLY ANALYZED`. No previous retained ProVerif result is inherited by an edited model.

## Claim boundary

This model is a decision-composition abstraction. `AuthorizationGenerationBound` and `AuthorizationGenerationCurrent` are authoritative facts supplied by owning lifecycle/authorization subsystems. The model does not prove that cryptographic parsing, persistence, revocation distribution, generation advancement, restart recovery, or storage rollback resistance derive those facts correctly.

Current evidence state:

`IMPLEMENTED(runtime) + TESTED(decision corpus, prior lane) + MODELED + IMPLEMENTATION-TRACEABLE != FRESHLY FORMALLY ANALYZED`

This checkpoint does not establish constant-time behavior, RNG quality, memory safety, rollback-resistant storage, computational soundness of the custom proof, independent cryptographic review, RFC-class completion, physical-target evidence, cross-implementation wire interoperability, or deployment qualification.
