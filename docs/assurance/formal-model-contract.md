# ZK-ARCHE Formal Model and Traceability Contract

This document defines the evidence contract for formal analysis of ZK-ARCHE. It advances TD-003 by making the property set, attacker assumptions, model ownership, model-to-spec-to-code mapping, and formal/runtime assumption boundaries explicit. It does **not** claim that the listed properties are proven.

## 1. Status and authority

Current synchronized model inputs:

- `rust/models/proverif/zk_arche_auth_skeleton.pv`
- `c/models/proverif/zk_arche_auth_skeleton.pv`

The two paths are synchronized copies of one abstract model, not independent formal implementations. `scripts/sync-formal-models.sh` is the repository synchronization check.

Current assurance state:

```text
PROPERTY/ATTACKER CONTRACT DEFINED
+ SYNCHRONIZED AUTH MODEL PRESENT
+ FM-04/FM-05/FM-09 QUERIES PRESENT
+ INITIAL MODEL→CODE TRACEABILITY PRESENT
!= FORMALLY ANALYZED
!= FORMALLY VERIFIED
!= CRYPTOGRAPHICALLY PROVEN
```

The custom Schnorr/role-membership proof remains subject to TD-001 independent cryptographic review. Symbolic verification cannot establish computational soundness of that proof, constant-time behavior, RNG quality, memory safety, side-channel resistance, secure key storage, or field readiness.

The 2026-08-26 daily research report is a controlling evidence refinement for replay semantics. It identified that the current ProVerif `replay_seen` table is persistent/unbounded while both concrete implementations use bounded volatile memory with different retention behavior. Therefore a positive formal result under the current replay abstraction would not, by itself, prove the runtime replay-lifetime property.

## 2. Model ownership rule

ZK-ARCHE should converge on **one canonical symbolic state model** or one mechanically generated source from which tool-specific models are derived.

Until that migration is complete:

1. the Rust and C ProVerif paths MUST remain byte-identical when both claim to represent the same abstract protocol;
2. a semantic change to one copy MUST be mirrored in the other in the same reviewed change;
3. a run against one synchronized copy is evidence about that model text, not independent Rust/C verification;
4. formal results MUST be tied to exact model blob, repository commit, tool name, and tool version;
5. no implementation claim may rely on modeled behavior absent from the concrete Rust/C path cited by the claim;
6. any abstraction materially stronger than runtime behavior MUST be named as an assumption gap in the retained result.

Preferred future shape:

```text
canonical abstract state model
        |
        +--> ProVerif model
        +--> Tamarin model when warranted
        +--> executable conformance scenarios
        +--> traceability manifest
```

Rust and C are implementation targets, not independent sources of protocol truth.

## 3. Property and attacker matrix

Every formal-analysis run must state which rows are actually modeled. An unmodeled row remains an evidence gap. A modeled row without retained tool output is **modeled, not analyzed**.

