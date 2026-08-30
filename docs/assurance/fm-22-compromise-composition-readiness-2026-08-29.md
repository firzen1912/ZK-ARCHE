# FM-22 Compromise and Composition Readiness — 2026-08-29

Status: **TD-003 assurance-readiness contract** for FM-22. This document defines the compromise-object inventory, corruption timing, guarantee vocabulary, cross-model shared-state boundaries, and proof-entry conditions that must exist before ZK-ARCHE can add a forward-secrecy, post-compromise-security, or recovery theorem.

It does **not** change protocol behavior, modify AUTH v3, add a ProVerif query, establish forward secrecy or post-compromise security, clear TD-003, or promote any unresolved recovery/rekey/resumption behavior into normative protocol semantics.

Repository anchor at preparation time:

```text
branch = dev
commit = 25cd1ee340943863b0c945ee2f63f564167ae45f
ci     = #90 / 33288617671 / success
```

## 1. Why FM-22 is not theorem-ready

Current AUTH-v3 formal evidence is deliberately scoped to attacker profile A0: active network control without endpoint or long-term-secret compromise. The existing FM-01 secrecy result therefore must not be interpreted as forward secrecy, post-compromise security, secure erasure, key-storage security, or recovery.

The current implementation contract also leaves the authenticated fresh replay-epoch transition unresolved and requires future rekey, key replacement, and resumption to avoid silently preserving state invalidated by credential, policy, or revocation changes. A process restart, address change, transport reconnection, or new outer session identifier is explicitly insufficient to create a fresh authenticated replay epoch.

Therefore a generic symbolic `compromise(secret)` event would be under-specified today. The security meaning depends on exactly which object is exposed, when exposure occurs, what state is shared with adjacent subprotocols, and which authenticated transition is claimed to restore which guarantee.

## 2. Attacker profile A3 — selective compromise contract

A3 is a refinement of the active-network attacker, not a replacement for A0. An eventual A3 experiment MUST identify each compromised object and corruption time explicitly.

Minimum corruption-time vocabulary:

```text
PRE_AUTH       before the affected AUTH run begins
DURING_AUTH    after run state exists but before authenticated completion
POST_AUTH      after authenticated completion for the target session
PRE_RECOVERY   after compromise but before a claimed recovery transition
POST_RECOVERY  after the exact authenticated recovery transition completes
```

A theorem MUST NOT collapse these timing classes when the guarantee differs between them.

## 3. Compromise-object inventory

| Object/state | Current repository surface | Timing that future A3 work must distinguish | Current proof status |
|---|---|---|---|
| Client long-term authentication scalar | `csk:skey` in AUTH-v3 model; implementation long-term credential/key state | PRE_AUTH / DURING_AUTH / POST_AUTH | no compromise theorem |
| Server long-term authentication scalar | `ssk:skey` in AUTH-v3 model | PRE_AUTH / DURING_AUTH / POST_AUTH | no compromise theorem |
| Role credential / role-proof secret or blinding state | idealized `role_proof(...)`; concrete custom proof remains TD-001-sensitive | PRE_AUTH / DURING_AUTH / POST_AUTH | no computational or compromise theorem |
| Client ephemeral DH secret | `ce:skey` | DURING_AUTH / POST_AUTH | erased-state behavior not modeled |
| Server ephemeral DH secret | `se:skey` | DURING_AUTH / POST_AUTH | erased-state behavior not modeled |
| Established AUTH session key | `session_key_v2(...)` / `SessionKeyEstablishedV3` | POST_AUTH | FM-01 secrecy only under A0/no compromise |
| Directional Finished/completion keys | `k_s2c_v3`, `k_c2s_v3`, `k_complete_v3` | DURING_AUTH / POST_AUTH | directional separation modeled; compromise recovery not modeled |
| Local identity-attribution/trust state | `TrustedRecordPresent`, `TrustedAttributionPresent`, resolver-backed runtime records | PRE_AUTH / DURING_AUTH / POST_AUTH / attacker-write or rollback | FM-06 attribution is scoped; trust-store compromise/recovery not modeled |
| Authorization context / policy-generation state | `authz_core`, admitted `secctx`, runtime generation/policy/revocation fields | stale / rollback / attacker-write before or during AUTH | policy authority/recovery semantics incomplete |
| Replay acceptance state | `replay_seen_v3`; separate replay-continuity model/runtime | loss / rollback / attacker-write / stale predecessor state | continuity is separately modeled; fresh authenticated epoch unresolved |
| Resumption secret/ticket state | roadmap/spec owner only; no complete selectable contract | compromise before issue / use / reuse / invalidation | FM-14/FM-15 blocked-normative |
| Future rekey/fresh-epoch material | not yet normatively defined | before / during / after transition | FM-22 recovery target unavailable |
| Persistent root seed / key storage representation | target-specific implementation evidence | extraction before/after reprovisioning or key replacement | TD-002/field evidence, not symbolic result |

