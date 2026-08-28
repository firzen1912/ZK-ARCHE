# AUTH-v3 Formal Property Readiness Audit — 2026-08-28

Status: **TD-003 assurance audit**. This document inventories which formal properties are currently supported by concrete specification/runtime evidence, which properties have retained symbolic results, and which properties must remain deferred because their normative semantics are incomplete. It does not change protocol behavior, promote AUTH v3, clear TD-003, or claim RFC-class status.

## 1. Repository anchor

```text
repository_commit = ff7e046c0056c13acfeb4ceabb35113faac25dcc
branch            = dev
ci_run            = #59 / 33213896662
ci_result         = success
```

The currently retained draft AUTH-v3 formal run remains:

```text
docs/assurance/formal-runs/2026-08-28-16de6da-proverif-auth-v3.md
model             = rust/models/proverif/zk_arche_auth_v3_draft.pv
model_mirror      = c/models/proverif/zk_arche_auth_v3_draft.pv
model_blob        = 0fefe938988feb49086ecf579f2b0ae5df8d16eb
proverif          = 2.05
retained_queries  = 7
```

The Rust/C model copies are synchronized mirrors of one abstract model, not independent formal implementations.

## 2. Why this audit is needed

`docs/assurance/formal-model-contract.md` still contains status text written for the earlier AUTH skeleton and pre-retained replay/formal state. Current repository evidence is materially newer:

- draft AUTH v3 has a dedicated synchronized ProVerif model;
- seven AUTH-v3 correspondence queries have a retained exact-head successful run;
- the authorization-context admission boundary is explicitly modeled;
- concrete Rust/C `iot-core` authorization parsing, profile bounds, semantic checks, and exact-byte hashing exist and are CI-tested;
- replay continuity has its own synchronized model/evidence lane;
- AUTH v3 remains non-advertised/non-selectable.

This audit does not silently rewrite the older contract. It identifies the exact rows that are ready for reconciliation in a later bounded documentation packet and, more importantly, prevents new formal work from being chosen against stale readiness assumptions.

## 3. Evidence vocabulary

This audit uses the following states:

```text
DEFINED                 property and attacker expectation are stated
IMPLEMENTED             relevant runtime/model surface exists
TESTED                  executable Rust/C evidence exists at a green repository state
MODELED                  a symbolic event/query/abstraction exists
FORMALLY ANALYZED       a retained exact-model tool run supports the scoped property
IMPLEMENTATION-TRACEABLE model/spec/code/test boundary is explicit enough to audit
BLOCKED-NORMATIVE       stronger formalization would invent semantics not yet owned by spec/ADR
EXTERNALLY BLOCKED      independent review or hardware evidence is required
```

None of these states imply `FORMALLY VERIFIED`, `RFC-CLASS DOCUMENTED`, `COMMON-CONFORMANT`, or `DEPLOYMENT-QUALIFIED`.

## 4. Property readiness matrix

