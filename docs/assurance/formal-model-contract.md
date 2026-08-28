# ZK-ARCHE Formal Model and Traceability Contract

This document defines the evidence contract for formal analysis of ZK-ARCHE. It advances TD-003 by making the property set, attacker assumptions, model ownership, model-to-spec-to-code mapping, retained-result requirements, and formal/runtime assumption boundaries explicit. It does **not** claim that every listed property is proven, that the custom proof is computationally sound, or that ZK-ARCHE is RFC-class, Common-Conformant, or deployment-qualified.

## 1. Status and authority

Current AUTH formal model inputs:

- `rust/models/proverif/zk_arche_auth_v3_draft.pv`
- `c/models/proverif/zk_arche_auth_v3_draft.pv`

Current replay-continuity formal model inputs:

- `rust/models/proverif/zk_arche_replay_continuity_draft.pv`
- `c/models/proverif/zk_arche_replay_continuity_draft.pv`

Historical/expected-negative AUTH-v2 evidence remains useful for regression provenance, but `zk_arche_auth_skeleton.pv` is no longer the current AUTH model authority.

The Rust/C paths in each model pair are synchronized copies of one abstract model, not independent formal implementations. `scripts/sync-formal-models.sh` is the repository synchronization check.

Current assurance state:

```text
PROPERTY/ATTACKER CONTRACT DEFINED
+ SYNCHRONIZED DRAFT AUTH-v3 MODEL PRESENT
+ 7 AUTH-v3 CORRESPONDENCE QUERIES RETAINED GREEN
+ AUTHORIZATION-ADMISSION ABSTRACTION BOUNDARY PRESENT
+ SYNCHRONIZED REPLAY-CONTINUITY MODEL PRESENT
+ 9 REPLAY-CONTINUITY CORRESPONDENCE QUERIES RETAINED GREEN
+ INITIAL MODEL→SPEC→RUST/C TRACEABILITY PRESENT
+ MATCHED-CAPACITY FIFO REPLAY CORPUS EXECUTED IN GREEN RUST+C CI
+ FT-022/FT-023 RUST+C EXECUTABLE SCENARIOS EXECUTED IN GREEN CI
!= FULL MODEL/RUNTIME EQUIVALENCE
!= COMPLETE PROPERTY COVERAGE
!= FORMALLY VERIFIED IMPLEMENTATION
!= CRYPTOGRAPHICALLY PROVEN
```

Current retained AUTH-v3 result:

```text
run record         = docs/assurance/formal-runs/2026-08-28-16de6da-proverif-auth-v3.md
repository commit  = 16de6da915a39e8d74dbab665ce1d98385681e8d
CI run             = #58 / 33209827503
tool               = ProVerif 2.05
model blob         = 0fefe938988feb49086ecf579f2b0ae5df8d16eb
retained queries   = 7
result             = fail-closed gate passed
```

Current retained replay-continuity result:

```text
traceability       = docs/assurance/replay-continuity-formal-traceability.md
repository commit  = b44b0c5961ace180f795fdba1b9a162c7c8d4ce3
CI run             = #45
tool               = ProVerif 2.05
model blob         = f44640a7ae7960db57a8782ab7ee260ac83c768d
retained queries   = 9
result             = fail-closed gate passed
```

The custom Schnorr/role-membership proof remains subject to TD-001 independent cryptographic review. Symbolic analysis cannot establish computational soundness of that proof, constant-time behavior, RNG quality, memory safety, side-channel resistance, secure key storage, rollback-resistant persistence, or field readiness.

## 2. Model ownership rule

ZK-ARCHE should converge on **one canonical symbolic state model** or one mechanically generated source from which tool-specific models are derived.

Until that migration is complete:

