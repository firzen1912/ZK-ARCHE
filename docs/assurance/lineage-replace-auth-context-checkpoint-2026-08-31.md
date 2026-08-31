# LINEAGE_REPLACE `iot-core` authorization-context checkpoint — 2026-08-31

## Scope

This checkpoint covers the wire-neutral binding between the existing `iot-core` AUTH authorization/attribution context and the LINEAGE_REPLACE authorization decision. It does not allocate wire syntax or claim cryptographic proof verification, durable atomicity, rollback resistance, formal verification, independent review, constrained-target measurement, or deployment qualification.

## Affected specification and implementation

- `spec/lineage-replace-authorization-contract.md`
- `c/include/auth/lineage_replace_auth_context.h`
- `c/src/proto/lineage_replace_auth_context.c`
- `rust/crates/proto/src/lineage_replace_auth_context.rs`
- `rust/crates/proto/src/lib.rs`

The adapter consumes the concrete `iot-core` authorization context and locally resolved attribution record. It derives session-authorization, peer/context binding, predecessor binding, and successor-scope preservation before invoking the existing shared authorization classifier. Current-credential control, successor-key control, and session authentication remain explicit upstream cryptographic/session evidence rather than being inferred from authorization state.

## Shared negative evidence

`rust/test-vectors/replay/lineage-replace-auth-context-v1.txt` defines AX-01..AX-12. Both implementation lanes have consumers:

- `c/tests/test_lineage_replace_auth_context.c`
- `rust/crates/proto/tests/lineage_replace_auth_context_corpus.rs`

The corpus covers the positive path and failures for current-credential control, successor-key control, session authentication, invalid authorization-context shape, authorization/attribution mismatch, wrong peer binding, wrong predecessor credential reference, privilege expansion, and deterministic multi-fault precedence.

## Validation performed in this environment

The available environment provides GCC but not Cargo/rustc/rustfmt, cppcheck, ProVerif, or a usable libsodium pkg-config installation. The affected C slice was therefore compiled independently of libsodium with:

```text
gcc -std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Werror
```

Result: `lineage-replace auth-context corpus: ok`.

The same affected C slice passed UBSan with `-fsanitize=undefined -fno-sanitize-recover=all`.

Rust compilation/tests, full repository C/libsodium qualification, static analysis, formal synchronization/ProVerif, and whole-release qualification were unavailable and are not inferred as passing.

## Claim boundary

This checkpoint advances IMPLEMENTED and locally executable C TESTED evidence only. It does not establish INTEROPERABLE, COMMON-CONFORMANT, MEASURED, FORMALLY ANALYZED, EXTERNALLY REVIEWED, RFC-CLASS DOCUMENTED, or DEPLOYMENT-QUALIFIED status.

The remaining zk213 threshold work is protocol integration: actual authenticated lifecycle handling must derive current-credential proof, successor-key proof, and session-authentication state from verified protocol/session evidence and feed this adapter before staging replacement. Durable rollback-resistant commit and complete Rust/C execution remain separate evidence gaps.
