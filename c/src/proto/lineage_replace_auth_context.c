#include "auth/lineage_replace_auth_context.h"
#include <stddef.h>
#include <string.h>
static bool bytes_nonzero(const uint8_t *value, size_t len) {
    size_t i; uint8_t acc = 0u;
    for (i = 0u; i < len; ++i) acc |= value[i];
    return acc != 0u;
}
static bool context_shape_valid(const auth_v3_iot_core_authorization_context_v1_t *context) {
    return context != NULL &&
           bytes_nonzero(context->holder_binding, sizeof(context->holder_binding)) &&
           bytes_nonzero(context->audience_id, sizeof(context->audience_id)) &&
           context->role_policy_id != 0u &&
           context->scope_bits == AUTH_V3_IOT_CORE_SCOPE_SECURE_ASSOCIATION &&
           context->authorization_generation != 0u &&
           context->policy_epoch != 0u &&
           context->revocation_epoch != 0u;
}
lineage_replace_authorization_decision_t lineage_replace_authorization_from_iot_core(
    const auth_v3_iot_core_authorization_context_v1_t *context,
    const auth_v3_iot_core_attribution_record_v1_t *attribution,
    const lineage_replace_auth_context_evidence_t *evidence) {
    lineage_replace_authorization_facts_t facts = {false, false, false, false, false, false, false};
    bool attribution_context_match; bool peer_match; bool predecessor_match;
    if (evidence == NULL) return LINEAGE_REPLACE_REJECT_CURRENT_CREDENTIAL_CONTROL;
    facts.current_credential_control_valid = evidence->current_credential_control_valid;
    facts.successor_key_control_valid = evidence->successor_key_control_valid;
    facts.current_session_authenticated = evidence->current_session_authenticated;
    if (!context_shape_valid(context) || attribution == NULL)
        return lineage_replace_classify_authorization(&facts);
    attribution_context_match =
        memcmp(attribution->holder_binding, context->holder_binding, 32u) == 0 &&
        memcmp(attribution->audience_id, context->audience_id, 32u) == 0 &&
        attribution->role_policy_id == context->role_policy_id &&
        attribution->scope_bits == context->scope_bits &&
        attribution->authorization_generation == context->authorization_generation &&
        attribution->policy_epoch == context->policy_epoch &&
        attribution->revocation_epoch == context->revocation_epoch;
    peer_match = memcmp(attribution->peer_identity, evidence->expected_peer_identity, 32u) == 0;
    predecessor_match = memcmp(attribution->credential_reference,
                               evidence->predecessor_credential_reference, 32u) == 0;
    facts.current_session_authorized = attribution_context_match;
    facts.current_context_bound = attribution_context_match && peer_match;
    facts.predecessor_binding_current = predecessor_match &&
        attribution->authorization_generation == context->authorization_generation;
    facts.successor_scope_within_authorized_scope =
        evidence->requested_successor_scope_bits != 0u &&
        (evidence->requested_successor_scope_bits & ~context->scope_bits) == 0u;
    return lineage_replace_classify_authorization(&facts);
}
