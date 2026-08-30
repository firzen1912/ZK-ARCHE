# LINEAGE_REPLACE Decision Conformance Contract

Status: **wire-neutral executable decision contract / runtime transition not implemented**.

This document refines the decision-evidence surface defined by `replay-epoch-transition-owner.md`. It does not allocate a wire message, registry value, packet encoding, or trust-store mutation API. The historical qualification snapshot in section 11 of `replay-epoch-transition-owner.md` predates this executable decision packet; this document supersedes only that snapshot, not the transition semantics.

## 1. Pure decision boundary

Rust and C expose independent pure evaluators over normalized lifecycle facts. The evaluators MUST NOT parse network packets, write trust or replay state, activate a successor lineage, or be invoked as an implicit consequence of ordinary AUTH.

Only the explicit `LIFECYCLE` trigger is eligible for `ACCEPT_SUCCESSOR`. Restart, transport/address change, and ordinary AUTH triggers map to `REJECT_AUTHORITY` even if every other normalized predicate is true.

## 2. Decision predicates

The executable decision surface covers these normalized predicates derived from the canonical `LineageReplaceRequest` and local state:

```text
trigger
authority_valid
predecessor_valid
successor_valid
context_valid
freshness_valid
replay_free
concurrent_free
rollback_clear
storage_safe
dependent_state_safe
```

These booleans are test/evaluator inputs, not wire fields. A future runtime implementation must derive them from authenticated canonical inputs and durable local state according to `replay-epoch-transition-owner.md`.

The internal decision classes remain:

```text
ACCEPT_SUCCESSOR
REJECT_AUTHORITY
REJECT_PREDECESSOR
REJECT_SUCCESSOR
REJECT_CONTEXT
REJECT_FRESHNESS
REJECT_REPLAY
REJECT_CONCURRENT
REJECT_ROLLBACK
REJECT_STORAGE
```

## 3. Canonical corpus

`rust/test-vectors/replay/lineage-replace-decisions-v1.txt` is the shared Rust/C decision corpus. Both language lanes MUST consume the same file and MUST agree on all RE-01 through RE-20 expected decisions.

The corpus is semantic only. It MUST NOT be interpreted as a packet grammar, registry, transcript encoding, or proof that atomic storage replacement is implemented.

## 4. Claim boundary

After this packet, the repository may claim:

```text
LINEAGE_REPLACE normalized decision predicate    IMPLEMENTED in Rust/C
RE-01..RE-20 shared decision corpus              TESTED for pure decision parity
wire grammar / registry allocation               NOT PRESENT
atomic predecessor->successor storage transition NOT IMPLEMENTED
successor epoch activation                       NOT IMPLEMENTED
profile replay_epoch_rule                        remains unresolved
COMMON-CONFORMANT                                 NOT CLAIMED
RFC-CLASS DOCUMENTED                              NOT CLAIMED
DEPLOYMENT-QUALIFIED                              NOT CLAIMED
```

This evidence does not establish rollback-proof hardware, crash consistency, cryptographic authority proof, successor key possession, PFS, PCS, KCI resistance, secure erasure, implementation-level formal verification, external cryptographic review, or constrained-target measurements.