| ID | Property | Required claim | Minimum attacker capability | Current model/evidence status |
|---|---|---|---|---|
| FM-01 | Session-key secrecy | Network attacker does not learn an accepted session key absent modeled compromise | Full active Dolev-Yao network control | Partial model; retained result still required |
| FM-02 | Client-to-server agreement | Server completion implies a matching authenticated client run for the bound context | Replay, injection, modification, interleaving | Partial correspondence present |
| FM-03 | Server-to-client agreement | Client completion implies a matching authenticated server run for the bound context | Replay, injection, modification, interleaving | Partial correspondence present |
| FM-04 | Injective agreement / replay resistance | One accepted AUTH run cannot be justified by replaying a prior accepted run unless explicit idempotent retransmission semantics permit it | Capture, replay, reordering, concurrent duplicate submission | **Modeled with accepted-message replay state; runtime-lifetime mismatch open** |
| FM-05 | Transcript/context integrity | Security-relevant version, suite/profile, identities/commitments, nonces, ephemeral keys, role/policy, deployment/audience, and extension choices cannot be changed without failure | Active transcript mutation | Partial: current KC context only; profile/capability/audience binding incomplete |
| FM-06 | Unknown-key-share resistance | Peers cannot complete while disagreeing about peer identity/commitment or security context | Identity substitution and session splicing | Missing/insufficient |
| FM-07 | Reflection resistance | Messages from one protocol direction cannot satisfy the opposite direction | Reflection and cross-role replay | Partial; explicit property still needed |
| FM-08 | Downgrade resistance | A peer cannot be induced to accept semantics below the authenticated mandatory profile/floor | Capability stripping and negotiation modification | Missing |
| FM-09 | NO-LEARNING AUTH | Successful AUTH cannot create or expand trust state | Active attacker plus unknown/untrusted peer | Modeled as completion relative to pre-existing trusted state; retained result required |
| FM-10 | Authentication/authorization separation | Possession proof alone does not imply authorization outside the bound scope/policy | Valid credential used in wrong audience/scope | Partial event exists; scope/audience model incomplete |
| FM-11 | Non-transitive trust | `A trusts B` and `B trusts C` do not authorize C at A without explicit bounded delegation accepted by A | Malicious/intermediate delegate | Missing |
| FM-12 | Delegation bounds | Delegation cannot exceed issuer scope, role, audience, depth, validity, or epoch | Delegate attempts privilege amplification | Missing |
| FM-13 | Revocation freshness semantics | Current local revocation/epoch state prevents acceptance of known-revoked lineage; stale-state behavior matches profile policy | Delayed/dropped revocation propagation, rollback | Missing |
| FM-14 | Resumption authorization preservation | Resumption cannot carry stale privilege across changed role/policy/audience/deployment/lineage/revocation context | Replay of valid ticket/PSK and context drift | Missing |
| FM-15 | Resumption reuse bounds | A bounded-use resumption credential cannot be accepted beyond modeled lifetime/use policy | Replay and repeated presentation | Missing |
| FM-16 | P2P role symmetry | Either initiator direction achieves equivalent mandatory authentication assurance | Active attacker, reversed initiator/responder roles | Missing |
| FM-17 | Infrastructure independence | Already-authorized peers can authenticate without online CA/cloud/registry/gateway authority in the core path | External service unavailable or unreachable | Missing as explicit property |
| FM-18 | Credential/reference binding | Lookup/reference identifiers cannot substitute for cryptographic identity unless bound to intended key/commitment and scope | Reference collision/substitution | Missing |
| FM-19 | Privacy: role confidentiality | Observable protocol behavior does not disclose exact role beyond modeled allowed-set claim | Declared passive/active observer | Not established; `role_proof` remains idealized |
| FM-20 | Privacy: unlinkability | Allowed runs are not linkable through modeled outputs beyond declared unavoidable metadata | Declared passive/active observer | Missing |
| FM-21 | Failure-observability privacy | Error/no-response/size/retry behavior does not distinguish protected states beyond policy | Adaptive active probing | Missing |
| FM-22 | Compromise recovery boundaries | Results state which guarantees survive compromise of keys/state/authority/replay state | Explicit key/state compromise | Missing |

## 4. Attacker profiles

### A0 — active network attacker

May read, drop, delay, reorder, replay, modify, inject, initiate concurrent sessions, and operate malicious peers. Does not initially know uncompromised long-term secrets.

### A1 — authorized-but-malicious peer

Possesses valid credential/trust state but attempts to exceed role, audience, deployment, operation, delegation, or validity bounds.

### A2 — stale/offline peer context

May delay or suppress synchronization/revocation information and replay older locally valid state. It cannot forge a newer signed/authorized state.

### A3 — selective compromise

The model explicitly reveals one of:

```text
peer long-term secret
session/resumption secret
issuer/delegation authority secret
persistent replay state
cached authorization state
```

Results must identify which guarantees are expected to fail and which remain for uncompromised peers/sessions.

### A4 — infrastructure loss

CA/cloud/central registry/DNS/gateway/manufacturer service is unavailable. Already-authorized peers with sufficiently fresh local state are expected to retain core P2P AUTH capability. This tests architecture dependency, not cryptographic compromise.

### A5 — privacy observer

A run must state whether the observer is passive or active and what lower-layer metadata is visible.

