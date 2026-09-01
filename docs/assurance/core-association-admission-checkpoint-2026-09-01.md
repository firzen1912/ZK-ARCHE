# CORE association-admission checkpoint — 2026-09-01

Parent evidence basis: exact `dev` at `8d351a25930673313b529c0c9bf049b806a82cbb`.

## Scope

This checkpoint records a bounded CORE/LINK lifecycle packet that composes existing AUTH/TRUST/LINK/BIND evidence into one fail-closed association-admission postcondition. It does not introduce a second trust engine or change wire bytes.

Implemented surfaces:

- Rust wire-neutral `association_admission` classifier;
- matching C11 classifier;
- canonical 14-case Rust/C decision corpus;
- C corpus test, including NULL-input fail-closed behavior;
- normative CORE association-admission contract.

The successful decision requires completed AUTH, pre-existing trust, present/current authorization, current revocation state, non-revoked holder, current lineage, current replay continuity, and valid binding when required. Rollback suspicion and any request for normal AUTH to mutate trust fail closed.

## Executed validation

The available shell could not resolve `github.com`, so a complete exact-head checkout and the broad repository qualification suite were unavailable.

The exact-current repository-owned P2P property checker and canonical matrix were fetched through the repository connector and reconstructed locally. The gate executed successfully:

```text
p2p-common-contract-properties: PASS canonical=12 exhaustive_states=2048 success=24 fail_closed=2024
```

Before publication, the new C classifier, header, test, and canonical corpus were compiled/executed independently with:

```text
gcc -std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Werror
association admission corpus: ok
```

Rust tooling was unavailable in the execution environment, so the Rust corpus consumer is implemented but not reported as executed.

## Evidence boundary

This packet advances IMPLEMENTED and narrow TESTED evidence for association admission. It does not establish full Rust/C runtime interoperability, key-erasure timing, session invalidation propagation, physical MCU evidence, formal proof, external cryptographic review, RFC-class completion, or deployment qualification.

The classifier is deliberately a postcondition over facts owned elsewhere. It does not itself prove that those upstream facts were derived correctly; owning subsystem tests and cross-module integration remain required.
