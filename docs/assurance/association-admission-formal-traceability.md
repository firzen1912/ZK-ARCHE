# Association Admission Formal Traceability

This document binds the current secure-association admission classifier to the formal-assurance property inventory without claiming a new symbolic proof. It advances TD-003 by identifying exactly which current lifecycle decisions are implementation/test evidence, which existing formal properties they refine, and which semantics still require model expansion and a retained exact-model run.

## Scope

Canonical runtime/test surfaces:

- `rust/crates/proto/src/association_admission.rs`
- `c/src/proto/association_admission.c`
- `rust/test-vectors/state/association-admission-v4.txt`
- Rust and C consumers of the canonical v4 corpus

Formal authority remains the synchronized AUTH-v3 and replay-continuity ProVerif model pairs named by `docs/assurance/formal-model-contract.md`. This document is traceability evidence only. It does not promote the association classifier to `FORMALLY ANALYZED` and does not let a prior formal run inherit semantics added after its exact model blob.

## Runtime decision boundary

Association admission is a postcondition over decisions owned by AUTH/TRUST/LINK/BIND and lifecycle layers. It does not authenticate a peer, create trust, mutate trust, issue authorization, perform revocation synchronization, or establish rollback-resistant persistence.

The current fail-closed precedence is:

```text
rollback suspected
  > trust mutation requested
  > AUTH incomplete
  > pre-existing trust record missing
  > authorization missing
  > authorization stale
  > authorization generation unbound
  > authorization generation stale
  > revocation state stale
  > explicitly revoked
  > lineage stale
  > replay continuity stale
  > restart continuity stale
  > usage-counter continuity stale
  > required channel binding invalid
  > establish/retain association
```

The ordering is security-significant because a later healthy fact must not repair an earlier invalid authority/lifecycle fact. The v4 corpus includes compound negatives that make authorization-generation and revocation precedence falsifiable rather than testing each condition only in isolation.

## Property mapping

| Runtime fact / transition | Formal property owner | Current evidence state | Formal gap before promotion |
|---|---|---|---|
| `auth_complete` | FM-02/FM-03 authentication agreement | Existing scoped AUTH-v3 formal evidence + runtime classifier evidence | Association admission itself is not emitted as a formal event |
| `preexisting_trust_record` and `trust_mutation_requested` | FM-09 NO-LEARNING AUTH | Existing scoped formal evidence + runtime fail-closed evidence | Model does not yet represent the full classifier transition/precedence |
| `authorization_present`, `authorization_fresh` | FM-10 authentication/authorization separation | Runtime/spec evidence only | Authorization policy semantics and admission event must be modeled rather than idealized |
| `authorization_generation_bound/current` | FM-10, FM-13 | Runtime/vector evidence only | Model needs authority/provenance namespace plus generation binding/currentness |
| `revocation_current`, `explicitly_revoked` | FM-13 revocation freshness | Runtime/vector evidence only | Model needs stale/offline revocation state, explicit revocation and convergence bound |
| `lineage_current` | FM-10/FM-12/FM-13 | Runtime/vector evidence only | Delegation/lineage semantics remain blocked on complete normative ownership |
| `replay_continuity_current` | FM-04 replay/injective acceptance | Existing replay-continuity formal evidence + runtime evidence | Need composition between AUTH replay state and association admission |
| `restart_continuity_current` | FM-04/FM-14/FM-15/FM-22 | Runtime/vector evidence only | Model needs restart/state-loss and recovery transitions |
| `usage_counter_continuity_current` | FM-14/FM-15 | Runtime/vector evidence only | Model needs retained-key/ticket/PSK use bounds and invalidation semantics |
| `binding_required`, `binding_valid` | FM-05 transcript/security-context integrity; FM-14 | Partial existing AUTH-v3 formal evidence + runtime evidence | Model needs association-level channel/exporter-binding change and reauthentication behavior |
| `rollback_suspected` | FM-13/FM-22 | Runtime/vector evidence only | Model needs rollback/state-restoration attacker and recovery semantics |

## Required next formal packet

A future association-lifecycle model MUST NOT merely encode the classifier as an oracle and then prove the oracle returns its own expected result. It should expose attacker-controlled temporal transitions sufficient to falsify at least these claims:

1. successful association admission implies prior authenticated completion and a pre-existing local trust record;
2. successful AUTH alone cannot create or expand the trust record used for admission;
3. stale/unbound authorization generation prevents admission even when later revocation/lineage/replay facts appear healthy;
4. explicit revocation or stale revocation state prevents retained authority from surviving reauthentication, rebinding, or restart;
5. replay/restart/usage continuity loss cannot be repaired by possession of retained session material;
6. a required channel-binding change cannot silently preserve association authority without the specified reauthentication/rebinding transition;
7. rollback of persistent lifecycle state cannot yield an accepted association that would be rejected under the newer authoritative state.

At minimum the attacker model must compose A0 active-network behavior with A2 stale/offline state and an explicitly bounded subset of A3/A22-style state compromise/recovery behavior. If the runtime semantics required to state a theorem are not normative yet, the property remains `BLOCKED-NORMATIVE`; the formal model must not invent them.

## Traceability acceptance rule

A future retained result may promote an association-lifecycle row to `FORMALLY ANALYZED` only when all of the following are recorded together:

- exact repository commit;
- exact synchronized model blob(s);
- tool and version;
- query/property identifiers;
- attacker profile and compromise assumptions;
- mapping to the owning specification text;
- mapping to both Rust and C classifier surfaces where both claim support;
- mapping to the canonical association-admission corpus and relevant negative cases;
- named abstraction gaps, including persistence, RNG, constant-time, memory safety and computational proof boundaries.

A model edit invalidates inheritance of an older retained run for the edited semantics. Symbolic success cannot establish TD-001 independent review, TD-002 physical measurements, parser/runtime equivalence, rollback-resistant storage, or deployment qualification.

## Current claim

The association-admission lifecycle is **IMPLEMENTED and cross-language vector-governed** for the current classifier surface. Relevant AUTH/replay properties have scoped retained formal evidence from earlier exact model blobs. The complete association lifecycle is **not FORMALLY ANALYZED** because authorization generation, revocation convergence, restart/usage continuity, rollback/recovery and association-level binding composition are not yet represented together in a retained synchronized model.