## 5. Current model coverage and known abstraction gaps

The synchronized ProVerif model now includes:

- explicit session identifiers;
- client and server possession-proof interfaces;
- pre-existing trusted-record state;
- explicit authorization-check event;
- AUTH_1/AUTH_2/AUTH_3 completion events;
- current key-confirmation context;
- persistent `replay_seen` state;
- `ReplayRecorded` and `ReplayRejected` events;
- FM-04 injective/correspondence queries;
- FM-05 current-context correspondence queries;
- FM-09 completion-to-pre-existing-trust query.

The following material gaps remain:

- protocol version and selected method/suite/profile are not fully modeled/bound;
- capability and critical-extension negotiation is incomplete;
- complete deployment/domain/audience and role/policy scope is incomplete;
- authorization lineage/generation and revocation epoch are missing;
- delegation and non-transitive trust are missing;
- rekey/re-registration lineage replacement is missing;
- resumption credential issue/use/invalidation is missing;
- P2P initiator/responder symmetry is missing;
- observable failure/retry behavior required for privacy is missing;
- compromise events and post-compromise boundaries are missing;
- `schnorr_proof` and `role_proof` remain idealized interfaces.

### 5.1 FM-04 replay abstraction gap

The model currently uses a persistent ProVerif table:

```text
table replay_seen(bitstring)
```

That models the semantic rule:

```text
accepted AUTH_1
  → replay key becomes remembered
  → later identical replay key is rejected
```

The current runtime implementations are weaker in **retention/lifetime**, even though they implement the same check-after-decode / record-after-success ordering:

| Lane | Runtime replay owner | Retention behavior | Restart behavior |
|---|---|---|---|
| Rust | `MemoryReplayCache` in `rust/crates/proto/src/store/fs.rs`, used by `ServerState.replay` in `rust/crates/server/src/main.rs` and `handle_auth_1` | bounded `HashSet`; arbitrary eviction at capacity; server capacity = `profile.max_cached_responses * 2` | volatile; reset on process restart |
| C | `auth_replay_cache_t` in `c/include/auth/replay.h` / `c/src/store/replay.c`, enforced through `auth_server_handle_auth1_guarded` in `c/src/proto/replay_guard.c` and shared server cache in `c/bin/server.c` | bounded fixed-capacity cache; default 64; replacement after capacity | volatile; reset on process restart |
| ProVerif | `replay_seen` table in synchronized model | persistent/unbounded abstraction | no restart transition modeled |

Therefore:

> **FM-04 may advance only as a scoped accepted-message replay property under an explicit replay-state assumption. It is not yet a proof of bounded-cache eviction behavior, restart continuity, rollback resistance, or Rust/C retention parity.**

The next formal evidence manifest MUST state whether replay state is assumed persistent/unbounded, bounded, or reset only by an authenticated replay-epoch transition.

The next executable conformance work MUST cover cache-pressure eviction and restart/state-loss behavior before Common Contract replay claims are promoted.

## 6. Model-to-spec-to-code traceability map

This table is a mapping obligation, not a proof result.