1. the Rust and C copies of a model MUST remain byte-identical when both claim to represent the same abstract protocol;
2. a semantic change to one copy MUST be mirrored in the other in the same reviewed change;
3. a run against one synchronized pair is evidence about that model text, not independent Rust/C verification;
4. formal results MUST be tied to an exact model blob, repository commit, tool name, and tool version;
5. no implementation claim may rely on modeled behavior absent from the concrete Rust/C path cited by the claim;
6. any abstraction materially stronger than runtime behavior MUST be named as an assumption gap in the retained result;
7. a new model edit requires a new exact-model retained run before the edited text can inherit a prior `FORMALLY ANALYZED` state.

Preferred future shape:

```text
canonical abstract state model
        |
        +--> ProVerif model
        +--> Tamarin/SAPIC+ model when warranted
        +--> executable conformance scenarios
        +--> model/spec/code traceability manifest
```

Rust and C are implementation targets, not independent sources of protocol truth.

## 3. Evidence vocabulary

Formal work uses the following states:

```text
DEFINED
  property, attacker, and expected security meaning are stated

IMPLEMENTED
  relevant runtime or formal surface exists

TESTED
  executable evidence exists at an identified green repository state

MODELED
  a symbolic event/query/abstraction exists

FORMALLY ANALYZED
  an exact-model retained tool run supports the scoped property

IMPLEMENTATION-TRACEABLE
  model/spec/Rust/C/test boundary is explicit enough to audit

BLOCKED-NORMATIVE
  stronger formalization would invent semantics not yet owned by spec/ADR

EXTERNALLY BLOCKED
  independent review or physical evidence is required
```

None of these states implies `FORMALLY VERIFIED`, `EXTERNALLY REVIEWED`, `RFC-CLASS DOCUMENTED`, `COMMON-CONFORMANT`, or `DEPLOYMENT-QUALIFIED`.

## 4. Property and attacker matrix

Every formal-analysis run must state which rows are actually modeled. An unmodeled row remains an evidence gap. A modeled row without retained tool output is **MODELED**, not **FORMALLY ANALYZED**.

