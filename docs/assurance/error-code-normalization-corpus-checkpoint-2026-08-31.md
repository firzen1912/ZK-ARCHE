# ERROR Code Normalization Corpus Checkpoint — 2026-08-31

## Scope

This checkpoint governs the bounded ERROR-code normalization surface defined by `spec/registries.md` §12. The registry requires every registered wire code to retain its allocation and every unknown received wire code to normalize to `UNSPECIFIED`; C local API errors `0x0001`–`0x0005` are not wire allocations.

The packet does not allocate a new error code, change response/no-response policy, add an alert, alter packet framing, or promote a protocol profile.

## Canonical corpus

`rust/test-vectors/wire/error-code-normalization-v1.txt` is the shared deterministic corpus for this surface. Version 1 contains 35 cases:

- EC-01..EC-29 enumerate every currently registered wire ERROR value, including `0x7FFF`, and require identity normalization;
- EC-30..EC-35 cover category-base/unassigned, future-looking, local-only, arbitrary, near-sentinel, and `0xFFFF` values and require normalization to `0x7FFF`.

The corpus records numeric wire semantics rather than language enum names so Rust and C consume the same expected result without sharing implementation-specific identifiers.

## Rust/C consumers

- Rust: `rust/crates/proto/tests/error_code_normalization_corpus.rs` checks both `ErrorCode::from_u16` and full `ProtoError::from_wire_payload` decoding against every corpus case. A short ERROR payload remains a `MALFORMED_PACKET` negative case.
- C: `c/tests/test_error_code_normalization.c` now consumes the same corpus through `auth_packet_parse_error`, checks the normalized numeric code, preserves the diagnostic-message slice, and retains the short-payload rejection test.

The C consumer is automatically part of the existing `c/Makefile` `tests/*.c` inventory. The Rust consumer is an ordinary Cargo integration test.

## Validation in this run

A clean exact-current repository checkout was unavailable in the execution environment because direct GitHub cloning could not resolve `github.com`. Cargo/rustc, ProVerif, cppcheck, and libsodium were also unavailable. Full repository-owned qualification is therefore **UNAVAILABLE**, not inferred green.

The changed C corpus consumer and exact 35-case corpus were independently materialized and checked with GCC 14.2 using:

`-std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Werror`

Result: **PASS**.

The same consumer/corpus was rerun with UBSan and `-fno-sanitize-recover=all`.

Result: **PASS**.

A structural corpus check confirmed exactly 35 unique cases: 29 registered identity cases and 6 unknown/local-only fallback cases.

The Rust consumer could not execute because the Rust toolchain is unavailable. No Rust PASS is claimed.

## Evidence boundary

This packet advances deterministic cross-language truth and regression coverage. It does **not** establish complete Rust/C interoperability, generalized ERROR response/state behavior, privacy-equivalent failure behavior, formal analysis, external review, constrained-device measurement, RFC-class completion, or deployment qualification.

Any future registry allocation must update the registry, both implementations as needed, this canonical corpus, and affected security/privacy review together; otherwise the shared corpus should fail rather than silently accepting drift.