| Model concept / property | Normative/spec owner | Rust implementation surface | C implementation surface | Executable evidence | Status |
|---|---|---|---|---|---|
| framing / message type / sequence | `spec/zk-arche-protocol.md`, `spec/registries.md`, `rust/wire-spec.md` | `rust/crates/proto/src/wire.rs` | `c/src/wire/**`, `c/include/auth/wire.h` | parser/vector tests | implementation exists; root spec incomplete |
| capability/profile negotiation | `spec/iot-profiles.md`, `spec/registries.md` | `rust/crates/proto/src/caps.rs`, `profile.rs` | `c/include/auth/**`, `c/src/proto/**` | compatibility corpus pending | model incomplete |
| transcript construction / FM-05 | AUTH transcript normative section pending | `rust/crates/proto/src/transcript.rs`, AUTH handlers | `c/include/auth/transcript.h`, corresponding C source, AUTH handlers | deterministic AUTH vectors | current KC subset modeled; full security context incomplete |
| cryptographic primitives / KDF / MAC | protocol spec + security considerations | `rust/crates/proto/src/crypto.rs` | `c/include/auth/crypto.h`, `c/src/crypto/**` | deterministic crypto/protocol vectors | abstracted in model; TD-001 separate |
| AUTH state machine / FM-02/FM-03 | `spec/zk-arche-protocol.md` | `rust/crates/proto/src/proto/auth.rs`, `rust/crates/server/src/main.rs` | `c/src/proto/**`, `c/bin/client.c`, `c/bin/server.c` | Rust/C AUTH tests/vectors | partial model |
| trusted records / FM-09 | TRUST/ENROLL normative sections pending | `RegistryStore`, `FsRegistryStore`, `handle_auth_1` lookup path | registry lookup callback/store path used by `auth_server_handle_auth1` | unknown-peer / enrollment separation tests where present | pre-existing trust modeled; exact test mapping still incomplete |
| replay accepted-message state / FM-04 | LINK/AUTH replay section pending | `ReplayCache`; `MemoryReplayCache::{contains,insert}`; `replay_key`; `handle_auth_1`; `ServerState.replay`; `dispatch_packet` | `auth_replay_key`; `auth_replay_cache_contains`; `auth_replay_cache_insert`; `auth_server_handle_auth1_guarded`; shared replay cache in `c/bin/server.c` | `c/tests/test_replay.c`; `c/tests/test_replay_guard.c`; `c/tests/test_server_dispatch_replay.c`; Rust replay tests where present | modeled ordering + C production-path evidence; retention parity unresolved |
| session reservation / concurrent acceptance support | LINK/AUTH state-machine section pending | shared `ServerState` mutex + auth session map | `auth_session_table_t`, reservation/activation path in `c/bin/server.c` | `c/tests/test_session_table.c`; production dispatch fixture | C bounded state evidenced; cross-lane concurrency parity pending |
| retry / source validation / pre-auth DoS | AUTH_RETRY / BIND work pending | no promoted common-contract retry path yet | no promoted common-contract retry path yet | benchmark/prototype pending | model missing; owned by zk219/R-014 |
| authorization scope/policy / FM-10 | TRUST/AUTH normative sections pending | concrete symbol mapping pending deeper audit | concrete symbol mapping pending deeper audit | wrong-scope/audience negatives pending | model incomplete |
| rekey/revocation/lineage / FM-13 | TRUST/ENROLL normative sections pending | mapping pending | mapping pending | lifecycle negatives pending | missing |
| resumption / FM-14/FM-15 | LINK normative sections pending | mapping pending | mapping pending | resumption corpus pending | missing |
| transport/channel binding | BIND normative work | `rust/crates/proto/src/transport/**` | `c/src/transport/**`, `c/include/auth/transport.h` | transport interop tests | model missing |
| deterministic conformance vectors | `spec/test-vectors.md` | `rust/test-vectors/0x0001/**` | C harness consumes Rust corpus | cross-language vectors | executable evidence exists; formal traceability incomplete |

A formal property is not implementation-traceable until its row contains:

```text
model event/query/lemma
+ normative spec section/field/state transition
+ exact Rust symbol(s)
+ exact C symbol(s)
+ positive/negative vector or executable test when representable
+ retained formal-tool result tied to commit/tool version
+ explicit abstraction/lifetime assumptions
```

## 7. FM-04 event/query traceability

Current symbolic elements:

```text
replay_key(...)
replay_seen(...)
ReplayRecorded(...)
ReplayRejected(...)
ServerAuth1Accepted(...)
ClientAuth1Sent(...)
```

Current queries include:

- injective `ServerAuth1Accepted ==> ClientAuth1Sent`;
- `ServerAuth1Accepted ==> ReplayRecorded`.

Concrete mapping:

### Rust

- `rust/crates/proto/src/store/mod.rs`
  - `ReplayCache::{contains, insert}` contract.
- `rust/crates/proto/src/store/fs.rs`
  - `MemoryReplayCache`.
  - `MemoryReplayCache::contains`.
  - `MemoryReplayCache::insert`.
  - `replay_key` using domain `iot-auth/replay-key/v2`.