| ID | Property | Required claim | Minimum attacker capability | Current evidence state |
|---|---|---|---|---|
| FM-01 | Session-key secrecy | Network attacker does not learn an accepted session/association secret absent modeled compromise | Full active Dolev-Yao control | **DEFINED; model key material exists; no retained explicit secrecy query** |
| FM-02 | Client-to-server agreement | Server completion implies a matching authenticated client run for the bound context | Replay, injection, modification, interleaving | **FORMALLY ANALYZED, scoped** — retained injective AUTH-v3 correspondence |
| FM-03 | Server-to-client agreement | Client completion implies matching server completion/authentication for the bound context | Replay, injection, modification, interleaving | **FORMALLY ANALYZED, scoped** — retained injective/completion correspondences |
| FM-04 | Replay / injective acceptance | Accepted AUTH cannot be justified by replay outside explicit retransmission semantics | Capture, replay, reordering, duplicate races | **FORMALLY ANALYZED only under persistent/unbounded AUTH-v3 replay table; runtime FIFO and replay-continuity are separate TESTED/analyzed lanes** |
| FM-05 | Transcript/security-context integrity | Security-relevant fields cannot be changed without failure | Active transcript mutation | **FORMALLY ANALYZED for modeled fields; partially IMPLEMENTATION-TRACEABLE** |
| FM-06 | Unknown-key-share resistance | Peers cannot complete while disagreeing about peer identity/commitment or security context | Identity substitution/session splicing | **DEFINED; partially covered by agreement identities; dedicated non-redundant scenario/query not yet justified** |
| FM-07 | Reflection resistance | Messages from one protocol direction cannot satisfy the opposite direction | Reflection/cross-role replay | **DEFINED; next plausible property after shared Rust/C cross-direction negative evidence** |
| FM-08 | Downgrade resistance | Peer cannot be induced below the authenticated mandatory floor | Capability stripping/selection modification | **BLOCKED-NORMATIVE** |
| FM-09 | NO-LEARNING AUTH | Successful AUTH cannot create or expand trust state | Active attacker + unknown/untrusted peer | **FORMALLY ANALYZED, scoped** relative to pre-existing modeled trust |
| FM-10 | Authentication/authorization separation | Possession proof alone does not imply authorization outside bound scope/policy | Valid credential in wrong audience/scope | **BLOCKED-NORMATIVE**; authorization-admission syntax/hash boundary is not authorization policy |
| FM-11 | Non-transitive trust | A trusts B and B trusts C does not authorize C at A without explicit accepted delegation | Malicious/intermediate delegate | **BLOCKED-NORMATIVE** |
| FM-12 | Delegation bounds | Delegation cannot exceed scope/role/audience/depth/validity/epoch | Privilege amplification by delegate | **BLOCKED-NORMATIVE** |
| FM-13 | Revocation freshness | Current local revocation/epoch state rejects known-revoked lineage within profile stale-state policy | Delayed/suppressed revocation, rollback | **BLOCKED-NORMATIVE** |
| FM-14 | Resumption authorization preservation | Resumption cannot carry stale privilege across changed authorization context | Valid resumption secret + context drift | **BLOCKED-NORMATIVE** |
| FM-15 | Resumption reuse bounds | Resumption credential cannot exceed lifetime/use policy | Replay/repeated presentation | **BLOCKED-NORMATIVE** |
| FM-16 | P2P role symmetry | Either initiator direction achieves equivalent mandatory assurance | Active attacker, reversed roles | **BLOCKED-NORMATIVE / runtime path incomplete** |
| FM-17 | Infrastructure independence | Already-authorized peers retain core AUTH without online CA/cloud/registry/gateway authority | Infrastructure loss | **Architecturally DEFINED; not yet a useful cryptographic theorem** |
| FM-18 | Credential/reference binding | Reference identifiers cannot substitute for cryptographic identity unless correctly bound | Mapping collision/substitution | **BLOCKED-NORMATIVE** |
| FM-19 | Role confidentiality | Observable behavior does not disclose exact role beyond declared claim | Passive/active privacy observer | **EXTERNALLY/ABSTRACTION BLOCKED**; idealized role proof is too strong |
| FM-20 | Unlinkability | Allowed runs are not linkable beyond declared unavoidable metadata | Passive/active privacy observer | **BLOCKED-NORMATIVE / modeling** |
| FM-21 | Failure-observability privacy | Error/no-response/size/retry behavior does not leak protected state beyond policy | Adaptive active probing | **BLOCKED-NORMATIVE / runtime evidence** |
| FM-22 | Compromise/recovery boundaries | Results state which guarantees survive specified compromise and recovery transitions | Selective key/state compromise | **BLOCKED-NORMATIVE / model expansion** |

## 5. Attacker profiles

### A0 — active network attacker

May read, drop, delay, reorder, replay, modify, inject, initiate concurrent sessions, and operate malicious peers. Does not initially know uncompromised long-term secrets.

Current AUTH-v3 retained correspondences are scoped primarily to A0.

### A1 — authorized-but-malicious peer

Possesses valid credential/trust state but attempts to exceed role, audience, deployment, operation, delegation, or validity bounds.

This profile is required for FM-10/FM-12 and remains blocked on authorization/delegation semantics.

### A2 — stale/offline peer context

May delay or suppress synchronization/revocation information and replay older locally valid state. It cannot forge a newer authorized state.

Replay continuity partially represents state-loss/staleness behavior, but authorization staleness and revocation convergence are not yet complete.

### A3 — selective compromise

A future model may reveal one or more of:

```text
peer long-term secret
session/resumption secret
issuer/delegation authority secret
persistent replay state
cached authorization state
```

Results must identify which guarantees fail and which remain for uncompromised peers/sessions. A3 is not yet instantiated in the AUTH-v3 model.

