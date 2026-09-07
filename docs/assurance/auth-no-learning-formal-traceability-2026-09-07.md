# AUTH NO-LEARNING Formal Traceability — 2026-09-07

Status: **scoped TD-003 traceability evidence**. This record binds the existing AUTH-v3 FM-09 symbolic property to the current normative NO-LEARNING contract and the concrete Rust/C qualification surface. It does not add or rerun a formal query, prove implementation/model equivalence, prove callee side-effect freedom, establish computational cryptographic security, or close TD-003.

## Exact reconciliation base

```text
branch = dev
reconciliation_head = 177e6902cda828e0d1f9e2d4060afc2f7f531ca5
rust_model = rust/models/proverif/zk_arche_auth_v3_draft.pv
c_model = c/models/proverif/zk_arche_auth_v3_draft.pv
synchronized_model_blob = 2f3817b5fb847ef948e4effab4b7d9871adc2e14
```

The Rust and C ProVerif paths are synchronized copies of one symbolic model, not independent formal implementations. This record therefore adds traceability only; it does not create new FORMALLY ANALYZED evidence for the reconciliation head.

## FM-09 property boundary

The current AUTH-v3 model contains the correspondence:

```text
event(ServerCompleteV3(cpk, spk, sid, secctx, kcctx))
  ==> event(TrustedRecordPresent(cpk)).
```

and initializes `TrustedRecordPresent(cpk)` before modeled client/server AUTH processes are launched. Under that symbolic abstraction, a server completion is downstream of pre-existing local trust. The model contains no transition that creates a trusted record from normal AUTH input.

Allowed interpretation:

> FM-09 represents NO-LEARNING AUTH at the symbolic trust boundary: accepted modeled AUTH depends on a pre-existing trusted record rather than learning one from the exchange.

Disallowed interpretations include:

- the concrete Rust/C AUTH implementation is formally proven side-effect free;
- every callee reachable from AUTH is proven unable to mutate storage;
- SETUP, ENROLL, LINEAGE_REPLACE, revocation, delegation, or future trust-management operations are proven correct;
- storage is rollback-resistant or crash-safe;
- the custom role-membership proof is computationally sound;
- constant-time behavior, RNG quality, memory safety, zeroization, or target resource behavior follows from FM-09.

## Model → spec → Rust/C → qualification map

| Layer | Current repository anchor | What it establishes | Remaining abstraction gap |
|---|---|---|---|
| Symbolic model | `rust/models/proverif/zk_arche_auth_v3_draft.pv`; synchronized C copy | `ServerCompleteV3 ==> TrustedRecordPresent`; modeled AUTH begins from local pre-existing trust | Model does not execute production registry/storage code or prove implementation equivalence |
| Normative contract | `spec/auth-trust-mutation-boundary.md` | Normal `AUTH_1`/`AUTH_3` MUST NOT mutate persistent trust/enrollment state; successful AUTH is not implicit enrollment, trust grant, or application authorization | Contract is deliberately narrow and does not define every trust-management lifecycle operation |
| Shared decision corpus | `rust/test-vectors/state/auth-trust-boundary-v1.txt` | Five canonical trust-effect cases distinguish normal AUTH unchanged-state outcomes from explicit `SETUP_3` mutation | Corpus is semantic qualification data, not formal proof or runtime persistence evidence |
| Rust production ownership | `rust/crates/server/src/main.rs` inspected by `scripts/check-auth-trust-boundary.py` | Normal AUTH retains immutable registry ownership while `SETUP_3` is the explicit mutable registry control path | Structural inspection does not prove arbitrary callees side-effect free |
| C production ownership | `c/bin/server.c` inspected by `scripts/check-auth-trust-boundary.py` | Normal AUTH dispatch/candidate scan rejects registry put/save markers while `SETUP_3` retains explicit persistence calls | Same structural/callee limitation as Rust |
| Repository-owned qualification | `scripts/check-auth-trust-boundary.py`, now invoked by `scripts/ci-all.sh` | Unified local qualification fails closed if the declared Rust/C trust-mutation ownership or required normative markers drift | A result is evidence only when the checker actually executes for the exact head; TD-005 remains separate |

## Attacker and lifecycle interpretation

FM-09 is evaluated within the AUTH-v3 model's active-network attacker scope and pre-provisioned local trust abstraction. The concrete NO-LEARNING contract additionally guards accepted and rejected AUTH dispatch paths against persistent registry mutation and keeps replay/retry failure behavior from becoming an implicit trust-repair path.

These layers are complementary rather than interchangeable:

```text
symbolic FM-09
  says accepted modeled AUTH requires pre-existing trust

normative NO-LEARNING contract
  says normal production AUTH must not mutate persistent trust

Rust/C structural qualification
  guards the current production ownership boundary against drift
```

None of the three establishes the other two automatically. In particular, the structural checker is not a formal proof, and the symbolic model does not establish source-level side-effect freedom.

## TD-003 effect

This closes one narrow traceability omission: FM-09 now has explicit current model → normative contract → Rust/C ownership → corpus → unified-qualification anchors.

TD-003 remains **open** because complete property/attacker coverage, parser/runtime-to-model equivalence, privacy and compromise semantics, lifecycle/restart/rollback coverage, retained exact-model results for future model changes, and complete model→spec→Rust/C→test mappings remain unfinished.

## Evidence posture

- IMPLEMENTED: unchanged; the production NO-LEARNING ownership boundary already existed.
- TESTED: unchanged unless repository-owned validation is actually executed for the exact head.
- INTEROPERABLE: unchanged.
- FORMALLY ANALYZED: traceability improved; no new ProVerif execution/result is claimed.
- MEASURED: unchanged.
- EXTERNALLY REVIEWED: unchanged; TD-001 remains open.
- RFC-CLASS DOCUMENTED: unchanged at rubric level.
- COMMON-CONFORMANT: unchanged.
- DEPLOYMENT-QUALIFIED: unchanged.