| ID | Current readiness | Concrete/formal basis | Blocking boundary / next requirement |
|---|---|---|---|
| FM-01 session-key secrecy | **DEFINED; model primitives present; not retained as an explicit secrecy result** | AUTH-v3 model derives symbolic DH/session keys and Finished/completion keys | add an explicit secrecy query only after deciding exactly which derived key is the claimed session/association secret; retain result and compromise assumptions |
| FM-02 client-to-server agreement | **FORMALLY ANALYZED, scoped** | retained injective `ServerCompleteV3 ==> ClientAuth3SentV3` | keep claim scoped to modeled identity/security-context fields and idealized proof primitives |
| FM-03 server-to-client agreement | **FORMALLY ANALYZED, scoped** | retained injective `ClientCompleteV3 ==> ServerCompleteV3` plus `ClientCompleteV3 ==> ServerAuth2SentV3` | same abstraction limits as FM-02 |
| FM-04 replay / injective acceptance | **FORMALLY ANALYZED only under persistent/unbounded model replay state; runtime behavior separately TESTED** | retained `ServerAuth1AcceptedV3 ==> ReplayRecordedV3`; Rust/C FIFO and replay-edge evidence; separate replay-continuity model | production replay capacity/lifetime, fresh authenticated replay epoch, restart/rollback equivalence remain open |
| FM-05 transcript/security-context integrity | **FORMALLY ANALYZED for modeled fields; partially IMPLEMENTATION-TRACEABLE** | completion/agreement queries carry identical `secctx`/`kcctx`; `iot-core` raw authorization boundary is executable and mapped to `AuthorizationContextAdmittedV3` | critical-extension and channel-binding canonical schemas are not yet equivalently concrete; parser-to-ProVerif equivalence is not established |
| FM-06 unknown-key-share resistance | **DEFINED; partially covered by agreement identities; not independently FORMALLY ANALYZED** | completion events carry both `cpk` and `spk`; client PID binds `cpk`, nonce, ephemeral key and intended `spk`; server verifies expected PID | before adding a dedicated UKS claim, reconcile exact runtime/spec identity-binding semantics and define the mismatch scenario to avoid merely duplicating FM-02/FM-03 |
| FM-07 reflection resistance | **DEFINED; candidate after runtime/spec audit** | directional Finished labels and distinct server/client key derivations exist in AUTH-v3 model | require an explicit cross-direction negative scenario mapped to Rust/C vectors/tests before promoting a formal correspondence/non-reachability claim |
| FM-08 downgrade resistance | **BLOCKED-NORMATIVE** | `security_context_v3` carries version/suite/profile/capabilities, but production AUTH-v3 negotiation is not selectable | negotiation, compatibility, unknown/critical selection and authenticated mandatory-floor rules must be normative and executable first |
| FM-09 NO-LEARNING AUTH | **FORMALLY ANALYZED, scoped** | retained `ServerCompleteV3 ==> TrustedRecordPresent(client)` | production trust-store mutation separation must remain independently tested/spec'd; this query proves only pre-existing modeled trust relation |
| FM-10 authentication/authorization separation | **BLOCKED-NORMATIVE** | concrete `iot-core` schema/validation and `AuthorizationCheckedV3` event exist | authority/provenance namespace, holder/audience/role lineage, revocation freshness and operation authorization remain incomplete; do not promote |
| FM-11 non-transitive trust | **BLOCKED-NORMATIVE** | roadmap invariant exists | explicit TRUST/delegation state and acceptance semantics required before useful symbolic modeling |
| FM-12 delegation bounds | **BLOCKED-NORMATIVE** | roadmap invariant exists | delegation credential/state schema, scope/depth/epoch rules and executable negatives required |
| FM-13 revocation freshness | **BLOCKED-NORMATIVE** | `iot-core` carries lifecycle epochs; replay-continuity state exists separately | authority namespace, revocation convergence/stale-window contract and rollback rules required |
| FM-14 resumption authorization preservation | **BLOCKED-NORMATIVE** | roadmap/research ownership exists | resumption credential schema plus authorization revalidation/fallback semantics required |
| FM-15 resumption reuse bounds | **BLOCKED-NORMATIVE** | roadmap/research ownership exists | issue/use/expiry/reuse counters and invalidation rules required |
| FM-16 P2P role symmetry | **BLOCKED-NORMATIVE / runtime path incomplete** | common-contract doctrine requires symmetric assurance | both initiator directions need concrete selectable flow and parity tests before formal claim |
| FM-17 infrastructure independence | **architecturally DEFINED; not a useful cryptographic theorem yet** | roadmap forbids CA/cloud/gateway ownership of root AUTH decision | model only after local trust/authorization inputs are explicit enough to distinguish optional synchronization services from required authority |
| FM-18 credential/reference binding | **BLOCKED-NORMATIVE** | R-013 owns the lookup/reference binding question | exact reference-to-key/commitment/role/policy semantics and substitution negatives required |
| FM-19 role confidentiality | **EXTERNALLY BLOCKED / model abstraction too strong** | `role_proof` is idealized in current ProVerif model | TD-001 cryptographic review plus explicit observable model required; do not infer privacy from symbolic constructor opacity |
| FM-20 unlinkability | **BLOCKED-NORMATIVE / modeling** | privacy backlog exists | define observer, stable identifiers, lower-layer metadata and allowed linkage before equivalence-style analysis |
| FM-21 failure-observability privacy | **BLOCKED-NORMATIVE / runtime evidence** | R-015 owns error/no-response/size/retry observability | explicit error/state-machine policy and differential response fixtures/measurements required |
| FM-22 compromise/recovery | **BLOCKED-NORMATIVE / model expansion** | attacker profile A3 is defined | choose exact compromise objects and post-compromise guarantees; fresh replay epoch/rekey/reprovisioning semantics are still unresolved |

## 5. Attacker-profile readiness

| Profile | Current use | Readiness |
|---|---|---|
| A0 active network attacker | current AUTH-v3 ProVerif lane | **active and retained** for scoped correspondences |
| A1 authorized malicious peer | needed for FM-10/FM-12 | **blocked** on authorization/delegation semantics |
| A2 stale/offline context | relevant to revocation/replay continuity | **partially represented** by separate replay-continuity work; authorization staleness not complete |
| A3 selective compromise | needed for FM-22 and privacy/recovery boundaries | **not yet instantiated in AUTH-v3 model** |
| A4 infrastructure loss | architectural common-contract condition | **not yet modeled**; local state dependencies need sharper spec ownership first |
| A5 privacy observer | needed for FM-19..FM-21 | **not yet sufficiently defined** for retained privacy claims |

