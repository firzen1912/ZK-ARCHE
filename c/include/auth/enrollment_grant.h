#ifndef AUTH_ENROLLMENT_GRANT_H
#define AUTH_ENROLLMENT_GRANT_H

#include <stdbool.h>

typedef struct {
    bool explicit_enroll_operation;
    bool normal_auth_path;
    bool commissioner_authenticated;
    bool commissioner_authorized;
    bool commissioner_authorization_fresh;
    bool commissioner_authorization_generation_bound;
    bool commissioner_authorization_generation_current;
    bool commissioner_not_revoked;
    bool enrollment_nonce_unused;
    bool subject_possession_verified;
    bool requested_authority_within_commissioner_scope;
    bool scope_bounded;
    bool audience_bound;
    bool deployment_bound;
    bool validity_bounded;
    bool epoch_current;
    bool revocation_current;
    bool lineage_current;
    bool delegation_depth_within_limit;
    bool rollback_suspected;
} zk_enrollment_grant_facts_t;

typedef enum {
    ZK_ENROLLMENT_GRANT_ISSUE = 0,
    ZK_ENROLLMENT_GRANT_DENY = 1
} zk_enrollment_grant_action_t;

typedef enum {
    ZK_ENROLLMENT_GRANT_REASON_CURRENT = 0,
    ZK_ENROLLMENT_GRANT_REASON_ROLLBACK_SUSPECTED,
    ZK_ENROLLMENT_GRANT_REASON_NORMAL_AUTH_FORBIDDEN,
    ZK_ENROLLMENT_GRANT_REASON_EXPLICIT_ENROLL_REQUIRED,
    ZK_ENROLLMENT_GRANT_REASON_COMMISSIONER_UNAUTHENTICATED,
    ZK_ENROLLMENT_GRANT_REASON_COMMISSIONER_UNAUTHORIZED,
    ZK_ENROLLMENT_GRANT_REASON_COMMISSIONER_AUTHORIZATION_STALE,
    ZK_ENROLLMENT_GRANT_REASON_COMMISSIONER_AUTHORIZATION_GENERATION_UNBOUND,
    ZK_ENROLLMENT_GRANT_REASON_COMMISSIONER_AUTHORIZATION_GENERATION_STALE,
    ZK_ENROLLMENT_GRANT_REASON_COMMISSIONER_REVOKED,
    ZK_ENROLLMENT_GRANT_REASON_ENROLLMENT_REPLAY_DETECTED,
    ZK_ENROLLMENT_GRANT_REASON_SUBJECT_POSSESSION_MISSING,
    ZK_ENROLLMENT_GRANT_REASON_AUTHORITY_ESCALATION,
    ZK_ENROLLMENT_GRANT_REASON_SCOPE_UNBOUNDED,
    ZK_ENROLLMENT_GRANT_REASON_AUDIENCE_UNBOUND,
    ZK_ENROLLMENT_GRANT_REASON_DEPLOYMENT_UNBOUND,
    ZK_ENROLLMENT_GRANT_REASON_VALIDITY_UNBOUNDED,
    ZK_ENROLLMENT_GRANT_REASON_EPOCH_STALE,
    ZK_ENROLLMENT_GRANT_REASON_REVOCATION_STALE,
    ZK_ENROLLMENT_GRANT_REASON_LINEAGE_STALE,
    ZK_ENROLLMENT_GRANT_REASON_DELEGATION_DEPTH_EXCEEDED
} zk_enrollment_grant_reason_t;

typedef struct {
    zk_enrollment_grant_action_t action;
    zk_enrollment_grant_reason_t reason;
} zk_enrollment_grant_decision_t;

zk_enrollment_grant_decision_t zk_enrollment_grant_classify(
    const zk_enrollment_grant_facts_t *facts);

#endif
