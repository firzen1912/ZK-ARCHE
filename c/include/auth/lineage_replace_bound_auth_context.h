#ifndef AUTH_LINEAGE_REPLACE_BOUND_AUTH_CONTEXT_H
#define AUTH_LINEAGE_REPLACE_BOUND_AUTH_CONTEXT_H
#include <stdbool.h>
#include <stdint.h>
#include "auth/auth_v3_iot_core_authz.h"
#include "auth/lineage_replace_authorization.h"
#include "auth/lineage_replace_possession.h"
#include "auth/lineage_replace_session_binding.h"
typedef struct {
    bool current_credential_control_valid;
    bool successor_key_control_valid;
    uint8_t expected_peer_identity[32];
    uint8_t predecessor_credential_reference[32];
    uint64_t requested_successor_scope_bits;
} lineage_replace_bound_auth_context_evidence_t;
typedef struct {
    lineage_replace_verified_lifecycle_possession_proof_t current_credential_proof;
    lineage_replace_verified_lifecycle_possession_proof_t successor_key_proof;
    uint8_t successor_key_reference[32];
    uint8_t expected_peer_identity[32];
    uint8_t predecessor_credential_reference[32];
    uint64_t requested_successor_scope_bits;
} lineage_replace_verified_bound_auth_context_evidence_t;
lineage_replace_authorization_decision_t lineage_replace_authorization_from_bound_iot_core(
    const auth_v3_context_t *auth_context,
    const lineage_replace_session_binding_expectation_t *session_expectation,
    const auth_v3_iot_core_authorization_context_v1_t *authorization_context,
    const auth_v3_iot_core_attribution_record_v1_t *attribution,
    const lineage_replace_bound_auth_context_evidence_t *evidence);
lineage_replace_authorization_decision_t lineage_replace_authorization_from_verified_bound_iot_core(
    const auth_v3_context_t *auth_context,
    const lineage_replace_session_binding_expectation_t *session_expectation,
    const auth_v3_iot_core_authorization_context_v1_t *authorization_context,
    const auth_v3_iot_core_attribution_record_v1_t *attribution,
    const lineage_replace_verified_bound_auth_context_evidence_t *evidence);
#endif
