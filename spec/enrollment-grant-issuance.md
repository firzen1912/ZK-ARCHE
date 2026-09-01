# Enrollment Grant Issuance Decision

Status: implementation-backed decision contract; not a complete ENROLL wire protocol.

## Security boundary

Enrollment is an explicit trust/authorization mutation. Normal AUTH is NO-LEARNING and MUST NOT issue, expand, or persist a grant as a side effect. A conformant implementation MUST reject grant issuance from the normal AUTH path.

Before issuing a grant, the local enrollment authority MUST establish all of the following: an explicit ENROLL operation; authenticated and locally authorized commissioner authority; commissioner non-revocation; proof of possession by the subject; requested authority no broader than commissioner authority; bounded scope, audience, deployment, validity, and delegation depth; current policy epoch, revocation view, and lineage; and no rollback suspicion.

Failure of any prerequisite is `DENY`. `ISSUE` means only that the local policy decision permits issuance. It does not define a wire encoding, signature/MAC, persistence transaction, or propagation protocol.

## Trust and delegation

A commissioner cannot confer authority it does not hold. Delegated commissioner authority is therefore capped by the already-accepted local grant and its delegation depth. Enrollment does not make trust transitive: subsequent peers still evaluate the issued grant under their own local trust policy.

Revocation and lineage freshness are mandatory issuance inputs. Stale local authority state cannot be repaired by normal AUTH or by successful subject possession proof.

## Conformance evidence

`rust/test-vectors/state/enrollment-grant-v1.txt` is the canonical decision corpus. Rust and C implementations claiming this decision contract must reproduce its ISSUE/DENY result and reason precedence.
