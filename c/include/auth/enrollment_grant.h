#ifndef ZK_ARCHE_ENROLLMENT_GRANT_H
#define ZK_ARCHE_ENROLLMENT_GRANT_H
#include <stdbool.h>
typedef struct {
    bool explicit_enroll_operation, normal_auth_path, commissioner_authenticated,
         commissioner_authorized, commissioner_not_revoked, subject_possession_verified,
         requested_authority_within_commissioner_scope, scope_bounded, audience_bound,
         deployment_bound, validity_bounded, epoch_current, revocation_current,
         lineage_current, delegation_depth_within_limit, rollback_suspected;
} zk_enrollment_grant_facts;
typedef enum { ZK_ENROLLMENT_GRANT_ISSUE = 0, ZK_ENROLLMENT_GRANT_DENY = 1 } zk_enrollment_grant_action;
typedef enum {
    ZK_ENROLLMENT_GRANT_CURRENT = 0, ZK_ENROLLMENT_GRANT_ROLLBACK_SUSPECTED,
    ZK_ENROLLMENT_GRANT_NORMAL_AUTH_FORBIDDEN, ZK_ENROLLMENT_GRANT_EXPLICIT_ENROLL_REQUIRED,
    ZK_ENROLLMENT_GRANT_COMMISSIONER_UNAUTHENTICATED, ZK_ENROLLMENT_GRANT_COMMISSIONER_UNAUTHORIZED,
    ZK_ENROLLMENT_GRANT_COMMISSIONER_REVOKED, ZK_ENROLLMENT_GRANT_SUBJECT_POSSESSION_MISSING,
    ZK_ENROLLMENT_GRANT_AUTHORITY_ESCALATION, ZK_ENROLLMENT_GRANT_SCOPE_UNBOUNDED,
    ZK_ENROLLMENT_GRANT_AUDIENCE_UNBOUND, ZK_ENROLLMENT_GRANT_DEPLOYMENT_UNBOUND,
    ZK_ENROLLMENT_GRANT_VALIDITY_UNBOUNDED, ZK_ENROLLMENT_GRANT_EPOCH_STALE,
    ZK_ENROLLMENT_GRANT_REVOCATION_STALE, ZK_ENROLLMENT_GRANT_LINEAGE_STALE,
    ZK_ENROLLMENT_GRANT_DELEGATION_DEPTH_EXCEEDED
} zk_enrollment_grant_reason;
typedef struct { zk_enrollment_grant_action action; zk_enrollment_grant_reason reason; } zk_enrollment_grant_decision;
zk_enrollment_grant_decision zk_classify_enrollment_grant(const zk_enrollment_grant_facts *f);
#endif
