# Lifecycle Formal Traceability

This file reconciles lifecycle-security properties that now have normative and executable repository surfaces with the formal-assurance inventory. It is an evidence map, not a proof result. `docs/assurance/formal-model-contract.md` remains the canonical formal-method contract; where that contract still labels a property `BLOCKED-NORMATIVE`, this file records exact-current evidence showing whether the normative blocker has been removed and what formal work remains.

## Evidence-state vocabulary

- **NORMATIVE-DEFINED** — a repository specification defines the property and its fail-closed semantics precisely enough to model.
- **IMPLEMENTATION-TRACEABLE** — Rust/C production behavior and shared qualification evidence can be mapped to the property.
- **MODEL-PRESENT** — a synchronized symbolic draft exists in the Rust/C model trees.
- **FORMALLY-ANALYZED** — reserved for a retained result produced by executing the exact synchronized model with the declared tool/version and preserving the result or counterexample.
- **BLOCKED-NORMATIVE** — the repository still lacks a sufficiently precise normative relation to model without inventing protocol semantics.

`MODEL-PRESENT` and `IMPLEMENTATION-TRACEABLE` MUST NOT be promoted to `FORMALLY-ANALYZED` without an executed, retained exact-model result. Symbolic analysis also does not establish constant-time behavior, RNG quality, memory safety/bounds, implementation-side-channel resistance, or computational soundness.

## Exact-current lifecycle traceability

| Property | Normative source | Executable/implementation trace | Symbolic model | Retained exact-model result | Current evidence claim |
| --- | --- | --- | --- | --- | --- |
| Authentication is not authorization and AUTH success does not mutate trust | `spec/core-association-admission.md`; normal AUTH NO-LEARNING contract | Rust/C association-admission classifiers and canonical shared association-admission corpus | `rust/models/proverif/zk_arche_association_admission_draft.pv` and mirrored C model | No retained run in `docs/assurance/formal-runs/` for this draft model | **NORMATIVE-DEFINED; IMPLEMENTATION-TRACEABLE; MODEL-PRESENT; NOT FORMALLY-ANALYZED** |
| Authorization-generation freshness/currentness | `spec/core-association-admission.md` | Rust/C association-admission classifiers and shared authorization/admission qualification | association-admission draft model | No retained matching run | **NORMATIVE-DEFINED; IMPLEMENTATION-TRACEABLE; MODEL-PRESENT; NOT FORMALLY-ANALYZED** |
| Revocation freshness and explicit revocation fail closed | `spec/core-association-admission.md`; `spec/p2p-bounded-delegation.md` | Shared Rust/C admission, delegation, and P2P lifecycle decision evidence | association-admission draft provides admission-side abstraction; other lifecycle drafts are partial | No retained matching run for the new lifecycle drafts | **NORMATIVE-DEFINED; IMPLEMENTATION-TRACEABLE; PARTIALLY MODEL-PRESENT; NOT FORMALLY-ANALYZED** |
| Trust is local and non-transitive | `spec/p2p-bounded-delegation.md` and the P2P Common Contract | Rust/C bounded-delegation classifier, canonical delegation evidence, and delegation fuzz/property harness | no complete retained model proving the Common Contract trust relation | None | **NORMATIVE-DEFINED; IMPLEMENTATION-TRACEABLE; FORMAL GAP OPEN** |
| Delegation is explicit, scoped, bounded, revocable, generation/epoch-aware, and rollback-aware | `spec/p2p-bounded-delegation.md` | Rust/C bounded-delegation classifier, canonical shared decision evidence, and `p2p_delegation` fuzz/property target | retained-authority/lifecycle drafts cover only portions of the required relation | None | **NORMATIVE-DEFINED; IMPLEMENTATION-TRACEABLE; FORMAL GAP OPEN** |
| Resumption requires current authorization and lifecycle continuity; retained material cannot resurrect invalid state | `spec/resumption-authorization-decision.md` | Rust/C resumption admission classifier and canonical shared resumption-authorization corpus | `zk_arche_retained_authority_draft.pv` captures part of retained-authority behavior but does not establish the complete future ticket/PSK/wire protocol | No retained matching run | **NORMATIVE-DEFINED; IMPLEMENTATION-TRACEABLE; PARTIALLY MODEL-PRESENT; NOT FORMALLY-ANALYZED** |
| Lineage replacement/commit remains fail closed across stale or invalid lineage state | lifecycle/lineage specification and shared decision evidence | Rust/C lineage lifecycle qualification | `zk_arche_lineage_replace_commit_draft.pv` in synchronized model trees | No retained matching run | **IMPLEMENTATION-TRACEABLE; MODEL-PRESENT; NOT FORMALLY-ANALYZED** |
| Mandatory-floor downgrade resistance | no complete selectable-suite / mandatory-floor negotiation relation yet | related negative qualification exists but cannot define the missing normative negotiation relation | do not extend a model by inventing this relation | None | **BLOCKED-NORMATIVE** |