This inventory is a minimum. Future implementations that add ticket keys, commissioner secrets, delegation credentials, DATA-plane keys, hardware-root material, or other persistent state MUST extend the matrix before inheriting any FM-22 claim.

## 4. Guarantee vocabulary

Future assurance MUST distinguish these claims rather than using “compromise resistance” generically.

| Guarantee | Minimum meaning required before claim | Current ZK-ARCHE state |
|---|---|---|
| Session secrecy under A0 | active network attacker cannot derive the modeled established key in the scoped honest run | scoped FM-01 retained |
| Forward secrecy (PFS) | compromise of specified long-term authentication secret(s) after session completion does not reveal the specified completed-session secret | **not established** |
| Key-compromise impersonation resistance | compromise of one party's long-term secret does not enable the precisely defined prohibited impersonation direction | **not established** |
| Post-compromise security (PCS) | after specified compromise, an authenticated transition introduces sufficient uncompromised fresh material such that later specified sessions regain a named guarantee | **not established** |
| Replay-state recovery | after state loss/rollback, a specified authenticated fresh-epoch transition safely establishes new replay continuity before predecessor state is discarded | **normatively unresolved** |
| Authorization recovery | stale/compromised authorization state is superseded through a named authenticated authority transition with bounded stale acceptance | **normatively unresolved** |
| Credential/key recovery | compromised credential/key is replaced or reprovisioned through an authorized transition and invalidated consistently | **partially specified at implementation-requirement level; not formally analyzed** |

A future theorem MUST name the guarantee and corruption timing instead of reporting a blanket “secure after compromise” result.

## 5. Composition and shared-state audit

The repository currently has at least two synchronized formal lanes with different state abstractions:

```text
AUTH v3 model
  long-term authentication keys
  ephemeral DH state
  session/security/key-confirmation context
  local trusted attribution
  authorization-admission abstraction
  symbolic replay table

replay-continuity model
  persistent replay generation/continuity behavior
  restart/rollback/freshness assumptions
```

Adjacent roadmap surfaces not yet represented by a complete selectable formal/runtime contract include:

```text
resumption
rekey / authenticated fresh replay epoch
revocation convergence
bounded stale authorization
delegation
DATA-plane key lifecycle
P2P reverse-role/common-contract lifecycle
```

Before modular/compositional formal reasoning, the following shared-state table MUST be completed for every model pair being composed:

| Boundary | Possible shared data/state | Current disposition |
|---|---|---|
| AUTH ↔ replay continuity | replay key/material, authenticated security context, persistence generation | material relationship exists; exact fresh-epoch composition incomplete |
| AUTH ↔ authorization | admitted authorization context, policy/revocation/generation state, peer attribution | partial concrete/runtime traceability; authority and recovery semantics incomplete |
| AUTH ↔ resumption | session/authorization context, future resumption secret/ticket, peer identity | blocked until resumption schema and revalidation rules exist |
| AUTH ↔ rekey/recovery | old/new security context, authenticated predecessor binding, fresh key material | blocked; no normative recovery transition yet |
| AUTH ↔ DATA | established/exported key and context | no complete DATA-plane formal ownership in current FM-22 scope |
| AUTH ↔ trust mutation | credential/key identity, trust record, enrollment/reprovisioning authority | normal AUTH is NO-LEARNING; recovery mutation flow must remain separate |

No external compositional theorem may be imported as ZK-ARCHE assurance merely because these surfaces look disjoint. The exact assumptions and shared-state/disjointness conditions must be checked against the concrete ZK-ARCHE model split first.

## 6. Recovery-transition entry criteria

A transition may become an FM-22 recovery boundary only after repository-owned normative and executable evidence identifies all of the following:

