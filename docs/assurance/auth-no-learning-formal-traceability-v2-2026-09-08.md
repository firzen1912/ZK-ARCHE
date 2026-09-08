# AUTH NO-LEARNING Formal Traceability v2 — 2026-09-08

Status: **scoped TD-003 traceability evidence**. This checkpoint reconciles the existing AUTH-v3 FM-09 symbolic property with the strengthened exact-current Rust/C NO-LEARNING qualification boundary at `f43dfa3e49fdc1b57b1d29316f84e3484b9b021d`. It does not add or rerun a ProVerif query, prove implementation/model equivalence, prove callee side-effect freedom, establish computational cryptographic security, or close TD-003.

## Reconciliation base

```text
branch = dev
reconciliation_head = f43dfa3e49fdc1b57b1d29316f84e3484b9b021d
symbolic_model = rust/models/proverif/zk_arche_auth_v3_draft.pv
synchronized_c_copy = c/models/proverif/zk_arche_auth_v3_draft.pv
symbolic_property = FM-09 / ServerCompleteV3 ==> TrustedRecordPresent
qualification = scripts/check-auth-trust-boundary.py
normative_contract = spec/auth-trust-mutation-boundary.md
shared_corpus = rust/test-vectors/state/auth-trust-boundary-v1.txt
```

The Rust and C ProVerif paths are synchronized copies of one symbolic model, not independent formal implementations. This record therefore strengthens traceability only.

## Property interpretation

FM-09 models successful AUTH as depending on pre-existing local trust. The model does not create trusted registry state from normal AUTH input. The concrete normative rule is stronger at the implementation boundary: normal AUTH must not mutate persistent trust/enrollment state and must not be interpreted as enrollment, trust grant, or application authorization.

The exact-current structural qualification now additionally constrains where mutable AUTH state may live:

- Rust `AUTH_1` must create only ephemeral pending state in `state.auth_sessions`.
- Rust terminal `AUTH_3` must consume that state through `take_terminal_session(&mut state.auth_sessions, ...)`.
- C `AUTH_1` must activate an ephemeral AUTH slot through `auth_session_table_activate_auth(...)`.
- C terminal `AUTH_3` must release that slot through `auth_session_table_release(...)`.
- Normal Rust/C AUTH paths remain rejected if recognized persistent trust-registry mutation primitives appear.
- `SETUP_3` remains the explicit persistent-registry mutation control path.

This adds an implementation-state ownership constraint that FM-09 itself does not model: mutable state created by normal AUTH is expected to remain ephemeral session state rather than persistent trust state.

## Model → spec → Rust/C → test map

| Layer | Exact repository anchor | Established property | Remaining abstraction gap |
|---|---|---|---|
| Symbolic model | `rust/models/proverif/zk_arche_auth_v3_draft.pv` and synchronized C copy | Modeled successful server AUTH requires a pre-existing trusted record | Does not execute production storage/session code or prove implementation equivalence |
| Normative contract | `spec/auth-trust-mutation-boundary.md` | Normal AUTH cannot create, replace, delete, or expand persistent trust and is not implicit authorization | Does not define all ENROLL/SETUP/rekey/revocation persistence semantics |
| Shared corpus | `rust/test-vectors/state/auth-trust-boundary-v1.txt` | Canonical cases distinguish normal AUTH unchanged trust from explicit SETUP mutation | Decision corpus is not runtime persistence proof |
| Rust ownership gate | `scripts/check-auth-trust-boundary.py` → `rust/crates/server/src/main.rs` | AUTH registry access stays read-only while ephemeral state is inserted/terminally consumed through `state.auth_sessions` | Structural markers cannot prove arbitrary callee side effects or hidden aliases |
| C ownership gate | `scripts/check-auth-trust-boundary.py` → `c/bin/server.c` | AUTH registry mutation markers remain forbidden while temporary AUTH state is activated/released in `auth_session_table` | Same structural/call-graph limitation as Rust |
| Unified qualification | `scripts/ci-all.sh` invokes `scripts/check-auth-trust-boundary.py` | Exact-head local qualification can fail closed on model-adjacent spec/code/corpus ownership drift | Evidence exists only when the gate actually runs for the exact head; TD-005 remains separate |

## Formal abstraction boundary

The strengthened source-level gate must not be described as a formal proof. It establishes a repository-owned structural correspondence between the symbolic NO-LEARNING trust assumption and current Rust/C state ownership. It does **not** establish:

- whole-program absence of persistent side effects;
- soundness of the custom role-membership proof;
- crash consistency or rollback resistance of trust/session storage;
- constant-time behavior, memory safety, RNG quality, zeroization, or target resource bounds;
- correctness of explicit trust mutation workflows;
- independent Rust/C formal equivalence.

## TD-003 effect

This checkpoint closes one traceability seam introduced by the strengthened NO-LEARNING implementation gate: the formal-assurance record now explicitly maps ephemeral AUTH-session ownership into the model → spec → Rust/C → qualification chain rather than describing only persistent-registry mutation exclusion.

TD-003 remains **open**. Missing exit evidence still includes complete property/attacker coverage, compromise and privacy semantics, lifecycle/restart/rollback modeling, a canonical or mechanically synchronized model source discipline, retained exact-model results for future model changes, and complete model → specification → Rust/C → vector/test mappings for the broader protocol.

## Evidence posture

- `IMPLEMENTED`: unchanged; ephemeral AUTH-session ownership already exists in Rust/C.
- `TESTED`: unchanged unless repository-owned validation executes for the exact head.
- `INTEROPERABLE`: unchanged.
- `FORMALLY ANALYZED`: traceability strengthened; no new ProVerif run/result claimed.
- `MEASURED`: unchanged.
- `EXTERNALLY REVIEWED`: unchanged; TD-001 remains open.
- `RFC-CLASS DOCUMENTED`: unchanged at roadmap score level.
- `COMMON-CONFORMANT`: unchanged.
- `DEPLOYMENT-QUALIFIED`: unchanged.