## Reconciliation with `formal-model-contract.md`

The lifecycle rows above remove the *normative-definition* blocker for authentication/authorization separation, local non-transitive trust, bounded delegation, revocation freshness, and authorization-aware resumption. Their next blocker is now formal-model coverage and retained exact-model execution, not absence of repository semantics.

This does **not** retroactively change any historical formal-run claim. Existing retained AUTH results remain scoped to the model and queries actually executed in those run records. The newer association-admission, retained-authority, and lineage-replacement drafts are not `FORMALLY-ANALYZED` merely because synchronized `.pv` files exist.

Mandatory-floor downgrade resistance remains `BLOCKED-NORMATIVE`. A future model MUST wait for the repository to define the selectable capability/suite relation, mandatory security floor, peer-offer/selection rules, and failure behavior precisely enough that the model is derived from protocol semantics rather than choosing them.

## Required TD-003 closure sequence

For each newly traceable lifecycle property, complete the following sequence without collapsing evidence classes:

1. Define the property and attacker capability from the normative source, including explicit abstraction limits.
2. Synchronize the Rust/C symbolic model copies and bind the model to concrete spec sections, production decision surfaces, and shared negative/positive qualification evidence.
3. Add queries/correspondence assertions for the property rather than relying on reachability of a nearby event.
4. Run the repository-owned formal lane with the declared ProVerif/Tamarin version against the exact model bytes.
5. Retain the command, tool version, model identity/hash, exact repository HEAD, full relevant result, and any counterexample under `docs/assurance/formal-runs/`.
6. Reconcile model→spec→Rust/C→test traceability and document any abstraction gap before using `FORMALLY-ANALYZED` language.
7. Keep computational, constant-time, RNG, memory/resource, hardware, and deployment claims outside the symbolic-proof boundary unless independently evidenced.

## Immediate formal priorities

1. **Association admission:** turn the synchronized draft into explicit queries for authentication/authorization separation, authorization-generation freshness, revocation freshness, rollback/restart continuity, and binding prerequisites; retain the first exact-model run.
2. **Bounded P2P delegation:** build a dedicated model for local/non-transitive issuer trust, bounded/scoped delegation, revocation/generation/epoch freshness, and forbidden redelegation; map it to the Rust/C classifier and existing fuzz/vector evidence.
3. **Resumption:** model the currently specified wire-neutral admission decision only. Do not infer ticket/PSK issuance, ticket protection, or a resumption handshake that the implementation does not yet claim.
4. **Lineage replacement:** execute and retain evidence for the synchronized draft only after confirming its events and queries map to current lineage specification and Rust/C qualification.
5. **Downgrade resistance:** remain blocked until the normative mandatory-floor negotiation relation exists.

This traceability map advances TD-003 by separating properties that are now model-ready from properties still normatively blocked while preserving the evidence ceiling: no new formal proof or formal PASS is claimed by this document.