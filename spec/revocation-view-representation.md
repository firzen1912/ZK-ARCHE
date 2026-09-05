# Versioned Revocation View Representation

Status: wire-neutral normalized representation contract for zk214. This document defines the minimum logical state that revocation-freshness and reconciliation implementations must agree on. It does not define a transport, signature container, CBOR/CDDL encoding, update-distribution protocol, or physical persistence mechanism.

## 1. Purpose and authority boundary

Revocation is authority-scoped convergent state. A conformant implementation MUST distinguish the representation of a revocation view from the mechanism that authenticated, transported, stored, or refreshed it.

A revocation view MUST NOT create trust, authenticate a peer, or become a second authorization authority. It only represents state issued by an already-authoritative enrollment/authorization authority. Normal AUTH remains NO-LEARNING.

## 2. Version 1 normalized record

A version-1 normalized revocation view contains:

```text
format_version       = 1
authority_ref        = non-empty opaque authority identifier
view_epoch           = monotonically increasing authority-scoped epoch
observed_time        = local time at which this authenticated view became usable
update_kind          = FULL | DIFF
base_epoch           = 0 for FULL; prior incorporated epoch for DIFF
entries              = bounded ordered set of revocation entries
```

Each revocation entry contains:

```text
holder_ref           = non-empty opaque holder identifier
lineage_ref          = non-empty opaque authorization lineage identifier
effective_epoch      = authority epoch at which revocation became effective
action               = REVOKE
```

`max_staleness`, `required_min_epoch`, scope policy, and current time are verifier/profile inputs and MUST NOT be serialized into the authority view as if they were authority facts.

The normalized representation intentionally has only `REVOKE` in version 1. A DIFF MUST NOT express implicit deletion, unrevocation, trust creation, or lineage repair. Restoring authority requires an explicit reviewed lifecycle transition such as new lineage/re-registration; it is not modeled as removing a revocation entry.

## 3. Structural validity

A version-1 view is structurally valid only when all of the following hold:

1. `format_version == 1`;
2. `authority_ref` is non-empty;
3. `view_epoch > 0`;
4. `update_kind == FULL` implies `base_epoch == 0`;
5. `update_kind == DIFF` implies `0 < base_epoch < view_epoch`;
6. entry count is within the selected profile's local bound;
7. every entry has non-empty holder and lineage references;
8. every entry action is `REVOKE`;
9. every `effective_epoch <= view_epoch`;
10. no duplicate `(holder_ref, lineage_ref)` entry exists within one view.

Unknown versions or actions MUST fail closed. A parser MUST NOT silently reinterpret them as version 1 semantics.

Structural validity does not imply authenticity. Authenticated update ingestion remains a separate required zk214 surface.

## 4. Full and differential composition

A FULL view is a complete authority-scoped revocation snapshot for its `view_epoch` under the selected profile's bounded representation rules.

A DIFF view is applicable only when the receiver has already incorporated the same authority's `base_epoch`. Applying a DIFF to a missing, different-authority, older-than-base, or newer-than-base local state MUST fail closed rather than guess reconciliation order.

Successful incorporation advances the local incorporated epoch to `view_epoch`. Epochs MUST NOT move backward. Duplicate receipt of an already-incorporated view MAY be idempotently ignored only after representation identity/authenticity has been established by the future ingestion layer; transport duplication itself is not authority.

## 5. Freshness evaluator projection

After authenticated ingestion and successful structural/reconciliation checks, the local lifecycle layer projects the normalized state into the existing freshness decision inputs:

```text
view_observed_time   <- observed_time
view_epoch           <- view_epoch
holder_revoked       <- matching REVOKE entry effective at or before view_epoch
lineage_current      <- separately owned lineage authority
```

The existing profile supplies `current_time`, `max_staleness`, `required_min_epoch`, and scope authorization. A valid representation cannot repair stale time, stale lineage, insufficient scope, or an epoch below `required_min_epoch`.

## 6. Conformance corpus

`rust/test-vectors/state/revocation-view-v1.txt` is the canonical structural/composition corpus for this representation. It is specification evidence until Rust and C parsers/composers consume it and exact-head execution evidence is retained.

## 7. Evidence boundary

This contract closes only the roadmap's missing *definition* of a versioned revocation-view representation. It does not establish authenticated ingestion, Rust/C parser parity, full/differential runtime reconciliation, persistence/rollback resistance, disconnected convergence timing, physical-target evidence, formal proof, independent review, RFC/IETF status, or deployment qualification.