## 6. Current AUTH-v3 retained query inventory

The exact retained model at `16de6da915a39e8d74dbab665ce1d98385681e8d` contains seven correspondences:

1. `ServerAuth1AcceptedV3 ==> ReplayRecordedV3`;
2. injective `ServerCompleteV3 ==> ClientAuth3SentV3`;
3. injective `ClientCompleteV3 ==> ServerCompleteV3`;
4. `ClientCompleteV3 ==> ServerAuth2SentV3`;
5. `ServerCompleteV3 ==> ClientAuth3SentV3`;
6. `ServerCompleteV3 ==> TrustedRecordPresent(client)`;
7. `ServerCompleteV3 ==> AuthorizationContextAdmittedV3(client, server, session, secctx)`.

CI #58 retained a successful fail-closed ProVerif run for those seven queries. CI #59 subsequently passed on the assurance-only commit that retained and documented that result.

## 7. Concrete runtime boundaries that may safely inform future formal work

### 7.1 Authorization-context admission

Already concrete enough for traceability:

```text
attacker-controlled raw authorization bytes
→ strict canonical ZKCTX parse
→ profile resource bounds
→ exact 7-entry / 148-byte iot-core schema
→ semantic zero/scope checks
→ SHA-256(exact accepted bytes)
→ authz_context_hash
→ AuthorizationContextAdmittedV3 abstraction boundary
```

This is executable Rust/C evidence plus a retained symbolic handoff. It is **not** a proof of authorization policy.

### 7.2 Identity agreement / UKS seam

The current model already carries peer public keys in completion events and binds the intended server public key into the client PID constructor. This makes FM-06 a plausible future target, but a new query should not be added merely to rename existing FM-02/FM-03 agreement evidence.

Before a dedicated UKS packet, the repository should define one exact adversarial mismatch scenario, for example:

```text
client believes peer = S1
server completion attributes run to S2
all remaining accepted transcript/security-context values otherwise attacker-spliced
```

Then map that scenario to:

```text
normative identity-binding rule
+ Rust symbols/tests
+ C symbols/tests
+ deterministic negative fixture where representable
+ formal query/event that is not redundant with existing agreement correspondences
```

### 7.3 Reflection seam

AUTH-v3 already uses directional labels and separate derived server-to-client/client-to-server Finished keys. FM-07 is therefore potentially dependency-ready **after** one cross-direction executable negative scenario is anchored in both lanes. That runtime negative evidence should precede a new formal claim.

## 8. Stale-contract reconciliation findings

The next maintenance update to `docs/assurance/formal-model-contract.md` should correct at least these statements:

- replace the old `zk_arche_auth_skeleton.pv` pair as the stated current AUTH model with the synchronized draft-v3 model, while retaining the legacy model only as historical/expected-negative evidence where appropriate;
- replace “retained result still required” for FM-02/FM-03/FM-09 with the current scoped retained state;
- record the seventh authorization-admission correspondence;
- distinguish the AUTH-v3 persistent/unbounded replay abstraction from the separate replay-continuity model rather than treating replay as one undifferentiated lane;
- remove statements that say FT-022/FT-023 or replay cross-language execution have no retained run when the current green Rust/C CI evidence already exercises those tests, while still keeping production lifetime/capacity equivalence open;
- update the next-packet ordering so unresolved normative seams are not bypassed by speculative formalization.

This audit deliberately does not perform that broader rewrite in the same packet because the goal here is to establish a reviewable current-state matrix before modifying the long-lived contract.

## 9. Priority decision after audit

No priority-#1 reproducibility defect is open at the repository anchor: CI #59 completed successfully.

The next TD-003 packet should **not** be FM-10, FM-08, delegation, revocation, resumption, P2P symmetry, privacy, or compromise modeling; those would outrun normative/runtime ownership.

The highest-value dependency-ready packet is:

```text
reconcile docs/assurance/formal-model-contract.md
against the retained seven-query AUTH-v3 evidence,
current replay-continuity lane,
and current Rust/C executable evidence.
```

After that reconciliation, the next actual new-property candidate is FM-07 reflection resistance, but only after a shared Rust/C cross-direction negative scenario is present. FM-06 should be considered in parallel only if the proposed UKS query adds a non-redundant identity-disagreement property beyond the existing injective agreement correspondences.

## 10. Claim boundary

This audit advances TD-003 traceability and prioritization only.

```text
TD-003                         OPEN
TD-004                         OPEN
TD-002                         OPEN
TD-001 external crypto review  OPEN
AUTH-v3 selectable             NO
FORMALLY VERIFIED              NOT CLAIMED
RFC-CLASS DOCUMENTED           NOT CLAIMED
COMMON-CONFORMANT              NOT CLAIMED
DEPLOYMENT-QUALIFIED           NOT CLAIMED
```
