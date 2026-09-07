# Authenticated Revocation Update Ingestion

Status: normative ingestion contract for zk214; transport-neutral and implementation-pending.

## 1. Scope

This contract defines when a structurally valid revocation view may become authoritative local revocation state. It composes with `revocation-view-representation.md` and `revocation-convergence-and-stale-authorization.md` and does not define a transport, cloud service, central registry, or always-online dependency.

Authentication, authorization, and trust mutation remain distinct. Successful AUTH does not authorize a revocation-state mutation. Revocation ingestion is an explicit trust/lifecycle mutation performed only by locally configured authority logic.

## 2. Required authenticated inputs

Before an incoming FULL or DIFF view can update local state, the implementation MUST establish all of the following from authenticated evidence or authenticated local metadata:

```text
format_version_supported
authority_identity_bound
authority_scope_bound
deployment_bound
profile_bound
update_integrity_valid
update_authenticity_valid
view_epoch_monotonic
base_epoch_exact_when_diff
issued_or_observed_time_acceptable
rollback_not_suspected
```

The authenticated authority identity MUST map to a locally accepted revocation authority for the exact authority scope. Transport address, DNS/SNI, gateway identity, cloud origin, successful TLS connection, successful ZK-ARCHE AUTH by an unrelated peer, or an unsigned caller-supplied authority identifier MUST NOT substitute for this mapping.

`profile_bound` means that the update is acceptable under the local profile governing revocation representation, freshness, and algorithm/suite requirements. Negotiation or a higher-capability sender MUST NOT downgrade the local minimum revocation security policy.

## 3. FULL update admission

A FULL view may replace the current authority-scoped local view only when:

1. the update passes all authenticated-input checks;
2. its authority and deployment scope exactly match the destination state partition;
3. its epoch is not lower than the locally required minimum epoch;
4. its epoch is strictly newer than the incorporated local epoch, except that an exact-byte duplicate MAY be treated as an idempotent no-op;
5. rollback suspicion is absent.

A same-epoch update with different authenticated content MUST fail closed as a conflict. It MUST NOT be resolved by arrival order.

## 4. DIFF update admission

A DIFF view may be incorporated only when:

1. all authenticated-input checks pass;
2. `base_epoch` exactly equals the currently incorporated epoch for the same authority scope;
3. the resulting epoch is strictly greater than `base_epoch`;
4. every entry is structurally valid under the active representation version;
5. the update does not imply an unrevocation or trust repair unless a future version explicitly defines and authenticates such a transition.

If a DIFF skips the current base epoch, the receiver MUST NOT guess missing updates or apply the DIFF speculatively. It must request or obtain a FULL view or an authenticated chain of updates sufficient to close the gap.

## 5. Bounded-resource admission

Revocation ingestion is part of the constrained Common Contract and therefore MUST have an explicit local resource envelope before an update is admitted for authoritative processing.

Every selectable profile that supports revocation ingestion MUST define at least:

```text
max_entries_per_view
max_update_payload_bytes   (once a concrete wire encoding exists)
max_reconciliation_scratch_bytes
```

Until a concrete revocation-view wire encoding exists, `max_update_payload_bytes` is an implementation/evidence bound rather than a wire-format constant. Implementations MUST NOT infer a protocol-wide byte limit from this document.

An implementation MUST reject an update before incorporation when any applicable local bound would be exceeded. A higher-capability sender, commissioner, cache, gateway, transport, or negotiated optional feature MUST NOT cause a constrained verifier to allocate beyond its selected profile bound. High-end peers adapt downward; the resource envelope does not authorize weaker revocation semantics.

Exceeding a resource bound MUST NOT partially apply a FULL or DIFF, truncate entries, discard unknown suffix data and continue, split one authoritative update into unauthenticated sub-updates, or silently reinterpret an oversized FULL as a partial snapshot.

A resource-limit rejection MUST leave the previously accepted authority-scoped state unchanged. Resource exhaustion, allocator failure, or storage-capacity failure during staging MUST be treated equivalently to an admission failure unless the implementation has already atomically committed a complete authenticated state according to Section 8.

## 6. Fail-closed result classes and deterministic precedence

An implementation claiming this contract MUST distinguish at least these local ingestion outcomes:

```text
INCORPORATE
IDEMPOTENT
REJECT_RESOURCE_LIMIT
REJECT_UNSUPPORTED_VERSION
REJECT_UNAUTHENTICATED
REJECT_WRONG_AUTHORITY
REJECT_WRONG_SCOPE
REJECT_WRONG_DEPLOYMENT
REJECT_PROFILE_MISMATCH
REJECT_ROLLBACK_OR_OLD_EPOCH
REJECT_BASE_EPOCH_MISMATCH
REJECT_SAME_EPOCH_CONFLICT
REJECT_TIME_POLICY
REJECT_MALFORMED
REJECT_ROLLBACK_SUSPECTED
```

For deterministic Rust/C qualification, overlapping faults MUST be classified in this precedence order:

1. `REJECT_RESOURCE_LIMIT` when the object cannot be processed within the selected profile's pre-authoritative resource envelope;
2. `REJECT_UNSUPPORTED_VERSION` for a recognized framing/container carrying an unsupported revocation-view version;
3. `REJECT_MALFORMED` for structurally invalid data under a supported version;
4. `REJECT_UNAUTHENTICATED` when integrity/authenticity evidence is absent or invalid;
5. `REJECT_WRONG_AUTHORITY`, then `REJECT_WRONG_SCOPE`, then `REJECT_WRONG_DEPLOYMENT`, then `REJECT_PROFILE_MISMATCH`;
6. `REJECT_ROLLBACK_SUSPECTED`, then `REJECT_TIME_POLICY`;
7. `REJECT_ROLLBACK_OR_OLD_EPOCH`, `REJECT_BASE_EPOCH_MISMATCH`, or `REJECT_SAME_EPOCH_CONFLICT` according to the authoritative epoch/reconciliation fault;
8. `IDEMPOTENT` only for an already-incorporated authenticated identical object;
9. `INCORPORATE` only when every required admission condition succeeds.

