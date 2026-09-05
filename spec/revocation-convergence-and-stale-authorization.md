# Revocation convergence and stale-authorization contract

Status: bounded zk214 lifecycle contract. This document defines fail-closed authorization freshness semantics for offline-capable peers. It does not claim instantaneous global revocation, an implemented distribution protocol, physical rollback resistance, or deployment qualification.

## 1. Scope

Authentication, authorization, and trust mutation remain distinct. A cryptographically valid AUTH result is not sufficient to authorize an operation when the local authorization/revocation view is outside the selected profile's permitted freshness bound.

Revocation is modeled as convergence of issuer/authority-scoped state, not as a local delete event. Always-online infrastructure MUST NOT be required for the root authentication decision between already-authorized peers, but a peer that cannot establish authorization freshness within the selected profile's bound MUST fail closed or enter an explicitly narrower safe authorization state defined by that profile.

## 2. Normalized local inputs

A conformant authorization-freshness evaluator consumes at least:

```text
current_time
view_observed_time
max_staleness
view_integrity_valid
view_epoch
required_min_epoch
holder_revoked
lineage_current
scope_authorized
```

`max_staleness` is profile policy, not a network timeout. `required_min_epoch` is the minimum authority epoch already known locally to be required for the decision. Transport address, DNS state, cloud reachability, or gateway reachability MUST NOT substitute for authority epoch/freshness evidence.

The versioned logical representation that produces the authority-scoped `view_epoch`, observation time, and revocation entries is defined by [`revocation-view-representation.md`](./revocation-view-representation.md). Representation validity and authenticity remain separate: a structurally valid view is not authoritative until the future authenticated-ingestion layer establishes provenance and integrity.

## 3. Decision classes

The evaluator MUST produce one of:

```text
AUTHORIZED
REVOKED
STALE_VIEW
ROLLBACK_OR_OLD_EPOCH
INVALID_VIEW
STALE_LINEAGE
SCOPE_DENIED
```

Decision precedence is:

1. invalid integrity -> `INVALID_VIEW`;
2. `view_epoch < required_min_epoch` -> `ROLLBACK_OR_OLD_EPOCH`;
3. local view older than `max_staleness` -> `STALE_VIEW`;
4. explicit holder revocation -> `REVOKED`;
5. non-current authorization lineage -> `STALE_LINEAGE`;
6. scope/policy denial -> `SCOPE_DENIED`;
7. only otherwise -> `AUTHORIZED`.

A non-`AUTHORIZED` decision MUST NOT be silently converted to authorization by successful AUTH, cached session state, transport continuity, retry, or resumption.

## 4. Offline and convergence semantics

Offline operation is permitted while the local revocation/authorization view remains within the profile's declared freshness bound and all other decision inputs remain valid. The bound MUST be explicit and testable; profiles MUST NOT use terms such as "reasonably fresh" without a numeric or otherwise deterministic rule.

If a peer remains disconnected beyond the bound, inability to refresh does not make old authorization current. The peer MUST fail closed for operations requiring current authorization or restrict itself to a separately specified safe subset. This contract does not define such a subset; absent explicit profile text, the required behavior is fail closed.

Full and differential update transport/distribution mechanisms remain future work, but their normalized composition semantics are now constrained by `revocation-view-representation.md`: authority scope must match, epochs are monotonic, a DIFF applies only to its exact incorporated `base_epoch`, missed-update recovery cannot guess across gaps, and version-1 DIFFs cannot express implicit unrevocation or trust repair.

Whatever ingestion mechanism is selected MUST preserve issuer/authority scope, monotonic epoch semantics, missed-update recovery, rollback detection, and the distinction between structural validity and authenticated authority provenance.

## 5. Dependent-state invalidation

When local state establishes that a holder/lineage is revoked or stale, dependent authorization state MUST NOT outlive that knowledge. Future implementations must invalidate or refuse reuse of affected:

- active authorization caches;
- resumption credentials/tickets/PSKs;
- derived association keys where authorization validity is a prerequisite;
- delegated authorization derived from the revoked lineage;
- DATA release authority bound to the revoked holder/lineage/epoch.

This requirement does not claim those dependent mechanisms are fully implemented today.

## 6. Common Contract boundaries

Core AUTH remains NO-LEARNING. Revocation processing cannot create trust. A commissioner, cloud service, registry server, gateway, CA, DNS service, or Internet connection may distribute fresher state, but none becomes mandatory authority for an already-authorized peer's cryptographic identity decision.

Local trust remains non-transitive. Delegation remains explicit, scoped, bounded, revocable, and subject to the same freshness/epoch decision before it can make a holder eligible for authorization.

## 7. Conformance corpora

`rust/test-vectors/state/revocation-freshness-v1.txt` is the canonical decision corpus for the bounded freshness contract. It covers current authorization, explicit revocation, stale local views, rollback/old epochs, invalid view integrity, stale lineage, scope denial, and an offline-but-within-bound case.

`rust/test-vectors/state/revocation-view-v1.txt` is the canonical structural/composition corpus for the version-1 normalized revocation-view representation. It covers full/differential forms plus unknown versions, invalid epochs/base epochs, future-effective revocations, unknown actions, and duplicate entries.

These corpora are specification/qualification evidence. They MUST NOT be described as runtime Rust/C interoperability until both implementations consume the same cases and retained exact-head execution evidence exists.

## 8. Evidence still required

This contract does not close zk214. The versioned revocation-view representation is now defined, but remaining evidence includes authenticated update ingestion, Rust/C shared parser/evaluator semantics for the representation and freshness decision, executable full/differential reconciliation, restart/rollback persistence, session/resumption/delegation invalidation, disconnected-peer convergence tests, target-specific storage evidence where required, formal lifecycle coverage, and external review where applicable.