### A4 — infrastructure loss

CA/cloud/central registry/DNS/gateway/manufacturer service is unavailable. Already-authorized peers with sufficiently fresh local state are expected to retain core P2P AUTH capability. This is an architecture-dependency property, not cryptographic compromise.

### A5 — privacy observer

A privacy run must state whether the observer is passive or active and what lower-layer metadata is visible. Current AUTH-v3 correspondence evidence does not establish FM-19 through FM-21.

## 6. Current AUTH-v3 model coverage

The synchronized draft AUTH-v3 model includes:

- explicit session identifiers;
- client and server possession-proof interfaces;
- pre-existing trusted-record state;
- an authorization-check event;
- explicit `AuthorizationContextAdmittedV3` abstraction boundary;
- AUTH_1/AUTH_2/AUTH_3/authenticated-completion events;
- security-context and key-confirmation context constructors;
- persistent/unbounded `replay_seen_v3` state;
- `ReplayRecordedV3` and `ReplayRejectedV3` events;
- seven retained correspondence queries.

The exact retained AUTH-v3 query inventory is:

1. `ServerAuth1AcceptedV3 ==> ReplayRecordedV3`;
2. injective `ServerCompleteV3 ==> ClientAuth3SentV3`;
3. injective `ClientCompleteV3 ==> ServerCompleteV3`;
4. `ClientCompleteV3 ==> ServerAuth2SentV3`;
5. `ServerCompleteV3 ==> ClientAuth3SentV3`;
6. `ServerCompleteV3 ==> TrustedRecordPresent(client)`;
7. `ServerCompleteV3 ==> AuthorizationContextAdmittedV3(client, server, session, secctx)`.

The following material gaps remain:

- FM-01 does not yet have an explicit retained secrecy query tied to a precisely named session/association secret;
- production AUTH-v3 negotiation is not selectable;
- critical-extension and channel-binding canonical schemas are less concrete than the `iot-core` authorization schema;
- authority/provenance namespace semantics for authorization generations/epochs remain unresolved;
- delegation, revocation convergence, resumption, and P2P symmetry are incomplete;
- observable error/retry behavior and privacy-equivalence properties are missing;
- compromise events/recovery boundaries are missing;
- `schnorr_proof` and `role_proof` remain idealized symbolic interfaces.

## 7. Authorization-context admission boundary

The current `iot-core` authorization receive contract is independently executable in Rust and C before the symbolic AUTH-v3 admission point:

```text
attacker-controlled raw authorization bytes
        ↓
strict canonical ZKCTX parsing
        ↓
profile resource bounds
        ↓
exact 7-entry / 148-byte iot-core schema
        ↓
semantic zero/scope checks
        ↓
SHA-256(exact accepted bytes)
        ↓
authz_context_hash / authz_core handoff
        ↓
AuthorizationContextAdmittedV3
        ↓
security_context_v3 / kc_context_v3 / completion
```

Concrete owners include:

```text
spec/auth-v3-context-encoding.md
spec/iot-core-authorization-context.md
rust/crates/proto/src/auth_v3_context_parser.rs
rust/crates/proto/src/auth_v3_iot_core_authz.rs
c/src/proto/auth_v3_context_parser.c
c/src/proto/auth_v3_iot_core_authz.c
```

The corresponding retained query is:

```text
ServerCompleteV3(client, server, session, secctx)
    ==>
AuthorizationContextAdmittedV3(client, server, session, secctx)
```

Allowed claim:

> Modeled server completion is downstream of the modeled authorization-context admission event for the same client/server/session/security context, and the admission boundary is traceable to independently executable Rust/C canonical parsing/profile/hash behavior.

Disallowed inference:

- parser correctness is formally proven;
- SHA-256 collision resistance is proven;
- the parser and symbolic model are equivalent;
- holder/audience/role/provenance/revocation policy has authorized the requested operation;
- FM-10 is complete.

