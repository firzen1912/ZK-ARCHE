# Lineage Replacement Authorization Checkpoint — 2026-08-31

## Scope

This checkpoint advances zk213 by making replacement authority explicit before the existing atomic lineage decision/state machinery. Rust and C now share an authorization classifier and an authorized lifecycle evaluation entrypoint covering current-credential control, successor-key control, current-session authentication/authorization, context binding, predecessor binding, and no privilege expansion.

## Evidence

Canonical AZ-01..AZ-10 vectors are shared by Rust/C consumers. The C consumer and affected lineage sources were compiled locally with GCC using `-std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Werror`; AZ-01..AZ-10 passed. A sanitizer binary could not be counted as passing because the available environment reported that the ASan runtime was not first in the library list.

Cargo/rustfmt, ProVerif, cppcheck, full libsodium-linked C CI, complete Rust/C interoperability, and release qualification were unavailable in this execution environment and are not inferred as passing.

## Claim boundary

This is a wire-neutral semantic gate, not proof that production packets cryptographically demonstrate the seven facts. It does not allocate wire fields, define retransmission/liveness, supply target power-loss or rollback-resistant storage evidence, provide independent review, close TD-001..TD-004, establish formal proof, or establish RFC-class/deployment qualification. The normalized legacy decision predicate remains for corpus compatibility; lifecycle request handlers are normatively required to use the explicit authorization entrypoint.
