# ZK-ARCHE Formal Model and Traceability Contract

This document defines the evidence contract for formal analysis of ZK-ARCHE. It advances TD-003 by making the property set, attacker assumptions, model ownership, and model-to-spec-to-implementation mapping explicit. It does **not** claim that the listed properties are proven.

## 1. Status and authority

Current model inputs:

- `rust/models/proverif/zk_arche_auth_skeleton.pv`
- `c/models/proverif/zk_arche_auth_skeleton.pv`

At the time this contract was introduced, those two files are byte-identical and resolve to the same Git blob. They are still duplicated paths, so duplication must not be treated as two independent formal implementations.

Current assurance state:

```text
MODEL SKELETON PRESENT
+ PROPERTY/ATTACKER CONTRACT DEFINED HERE
+ TRACEABILITY CONTRACT DEFINED HERE
!= FORMALLY ANALYZED
!= FORMALLY VERIFIED
!= CRYPTOGRAPHICALLY PROVEN
```

The custom Schnorr/role-membership proof remains subject to TD-001 independent cryptographic review. Symbolic verification cannot establish computational soundness of that proof, constant-time behavior, RNG quality, memory safety, side-channel resistance, or secure key storage.

## 2. Model ownership rule

ZK-ARCHE should converge on **one canonical symbolic state model** or one mechanically generated source from which tool-specific models are derived.

Until that migration is complete:

1. the Rust and C ProVerif skeletons MUST remain byte-identical when both paths claim to represent the same abstract protocol;
2. a semantic change to one model copy MUST be mirrored in the other in the same reviewed change;
3. a run against only one duplicate copy is evidence about that model text, not independent Rust/C verification;
4. model results MUST be tied to the exact model blob/commit and tool version;
5. no implementation claim may rely on a model behavior that is absent from the concrete Rust/C path being cited.

Preferred future shape:

```text
canonical abstract model / state machine
        |
        +--> ProVerif model
        +--> Tamarin model (when warranted)
        +--> executable conformance scenarios
        +--> traceability manifest
```

The canonical source must remain implementation-neutral. Rust and C are implementation targets, not separate sources of protocol truth.

## 3. Property and attacker matrix

Every formal-analysis run must state which rows below are actually modeled. An unmodeled row remains an evidence gap.

| ID | Property | Required claim | Minimum attacker capability | Current skeleton coverage |
|---|---|---|---|---|
| FM-01 | Session-key secrecy | Network attacker does not learn an accepted session key absent modeled compromise | Full active Dolev-Yao network control | Partial: abstract secrecy query exists |
| FM-02 | Client-to-server agreement | Server completion implies a matching authenticated client run for the bound context | Replay, injection, modification, interleaving | Partial: correspondence query exists |
| FM-03 | Server-to-client agreement | Client completion implies a matching authenticated server run for the bound context | Replay, injection, modification, interleaving | Partial: correspondence query exists |
| FM-04 | Injective agreement / replay resistance | One accepted completion cannot be justified by replaying a prior completed run unless explicit idempotent retransmission semantics permit it | Capture and arbitrary replay/reordering | Missing |
| FM-05 | Transcript/context integrity | Security-relevant version, suite/profile, identities/commitments, nonces, ephemeral keys, role/policy, deployment/audience, and extension choices cannot be changed without failure | Active transcript mutation | Partial: skeleton binds only a subset |
| FM-06 | Unknown-key-share resistance | Peers cannot complete while disagreeing about peer identity/commitment or security context | Identity substitution and session splicing | Missing/insufficient |
| FM-07 | Reflection resistance | Messages from one protocol direction cannot satisfy the opposite direction | Reflection and cross-role replay | Partial via distinct finished labels; needs explicit property |
| FM-08 | Downgrade resistance | A peer cannot be induced to accept semantics below the authenticated mandatory profile/floor | Capability stripping and negotiation modification | Missing |
| FM-09 | NO-LEARNING AUTH | Successful AUTH cannot create or expand trust state | Active attacker plus unknown/untrusted peer | Missing; trust state not modeled |
| FM-10 | Authentication/authorization separation | Possession proof alone does not imply authorization outside the bound scope/policy | Valid credential used in wrong audience/scope | Missing |
| FM-11 | Non-transitive trust | `A trusts B` and `B trusts C` do not authorize C at A without explicit bounded delegation accepted by A | Malicious/intermediate delegate | Missing |
| FM-12 | Delegation bounds | Delegation cannot exceed issuer scope, role, audience, depth, validity, or epoch | Delegate attempts privilege amplification | Missing |
| FM-13 | Revocation freshness semantics | Current local revocation/epoch state prevents acceptance of known-revoked lineage; stale-state behavior matches profile policy | Delayed/dropped revocation propagation, rollback | Missing |
| FM-14 | Resumption authorization preservation | Resumption cannot carry stale privilege across changed role/policy/audience/deployment/lineage/revocation context | Replay of valid ticket/PSK and context drift | Missing |
| FM-15 | Resumption reuse bounds | A bounded-use resumption credential cannot be accepted beyond modeled lifetime/use policy | Replay and repeated presentation | Missing |
| FM-16 | P2P role symmetry | Either initiator direction achieves equivalent mandatory authentication assurance | Active attacker, reversed initiator/responder roles | Missing |
| FM-17 | Infrastructure independence | Already-authorized peers can authenticate without an online CA/cloud/registry/gateway authority in the modeled core path | External service unavailable or adversarially unreachable | Missing as explicit property |
| FM-18 | Credential/reference binding | Lookup/reference identifiers cannot substitute for cryptographic identity unless bound to the intended key/commitment and scope | Reference collision/substitution | Missing |
| FM-19 | Privacy: role confidentiality | Observable protocol behavior does not disclose the exact role beyond the modeled allowed-set claim | Passive/active observer as declared | Not established; abstract `role_proof` is idealized |
| FM-20 | Privacy: unlinkability | Two allowed runs are not linkable through modeled protocol outputs beyond declared unavoidable metadata | Passive/active observer as declared | Missing |
| FM-21 | Failure-observability privacy | Error/no-response/size/retry behavior does not distinguish protected states beyond the declared policy | Adaptive active probing | Missing |
| FM-22 | Compromise recovery boundaries | Results clearly state which guarantees survive compromise of long-term keys, session state, issuer/delegation authority, or replay state | Explicit key/state compromise | Missing |

