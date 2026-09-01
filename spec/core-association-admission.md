# CORE secure-association admission postcondition

Status: implementation-backed draft for the current `dev` baseline. This document defines a wire-neutral CORE/LINK postcondition; it does not create a new trust store, authorization engine, revocation engine, replay store, or transport identity.

## 1. Purpose

A completed cryptographic AUTH exchange is necessary but not sufficient to establish or retain a secure association. Association admission MUST compose the current decisions of the owning AUTH, TRUST, LINK, and BIND layers and MUST fail closed when any mandatory prerequisite is unsafe.

The classifier defined by this contract is a postcondition over authoritative facts. It MUST NOT independently learn peers, create trust, repair missing state, infer authorization from authentication, or treat transport metadata as identity.

## 2. Authoritative inputs

The association-admission decision consumes the following facts:

| Fact | Owning authority |
|---|---|
| `auth_complete` | AUTH completion/key-confirmation state |
| `preexisting_trust_record` | locally authoritative TRUST state |
| `authorization_present` | TRUST authorization state |
| `authorization_fresh` | authorization freshness/policy state |
| `revocation_current` | current local revocation view |
| `explicitly_revoked` | current local revocation view |
| `lineage_current` | TRUST lineage/replacement state |
| `replay_continuity_current` | LINK replay/restart continuity state |
| `binding_required` / `binding_valid` | selected profile plus BIND decision |
| `rollback_suspected` | persistent lifecycle-state integrity/recovery state |
| `trust_mutation_requested` | caller intent at the normal AUTH boundary |

Callers MUST derive these facts from the corresponding owning subsystem. They MUST NOT manufacture a successful fact merely to retain an association.

## 3. Decision

The only successful result is `ESTABLISH`. Every other result is `FAIL_CLOSED` with a deterministic reason.

Association admission MUST fail closed, in precedence order, when:

1. rollback is suspected;
2. normal AUTH is being asked to mutate trust;
3. AUTH is incomplete;
4. no pre-existing local trust record exists;
5. scoped authorization is absent;
6. authorization state is stale;
7. the revocation view is stale;
8. the holder is explicitly revoked;
9. authorization/trust lineage is stale;
10. replay/restart continuity is not current; or
11. the selected profile requires a binding and the binding is not valid.

Only when every mandatory condition is satisfied may the association be established or retained.

## 4. NO-LEARNING AUTH

Normal AUTH is NO-LEARNING. A mathematically valid proof from an unknown peer MUST NOT create a trust record, enrollment grant, delegation, or authorization record. If association admission observes `trust_mutation_requested=true`, it MUST fail closed.

Trust mutation belongs to explicit ENROLL, commissioner/grant, reviewed rekey/re-registration, revocation, or equivalent lifecycle transitions. A successful association-admission result therefore confirms only that the locally authoritative prerequisites were already satisfactory.

## 5. Re-evaluation and invalidation

The same postcondition applies to retaining an existing association. When locally authoritative lifecycle state changes, implementations MUST re-evaluate the association before security-sensitive continued use when the profile requires that state to be current.

In particular, an existing association MUST NOT remain security-authoritative merely because its traffic keys still exist when authorization becomes stale, the holder becomes revoked, lineage becomes stale, replay continuity is lost, rollback is suspected, or a required channel binding becomes invalid.

This contract does not define key erasure timing or a wire alert. Those behaviors remain LINK/state-machine work and require separate specification and tests.

## 6. Binding and transport independence

When `binding_required=false`, the validity of optional binding metadata MUST NOT become implicit authorization or identity authority. When `binding_required=true`, an invalid or missing effective binding MUST fail closed.

Transport addresses, sockets, MAC addresses, connection identifiers, radio endpoints, or gateway metadata do not satisfy `preexisting_trust_record` and cannot override an invalid BIND decision.

## 7. Infrastructure independence

Association admission is based on local security evidence. Availability of a CA, cloud service, central registry, DNS, gateway, Internet connection, or manufacturer service is not an input to this classifier.

Infrastructure may synchronize policy or revocation state. If local state exceeds the permitted freshness bound, the resulting stale fact causes fail-closed behavior; infrastructure availability itself does not become protocol authority.

## 8. Canonical decision corpus

`rust/test-vectors/state/association-admission-v1.txt` is the canonical Rust/C decision corpus for this postcondition. It covers successful admission plus negative cases for incomplete AUTH, missing trust, missing/stale authorization, stale revocation, explicit revocation, stale lineage, replay continuity loss, required-binding failure, rollback suspicion, and attempted trust mutation.

The corpus is decision-level evidence only. It is not byte-level handshake interoperability, physical constrained-target evidence, formal proof, or independent cryptographic review.
