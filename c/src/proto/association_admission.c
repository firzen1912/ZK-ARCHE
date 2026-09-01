#include "auth/association_admission.h"

#include <stddef.h>

static association_admission_decision_t decision(association_admission_action_t action,
                                                  association_admission_reason_t reason) {
    association_admission_decision_t out = {action, reason};
    return out;
}

association_admission_decision_t association_admission_classify(
    const association_admission_facts_t *facts) {
    if (facts == NULL)
        return decision(ASSOCIATION_ADMISSION_FAIL_CLOSED,
                        ASSOCIATION_ADMISSION_REASON_INVALID_FACTS);
    if (facts->rollback_suspected)
        return decision(ASSOCIATION_ADMISSION_FAIL_CLOSED,
                        ASSOCIATION_ADMISSION_REASON_ROLLBACK_SUSPECTED);
    if (facts->trust_mutation_requested)
        return decision(ASSOCIATION_ADMISSION_FAIL_CLOSED,
                        ASSOCIATION_ADMISSION_REASON_TRUST_MUTATION_REQUESTED);
    if (!facts->auth_complete)
        return decision(ASSOCIATION_ADMISSION_FAIL_CLOSED,
                        ASSOCIATION_ADMISSION_REASON_AUTH_INCOMPLETE);
    if (!facts->preexisting_trust_record)
        return decision(ASSOCIATION_ADMISSION_FAIL_CLOSED,
                        ASSOCIATION_ADMISSION_REASON_TRUST_RECORD_MISSING);
    if (!facts->authorization_present)
        return decision(ASSOCIATION_ADMISSION_FAIL_CLOSED,
                        ASSOCIATION_ADMISSION_REASON_AUTHORIZATION_MISSING);
    if (!facts->authorization_fresh)
        return decision(ASSOCIATION_ADMISSION_FAIL_CLOSED,
                        ASSOCIATION_ADMISSION_REASON_AUTHORIZATION_STALE);
    if (!facts->revocation_current)
        return decision(ASSOCIATION_ADMISSION_FAIL_CLOSED,
                        ASSOCIATION_ADMISSION_REASON_REVOCATION_STALE);
    if (facts->explicitly_revoked)
        return decision(ASSOCIATION_ADMISSION_FAIL_CLOSED,
                        ASSOCIATION_ADMISSION_REASON_REVOKED);
    if (!facts->lineage_current)
        return decision(ASSOCIATION_ADMISSION_FAIL_CLOSED,
                        ASSOCIATION_ADMISSION_REASON_LINEAGE_STALE);
    if (!facts->replay_continuity_current)
        return decision(ASSOCIATION_ADMISSION_FAIL_CLOSED,
                        ASSOCIATION_ADMISSION_REASON_REPLAY_CONTINUITY_STALE);
    if (facts->binding_required && !facts->binding_valid)
        return decision(ASSOCIATION_ADMISSION_FAIL_CLOSED,
                        ASSOCIATION_ADMISSION_REASON_BINDING_INVALID);
    return decision(ASSOCIATION_ADMISSION_ESTABLISH,
                    ASSOCIATION_ADMISSION_REASON_CURRENT);
}
