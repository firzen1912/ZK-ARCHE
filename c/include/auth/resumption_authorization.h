#ifndef AUTH_RESUMPTION_AUTHORIZATION_H
#define AUTH_RESUMPTION_AUTHORIZATION_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    bool credential_present;
    bool credential_integrity_valid;
    bool binding_valid;
    bool expired;
    uint32_t usage_count;
    uint32_t usage_limit;
    bool usage_counter_continuity_current;
    bool authorization_context_present;
    bool authorization_context_fresh;
    bool authorization_generation_bound;
    bool authorization_generation_current;
    bool revocation_current;
    bool explicitly_revoked;
    bool lineage_current;
    bool restart_continuity_current;
    bool credential_epoch_current;
    bool session_invalidated;
    bool peer_match;
    bool deployment_match;
    bool audience_match;
    bool profile_match;
    bool rollback_suspected;
} resumption_authorization_facts_t;

typedef enum {
    RESUMPTION_ACTION_RESUME = 0,
    RESUMPTION_ACTION_FULL_AUTH_REQUIRED = 1,
    RESUMPTION_ACTION_REJECT = 2
} resumption_action_t;

typedef enum {
    RESUMPTION_REASON_CURRENT = 0,
    RESUMPTION_REASON_INVALID_FACTS = 1,
    RESUMPTION_REASON_ROLLBACK_SUSPECTED = 2,
    RESUMPTION_REASON_USAGE_COUNTER_CONTINUITY_STALE = 3,
    RESUMPTION_REASON_CREDENTIAL_MISSING = 4,
    RESUMPTION_REASON_CREDENTIAL_INVALID = 5,
    RESUMPTION_REASON_BINDING_MISMATCH = 6,
    RESUMPTION_REASON_EXPIRED = 7,
    RESUMPTION_REASON_REUSE_LIMIT_REACHED = 8,
    RESUMPTION_REASON_AUTHORIZATION_CONTEXT_MISSING = 9,
    RESUMPTION_REASON_AUTHORIZATION_STALE = 10,
    RESUMPTION_REASON_AUTHORIZATION_GENERATION_UNBOUND = 11,
    RESUMPTION_REASON_AUTHORIZATION_GENERATION_STALE = 12,
    RESUMPTION_REASON_REVOCATION_STALE = 13,
    RESUMPTION_REASON_REVOKED = 14,
    RESUMPTION_REASON_LINEAGE_STALE = 15,
    RESUMPTION_REASON_RESTART_CONTINUITY_STALE = 16,
    RESUMPTION_REASON_CREDENTIAL_EPOCH_STALE = 17,
    RESUMPTION_REASON_SESSION_INVALIDATED = 18,
    RESUMPTION_REASON_PEER_MISMATCH = 19,
    RESUMPTION_REASON_DEPLOYMENT_MISMATCH = 20,
    RESUMPTION_REASON_AUDIENCE_MISMATCH = 21,
    RESUMPTION_REASON_PROFILE_MISMATCH = 22
} resumption_reason_t;

typedef struct {
    resumption_action_t action;
    resumption_reason_t reason;
} resumption_authorization_decision_t;

resumption_authorization_decision_t resumption_authorization_classify(
    const resumption_authorization_facts_t *facts);

#endif
