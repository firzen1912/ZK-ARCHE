# FM-09 NO-LEARNING Admission Traceability — 2026-09-12

Status: **scoped TD-003 implementation-traceability evidence** for exact repository state `93f0a45f34176afb5ebeb188041a6c44734a3666` and the existing retained AUTH-v3 formal result. This record does not modify the symbolic model, does not create a new formal theorem, does not claim parser/model equivalence, does not clear TD-003, and does not broaden the existing `FORMALLY ANALYZED` claim.

## 1. Purpose

The repository already has two separate evidence lanes for the NO-LEARNING AUTH invariant:

1. a retained scoped ProVerif correspondence showing that modeled successful server completion is downstream of pre-existing modeled trust; and
2. a production association-admission classifier plus shared Rust/C deterministic corpus that fail closed when normal AUTH attempts to mutate trust or when pre-existing trust is absent.

The 2026-09-12 association-admission update added compound cases that make the runtime boundary more explicit. This record binds those executable cases to the existing FM-09 abstraction without pretending that the symbolic model proves the production classifier or that the classifier proves the symbolic theorem.

## 2. Normative invariant

Current repository-owned security language requires normal AUTH to be NO-LEARNING:

- successful AUTH MUST NOT install or expand trusted peer/credential state;
- authentication, authorization, and trust mutation are separate decisions;
- discovery or transport reachability MUST NOT become trusted identity as an AUTH side effect;
- trust mutation belongs only to explicit enrollment, grant, commissioner, rekey/re-registration, or equivalent reviewed lifecycle transitions.

Primary normative owner:

```text
spec/security-considerations.md §3 — Prior trust and NO-LEARNING AUTH
```

This traceability record does not allocate any new trust-mutation flow.

## 3. Existing symbolic FM-09 boundary

The synchronized AUTH-v3 model contains:

```text
event TrustedRecordPresent(pkey).
...
query cpk:pkey, spk:pkey, sid:sessionid, secctx:bitstring, kcctx:bitstring;
  event(ServerCompleteV3(cpk, spk, sid, secctx, kcctx))
  ==> event(TrustedRecordPresent(cpk)).
```

The model process emits `TrustedRecordPresent(cpk)` before spawning server/client AUTH activity. The retained result therefore supports only this scoped statement:

> In the retained draft AUTH-v3 symbolic model, successful modeled server completion is reachable only relative to a pre-existing modeled trusted record.

It does **not** model a mutable production trust store, persistent enrollment state, rollback-resistant trust storage, grant issuance, generalized delegation, commissioner authority, or production trust-mutation APIs. Therefore it does not establish that every concrete runtime path is NO-LEARNING by itself.

Retained formal evidence remains the existing exact-model ProVerif run recorded by `docs/assurance/auth-v3-formal-traceability.md`; this document does not supersede that run and no edited model inherits a prior result.

## 4. Concrete association-admission boundary

Canonical runtime decision corpus:

```text
rust/test-vectors/state/association-admission-v4.txt
```

The current corpus contains 24 cases and is consumed independently by Rust and C association-admission tests. The following cases are the most direct FM-09/runtime anchors:

| Case | Runtime condition | Expected result | FM-09 relevance |
|---|---|---|---|
| `ASC4-003` | AUTH complete, pre-existing trust record missing | `FAIL_CLOSED / TRUST_RECORD_MISSING` | Authentication cannot substitute for absent prior trust. |
| `ASC4-017` | otherwise-current association requests trust mutation | `FAIL_CLOSED / TRUST_MUTATION_REQUESTED` | Normal association admission cannot mutate trust as a side effect. |
| `ASC4-019` | broadly invalid/incomplete state also requests trust mutation | `FAIL_CLOSED / TRUST_MUTATION_REQUESTED` | Mutation request is independently terminal rather than an implicit bootstrap path. |
| `ASC4-021` | AUTH complete, trust record missing, no mutation request | `FAIL_CLOSED / TRUST_RECORD_MISSING` | Pins the authenticated-but-untrusted boundary. |
| `ASC4-022` | AUTH complete, trust record missing, trust mutation requested | `FAIL_CLOSED / TRUST_MUTATION_REQUESTED` | Explicitly proves that an authenticated unknown peer cannot bootstrap trust through normal AUTH admission. |
| `ASC4-023` | AUTH complete, trusted peer present, authorization absent, trust mutation requested | `FAIL_CLOSED / TRUST_MUTATION_REQUESTED` | Missing authorization cannot be repaired by turning AUTH into a trust-mutation path. |
| `ASC4-024` | otherwise-current state has rollback suspicion and trust mutation request | `FAIL_CLOSED / ROLLBACK_SUSPECTED` | Rollback remains stronger than mutation handling; unsafe durable state cannot be repaired by a trust update request. |

