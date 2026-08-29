# AUTH-v3 Canonical Context Parser Resource Evidence

Status: **bounded software qualification evidence** for the draft `ZKCTX` v1 parser. This record captures current Rust/C behavior for attacker-controlled `entry_count` handling and the exact hosted CI state that exercises it. It does not promote AUTH v3, clear TD-002 or TD-004, claim constrained-target measurements, or establish a general denial-of-service proof.

## 1. Repository and CI anchor

```text
repository_commit = 17b0eafbf29bab428f18e54483cafa85591a11b2
branch            = dev
ci_run            = #72 / 33248673472
ci_result         = success
```

All five applicable jobs completed successfully on that exact commit:

```text
Rust lane — fmt, check, test, clippy, audit                 success
C lane — build, tests, static analysis, sanitizers          success
Formal lane — legacy v2 + AUTH v3 + replay continuity       success
Release qualification — Rust/C interop & security gate      success
CI complete — required-lane gate                            success
```

The exact commit adds `rust/crates/proto/tests/auth_v3_context_parser_allocation.rs`; the Rust lane therefore executes the allocation regression through the repository-owned test path.

## 2. Historical research hypothesis and current repository result

`docs/research/daily/2026-08-28.md` recorded D20260828-F01 against an older repository state: the then-current Rust parser could derive `Vec` capacity directly from the untrusted 16-bit `entry_count` before proving that the input structurally contained that many entries. The report correctly classified this as a resource-asymmetry hypothesis requiring reproduction rather than as a confirmed vulnerability.

Current repository evidence is newer and supersedes that hypothesis for the tested malformed-count case. The dated research report remains unchanged as provenance.

## 3. Rust receive-side boundary

`rust/crates/proto/src/auth_v3_context_parser.rs` now performs both relevant entry-count checks before `Vec::with_capacity(entry_count)`:

```text
read entry_count
    ↓
if selected max_entries exists and entry_count > max_entries
    → EntryLimitExceeded
    ↓
structural_entry_limit = (input.len() - 9) / 5
if entry_count > structural_entry_limit
    → Truncated
    ↓
Vec::with_capacity(entry_count)
```

The structural bound follows from the five-byte minimum entry header. Consequently, a declared count cannot cause entry-vector materialization unless the input contains at least five bytes per declared entry. Profile-specific callers additionally have `parse_canonical_context_bounded(input, max_entries)`, which enforces the selected entry-count ceiling before materialization.

This is a resource-safety property of the current parser ordering; it is not a statement that every accepted context is allocation-free.

## 4. Direct hostile-count allocation regression

`rust/crates/proto/tests/auth_v3_context_parser_allocation.rs` installs a test-process tracking allocator and evaluates the exact nine-byte header:

```text
5a4b4354580101ffff
```

The value declares an AUTHORIZATION context with `entry_count = 65,535` and carries no entry bytes. The regression requires both:

```text
parse result       = ContextParseError::Truncated
allocation calls   = 0
```

The counter covers allocation, zeroed allocation, and reallocation while tracking is enabled around `parse_canonical_context()`.

This directly falsifies the older hypothesis that this specific hostile count reaches Rust heap materialization before structural rejection.

## 5. C receive-side boundary

`c/src/proto/auth_v3_context_parser.c` uses caller-owned entry storage rather than allocating an entry array internally. Its parser performs:

```text
entry_count > (input_len - 9) / 5
    → AUTH_V3_CONTEXT_PARSE_TRUNCATED

capture_entries && entry_count > entries_capacity
    → AUTH_V3_CONTEXT_PARSE_ENTRY_BUFFER_TOO_SMALL
```

before entry capture. Its hash-only path invokes the same parser with capture disabled and no entry array.

This supports the common-contract design direction that malformed pre-authentication count fields remain bounded by actual input structure and caller-selected storage capacity rather than attacker-declared count alone.

## 6. Rust/C decision evidence

The shared corpus `rust/test-vectors/auth-v3/context-parser-negative-v1.txt` contains:

```text
hostile-entry-count|TRUNCATED|5a4b4354580101ffff
```

Both Rust and C consume the shared parser-negative corpus in their CI test paths. The exact-head CI #72 and release-qualification lane passed after the Rust allocation regression was added.

The evidence supported here is therefore:

```text
hostile 65,535-count truncated header
    Rust decision = TRUNCATED
    C decision    = TRUNCATED
    Rust tracked heap materialization before rejection = 0 calls
```

## 7. Claim boundary and remaining work

This record does **not** establish any of the following:

- zero allocations for every malformed or valid Rust parser input;
- a fixed-RAM generic Rust parser for all representable `ZKCTX` values;
- peak stack/heap/flash or latency on STM32, ESP32-S3, or other physical targets;
- complete equivalence between Rust heap behavior and C caller-owned-buffer behavior;
- resilience to all parser CPU, memory-bandwidth, cache, or transport-level denial-of-service strategies;
- production profile limits beyond limits already defined and exercised by a selected profile;
- RFC-class completeness of the bounded parser contract;
- field readiness or deployment qualification.

The generic Rust parser may still materialize an entry vector for structurally present accepted input. Constrained/profile-specific receive paths should use their approved bounds rather than treating the generic representable `u16` envelope as a target resource budget.

Remaining evidence includes:

1. normative profile-specific pre-materialization limits wherever a production receive path is promoted;
2. retained boundary fixtures around those promoted profile limits;
3. actual target stack/heap/static-RAM/flash/CPU measurements under the TD-002 execution-context manifest;
4. broader malformed-input resource tests where they can falsify a concrete bounded-failure claim.

## 8. Evidence-state effect

```text
D20260828-F01 hostile-count pre-allocation hypothesis
    → REPRODUCED against newer implementation
    → specific 65,535 truncated-header heap-materialization concern NOT PRESENT

Rust malformed-count decision evidence       TESTED
C malformed-count decision evidence          TESTED
Rust specific hostile-count allocation bound TESTED on hosted Linux CI
Rust/C shared failure class                   TESTED
physical constrained-target resource evidence NOT MEASURED
TD-002                                        OPEN
TD-004                                        OPEN
AUTH-v3 production/selectable                 NO
COMMON-CONFORMANT                              NOT CLAIMED
DEPLOYMENT-QUALIFIED                           NOT CLAIMED
```

This is qualification evidence for current implementation behavior, not a maturity or certification claim.