## 4. Attacker profiles

Formal results must identify one or more named attacker profiles instead of using an unspecified "network attacker".

### A0 — active network attacker

May read, drop, delay, reorder, replay, modify, and inject messages; initiate concurrent sessions; and operate malicious peers. Does not initially know uncompromised long-term secrets.

This is the minimum profile for authentication, agreement, replay, transcript-binding, downgrade, and UKS claims.

### A1 — authorized-but-malicious peer

Possesses a valid credential/trust relationship but attempts to exceed role, audience, deployment, operation, delegation, or validity bounds.

Required for authorization, delegation, non-transitivity, and privilege-amplification claims.

### A2 — stale/offline peer context

Can delay or suppress synchronization/revocation information and can replay older locally valid state. It does not magically forge a newer signed/authorized state.

Required for revocation convergence, stale-authorization windows, rollback, and offline P2P claims.

### A3 — selective compromise

The model explicitly reveals one of:

```text
peer long-term secret
session/resumption secret
issuer/delegation authority secret
persistent replay state
cached authorization state
```

Results under A3 must state exactly which guarantees are expected to fail and which should remain for uncompromised peers/sessions. "Compromised" must never be a single undifferentiated state.

### A4 — infrastructure loss

CA/cloud/central registry/DNS/gateway/manufacturer service is unavailable. For already-authorized peers with sufficiently fresh local state, core P2P AUTH is expected to remain executable. This profile tests architecture dependency; it is not a cryptographic compromise model.

### A5 — privacy observer

Define separately whether the observer is passive or active and which lower-layer metadata is visible. Role privacy, unlinkability, and failure-observability claims are invalid without this explicit observation boundary.

## 5. Concrete model gaps in the current skeleton

The current ProVerif skeleton is intentionally useful only as a starting point. Before stronger claims, it must stop idealizing or omitting the following security-relevant semantics:

- protocol version and selected method/suite/profile;
- capability and critical-extension negotiation;
- explicit session/sequence identifiers and retransmission/replay state;
- complete transcript field ordering and domain separation;
- explicit server possession/authentication rather than only a finished MAC derived from ephemeral DH;
- local trusted-record lookup and NO-LEARNING behavior;
- authentication versus authorization decision events;
- deployment/domain/audience and role/policy scope;
- authorization lineage/generation and revocation epoch;
- explicit delegation and non-transitive trust;
- rekey/re-registration lineage replacement;
- resumption credentials, use/lifetime bounds, and full-AUTH fallback;
- infrastructure-independent P2P initiator/responder symmetry;
- observable error/retry behavior needed for privacy claims;
- compromise events and post-compromise claim boundaries.

The abstract functions `schnorr_proof` and `role_proof` must remain labeled as idealized proof interfaces unless/until the computational proof/review work for TD-001 justifies a stronger abstraction.

## 6. Model-to-spec-to-code traceability map

This table is a **mapping obligation**, not a proof result. Each row must eventually identify exact spec sections and concrete Rust/C symbols. Path-level mappings below reflect current repository structure and may be refined as the normative spec matures.

