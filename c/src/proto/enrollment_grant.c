#include "auth/enrollment_grant.h"
#define DENY(r) ((zk_enrollment_grant_decision){ZK_ENROLLMENT_GRANT_DENY, (r)})
zk_enrollment_grant_decision zk_classify_enrollment_grant(const zk_enrollment_grant_facts *f) {
    if (f->rollback_suspected) return DENY(ZK_ENROLLMENT_GRANT_ROLLBACK_SUSPECTED);
    if (f->normal_auth_path) return DENY(ZK_ENROLLMENT_GRANT_NORMAL_AUTH_FORBIDDEN);
    if (!f->explicit_enroll_operation) return DENY(ZK_ENROLLMENT_GRANT_EXPLICIT_ENROLL_REQUIRED);
    if (!f->commissioner_authenticated) return DENY(ZK_ENROLLMENT_GRANT_COMMISSIONER_UNAUTHENTICATED);
    if (!f->commissioner_authorized) return DENY(ZK_ENROLLMENT_GRANT_COMMISSIONER_UNAUTHORIZED);
    if (!f->commissioner_not_revoked) return DENY(ZK_ENROLLMENT_GRANT_COMMISSIONER_REVOKED);
    if (!f->subject_possession_verified) return DENY(ZK_ENROLLMENT_GRANT_SUBJECT_POSSESSION_MISSING);
    if (!f->requested_authority_within_commissioner_scope) return DENY(ZK_ENROLLMENT_GRANT_AUTHORITY_ESCALATION);
    if (!f->scope_bounded) return DENY(ZK_ENROLLMENT_GRANT_SCOPE_UNBOUNDED);
    if (!f->audience_bound) return DENY(ZK_ENROLLMENT_GRANT_AUDIENCE_UNBOUND);
    if (!f->deployment_bound) return DENY(ZK_ENROLLMENT_GRANT_DEPLOYMENT_UNBOUND);
    if (!f->validity_bounded) return DENY(ZK_ENROLLMENT_GRANT_VALIDITY_UNBOUNDED);
    if (!f->epoch_current) return DENY(ZK_ENROLLMENT_GRANT_EPOCH_STALE);
    if (!f->revocation_current) return DENY(ZK_ENROLLMENT_GRANT_REVOCATION_STALE);
    if (!f->lineage_current) return DENY(ZK_ENROLLMENT_GRANT_LINEAGE_STALE);
    if (!f->delegation_depth_within_limit) return DENY(ZK_ENROLLMENT_GRANT_DELEGATION_DEPTH_EXCEEDED);
    return (zk_enrollment_grant_decision){ZK_ENROLLMENT_GRANT_ISSUE, ZK_ENROLLMENT_GRANT_CURRENT};
}
