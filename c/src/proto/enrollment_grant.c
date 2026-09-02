#include "auth/enrollment_grant.h"

#include <stddef.h>

static zk_enrollment_grant_decision_t decision(
    zk_enrollment_grant_action_t action,
    zk_enrollment_grant_reason_t reason) {
    zk_enrollment_grant_decision_t out = {action, reason};
    return out;
}

zk_enrollment_grant_decision_t zk_enrollment_grant_classify(
    const zk_enrollment_grant_facts_t *f) {
    if (f == NULL)
        return decision(ZK_ENROLLMENT_GRANT_DENY,
                        ZK_ENROLLMENT_GRANT_REASON_EXPLICIT_ENROLL_REQUIRED);
    if (f->rollback_suspected)
        return decision(ZK_ENROLLMENT_GRANT_DENY,
                        ZK_ENROLLMENT_GRANT_REASON_ROLLBACK_SUSPECTED);
    if (f->normal_auth_path)
        return decision(ZK_ENROLLMENT_GRANT_DENY,
                        ZK_ENROLLMENT_GRANT_REASON_NORMAL_AUTH_FORBIDDEN);
    if (!f->explicit_enroll_operation)
        return decision(ZK_ENROLLMENT_GRANT_DENY,
                        ZK_ENROLLMENT_GRANT_REASON_EXPLICIT_ENROLL_REQUIRED);
    if (!f->commissioner_authenticated)
        return decision(ZK_ENROLLMENT_GRANT_DENY,
                        ZK_ENROLLMENT_GRANT_REASON_COMMISSIONER_UNAUTHENTICATED);
    if (!f->commissioner_authorized)
        return decision(ZK_ENROLLMENT_GRANT_DENY,
                        ZK_ENROLLMENT_GRANT_REASON_COMMISSIONER_UNAUTHORIZED);
    if (!f->commissioner_authorization_fresh)
        return decision(ZK_ENROLLMENT_GRANT_DENY,
                        ZK_ENROLLMENT_GRANT_REASON_COMMISSIONER_AUTHORIZATION_STALE);
    if (!f->commissioner_not_revoked)
        return decision(ZK_ENROLLMENT_GRANT_DENY,
                        ZK_ENROLLMENT_GRANT_REASON_COMMISSIONER_REVOKED);
    if (!f->enrollment_nonce_unused)
        return decision(ZK_ENROLLMENT_GRANT_DENY,
                        ZK_ENROLLMENT_GRANT_REASON_ENROLLMENT_REPLAY_DETECTED);
    if (!f->subject_possession_verified)
        return decision(ZK_ENROLLMENT_GRANT_DENY,
                        ZK_ENROLLMENT_GRANT_REASON_SUBJECT_POSSESSION_MISSING);
    if (!f->requested_authority_within_commissioner_scope)
        return decision(ZK_ENROLLMENT_GRANT_DENY,
                        ZK_ENROLLMENT_GRANT_REASON_AUTHORITY_ESCALATION);
    if (!f->scope_bounded)
        return decision(ZK_ENROLLMENT_GRANT_DENY,
                        ZK_ENROLLMENT_GRANT_REASON_SCOPE_UNBOUNDED);
    if (!f->audience_bound)
        return decision(ZK_ENROLLMENT_GRANT_DENY,
                        ZK_ENROLLMENT_GRANT_REASON_AUDIENCE_UNBOUND);
    if (!f->deployment_bound)
        return decision(ZK_ENROLLMENT_GRANT_DENY,
                        ZK_ENROLLMENT_GRANT_REASON_DEPLOYMENT_UNBOUND);
    if (!f->validity_bounded)
        return decision(ZK_ENROLLMENT_GRANT_DENY,
                        ZK_ENROLLMENT_GRANT_REASON_VALIDITY_UNBOUNDED);
    if (!f->epoch_current)
        return decision(ZK_ENROLLMENT_GRANT_DENY,
                        ZK_ENROLLMENT_GRANT_REASON_EPOCH_STALE);
    if (!f->revocation_current)
        return decision(ZK_ENROLLMENT_GRANT_DENY,
                        ZK_ENROLLMENT_GRANT_REASON_REVOCATION_STALE);
    if (!f->lineage_current)
        return decision(ZK_ENROLLMENT_GRANT_DENY,
                        ZK_ENROLLMENT_GRANT_REASON_LINEAGE_STALE);
    if (!f->delegation_depth_within_limit)
        return decision(ZK_ENROLLMENT_GRANT_DENY,
                        ZK_ENROLLMENT_GRANT_REASON_DELEGATION_DEPTH_EXCEEDED);
    return decision(ZK_ENROLLMENT_GRANT_ISSUE,
                    ZK_ENROLLMENT_GRANT_REASON_CURRENT);
}
