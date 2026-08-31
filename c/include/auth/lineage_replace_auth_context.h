#ifndef AUTH_LINEAGE_REPLACE_AUTH_CONTEXT_H
#define AUTH_LINEAGE_REPLACE_AUTH_CONTEXT_H
#include <stdbool.h>
#include <stdint.h>
#include "auth/auth_v3_iot_core_authz.h"
#include "auth/lineage_replace_authorization.h"
typedef struct {
    bool current_credential_control_valid;
    bool successor_key_control_valid;
    bool current_session_authenticated;
    uint8_t expected_peer_identity[32];
    uint8_t predecessor_credential_reference[32];
    uint64_t requested_successor_scope_bits;
} lineage_replace_auth_context_evidence_t;
lineage_replace_authorization_decision_t lineage_replace_authorization_from_iot_core(
    const auth_v3_iot_core_authorization_context_v1_t *context,
    const auth_v3_iot_core_attribution_record_v1_t *attribution,
    const lineage_replace_auth_context_evidence_t *evidence);
#endif
