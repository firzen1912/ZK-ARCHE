#ifndef AUTH_ASSOCIATION_ADMISSION_H
#define AUTH_ASSOCIATION_ADMISSION_H

#include <stdbool.h>

typedef struct {
    bool auth_complete;
    bool preexisting_trust_record;
    bool authorization_present;
    bool authorization_fresh;
    bool revocation_current;
    bool explicitly_revoked;
    bool lineage_current;
    bool replay_continuity_current;
    bool binding_required;
    bool binding_valid;
    bool rollback_suspected;
    bool trust_mutation_requested;
} association_admission_facts_t;

typedef enum {
    ASSOCIATION_ADMISSION_ESTABLISH = 0,
    ASSOCIATION_ADMISSION_FAIL_CLOSED = 1
} association_admission_action_t;

typedef enum {
    ASSOCIATION_ADMISSION_REASON_CURRENT = 0,
    ASSOCIATION_ADMISSION_REASON_INVALID_FACTS = 1,
    ASSOCIATION_ADMISSION_REASON_ROLLBACK_SUSPECTED = 2,
    ASSOCIATION_ADMISSION_REASON_TRUST_MUTATION_REQUESTED = 3,
    ASSOCIATION_ADMISSION_REASON_AUTH_INCOMPLETE = 4,
    ASSOCIATION_ADMISSION_REASON_TRUST_RECORD_MISSING = 5,
    ASSOCIATION_ADMISSION_REASON_AUTHORIZATION_MISSING = 6,
    ASSOCIATION_ADMISSION_REASON_AUTHORIZATION_STALE = 7,
    ASSOCIATION_ADMISSION_REASON_REVOCATION_STALE = 8,
    ASSOCIATION_ADMISSION_REASON_REVOKED = 9,
    ASSOCIATION_ADMISSION_REASON_LINEAGE_STALE = 10,
    ASSOCIATION_ADMISSION_REASON_REPLAY_CONTINUITY_STALE = 11,
    ASSOCIATION_ADMISSION_REASON_BINDING_INVALID = 12
} association_admission_reason_t;

typedef struct {
    association_admission_action_t action;
    association_admission_reason_t reason;
} association_admission_decision_t;

association_admission_decision_t association_admission_classify(
    const association_admission_facts_t *facts);

#endif
