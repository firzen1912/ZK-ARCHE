# Association-Admission Formal Boundary

This note records the exact formal-analysis boundary for the shared association-admission contract. It is an assurance/traceability artifact; it does not change protocol behavior, wire format, classifier behavior, or conformance requirements.

## Runtime contract

Association establishment is intentionally stricter than successful authentication. The executable contract distinguishes authentication, pre-existing local trust, authorization, authorization-generation provenance, revocation/lineage state, replay/restart/key-usage continuity, required binding, rollback state, and caller trust-mutation intent. Normal AUTH is **NO-LEARNING**: it cannot create or mutate trust.

The current fail-closed decision order treats rollback as the highest-priority lifecycle failure and then rejects an explicit trust-mutation request before downstream authentication, trust-presence, authorization, revocation/lineage, continuity, or binding failures. The shared association-admission v4 corpus now qualifies this boundary with multiple compound cases:

- `ASC4-017`: trust mutation plus invalid required binding still yields `FAIL_CLOSED|TRUST_MUTATION_REQUESTED`;
- `ASC4-019`: trust mutation plus multiple downstream unsafe facts still yields `FAIL_CLOSED|TRUST_MUTATION_REQUESTED` when rollback is clear;
- `ASC4-022`: trust mutation plus missing pre-existing trust and stale revocation state still yields `FAIL_CLOSED|TRUST_MUTATION_REQUESTED`;
- `ASC4-023`: trust mutation plus missing authorization, explicit revocation, and stale lineage still yields `FAIL_CLOSED|TRUST_MUTATION_REQUESTED`;
- `ASC4-024`: rollback plus trust mutation yields `FAIL_CLOSED|ROLLBACK_SUSPECTED`, retaining rollback's higher precedence.

These corpus cases are executable qualification evidence only when the relevant Rust/C corpus consumers are actually run. Their presence in the repository is not itself a current-run PASS.

## Current ProVerif projection

The exact-current dedicated association-admission model is `rust/models/proverif/zk_arche_association_admission_draft.pv`, with a byte-identical synchronized copy at `c/models/proverif/zk_arche_association_admission_draft.pv`. The model projects the wire-neutral CORE/LINK postcondition in `spec/core-association-admission.md`; it does not model the full AUTH wire protocol.

Unlike the older coarse AUTH FM-09 projection, this dedicated model explicitly contains `TrustMutationRequested(evaluation)` and the safety query:

```text
event(AssociationEstablished(e)) && event(TrustMutationRequested(e)) ==> false
```

It also models successful establishment as depending on the mandatory admission facts for authentication, pre-existing trust, authorization, authorization-generation binding/currentness, revocation/current holder state, lineage, replay/restart/key-usage continuity, required binding, and rollback clearance. This closes the earlier traceability ambiguity in which explicit trust-mutation intent existed only at the normative/runtime decision surface.

The symbolic model still does **not** model the runtime classifier's exact failure-reason precedence. Its `cmd_trust_mutation` path establishes that the modeled trust-mutation attempt is rejected rather than establishing an association; it does not prove that `TRUST_MUTATION_REQUESTED` must be the observable discriminator when binding, revocation, lineage, trust-presence, or authorization faults are simultaneously present. Likewise, the model's rollback-clear correspondence requirement does not prove that `ROLLBACK_SUSPECTED` wins over trust-mutation intent in the concrete classifier.

That distinction is deliberate: symbolic association-safety correspondence and executable deterministic discriminator precedence remain separate evidence classes.

## Model -> spec -> Rust/C -> test traceability

