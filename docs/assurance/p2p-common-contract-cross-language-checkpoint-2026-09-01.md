# P2P Common Contract cross-language checkpoint — 2026-09-01

## Scope

This packet converts the previously C-named lifecycle corpus into one language-neutral canonical Common Contract corpus and adds a Rust consumer for the same 16 cases. It does not change authentication, authorization, trust, replay, binding, or negotiation semantics.

## Evidence map

- canonical corpus: `rust/test-vectors/p2p/common-contract-lifecycle-v1.txt`
- C consumer: `c/tests/test_p2p_common_contract_lifecycle.c`
- Rust consumer: `rust/crates/proto/tests/p2p_common_contract_lifecycle.rs`
- fail-closed combined runner: `scripts/check-p2p-common-contract-cross-language.sh`

Both consumers invoke their language's existing association-admission implementation and apply the same separate mandatory-floor compatibility guard. Optional infrastructure and peer class are context dimensions, not protocol-authority inputs.

## Qualification contract

The combined runner succeeds only if both consumers execute successfully against the same canonical corpus. If Rust tooling is unavailable, the runner exits non-zero and reports the Rust lane unavailable; it must not reinterpret a C-only pass as cross-language qualification.

The canonical corpus retains MCU↔MCU, MCU↔Linux-edge, Linux-edge↔MCU, offline/local-state success, optional-infrastructure invariance, failed AUTH, missing trust, stale authorization/revocation/lineage/replay state, explicit revocation, mandatory-floor incompatibility, required-binding success/failure, and NO-LEARNING trust-mutation rejection.

## Evidence boundary

This is deterministic decision-level interoperability scaffolding. A successful combined run demonstrates Rust/C accept/reject parity for this bounded corpus only. It does not establish packet-level network interoperability, physical MCU execution, target resource budgets, formal proof, independent cryptographic review, or deployment qualification.
