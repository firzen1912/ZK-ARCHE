#ifndef AUTH_LINEAGE_REPLACE_SESSION_BINDING_H
#define AUTH_LINEAGE_REPLACE_SESSION_BINDING_H

#include <stdbool.h>
#include <stdint.h>
#include "auth/auth_v3.h"

typedef enum {
    LINEAGE_REPLACE_SESSION_BOUND = 0,
    LINEAGE_REPLACE_SESSION_REJECT_COMPLETION,
    LINEAGE_REPLACE_SESSION_REJECT_VERSION,
    LINEAGE_REPLACE_SESSION_REJECT_SUITE,
    LINEAGE_REPLACE_SESSION_REJECT_PROFILE,
    LINEAGE_REPLACE_SESSION_REJECT_SESSION_ID,
    LINEAGE_REPLACE_SESSION_REJECT_AUTHZ_CONTEXT,
    LINEAGE_REPLACE_SESSION_REJECT_CHANNEL_BINDING
} lineage_replace_session_binding_decision_t;

typedef struct {
    bool auth_completion_verified;
    uint8_t expected_protocol_version;
    auth_suite_t expected_suite_id;
    uint16_t expected_profile_id;
    uint8_t expected_session_id[AUTH_SESSION_ID_LEN];
    uint8_t expected_authz_context_hash[AUTH_HASH_LEN];
    uint8_t expected_channel_binding_hash[AUTH_HASH_LEN];
} lineage_replace_session_binding_expectation_t;

lineage_replace_session_binding_decision_t lineage_replace_classify_session_binding(
    const auth_v3_context_t *context,
    const lineage_replace_session_binding_expectation_t *expectation);

#endif
