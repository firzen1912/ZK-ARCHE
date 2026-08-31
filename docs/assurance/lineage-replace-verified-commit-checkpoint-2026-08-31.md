# LINEAGE_REPLACE verified commit checkpoint — 2026-08-31

## Scope

Checkpoint for the Rust/C composition that binds verified lifecycle possession, exact AUTH-session context, current `iot-core` authorization/attribution, normalized lineage acceptance, and the ordered storage transaction into one fail-closed entrypoint.

## Security invariants checked by construction

- Storage is unreachable after failed possession, session, authorization, predecessor, or privilege checks.
- Storage is unreachable after normalized freshness, replay, concurrency, rollback, context, successor/predecessor, or storage-safety rejection.
- The lower-level caller-provided `authority_valid` bit cannot authorize this path; the verified bound authorization decision overwrites it.
- Only an accepted decision can create the complete replacement plan.
- The first storage operation is `PERSIST_PENDING`; successful completion retains the existing successor/retirement/invalidation/clear order.
- Intermediate storage failures retain their exact failure result and are never translated into commit.
- AUTH remains NO-LEARNING and this composition introduces no new trust mutation surface outside explicit lineage replacement.

## Deterministic evidence

Canonical corpus: `rust/test-vectors/replay/lineage-replace-verified-commit-v1.txt` (`OC-01` through `OC-08`).

Both language consumers assert the same authorization decision, normalized lineage decision, storage result, and storage trace. Negative cases cover invalid current-credential proof, unauthenticated session completion, privilege expansion, stale freshness, replay rejection, failure at `PERSIST_PENDING`, and failure at `ACTIVATE_SUCCESSOR`.

## Claim boundary

IMPLEMENTED: yes, as a wire-neutral Rust/C semantic composition surface.

TESTED: deterministic Rust/C test consumers are present; execution results must be reported separately from source presence.

INTEROPERABLE: no new wire interoperability claim; this packet is wire-neutral.

COMMON-CONFORMANT: no new claim.

MEASURED: no target measurement claim.

FORMALLY ANALYZED: no new formal result.

EXTERNALLY REVIEWED: no; TD-001 remains open.

RFC-CLASS DOCUMENTED: no; this checkpoint is not an RFC-class completion artifact.

DEPLOYMENT-QUALIFIED: no.

## Retained gaps

The typed possession objects are still outputs of an upstream lifecycle cryptographic verifier rather than the verifier itself. The storage callback remains an abstract durability assertion. Real filesystem/flash/secure-element mapping, crash/power-cut testing, trusted freshness/high-water evidence, rollback attacker coverage, resource measurements, Rust/C executable qualification, and independent review remain required before stronger lifecycle or deployment claims.
