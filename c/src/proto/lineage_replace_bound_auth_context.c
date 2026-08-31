#include "auth/lineage_replace_bound_auth_context.h"
#include "auth/lineage_replace_auth_context.h"
#include <string.h>

lineage_replace_authorization_decision_t lineage_replace_authorization_from_bound_iot_core(
    const auth_v3_context_t *auth_context,
    const lineage_replace_session_binding_expectation_t *session_expectation,
    const auth_v3_iot_core_authorization_context_v1_t *authorization_context,
    const auth_v3_iot_core_attribution_record_v1_t *attribution,
    const lineage_replace_bound_auth_context_evidence_t *evidence) {
    lineage_replace_auth_context_evidence_t normalized;
    if (evidence == NULL) return LINEAGE_REPLACE_REJECT_CURRENT_CREDENTIAL_CONTROL;
    memset(&normalized, 0, sizeof(normalized));
    normalized.current_credential_control_valid = evidence->current_credential_control_valid;
    normalized.successor_key_control_valid = evidence->successor_key_control_valid;
    normalized.current_session_authenticated =
        lineage_replace_classify_session_binding(auth_context, session_expectation) ==
        LINEAGE_REPLACE_SESSION_BOUND;
    memcpy(normalized.expected_peer_identity, evidence->expected_peer_identity,
           sizeof(normalized.expected_peer_identity));
    memcpy(normalized.predecessor_credential_reference,
           evidence->predecessor_credential_reference,
           sizeof(normalized.predecessor_credential_reference));
    normalized.requested_successor_scope_bits = evidence->requested_successor_scope_bits;
    return lineage_replace_authorization_from_iot_core(
        authorization_context, attribution, &normalized);
}

lineage_replace_authorization_decision_t lineage_replace_authorization_from_verified_bound_iot_core(
    const auth_v3_context_t *auth_context,
    const lineage_replace_session_binding_expectation_t *session_expectation,
    const auth_v3_iot_core_authorization_context_v1_t *authorization_context,
    const auth_v3_iot_core_attribution_record_v1_t *attribution,
    const lineage_replace_verified_bound_auth_context_evidence_t *evidence) {
    lineage_replace_possession_decision_t possession;
    lineage_replace_bound_auth_context_evidence_t normalized;
    if (evidence == NULL) return LINEAGE_REPLACE_REJECT_CURRENT_CREDENTIAL_CONTROL;
    if (session_expectation == NULL) return LINEAGE_REPLACE_REJECT_SESSION_AUTHENTICATION;
    possession = lineage_replace_classify_possession(
        &evidence->current_credential_proof, &evidence->successor_key_proof,
        session_expectation->expected_session_id,
        evidence->predecessor_credential_reference,
        evidence->successor_key_reference);
    if (possession == LINEAGE_REPLACE_POSSESSION_REJECT_CURRENT_CREDENTIAL_CONTROL)
        return LINEAGE_REPLACE_REJECT_CURRENT_CREDENTIAL_CONTROL;
    if (possession == LINEAGE_REPLACE_POSSESSION_REJECT_SUCCESSOR_KEY_CONTROL)
        return LINEAGE_REPLACE_REJECT_SUCCESSOR_KEY_CONTROL;
    memset(&normalized, 0, sizeof(normalized));
    normalized.current_credential_control_valid = true;
    normalized.successor_key_control_valid = true;
    memcpy(normalized.expected_peer_identity, evidence->expected_peer_identity,
           sizeof(normalized.expected_peer_identity));
    memcpy(normalized.predecessor_credential_reference,
           evidence->predecessor_credential_reference,
           sizeof(normalized.predecessor_credential_reference));
    normalized.requested_successor_scope_bits = evidence->requested_successor_scope_bits;
    return lineage_replace_authorization_from_bound_iot_core(
        auth_context, session_expectation, authorization_context, attribution, &normalized);
}