| Model concept | Normative/spec owner | Rust implementation surface | C implementation surface | Evidence status |
|---|---|---|---|---|
| framing / message type / sequence | `spec/zk-arche-protocol.md`, `spec/registries.md`, `rust/wire-spec.md` | `rust/crates/proto/src/wire.rs` | `c/src/wire/**`, `c/include/auth/wire.h` | implementation exists; normative root spec incomplete |
| capability/profile negotiation | `spec/iot-profiles.md`, `spec/registries.md` | `rust/crates/proto/src/caps.rs`, `profile.rs` | protocol headers/source under `c/include/auth/**`, `c/src/proto/**` | model missing |
| transcript construction | protocol spec + future exact transcript section | `rust/crates/proto/src/transcript.rs` | `c/include/auth/transcript.h`, corresponding C source | skeleton partial |
| cryptographic primitives / KDF / MAC | protocol spec + security considerations | `rust/crates/proto/src/crypto.rs` | `c/include/auth/crypto.h`, `c/src/crypto/**` | abstracted in model |
| AUTH state machine | `spec/zk-arche-protocol.md` | `rust/crates/proto/src/proto/**`, client/server crates | `c/src/proto/**`, `c/bin/client.c`, `c/bin/server.c` | skeleton only |
| trusted records / lookup | TRUST/ENROLL normative sections pending | `rust/crates/proto/src/store/**` | `c/src/store/**`, `c/include/auth/store.h` | model missing |
| replay / retry / retransmission | LINK/AUTH normative sections pending | protocol/transport/state code as implemented | protocol/transport/state code as implemented | model missing |
| authorization scope/policy | TRUST/AUTH normative sections pending | concrete symbol mapping pending audit | concrete symbol mapping pending audit | model missing |
| rekey/revocation/lineage | TRUST/ENROLL normative sections pending | implementation mapping pending | implementation mapping pending | model missing |
| resumption | LINK normative sections pending | implementation mapping pending | implementation mapping pending | model missing |
| transport/channel binding | `spec/zk-arche-protocol.md` + BIND work | `rust/crates/proto/src/transport/**` | `c/src/transport/**`, `c/include/auth/transport.h` | model missing |
| deterministic conformance vectors | `spec/test-vectors.md` | `rust/test-vectors/0x0001/**` | C vector harness consumes Rust corpus | executable evidence exists; model trace pending |

A model property is not implementation-traceable until its row contains:

```text
model event/query/lemma
+ normative spec section/field/state transition
+ exact Rust symbol(s)
+ exact C symbol(s)
+ positive/negative vector or executable test when representable
+ retained formal-tool result tied to commit/tool version
```

## 7. Event vocabulary required for the next model revision

The next canonical model should introduce an event vocabulary that separates authentication from authorization and trust mutation. Exact syntax is tool-specific, but the semantic events should include equivalents of:

```text
AuthBegin(peer_a, peer_b, security_context, session)
AuthPeerVerified(verifier, peer, key_or_commitment, session)
AuthComplete(peer_a, peer_b, security_context, session)
AuthorizationChecked(verifier, holder, scope, audience, epoch, decision, session)
TrustMutationAuthorized(authority, operation, target, lineage, epoch)
TrustMutationApplied(operation, target, lineage, epoch)
RevocationObserved(peer, issuer, epoch, lineage)
ResumptionIssued(peer_a, peer_b, authz_context, expiry, use_policy)
ResumptionAccepted(peer_a, peer_b, authz_context, current_epoch)
DelegationAccepted(verifier, issuer, holder, scope, depth, epoch)
```

The model MUST NOT encode `AuthComplete` as an implicit trust-store insertion. A successful possession proof for an unknown peer remains insufficient for authorization unless explicit trusted state or an authorized trust-mutation flow provides that state.

## 8. Required traceability scenarios

At minimum, future formal/conformance work should share scenario identifiers for:

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
FT-017 transport address changes while cryptographic peer identity remains stable
FT-018 credential/reference substitution
FT-019 privacy-equivalent protected failure classes
```

Where a scenario can be represented as deterministic bytes/state, it should receive Rust/C positive or negative conformance coverage in addition to formal analysis.

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
  - explicit abstraction/compromise assumptions
results:
  FM-XX: proved | counterexample | inconclusive | not-modeled
counterexample_artifacts:
  - path or retained transcript when applicable
traceability_revision: document/model mapping revision
limitations:
  - what the result does not establish
```

A clean tool exit alone is not sufficient evidence. The result must identify the queries/lemmas actually evaluated and preserve counterexamples when the tool produces them.

## 10. Validation and promotion gates

TD-003 remains **open** after this document because no new ProVerif/Tamarin theorem result is produced here.

Progression should be reported as:

```text
CONTRACT DEFINED
  property/attacker/traceability obligations exist

MODEL EXPANDED
  required protocol/trust/lifecycle semantics represented

FORMALLY ANALYZED
  scoped queries/lemmas run with retained results and limitations

IMPLEMENTATION-TRACEABLE
  model events/states map to normative spec + exact Rust/C symbols + tests
```

`FORMALLY ANALYZED` does not mean `EXTERNALLY REVIEWED`, `CONSTANT-TIME`, `MEMORY-SAFE`, `FIELD-READY`, or `RFC-CLASS DOCUMENTED`.

## 11. Next dependency-ready packet

The next TD-003 packet should replace the current minimal correspondence queries with explicit session/context events and add at least FM-04 (injective/replay agreement), FM-05 (complete authenticated security context), and FM-09 (NO-LEARNING AUTH) while preserving the idealized-proof limitation. Because the Rust and C skeleton paths currently duplicate the same model, that semantic edit should update both copies together or first establish a single canonical generated model source.
