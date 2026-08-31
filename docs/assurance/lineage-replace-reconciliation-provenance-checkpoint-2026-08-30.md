# Lineage Replacement Reconciliation Provenance Checkpoint — 2026-08-30

## Scope

This checkpoint adds shared Rust/C qualification for the provenance of `fresh_authenticated_attempt_evidence` used by the zk213 reconciliation transition guard.

## Reviewed surfaces

- existing reconciliation transition classifier remains unchanged;
- new canonical RP-01..RP-14 event corpus;
- Rust and C corpus consumers;
- normative wire-neutral provenance contract.

## Security invariants exercised

1. old-attempt confirmation cannot activate a newer successor;
2. replacing an observed attempt clears that side's previous confirmation provenance;
3. volatile restart loss cannot be reconstructed from durable successor state alone;
4. explicit clean retry does not imply confirmation;
5. both sides must currently agree on one attempt before confirmation can count as fresh evidence;
6. duplicate current confirmation is idempotent;
7. late retired-attempt confirmation is harmless and cannot replace current confirmation;
8. continuity failure and successor divergence remain fail-closed under the existing transition guard.

## Validation evidence

The C corpus consumer was compiled in the available execution environment with GCC using `-std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Werror` against the exact existing reconciliation-transition implementation semantics. RP-01..RP-14 passed.

Cargo/rustfmt, ProVerif, cppcheck, full libsodium-linked C qualification, sanitizers, and full release qualification were unavailable in this environment and are not claimed as passing.

## Claim boundary

This checkpoint is implementation/test evidence only. It is not independent cryptographic review, formal proof, physical rollback evidence, RFC-class completion, Common Contract conformance, or deployment qualification.