## 8. Replay evidence is split into two complementary lanes

Replay must not be treated as one undifferentiated model.

### 8.1 AUTH-v3 accepted-message replay lane

The AUTH-v3 model uses a persistent/unbounded symbolic replay table:

```text
table replay_seen_v3(bitstring)
```

The retained AUTH-v3 correspondence establishes replay-record ordering only under that abstraction. It does not model bounded eviction, restart, rollback, or fresh replay epochs.

Concrete Rust/C replay caches are bounded and use deterministic FIFO behavior when configured to the same capacity. The shared capacity-64 corpus is:

```text
rust/test-vectors/replay-cache/fifo-capacity-64.txt
```

Green repository CI executes the Rust and C consumers, including FT-022/FT-023 replay-edge tests. This is retained executable evidence through successful exact-head CI runs, but it does not make production capacities identical.

### 8.2 Replay-continuity lane

`spec/replay-continuity.md` defines:

```text
TRUSTED
RESTORING
CONTINUITY_BROKEN
```

and requires fail-closed behavior when restored replay state is missing, stale, unverifiable, or rollback-suspected. A restart, fresh transport, new address, or new outer `session_id` is not a fresh replay epoch.

The dedicated replay-continuity model has a retained nine-query ProVerif 2.05 run covering the already-specified state transitions. It does **not** model:

- physical persistence;
- FIFO contents/eviction;
- rollback detection implementation;
- crash/power-loss atomicity;
- an authenticated fresh replay epoch.

The authenticated fresh replay-epoch mechanism remains unresolved and blocks stronger replay-lifecycle claims.

### 8.3 FM-04 claim boundary

FM-04 may currently be reported only as:

> **SCOPED FORMALLY ANALYZED for accepted-message replay ordering under persistent/unbounded AUTH-v3 replay state, plus separately TESTED bounded FIFO decisions and SCOPED FORMALLY ANALYZED replay-continuity state semantics.**

It is not a proof of runtime persistence, production-capacity equivalence, rollback resistance, or safe fresh-epoch recovery.

## 9. Model-to-spec-to-code traceability map

This table is a mapping obligation, not a proof result.

| Model concept / property | Normative/spec owner | Rust surface | C surface | Executable evidence | Status |
|---|---|---|---|---|---|
| framing/message type/sequence | `spec/zk-arche-protocol.md`, `spec/registries.md`, `rust/wire-spec.md` | `rust/crates/proto/src/wire.rs` | `c/src/wire/**`, `c/include/auth/wire.h` | parser/vector tests | implementation exists; root spec incomplete |
| capability/profile negotiation | `spec/iot-profiles.md`, `spec/registries.md` | `caps.rs`, `profile.rs` | `c/include/auth/**`, `c/src/proto/**` | compatibility/profile fixtures | production AUTH-v3 selection incomplete |
| transcript/security context / FM-05 | AUTH-v3 design + context specs | AUTH-v3 context/transcript helpers | corresponding AUTH-v3 C helpers | deterministic vectors + context tests | modeled fields scoped analyzed; full context incomplete |
| cryptographic primitives/KDF/MAC | protocol spec + Security Considerations | `crypto.rs` | `c/src/crypto/**` | deterministic crypto/protocol vectors | abstracted; TD-001 separate |
| AUTH state/agreement / FM-02/FM-03 | AUTH-v3 design/spec work | AUTH-v3 reference primitives | AUTH-v3 reference primitives | Rust/C tests/vectors | scoped formally analyzed |
| trusted records / FM-09 | future TRUST/AUTH normative text | registry lookup path | registry callback/store path | unknown-peer/trust separation tests | scoped pre-existing trust modeled |
| authorization admission / FM-05 seam | `spec/auth-v3-context-encoding.md`, `spec/iot-core-authorization-context.md` | raw parser + iot-core decode/hash | raw parser + iot-core decode/hash | canonical + negative corpora | implementation-traceable boundary; parser equivalence not proven |
| accepted-message replay / FM-04 | AUTH/LINK replay + `spec/replay-continuity.md` | replay cache, guard, edge tests | replay cache, guard, dispatch tests | shared FIFO corpus + FT-022/FT-023 | tested; production capacity/lifetime mismatch remains |
| replay continuity | `spec/replay-continuity.md` | replay-continuity primitive/tests | replay-continuity primitive/tests | RC state tests + retained 9-query model | scoped formally analyzed; fresh epoch unresolved |
| retry/source validation/DoS | future AUTH_RETRY/BIND work | not promoted | not promoted | benchmark/prototype pending | blocked |
| authorization policy / FM-10 | TRUST/AUTH normative work | partial schema/validation only | partial schema/validation only | scope/provenance negatives incomplete | blocked-normative |
| rekey/revocation/lineage / FM-13 | lifecycle normative work | mapping pending | mapping pending | lifecycle negatives pending | blocked-normative |
| resumption / FM-14/FM-15 | LINK normative work | mapping pending | mapping pending | corpus pending | blocked-normative |
| transport/channel binding | BIND normative work | transport modules | transport modules | transport tests | model incomplete |
| deterministic conformance vectors | `spec/test-vectors.md` | Rust-owned vectors/corpora | C consumers | cross-language CI | executable evidence present; RFC-class package incomplete |

