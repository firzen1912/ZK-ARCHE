# Secure-association admission formal traceability — 2026-09-01

Status: **MODELED + IMPLEMENTATION-TRACEABLE; NOT FORMALLY ANALYZED**.

This checkpoint advances TD-003 by mapping the implementation-backed secure-association admission postcondition into the synchronized ProVerif model set. It does not claim formal verification, computational cryptographic soundness, parser/model equivalence, persistence correctness, physical-target evidence, external review, or deployment qualification.

## Property scope

The model covers the post-AUTH decision composition defined by `spec/core-association-admission.md`. For one admission evaluation, `AssociationEstablished` implies modeled evidence for:

- completed AUTH;
- a pre-existing local trust record;
- authorization presence and freshness;
- current revocation state and a non-revoked holder;
- current lineage;
- current replay/restart continuity;
- satisfaction of any required binding;
- no rollback suspicion.

Two explicit negative queries additionally state that the same evaluation cannot both establish an association and request trust mutation, and cannot both establish an association and observe explicit revocation.

These properties sharpen existing FM-09, FM-10, and FM-13 traceability. They do not by themselves complete FM-10 or FM-13 across every wire/runtime path, and they do not establish FM-14/FM-15 resumption properties.

## Model ownership

Canonical symbolic source:

`rust/models/proverif/zk_arche_association_admission_draft.pv`

Synchronized mirror:

`c/models/proverif/zk_arche_association_admission_draft.pv`

`scripts/sync-formal-models.sh` now owns this pair. The two files are intentionally byte-identical copies of one abstract model, not independent Rust/C formal implementations.

## Model → specification → implementation → test mapping

| Formal event/query boundary | Normative/spec owner | Rust owner | C owner | Deterministic evidence |
|---|---|---|---|---|
| `AssociationEstablished -> AuthComplete` | `spec/core-association-admission.md` §2–3 | `association_admission.rs`: `auth_complete` | `association_admission.c`: `auth_complete` | `association-admission-v1.txt` |
| `AssociationEstablished -> PreexistingTrustRecord` | §2–4 NO-LEARNING AUTH | `preexisting_trust_record` | `preexisting_trust_record` | missing-trust negative case |
| `AssociationEstablished -> AuthorizationPresent/Fresh` | §2–3 | `authorization_present`, `authorization_fresh` | same facts | missing/stale authorization cases |
| `AssociationEstablished -> RevocationCurrent/HolderNotRevoked` | §2–3 | `revocation_current`, `explicitly_revoked` | same facts | stale-revocation/revoked cases |
| `AssociationEstablished -> LineageCurrent` | §2–3 | `lineage_current` | `lineage_current` | stale-lineage case |
| `AssociationEstablished -> ReplayContinuityCurrent` | §2–3 | `replay_continuity_current` | same fact | replay-continuity-stale case |
| `AssociationEstablished -> BindingSatisfied` | §2, §6 | `binding_required && binding_valid` | same facts | required-binding-invalid case |
| `AssociationEstablished -> RollbackClear` | §2–3 | `rollback_suspected` | same fact | rollback-suspected case |
| establishment + trust mutation impossible | §4 | `trust_mutation_requested` | same fact | trust-mutation-requested case |
| establishment + explicit revocation impossible | §3, §5 | `explicitly_revoked` | same fact | revoked case |

Concrete files at the modeling base include:

- `rust/crates/proto/src/association_admission.rs`;
- `c/include/auth/association_admission.h`;
- `c/src/proto/association_admission.c`;
- `c/tests/test_association_admission.c`;
- `rust/test-vectors/state/association-admission-v1.txt`.

The model intentionally does not duplicate the classifier's reason-precedence ordering. Its assurance claim is narrower: an establishment event must not exist without the modeled positive prerequisites, and the two security-critical forbidden combinations remain unreachable within one modeled evaluation.

## Attacker and abstraction boundary

The public command channel gives the symbolic attacker control over which abstract evaluation path is exercised. Each evaluation receives a fresh identity, preventing evidence from different evaluations from satisfying a same-evaluation correspondence.

The model treats owning subsystem outputs as already-normalized facts. Therefore it does **not** prove that AUTH, revocation, lineage, replay, persistence, or BIND implementations derive those facts correctly. It also does not model:

- cryptographic proof verification or computational assumptions;
- authorization/revocation distribution;
- stale-time calculation or clock behavior;
- rollback-resistant storage;
- session-key erasure/invalidation;
- parser behavior or memory safety;
- transport-address behavior;
- concurrency inside owning subsystem state machines.

Those remain separate runtime, formal, constrained-device, or external-review evidence obligations.

## Qualification state

The repository formal gate now includes the association-admission model with an expected inventory of 12 successful ProVerif queries. In this automation environment, ProVerif is unavailable and a direct exact-head checkout cannot be obtained because DNS resolution for `github.com` fails. Therefore this edit is **not** a retained successful formal run.

The correct state is:

`DEFINED + IMPLEMENTED(runtime) + TESTED(decision corpus) + MODELED + IMPLEMENTATION-TRACEABLE != FORMALLY ANALYZED`

A future `FORMALLY ANALYZED` claim requires `scripts/ci-formal.sh` to execute ProVerif against the exact tracked model blob and retain its exact-head manifest/log with all 12 queries true. Any false/unproved query remains a failure, not a documentation exception.
