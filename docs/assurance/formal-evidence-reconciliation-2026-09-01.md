# Formal evidence reconciliation — AUTH-v3 query count and FM-09 boundary

Date: 2026-09-01
Scope: TD-003 formal evidence bookkeeping and NO-LEARNING traceability

This checkpoint reconciles two exact-current formal-assurance discrepancies without changing the protocol, the Rust/C runtime, or the ProVerif model. It is an evidence-control artifact, not a new proof result.

## 1. Current AUTH-v3 retained model result

The current synchronized AUTH-v3 model blob is:

```text
2f3817b5fb847ef948e4effab4b7d9871adc2e14
```

That exact model blob has retained ProVerif 2.05 evidence at:

```text
docs/assurance/formal-runs/2026-08-29-ae1eeb4-proverif-auth-v3-fm06.md
repository commit: ae1eeb47b830996470beb489fe3875e5fc2635a2
queries: 10
result: 10/10 true under the retained scoped model
```

Therefore the AUTH-v3 expectation in `scripts/ci-formal.sh` is 10, not 9. A nine-result expectation is stale pre-FM-06 bookkeeping and would reject the already-retained current model if ProVerif executes it.

This checkpoint does not inherit that old run onto any future model blob. Any semantic edit to the model still requires a new exact-model retained run before the edited model may be called FORMALLY ANALYZED.

## 2. FM-09 NO-LEARNING evidence boundary

The current AUTH-v3 model contains pre-existing trust as an input and retains the correspondence:

```text
ServerCompleteV3(...) ==> TrustedRecordPresent(client)
```

That correspondence supports the scoped statement that modeled AUTH completion depends on a modeled pre-existing trusted record. It does **not** by itself prove that normal AUTH cannot create, replace, delete, repair, or expand persistent trust state, because the current model does not represent a persistent trust-mutation operation whose absence/reachability is queried.

The concrete repository now separately owns that runtime invariant through:

```text
spec/auth-trust-mutation-boundary.md
rust/test-vectors/state/auth-trust-boundary-v1.txt
scripts/check-auth-trust-boundary.py
docs/assurance/auth-trust-boundary-checkpoint-2026-09-01.md
```

Rust normal AUTH receives the registry through immutable ownership at the dispatcher boundary, and C normal AUTH uses lookup/read paths while explicit SETUP owns the persistent registry mutation path. The structural checker and corpus make that implementation boundary auditable, but they are not a ProVerif theorem.

Accordingly, until a future symbolic model explicitly represents trust mutation and receives a retained exact-model tool result, FM-09 must be reported as:

```text
NO-LEARNING normative/runtime boundary: DEFINED + IMPLEMENTATION-TRACEABLE, structurally TESTED
pre-existing-trust prerequisite:         FORMALLY ANALYZED, scoped
no persistent trust mutation by AUTH:    NOT YET FORMALLY ANALYZED as a mutation property
```

This checkpoint supersedes the broader `FM-09 ... FORMALLY ANALYZED` shorthand and stale nine-query AUTH-v3 count in `docs/assurance/formal-model-contract.md` wherever those statements conflict with this more precise boundary. A later consolidation may fold this reconciliation into the long-lived contract without changing the evidence meaning.

## 3. Claim boundary

This reconciliation establishes none of the following:

- computational soundness of the custom proof;
- implementation equivalence to the symbolic model;
- constant-time behavior, RNG quality, memory safety, or secure storage;
- authorization-policy correctness;
- revocation, delegation, resumption, downgrade, or privacy closure;
- independent cryptographic review;
- RFC-class documentation;
- Common Contract completion;
- deployment qualification.

TD-003 remains open. The next FM-09 model expansion should occur only if a useful symbolic trust-mutation state can be introduced without abstracting away the very runtime side effect the property is meant to constrain, and it must receive a new retained exact-model ProVerif result before any stronger formal claim is restored.