A formal property is not implementation-traceable until its row can provide:

```text
model event/query/lemma
+ normative spec section/field/state transition
+ exact Rust symbol(s)
+ exact C symbol(s)
+ positive/negative vector or executable test when representable
+ retained formal-tool result tied to commit/tool version
+ explicit abstraction/lifetime assumptions
```

## 10. Shared formal/conformance scenarios

At minimum, formal/conformance work should share scenario identifiers:

```text
FT-001 valid full AUTH
FT-002 replay AUTH_1 after completion
FT-003 cross-session AUTH_2/AUTH_3 replay
FT-004 reflected directional message
FT-005 changed selected profile after transcript start
FT-006 unsupported critical extension
FT-007 valid proof but unknown/untrusted peer (NO-LEARNING)
FT-008 valid credential in wrong audience/deployment
FT-009 implicit A->B->C trust-transitivity attempt
FT-010 delegation scope/depth amplification attempt
FT-011 revoked lineage with current local revocation view
FT-012 stale revocation view beyond profile freshness bound
FT-013 resumption after role/policy/epoch change
FT-014 repeated resumption beyond use policy
FT-015 P2P reverse-direction initiation
FT-016 already-authorized P2P with CA/cloud/gateway unavailable
FT-017 transport address changes while cryptographic identity remains stable
FT-018 credential/reference substitution
FT-019 privacy-equivalent protected failure classes
FT-020 replay after cache-pressure eviction
FT-021 replay after process restart/state loss
FT-022 replay under fresh outer session id/sequence with identical AUTH_1 identity material
FT-023 concurrent duplicate AUTH_1: at most one new accepted transition
```

Current important coverage:

- FT-020: shared capacity-64 FIFO decision corpus is executed by both lanes; production capacities still differ.
- FT-021: replay-continuity contract/tests fail closed on state loss; physical persistence is not proven.
- FT-022: Rust and C executable scenarios are exercised in green CI.
- FT-023: Rust and C executable scenarios are exercised in green CI; future state/locking changes must preserve the exactly-one-new-acceptance invariant.
- FT-004: remains the preferred next new-property seam only after a shared cross-direction Rust/C negative fixture is present.

Where a scenario can be represented as deterministic bytes/state, it should receive Rust/C positive or negative conformance coverage in addition to formal analysis.

## 11. Formal-run evidence manifest

Every retained formal run should record at least:

