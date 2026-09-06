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

## 3. Format-version registry and change control

`format_version` is a protocol-state version namespace and is therefore subject to the repository's registry/change-control discipline even though this document does not yet assign a wire encoding.

The current allocation is:

| Value | Name | State | Semantics |
|---:|---|---|---|
| `0` | INVALID | reserved | MUST NOT identify a usable revocation view |
| `1` | REVOCATION-VIEW-v1 | draft | normalized FULL/DIFF representation defined by this document |
| `2`–`239` | — | unassigned | require reviewed allocation before use |
| `240`–`254` | — | reserved | unknown-version / negative-conformance test space; MUST NOT acquire production semantics |
| `255` | INVALID | reserved | MUST NOT identify a usable revocation view |

Version 1 remains `draft` until Rust and C consume the canonical structural/composition corpus and retained exact-head qualification demonstrates compatible accept/reject behavior. Defining the logical record and corpus is not sufficient to make the allocation stable.

A new format version MUST NOT silently reinterpret version-1 fields. A version allocation that changes entry identity, epoch ordering, FULL/DIFF composition, revocation action semantics, authority scoping, authenticity requirements, lineage semantics, rollback behavior, or privacy-relevant identifiers requires all of the following before promotion:

```text
normative version semantics
+ compatibility and migration rule
+ explicit unknown-version behavior
+ Rust/C parser/composer semantics where both claim support
+ positive canonical vectors
+ negative cross-version / incompatible-version vectors
+ lifecycle and rollback impact analysis
+ security/privacy review of semantic changes
+ versioned change record
```

An implementation receiving an unknown or unsupported `format_version` MUST fail closed before treating the object as current revocation state. It MUST NOT downgrade the object to version 1, infer a compatible prefix, or retain only fields it recognizes when doing so could alter authorization, lineage, revocation, or freshness semantics.

Format-version negotiation, if introduced later, MUST NOT permit a peer, transport, gateway, cache, or higher-capability node to force a verifier below the minimum version required by the verifier's selected profile or locally authoritative lifecycle policy.

This namespace is logically governed by `spec/registries.md`. Until the central registry document is extended with a dedicated revocation-view subsection, this section is the authoritative allocation table for `format_version`; any future central-registry entry MUST reproduce these values and semantics exactly rather than renumbering them.

## 4. Structural validity

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

## 5. Full and differential composition

A FULL view is a complete authority-scoped revocation snapshot for its `view_epoch` under the selected profile's bounded representation rules.

A DIFF view is applicable only when the receiver has already incorporated the same authority's `base_epoch`. Applying a DIFF to a missing, different-authority, older-than-base, or newer-than-base local state MUST fail closed rather than guess reconciliation order.

Successful incorporation advances the local incorporated epoch to `view_epoch`. Epochs MUST NOT move backward. Duplicate receipt of an already-incorporated view MAY be idempotently ignored only after representation identity/authenticity has been established by the future ingestion layer; transport duplication itself is not authority.

## 6. Freshness evaluator projection

After authenticated ingestion and successful structural/reconciliation checks, the local lifecycle layer projects the normalized state into the existing freshness decision inputs:

```text
view_observed_time   <- observed_time
view_epoch           <- view_epoch
holder_revoked       <- matching REVOKE entry effective at or before view_epoch
lineage_current      <- separately owned lineage authority
```

The existing profile supplies `current_time`, `max_staleness`, `required_min_epoch`, and scope authorization. A valid representation cannot repair stale time, stale lineage, insufficient scope, or an epoch below `required_min_epoch`.

## 7. Conformance corpus

`rust/test-vectors/state/revocation-view-v1.txt` is the canonical structural/composition corpus for this representation. It is specification evidence until Rust and C parsers/composers consume it and exact-head execution evidence is retained.

The corpus MUST retain at least one unknown-version negative case. Future format versions MUST add explicit cross-version positive/negative vectors rather than modifying version-1 cases in place.

## 8. Evidence boundary

This contract closes only the roadmap's missing *definition* of a versioned revocation-view representation and now also defines its allocation/change-control boundary. It does not establish authenticated ingestion, Rust/C parser parity, full/differential runtime reconciliation, persistence/rollback resistance, disconnected convergence timing, physical-target evidence, formal proof, independent review, RFC/IETF status, or deployment qualification.