1. **Authority** — which already-authorized local state permits the transition.
2. **Trigger** — explicit rekey, reprovisioning, authenticated fresh epoch, full AUTH fallback, or another named state-machine transition.
3. **Fresh material** — exact nonce/ephemeral/key/credential material whose freshness is required.
4. **Predecessor binding** — how the new context is cryptographically bound to the intended predecessor or replacement authority when required.
5. **Invalidation** — which old keys, replay generations, tickets, credentials, policy generations, aliases, and caches become unusable.
6. **Rollback rule** — what happens when persistent state is stale, lost, partially written, or restored from an earlier generation.
7. **Authorization rule** — whether authorization is revalidated, replaced, narrowed, or rejected.
8. **Cross-language decision contract** — Rust/C make the same accept/reject transition decision.
9. **Negative evidence** — stale predecessor, wrong authority, wrong context, reused recovery material, rollback, and mismatched generation cases fail closed.
10. **Claim scope** — exact corruption object/timing and exact guarantee restored after the transition.

Until those conditions exist, FM-22 remains `BLOCKED-NORMATIVE / MODEL-READINESS`, not `FORMALLY ANALYZED`.

## 7. Model-entry gate for the first A3 experiment

The first compromise-aware ProVerif/Tamarin packet MUST satisfy all of the following before model text is changed:

```text
[ ] choose exactly one guarantee (for example PFS of the AUTH session key)
[ ] choose exact corruption object(s)
[ ] choose exact corruption timing
[ ] identify the corresponding normative key/state lifecycle
[ ] map the guarantee to concrete Rust and C state symbols where available
[ ] enumerate shared state with replay/authz/resumption/rekey models
[ ] record non-goals and environmental assumptions
[ ] define at least one representable negative/counterexample expectation
[ ] preserve the current A0 theorem as a separate scoped claim
[ ] require retained exact-model results after any model edit
```

A first experiment SHOULD be intentionally narrow. For example, a future PFS experiment may ask only whether POST_AUTH disclosure of one or both long-term authentication scalars reveals the already-established AUTH session key, while leaving PCS and recovery entirely out of scope. That experiment is not ready merely because the symbolic DH exchange uses ephemeral secrets; the implementation/spec erasure and lifecycle assumptions must be stated separately.

## 8. Evidence/claim boundary

This artifact advances property/attacker coverage by converting A3/FM-22 from a generic future label into an auditable experiment contract.

It establishes:

```text
A3 compromise timing vocabulary                    DEFINED
FM-22 compromise-object inventory                 DEFINED
PFS / PCS / recovery claim distinctions           DEFINED
cross-model shared-state audit surface            DEFINED
recovery-transition proof-entry criteria          DEFINED
first A3 model-entry checklist                     DEFINED
```

It does **not** establish:

```text
forward secrecy                                    NO
post-compromise security                           NO
key-compromise impersonation resistance            NO
secure erasure                                     NO
rollback-resistant storage                         NO
fresh replay-epoch recovery                        NO
authorization/revocation recovery                  NO
model/runtime equivalence                          NO
TD-001 independent cryptographic review            NO
TD-002 physical constrained-target measurements    NO
TD-003 closure                                     NO
TD-004 RFC-class specification                     NO
COMMON-CONFORMANT                                  NO
DEPLOYMENT-QUALIFIED                               NO
```

Symbolic compromise results, when eventually added, must remain separate from constant-time behavior, RNG quality, memory safety, secret zeroization, secure storage, physical extraction resistance, and computational soundness of the custom proof.

## 9. Dependency transfer

This readiness contract makes the remaining FM-22 blockers explicit rather than resolving them by invention.

The next substantive prerequisite is normative lifecycle ownership, especially:

```text
TD-004 / replay lifecycle
  authenticated fresh replay-epoch transition
  predecessor/new-context binding
  state-machine/error semantics

zk221 / resumption
  authorization revalidation
  ticket/PSK issue-use-reuse-invalidation rules

zk211–zk214
  authorization/revocation authority and generation semantics

rekey / reprovisioning
  authorized trust-mutation transition and dependent-state invalidation
```

Only after one such transition is concrete enough should FM-22 move from readiness analysis into a new symbolic theorem.

## 10. Research provenance

This packet consumes `docs/research/daily/2026-08-29.md`, especially D20260829-F02, which recommends an explicit dynamic-corruption timing matrix and composition/disjointness audit before FS/PCS expansion. The cited USENIX Security 2026 compositional-analysis work is treated as methodology guidance only; ZK-ARCHE does not claim to satisfy that paper's assumptions or inherit its results.