```yaml
tool: proverif | tamarin | other
tool_version: exact version
repository_commit: exact commit SHA
model_path: exact path
model_blob: exact blob/hash
attacker_profiles: [A0, ...]
properties: [FM-01, ...]
assumptions:
  replay_state:
    persistence: persistent | volatile | bounded | epoch-scoped
    capacity: exact-or-unbounded
    eviction: none | fifo | arbitrary | other
    restart_behavior: preserved | reset | modeled-epoch-transition
  - other explicit abstraction/compromise assumptions
results:
  FM-XX: proved | counterexample | inconclusive | not-modeled
counterexample_artifacts:
  - path or retained transcript when applicable
traceability_revision: document/model mapping revision
limitations:
  - what the result does not establish
```

A clean tool exit alone is not sufficient. CI formal gates must fail on zero results, false results, or unproved results, and retained records must identify the exact query/lemma set that was evaluated.

## 12. Validation and promotion gates

TD-003 remains **open**.

Current progression:

```text
CONTRACT DEFINED
  property/attacker/traceability obligations exist

MODEL EXPANDED
  draft AUTH-v3 and replay-continuity semantics are represented

FORMALLY ANALYZED
  selected scoped correspondences have retained ProVerif results

IMPLEMENTATION-TRACEABLE
  selected boundaries map to spec + exact Rust/C code + executable evidence
```

Current scoped advances:

```text
FM-02/FM-03 agreement                         FORMALLY ANALYZED, scoped
FM-04 accepted-message replay ordering        FORMALLY ANALYZED under persistent/unbounded model state
FM-04 replay-continuity state semantics       FORMALLY ANALYZED in separate state model
FM-05 modeled security-context integrity      FORMALLY ANALYZED for modeled fields
FM-05 iot-core authz admission boundary       IMPLEMENTATION-TRACEABLE + FORMALLY ANALYZED at handoff
FM-09 pre-existing trust / NO-LEARNING        FORMALLY ANALYZED, scoped
```

Still open:

```text
FM-01 explicit secrecy result
FM-06 non-redundant UKS property/scenario
FM-07 reflection property + shared runtime negative fixture
FM-08 downgrade semantics
FM-10 full authorization/provenance semantics
FM-11..FM-18 lifecycle/trust/binding properties
FM-19..FM-21 privacy properties
FM-22 compromise/recovery properties
fresh authenticated replay epoch
parser↔symbolic equivalence
complete model/spec/code traceability
```

`FORMALLY ANALYZED` does not imply `EXTERNALLY REVIEWED`, `CONSTANT-TIME`, `MEMORY-SAFE`, `COMMON-CONFORMANT`, `FIELD-READY`, or `RFC-CLASS DOCUMENTED`.

## 13. Dependency ordering for the next formal packets

Formal work must not outrun normative/runtime ownership.

The next packets should follow this order:

1. **FM-07 reflection seam:** first add one deterministic/shared Rust+C cross-direction negative scenario proving that server-direction material cannot satisfy the client-direction acceptance path (and vice versa where representable); only then add a non-redundant formal query/event.
2. **FM-06 UKS only if non-redundant:** define an identity-disagreement scenario that is not already implied by FM-02/FM-03 identity agreement before adding a dedicated query.
3. **FM-01 secrecy:** name the exact derived secret whose secrecy is being claimed and the compromise assumptions, then retain an explicit secrecy query.
4. **TD-004 prerequisites:** FM-08, FM-10 through FM-18, privacy, and compromise recovery remain downstream of missing normative/runtime semantics.
5. **Replay lifecycle:** do not model a fresh replay epoch until its authenticated transition, predecessor binding, crash behavior, and Rust/C conformance semantics are specified.

The August 28 research finding on authorization authority/provenance remains a blocker for stronger FM-10/FM-13 claims; no formal model should invent an issuer namespace or lifecycle authority that the specification has not selected.

## 14. Claim boundary

This contract is an assurance-control artifact.

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