- `rust/crates/proto/src/proto/auth.rs`
  - `handle_auth_1` accepted AUTH_1 processing.
- `rust/crates/server/src/main.rs`
  - `ServerState.replay`.
  - `dispatch_packet` production AUTH dispatch.

### C

- `c/include/auth/replay.h` / `c/src/store/replay.c`
  - `auth_replay_cache_t`.
  - `auth_replay_key`.
  - `auth_replay_cache_contains`.
  - `auth_replay_cache_insert`.
- `c/src/proto/replay_guard.c`
  - `auth_server_handle_auth1_guarded` implements check → full AUTH_1 verification → record-on-success.
- `c/bin/server.c`
  - shared replay cache used by production UDP/TCP dispatch.
- `c/tests/test_replay.c`
  - replay-key determinism/field binding/bounded cache behavior.
- `c/tests/test_replay_guard.c`
  - duplicate rejection and malformed-AUTH non-poisoning.
- `c/tests/test_server_dispatch_replay.c`
  - production-dispatch accepted-once/replayed-rejected lifecycle.

Current evidence statement:

```text
FM-04 MODEL STATE: modeled
FM-04 TOOL RESULT: not retained / not claimed
FM-04 C EXECUTABLE EVIDENCE: present for accepted-once/replay-reject ordering
FM-04 RUST/C RETENTION PARITY: not established
FM-04 RESTART/ROLLBACK CONTINUITY: not established
FM-04 COMMON-CONTRACT CLAIM: not promotable yet
```

## 8. Required traceability scenarios

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

Where a scenario can be represented as deterministic bytes/state, it should receive Rust/C positive or negative conformance coverage in addition to formal analysis.

For FT-020 through FT-023, a result must record profile/cache capacity and replay-state lifetime. A test that passes only because the replay key remains in an unbounded or oversized test cache is not evidence for the constrained runtime profile.

## 9. Formal-run evidence manifest

Every retained formal run should record at least:

```yaml
tool: proverif | tamarin | other
tool_version: exact version
repository_commit: exact commit SHA
model_path: exact path
model_blob: exact blob/hash when practical
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

A clean tool exit alone is not sufficient evidence. The result must identify queries/lemmas actually evaluated and preserve counterexamples when produced.

## 10. Validation and promotion gates

TD-003 remains **open**. The synchronized model and this traceability update do not produce a new ProVerif/Tamarin theorem result.

Progression is reported as:

```text
CONTRACT DEFINED
  property/attacker/traceability obligations exist

MODEL EXPANDED
  AUTH replay/trust/current-context semantics represented

FORMALLY ANALYZED
  scoped queries/lemmas run with retained results + assumptions + limitations

IMPLEMENTATION-TRACEABLE
  model events/states map to normative spec + exact Rust/C symbols + executable evidence
```

FM-04 is now materially more implementation-traceable, but **not complete** because the normative replay lifetime/epoch contract, Rust/C bounded-retention parity, restart/rollback behavior, and retained formal-tool result are still missing.

`FORMALLY ANALYZED` does not imply `EXTERNALLY REVIEWED`, `CONSTANT-TIME`, `MEMORY-SAFE`, `COMMON-CONFORMANT`, `FIELD-READY`, or `RFC-CLASS DOCUMENTED`.

## 11. Next dependency-ready packets

The highest-priority next work for replay/formal fidelity is:

1. add deterministic Rust and C stateful negatives for FT-020/FT-021/FT-022 and compare accept/reject decisions at explicit profile capacities;
2. define a normative replay lifetime/epoch contract: retention, eviction, restart/state-loss behavior, rollback handling, and when a fresh authenticated context legally resets replay memory;
3. revise the symbolic replay abstraction to match the claim being tested, or explicitly scope formal results to persistent replay state;
4. retain exact ProVerif output with the replay-state assumption manifest above.

After that, the next TD-003/T​​D-004 seam is FM-05: bind and specify the complete authenticated security context, especially selected profile/capability/critical extensions and deployment/audience semantics, before claiming transcript-context completeness.