| Layer | Exact repository anchor | Established property | Remaining abstraction gap |
|---|---|---|---|
| Symbolic model | `rust/models/proverif/zk_arche_association_admission_draft.pv` plus synchronized C copy | `AssociationEstablished` requires the modeled admission facts and cannot coexist with `TrustMutationRequested` or explicit revocation | Does not execute production classifier/storage code, model exact reason precedence, or prove implementation equivalence |
| Normative contract | `spec/core-association-admission.md` | Authentication != authorization != trust mutation; rollback precedes trust-mutation rejection; normal AUTH is NO-LEARNING | Does not itself prove implementation conformance or persistence correctness |
| Rust/C classifiers | shared association-admission decision implementations consumed by their language-specific qualification surfaces | Concrete deterministic fail-closed decision order | Requires exact-head execution for TESTED/INTEROPERABLE evidence; source correspondence is not a formal proof |
| Shared corpus | `rust/test-vectors/state/association-admission-v4.txt` | Cross-language decision cases, including `ASC4-017`, `ASC4-019`, `ASC4-022`, `ASC4-023`, and `ASC4-024`, pin trust-mutation/rollback compound-fault precedence | Corpus presence is not execution and does not establish caller-side persistence/teardown behavior |
| Retained formal results | `docs/assurance/formal-runs/` | Historical ProVerif results exist for other exact model revisions/properties | No retained exact-model ProVerif result for the current dedicated association-admission model was identified by this reconciliation; no PASS is inferred |

The synchronized Rust/C ProVerif files are copies of one symbolic model, not independent formal implementations. Byte identity supports synchronization discipline only; it does not establish independent implementation equivalence.

## Formal abstraction boundary

The dedicated symbolic model must not be described as proving concrete classifier precedence or whole-program behavior. In particular it does **not** establish:

- that `TRUST_MUTATION_REQUESTED` is the concrete observable reason for every compound runtime state;
- that `ROLLBACK_SUSPECTED` concretely dominates trust-mutation intent;
- whole-program absence of persistent side effects from normal AUTH;
- correctness, crash consistency, or rollback resistance of trust/authorization/revocation storage;
- soundness of the custom role-membership proof or other computational cryptography;
- constant-time behavior, memory safety, RNG quality, zeroization, or target resource bounds;
- independent Rust/C formal equivalence;
- physical constrained-target behavior or deployment qualification.

The compound v4 corpus is therefore the appropriate decision-level evidence for exact discriminator ordering, while the dedicated ProVerif model is the appropriate symbolic surface for the modeled no-establishment safety correspondence.

## Closure criteria and retained-results discipline

The explicit trust-mutation **modeling** gap recorded by earlier versions of this note is now closed at the model-source level because `TrustMutationRequested` and its no-establishment query exist in the dedicated synchronized model. TD-003 is not closed. The remaining work for this surface is to:

- execute ProVerif against the exact dedicated association-admission model revision and retain the result or counterexample with model hash/commit provenance;
- keep the synchronized Rust/C model copies mechanically checked as the model evolves;
- keep attacker assumptions and abstraction choices synchronized with `spec/core-association-admission.md`;
- preserve model -> spec -> Rust/C -> shared-corpus traceability as admission facts and lifecycle semantics evolve;
- model additional lifecycle/privacy/compromise properties only where the abstraction can faithfully support them, rather than encoding concrete discriminator behavior as an unjustified symbolic claim.

Even after an exact-model ProVerif run, symbolic analysis would establish only the queries actually modeled and executed. Exact runtime discriminator ordering remains executable classifier/corpus evidence unless it is separately and faithfully represented by the model.

## Evidence status

This reconciliation repairs stale TD-003 traceability and records that explicit trust-mutation rejection already exists in the dedicated synchronized symbolic model. It does **not** create a new ProVerif result, does not assert an exact-head formal PASS, does not prove Rust/C implementation equivalence, and does not justify a roadmap score increase without the declared exit evidence for the affected roadmap item.

- `IMPLEMENTED`: unchanged.
- `TESTED`: unchanged unless repository-owned validation executes for the exact head.
- `INTEROPERABLE`: unchanged.
- `FORMALLY ANALYZED`: traceability/abstraction accounting strengthened; no new tool result claimed.
- `MEASURED`: unchanged.
- `EXTERNALLY REVIEWED`: unchanged; TD-001 remains open.
- `RFC-CLASS DOCUMENTED`: unchanged at roadmap score level.
- `COMMON-CONFORMANT`: unchanged.
- `DEPLOYMENT-QUALIFIED`: unchanged.
