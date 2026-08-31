#include "auth/lineage_replace_session_binding.h"

#include <stddef.h>
#include <string.h>

lineage_replace_session_binding_decision_t lineage_replace_classify_session_binding(
    const auth_v3_context_t *context,
    const lineage_replace_session_binding_expectation_t *expectation) {
    if (context == NULL || expectation == NULL || !expectation->auth_completion_verified)
        return LINEAGE_REPLACE_SESSION_REJECT_COMPLETION;
    if (context->protocol_version != expectation->expected_protocol_version)
        return LINEAGE_REPLACE_SESSION_REJECT_VERSION;
    if (context->suite_id != expectation->expected_suite_id)
        return LINEAGE_REPLACE_SESSION_REJECT_SUITE;
    if (context->profile_id != expectation->expected_profile_id)
        return LINEAGE_REPLACE_SESSION_REJECT_PROFILE;
    if (memcmp(context->session_id, expectation->expected_session_id, AUTH_SESSION_ID_LEN) != 0)
        return LINEAGE_REPLACE_SESSION_REJECT_SESSION_ID;
    if (memcmp(context->authz_context_hash, expectation->expected_authz_context_hash, AUTH_HASH_LEN) != 0)
        return LINEAGE_REPLACE_SESSION_REJECT_AUTHZ_CONTEXT;
    if (memcmp(context->channel_binding_hash, expectation->expected_channel_binding_hash, AUTH_HASH_LEN) != 0)
        return LINEAGE_REPLACE_SESSION_REJECT_CHANNEL_BINDING;
    return LINEAGE_REPLACE_SESSION_BOUND;
}