This precedence defines local conformance decisions, not a requirement to reveal detailed rejection reasons to an unauthenticated sender. A transport or wire-error mapping MAY intentionally collapse multiple local rejection classes to reduce oracle surface, but it MUST NOT alter the underlying admission result or permit a rejected update to mutate state.

A reject result MUST leave the previously accepted authority-scoped revocation view unchanged. Parsing success, network success, retry, resumption, cached authorization, or commissioner reachability MUST NOT turn a reject into incorporation.

## 7. Non-downgradable reconciliation semantics

Resource pressure, stale connectivity, peer asymmetry, or missing infrastructure MUST NOT weaken FULL/DIFF reconciliation rules.

In particular:

- a DIFF whose `base_epoch` does not exactly equal the incorporated epoch MUST be rejected rather than rebased locally;
- an older FULL MUST NOT replace a newer accepted view;
- a same-epoch conflicting authenticated object MUST be rejected rather than resolved by arrival order;
- an unauthenticated update MUST NOT be cached as authoritative pending later validation;
- an oversized update MUST NOT be treated as a partial authoritative view;
- absence of a CA, cloud, DNS, gateway, or central registry MUST NOT change the local accept/reject semantics when sufficient local authority state exists.

These requirements preserve identical security semantics across constrained and higher-capability peers. A constrained peer may support a smaller bounded view, but not a weaker interpretation of epochs, authority, authenticity, or revocation actions under the same profile.

## 8. Atomicity and persistence boundary

An accepted update MUST become visible to authorization decisions atomically with its new incorporated epoch. Implementations MUST NOT expose the new epoch while retaining only part of its revocation set, nor expose new entries while reporting the previous epoch.

Crash/restart behavior must preserve either the complete prior accepted view or the complete newly accepted view. Partial-write recovery, anti-rollback storage, and target-specific durability are implementation/evidence requirements and are not claimed complete by this specification.

Staging an update for authentication/reconciliation MUST NOT make it authoritative. Any temporary representation used before commit MUST be discarded or recoverably marked non-authoritative after a rejection or interrupted staging operation.

## 9. Dependent-state invalidation

After incorporation, any newly revoked holder, stale lineage, or advanced authority epoch MUST be propagated to the existing lifecycle authorities before affected protected operations are authorized again. At minimum, affected authorization caches, retained associations, resumption state, delegation, and DATA-release authority must be re-evaluated under the current lifecycle contracts.

Revocation ingestion MUST NOT create a second authorization-generation, replay, session, or DATA authority. It updates the authoritative revocation view; owning modules consume that result through their existing fail-closed lifecycle decisions.

## 10. Offline and infrastructure-independent operation

A conformant peer MAY ingest authenticated revocation state from any transport or synchronization mechanism. No particular CA, cloud service, DNS service, gateway, blockchain, registry server, or Internet connection is mandatory.

Two already-authorized peers may continue mandatory-core authentication without external infrastructure when their local state is sufficient. Authorization remains subject to the profile freshness bound in `revocation-convergence-and-stale-authorization.md`; inability to obtain a sufficiently fresh view never makes stale authority current.

## 11. Qualification requirements

Future Rust/C qualification for this contract MUST use a shared deterministic corpus covering at least:

- valid newer FULL incorporation;
- exact duplicate FULL idempotence;
- same-epoch conflicting FULL rejection;
- old-epoch/rollback FULL rejection;
- valid DIFF on exact base epoch;
- DIFF base-epoch mismatch;
- skipped-update DIFF rejection;
- wrong authority and wrong deployment;
- unauthenticated or integrity-invalid update;
- unsupported representation version;
- profile mismatch/downgrade attempt;
- malformed entry;
- rollback suspicion;
- failed ingestion leaves previous state unchanged;
- newly incorporated revocation forces dependent-state re-evaluation;
- entry-count or applicable byte/scratch bound exceeded;
- overlapping-fault cases that prove the Section 6 precedence order;
- oversized FULL/DIFF rejection leaves the previous view unchanged;
- constrained and higher-capability peers produce the same semantic decision when both operate under the same selected profile bounds.

Where both Rust and C claim support, they MUST produce the same admission decision and resulting normalized authority-scoped state for the corpus. Restart/partial-write qualification requires persistent-state tests rather than decision-only vectors.

The deterministic reconciliation corpus at `rust/test-vectors/state/revocation-reconciliation-v1.txt` and its repository-owned checker are qualification scaffolding for epoch/authority/reconciliation behavior. They do not by themselves establish Rust/C ingestion parity, physical constrained-target behavior, or wire-format interoperability.

## 12. Evidence boundary

This file closes specification ambiguity around authenticated ingestion, bounded-resource admission, deterministic rejection precedence, and non-downgradable reconciliation semantics. It does not itself establish IMPLEMENTED, TESTED, INTEROPERABLE, FORMALLY ANALYZED, MEASURED, EXTERNALLY REVIEWED, COMMON-CONFORMANT, or DEPLOYMENT-QUALIFIED status.

zk214 remains open until shared Rust/C ingestion/reconciliation behavior, persistent restart/rollback evidence, disconnected-peer convergence tests, dependent-state invalidation evidence, and the other declared roadmap exits exist. The representation namespace and change-control rules remain owned by `revocation-view-representation.md`; this contract MUST NOT silently renumber or redefine them.
