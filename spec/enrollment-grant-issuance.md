# Enrollment Grant Issuance Decision

Status: implementation-backed decision contract; not a complete ENROLL wire protocol.

## Security boundary

Enrollment is an explicit trust/authorization mutation. Normal AUTH is NO-LEARNING and MUST NOT issue, expand, refresh, or persist a grant as a side effect. A conformant implementation MUST reject grant issuance from the normal AUTH path.

Before issuing a grant, the local enrollment authority MUST establish all of the following: an explicit ENROLL operation; authenticated and locally authorized commissioner authority; **fresh commissioner authorization**; commissioner non-revocation; an **unused enrollment nonce/operation identifier** for the locally enforced replay window; proof of possession by the subject; requested authority no broader than commissioner authority; bounded scope, audience, deployment, validity, and delegation depth; current policy epoch, revocation view, and lineage; and no rollback suspicion.

Failure of any prerequisite is `DENY`. `ISSUE` means only that the local policy decision permits issuance. It does not define a wire encoding, signature/MAC, persistence transaction, replay-store implementation, or propagation protocol.

## Authorization freshness and replay

`commissioner_authorized` and `commissioner_authorization_fresh` are distinct inputs. A commissioner may still possess an otherwise valid-looking authorization artifact after its accepted freshness window has expired; that stale state MUST NOT authorize new enrollment or expand trust.

`enrollment_nonce_unused` represents the local replay decision for the explicit ENROLL operation. A previously consumed nonce/operation identifier MUST fail closed with `ENROLLMENT_REPLAY_DETECTED`. Successful subject possession, successful prior AUTH, or a still-valid commissioner credential MUST NOT make a replay reusable.

The replay store and its persistence/restart guarantees are owned by the enrollment/replay lifecycle implementation. This classifier consumes the verified fact and does not manufacture freshness.

## Trust and delegation

A commissioner cannot confer authority it does not hold. Delegated commissioner authority is therefore capped by the already-accepted local grant and its delegation depth. Enrollment does not make trust transitive: subsequent peers still evaluate the issued grant under their own local trust policy.

Revocation, authorization freshness, and lineage freshness are mandatory issuance inputs. Stale local authority state cannot be repaired by normal AUTH or by successful subject possession proof.

## Required precedence

A conformant implementation applies the following fail-closed order:

1. rollback suspicion;
2. normal-AUTH-path prohibition;
3. explicit ENROLL requirement;
4. commissioner authentication;
5. commissioner authorization;
6. commissioner authorization freshness;
7. commissioner non-revocation;
8. unused enrollment nonce / replay decision;
9. subject possession;
10. authority scope and bounded scope/audience/deployment/validity;
11. current epoch, revocation view, and lineage;
12. bounded delegation depth;
13. otherwise `ISSUE`.

## Conformance evidence

`rust/test-vectors/state/enrollment-grant-v2.txt` is the current canonical decision corpus. Rust and C implementations claiming the current contract MUST reproduce its `ISSUE`/`DENY` result and reason precedence. Version 1 remains historical evidence for the earlier decision surface.

The v2 corpus adds dedicated negative evidence for stale commissioner authorization and replayed enrollment operations while retaining the prior normal-AUTH, commissioner, possession, scope, epoch, revocation, lineage, delegation-depth, and rollback cases.

## Evidence boundary

This contract and corpus establish wire-neutral decision semantics only. They do not establish the ENROLL wire format, cryptographic grant verification, durable one-time replay storage, restart/rollback persistence, revocation propagation latency, physical MCU evidence, formal proof, independent cryptographic review, RFC/IETF status, or deployment qualification.
