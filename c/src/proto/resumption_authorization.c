#include "auth/resumption_authorization.h"

#include <stddef.h>

static resumption_authorization_decision_t decision(resumption_action_t action,
                                                     resumption_reason_t reason) {
    resumption_authorization_decision_t out = {action, reason};
    return out;
}

resumption_authorization_decision_t resumption_authorization_classify(
    const resumption_authorization_facts_t *facts) {
    if (facts == NULL)
        return decision(RESUMPTION_ACTION_REJECT, RESUMPTION_REASON_INVALID_FACTS);
    if (facts->rollback_suspected)
        return decision(RESUMPTION_ACTION_REJECT, RESUMPTION_REASON_ROLLBACK_SUSPECTED);
    if (!facts->restart_continuity_current)
        return decision(RESUMPTION_ACTION_REJECT, RESUMPTION_REASON_RESTART_CONTINUITY_STALE);
    if (!facts->usage_counter_continuity_current)
        return decision(RESUMPTION_ACTION_REJECT, RESUMPTION_REASON_USAGE_COUNTER_CONTINUITY_STALE);
    if (facts->session_invalidated)
        return decision(RESUMPTION_ACTION_REJECT, RESUMPTION_REASON_SESSION_INVALIDATED);
    if (!facts->credential_epoch_current)
        return decision(RESUMPTION_ACTION_FULL_AUTH_REQUIRED, RESUMPTION_REASON_CREDENTIAL_EPOCH_STALE);
    if (!facts->credential_present)
        return decision(RESUMPTION_ACTION_FULL_AUTH_REQUIRED, RESUMPTION_REASON_CREDENTIAL_MISSING);
    if (!facts->credential_integrity_valid)
        return decision(RESUMPTION_ACTION_FULL_AUTH_REQUIRED, RESUMPTION_REASON_CREDENTIAL_INVALID);
    if (!facts->binding_valid)
        return decision(RESUMPTION_ACTION_FULL_AUTH_REQUIRED, RESUMPTION_REASON_BINDING_MISMATCH);
    if (facts->expired)
        return decision(RESUMPTION_ACTION_FULL_AUTH_REQUIRED, RESUMPTION_REASON_EXPIRED);
    if (facts->usage_limit == 0u || facts->usage_count >= facts->usage_limit)
        return decision(RESUMPTION_ACTION_FULL_AUTH_REQUIRED, RESUMPTION_REASON_REUSE_LIMIT_REACHED);
    if (!facts->authorization_context_present)
        return decision(RESUMPTION_ACTION_FULL_AUTH_REQUIRED,
                        RESUMPTION_REASON_AUTHORIZATION_CONTEXT_MISSING);
    if (!facts->authorization_context_fresh)
        return decision(RESUMPTION_ACTION_FULL_AUTH_REQUIRED, RESUMPTION_REASON_AUTHORIZATION_STALE);
    if (!facts->authorization_generation_bound)
        return decision(RESUMPTION_ACTION_FULL_AUTH_REQUIRED,
                        RESUMPTION_REASON_AUTHORIZATION_GENERATION_UNBOUND);
    if (!facts->authorization_generation_current)
        return decision(RESUMPTION_ACTION_FULL_AUTH_REQUIRED,
                        RESUMPTION_REASON_AUTHORIZATION_GENERATION_STALE);
    if (!facts->revocation_current)
        return decision(RESUMPTION_ACTION_REJECT, RESUMPTION_REASON_REVOCATION_STALE);
    if (facts->explicitly_revoked)
        return decision(RESUMPTION_ACTION_REJECT, RESUMPTION_REASON_REVOKED);
    if (!facts->lineage_current)
        return decision(RESUMPTION_ACTION_REJECT, RESUMPTION_REASON_LINEAGE_STALE);
    if (!facts->peer_match)
        return decision(RESUMPTION_ACTION_FULL_AUTH_REQUIRED, RESUMPTION_REASON_PEER_MISMATCH);
    if (!facts->deployment_match)
        return decision(RESUMPTION_ACTION_FULL_AUTH_REQUIRED, RESUMPTION_REASON_DEPLOYMENT_MISMATCH);
    if (!facts->audience_match)
        return decision(RESUMPTION_ACTION_FULL_AUTH_REQUIRED, RESUMPTION_REASON_AUDIENCE_MISMATCH);
    if (!facts->profile_match)
        return decision(RESUMPTION_ACTION_FULL_AUTH_REQUIRED, RESUMPTION_REASON_PROFILE_MISMATCH);
    return decision(RESUMPTION_ACTION_RESUME, RESUMPTION_REASON_CURRENT);
}
