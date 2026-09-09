# Association-Admission Formal Boundary

This note records the exact formal-analysis boundary for the shared association-admission contract. It is an assurance/traceability artifact; it does not change protocol behavior, wire format, classifier behavior, or conformance requirements.

## Runtime contract

Association establishment is intentionally stricter than successful authentication. The executable contract distinguishes authentication, pre-existing local trust, authorization, authorization-generation provenance, revocation/lineage state, replay/restart/key-usage continuity, and required binding. Normal AUTH is **NO-LEARNING**: it cannot create or mutate trust.

The current fail-closed decision order treats rollback as the highest-priority lifecycle failure and then rejects an explicit trust-mutation request before downstream authentication, trust-presence, authorization, lifecycle, or binding failures. The shared association-admission v4 corpus includes compound case `ASC4-019` to exercise that boundary: with rollback clear and a trust-mutation request present alongside multiple other unsafe facts, the required classifier outcome is `FAIL_CLOSED|TRUST_MUTATION_REQUESTED`.

`ASC4-019` is executable qualification evidence only when the relevant Rust/C corpus consumers are actually run. Its presence in the repository is not itself a current-run PASS.

## Current ProVerif projection

`rust/models/proverif/zk_arche_auth_skeleton.pv` FM-09 models the coarser association-establishment safety property. Establishment requires authenticated state plus pre-existing local trust and current authorization scope/generation, revocation, lineage, replay, restart, key-usage, and binding predicates. That projection supports the claim that normal AUTH does not establish an association merely by learning trust during AUTH.

FM-09 currently does **not** expose `trust_mutation_requested` as a distinct symbolic input and does **not** model the runtime classifier's exact error/discriminator precedence. Consequently, FM-09 must not be cited as proving that `TRUST_MUTATION_REQUESTED` wins over downstream authentication, trust, authorization, lifecycle, or binding failures.

This distinction is deliberate: symbolic correspondence for association establishment and executable decision-precedence qualification are separate evidence classes.

## Traceability boundary

The evidence chain is therefore:

1. normative association-admission semantics define authentication != authorization != trust mutation and require pre-existing trust for normal AUTH;
2. Rust and C classifiers implement the fail-closed decision surface and discriminator order;
3. the shared association-admission v4 corpus, including `ASC4-019`, specifies cross-language decision evidence;
4. FM-09 models the broader establishment/pre-existing-trust safety property but not exact classifier precedence;
5. retained ProVerif output, when produced against an exact model revision, supports only the queries actually executed.

A symbolic result does not establish Rust/C byte compatibility, classifier error precedence, constant-time behavior, RNG quality, memory bounds, computational soundness, hardware behavior, or deployment qualification.

## Closure criteria for the explicit trust-mutation abstraction gap

The gap is closed only if all of the following are deliberately completed:

- the symbolic model gains an explicit trust-mutation-attempt fact/event without weakening NO-LEARNING AUTH;
- synchronized queries establish that association establishment cannot succeed through that path;
- attacker assumptions and abstraction choices remain documented and synchronized with the normative contract;
- model -> spec -> Rust/C -> shared-corpus traceability identifies the exact corresponding surfaces;
- ProVerif is executed against the exact model revision and the result or counterexample is retained with provenance.

Even after those steps, symbolic analysis would establish only the modeled security property. Exact runtime discriminator ordering remains executable classifier/corpus evidence unless it is itself faithfully represented by the model.

## Evidence status

This artifact closes a documentation/traceability ambiguity in TD-003. It does **not** create a new `FORMALLY ANALYZED` claim, does not assert a ProVerif PASS, and does not justify a roadmap score increase without the declared exit evidence for the affected roadmap item.
