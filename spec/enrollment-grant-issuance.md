# Enrollment Grant Issuance Decision

Status: implementation-backed decision contract; not a complete ENROLL wire protocol.

## Security boundary

Enrollment is an explicit trust/authorization mutation. Normal AUTH is NO-LEARNING and MUST NOT issue, expand, refresh, or persist a grant as a side effect. A conformant implementation MUST reject grant issuance from the normal AUTH path.

Before issuing a grant, the local enrollment authority MUST establish all of the following: an explicit ENROLL operation; authenticated and locally authorized commissioner authority; fresh commissioner authorization that is **authentically bound to a specific local authorization generation**; confirmation that the bound generation is the current local authorization generation; commissioner non-revocation; an **unused enrollment nonce/operation identifier** for the locally enforced replay window; proof of possession by the subject; requested authority no broader than commissioner authority; bounded scope, audience, deployment, validity, and delegation depth; current policy epoch, revocation view, and lineage; and no rollback suspicion.

Failure of any prerequisite is `DENY`. `ISSUE` means only that the local policy decision permits issuance. It does not define a wire encoding, signature/MAC, persistence transaction, replay-store implementation, or propagation protocol.

## Authorization freshness, generation provenance, and replay

`commissioner_authorized`, `commissioner_authorization_fresh`, `commissioner_authorization_generation_bound`, and `commissioner_authorization_generation_current` are distinct inputs. A commissioner may still possess an otherwise valid-looking authorization artifact after its accepted freshness window has expired, may present authority whose generation provenance is absent, or may present authority authentically bound to an older generation. None of those states may authorize new enrollment or expand trust.

`commissioner_authorization_generation_bound` means the authorization evidence identifies the generation it belongs to through authenticated evidence or authenticated local metadata governed by the existing authorization/lifecycle authority. Transport addresses, socket identity, optional infrastructure, successful AUTH, possession proof, delegation evidence, or a caller-provided generation number MUST NOT synthesize this binding. Missing provenance fails closed with `COMMISSIONER_AUTHORIZATION_GENERATION_UNBOUND`.

`commissioner_authorization_generation_current` is evaluated only after provenance has been established. It is supplied by the existing authorization/lifecycle authority and compares the authentically bound generation with current local state. Enrollment MUST NOT create a parallel generation counter or repair an older generation. An older-generation grant fails closed with `COMMISSIONER_AUTHORIZATION_GENERATION_STALE`.

`enrollment_nonce_unused` represents the local replay decision for the explicit ENROLL operation. A previously consumed nonce/operation identifier MUST fail closed with `ENROLLMENT_REPLAY_DETECTED`. Successful subject possession, successful prior AUTH, or a still-valid commissioner credential MUST NOT make a replay reusable.

The replay store and its persistence/restart guarantees are owned by the enrollment/replay lifecycle implementation. This classifier consumes verified lifecycle facts and does not manufacture freshness or provenance.

## Trust and delegation

A commissioner cannot confer authority it does not hold. Delegated commissioner authority is therefore capped by the already-accepted local grant and its delegation depth. Enrollment does not make trust transitive: subsequent peers still evaluate the issued grant under their own local trust policy.

Revocation, authorization freshness, authorization-generation provenance, authorization-generation currentness, and lineage freshness are mandatory issuance inputs. Stale or unproven local authority state cannot be repaired by normal AUTH or successful subject possession proof.

## Required precedence

A conformant implementation applies the following fail-closed order:

1. rollback suspicion;
2. normal-AUTH-path prohibition;
3. explicit ENROLL requirement;
4. commissioner authentication;
5. commissioner authorization;
6. commissioner authorization freshness;
7. commissioner authorization-generation provenance/binding;
8. commissioner authorization-generation currentness;
9. commissioner non-revocation;
10. unused enrollment nonce / replay decision;
11. subject possession;
12. authority scope and bounded scope/audience/deployment/validity;
13. current epoch, revocation view, and lineage;
14. bounded delegation depth;
15. otherwise `ISSUE`.

## Conformance evidence

`rust/test-vectors/state/enrollment-grant-v4.txt` is the current canonical decision corpus. Rust and C implementations claiming the current contract MUST reproduce its `ISSUE`/`DENY` result and reason precedence. Versions 1 through 3 remain historical evidence for earlier decision surfaces.

The v4 corpus adds dedicated negative evidence for missing commissioner authorization-generation provenance while retaining v3's stale-generation case and the prior stale-authorization, replay, normal-AUTH, commissioner, possession, scope, epoch, revocation, lineage, delegation-depth, and rollback cases.

## Evidence boundary

This contract and corpus establish wire-neutral decision semantics only. They do not establish the ENROLL wire format, cryptographic grant verification, durable one-time replay storage, restart/rollback persistence, revocation propagation latency, physical MCU evidence, formal proof, independent cryptographic review, RFC/IETF status, or deployment qualification.