The corpus is decision evidence, not persistence evidence. It demonstrates expected classifier outcomes for represented state inputs; it does not establish flash/filesystem atomicity, monotonic trust epochs, secure storage, power-loss survival, or authorization of a separate enrollment/rekey flow.

## 5. Model → spec → runtime → test mapping

```text
FM-09 property
  successful AUTH cannot create or expand trust
        ↓
rust/models/proverif/zk_arche_auth_v3_draft.pv
  ServerCompleteV3 ==> TrustedRecordPresent(client)
        ↓
spec/security-considerations.md §3
  normal AUTH MUST NOT install new trust; mutation is a distinct lifecycle act
        ↓
production association-admission semantics
  missing trust fails closed
  trust_mutation_requested fails closed in normal admission
        ↓
rust/test-vectors/state/association-admission-v4.txt
  ASC4-003 / 017 / 019 / 021 / 022 / 023 / 024
        ↓
independent Rust/C corpus consumers
  same expected decision/reason vocabulary
```

This is **IMPLEMENTATION-TRACEABLE** evidence for the represented NO-LEARNING admission boundary. It is not a formal proof that the Rust or C implementation refines the ProVerif model.

## 6. Evidence-composition rules

Allowed combined claim:

> ZK-ARCHE has scoped symbolic FM-09 evidence that modeled successful AUTH completion depends on pre-existing modeled trust, plus shared Rust/C association-admission vectors that fail closed when pre-existing trust is absent or when normal AUTH admission requests trust mutation. These lanes jointly strengthen audit traceability for the NO-LEARNING boundary while remaining distinct forms of evidence.

Disallowed inferences:

- the production Rust/C implementations are formally verified against ProVerif;
- the symbolic model proves trust-store persistence, rollback resistance, or storage atomicity;
- an authenticated peer is authorized merely because prior trust exists;
- any enrollment, commissioner, grant, rekey, or re-registration flow is automatically safe because normal AUTH is NO-LEARNING;
- the custom role-membership proof is computationally sound or independently reviewed;
- TD-003 is closed;
- TD-001, TD-002, TD-004, Common Contract qualification, or deployment qualification follows from this mapping.

## 7. Remaining FM-09 / adjacent gaps

FM-09 itself remains scoped to pre-existing modeled trust. Material adjacent work still includes:

- formal/runtime treatment of explicit trust-mutation lifecycle transitions rather than normal AUTH;
- durable trust-state recovery and rollback behavior;
- authorization-provenance and revocation interactions around re-registration/rekey;
- bounded delegation semantics;
- model/runtime equivalence or a mechanically generated executable trace layer;
- exact-head execution evidence for the current 24-case Rust/C corpus where such evidence is required for a `TESTED` claim;
- physical constrained-target persistence evidence under TD-002.

No additional FM-09 theorem is justified by this record alone. A future model edit intended to represent mutable trust or explicit learning flows must define the normative lifecycle semantics first and retain a new exact-model formal run.

## 8. TD-003 effect

```text
FM-09 property definition                         PRESENT
FM-09 scoped retained symbolic result             PRESENT (existing exact-model run)
NO-LEARNING normative security requirement        PRESENT
Rust/C shared admission decision corpus           PRESENT
Authenticated-but-untrusted mutation negative     PRESENT (ASC4-022)
Authorization-missing mutation negative           PRESENT (ASC4-023)
Rollback + mutation precedence negative           PRESENT (ASC4-024)
Model→spec→runtime→vector audit mapping            IMPROVED by this record
Full formal/runtime refinement proof               ABSENT
Mutable trust-store formal semantics               ABSENT
TD-003                                              OPEN
```

This packet advances TD-003 traceability and evidence hygiene only. It does not change roadmap scoring by itself because no declared phase gains a new completed exit-evidence class.
