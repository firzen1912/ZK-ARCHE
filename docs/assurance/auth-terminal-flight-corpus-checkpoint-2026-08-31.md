# AUTH terminal-flight corpus/checkpoint — 2026-08-31

## Scope

This checkpoint advances TD-004 / zk218 / zk225 conformance governance for the already-owned AUTH_3 terminal-state rule. It does not change wire encoding, cryptography, authorization, replay identity, or trust mutation.

## Evidence added

A canonical state corpus now records five required decisions: successful AUTH_3 consumes pending state; failed AUTH_3 consumes pending state; a non-cached same-session continuation after failure is unknown-session; exact cached duplicate handling does not recreate pending state; and a retry starts through AUTH_1 under a fresh session identifier.

`scripts/check-auth-terminal-flight-contract.py` binds that corpus to the normative contract and to both production dispatchers. It fails closed if the corpus drifts, if Rust stops consulting its exact-duplicate response cache before AUTH_3 dispatch, if Rust stops removing pending AUTH state before terminal verification, if C stops unconditionally releasing the AUTH slot before its error branch, or if the core normative markers disappear from the specification.

The checker is included in `scripts/ci-release-qualification.sh`, so future exact-checkout qualification cannot report PASS when this bounded state contract drifts across specification, corpus, or production dispatcher structure.

## Validation boundary

The available execution environment still lacks Cargo/rustc/ProVerif and cannot resolve github.com from the shell, so complete exact-head repository-owned Rust/C/formal/release execution was unavailable. The new checker was syntax-checked and exercised against a reconstructed fixture representing the governed Rust/C/source ordering and all five corpus cases; that narrow gate passed.

This is static contract-drift evidence, not executable end-to-end Rust/C interoperability evidence. It does not establish cryptographic correctness, formal proof, privacy equivalence, replay correctness beyond this bounded rule, memory safety, physical measurements, external review, Common Contract conformance, RFC status, or deployment qualification.

## Remaining work

The next qualification step is to execute the new gate plus the actual Rust and C state-transition tests in a clean exact-head checkout. Full zk218/zk225 evidence still requires deterministic executable cross-language failure/retry behavior, broader state-machine coverage, and retained exact-head qualification results.
