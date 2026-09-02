#ifndef AUTH_DATA_RELEASE_AUTHORIZATION_H
#define AUTH_DATA_RELEASE_AUTHORIZATION_H
#include <stdbool.h>
typedef struct {
    bool authenticated;
    bool device_release_authority_present;
    bool device_release_authority_current;
    bool protected_data_encrypted;
    bool release_key_scope_match;
    bool authorization_present;
    bool authorization_fresh;
    bool revocation_current;
    bool explicitly_revoked;
    bool lineage_current;
    bool holder_match;
    bool audience_match;
    bool purpose_match;
    bool data_type_match;
    bool policy_match;
    bool epoch_match;
    bool channel_binding_required;
    bool channel_binding_valid;
    bool rollback_suspected;
} data_release_facts_t;
typedef enum { DATA_RELEASE_ACTION_RELEASE=0, DATA_RELEASE_ACTION_FRESH_AUTH_REQUIRED=1, DATA_RELEASE_ACTION_DENY=2 } data_release_action_t;
typedef enum {
    DATA_RELEASE_REASON_CURRENT=0, DATA_RELEASE_REASON_ROLLBACK_SUSPECTED,
    DATA_RELEASE_REASON_UNAUTHENTICATED, DATA_RELEASE_REASON_DEVICE_RELEASE_AUTHORITY_MISSING,
    DATA_RELEASE_REASON_DEVICE_RELEASE_AUTHORITY_STALE, DATA_RELEASE_REASON_PROTECTED_DATA_NOT_ENCRYPTED,
    DATA_RELEASE_REASON_RELEASE_KEY_SCOPE_MISMATCH, DATA_RELEASE_REASON_AUTHORIZATION_MISSING,
    DATA_RELEASE_REASON_AUTHORIZATION_STALE, DATA_RELEASE_REASON_REVOCATION_STALE,
    DATA_RELEASE_REASON_REVOKED, DATA_RELEASE_REASON_LINEAGE_STALE,
    DATA_RELEASE_REASON_HOLDER_MISMATCH, DATA_RELEASE_REASON_AUDIENCE_MISMATCH,
    DATA_RELEASE_REASON_PURPOSE_MISMATCH, DATA_RELEASE_REASON_DATA_TYPE_MISMATCH,
    DATA_RELEASE_REASON_POLICY_MISMATCH, DATA_RELEASE_REASON_EPOCH_MISMATCH,
    DATA_RELEASE_REASON_CHANNEL_BINDING_MISSING_OR_INVALID
} data_release_reason_t;
typedef struct { data_release_action_t action; data_release_reason_t reason; } data_release_decision_t;
data_release_decision_t data_release_authorization_classify(const data_release_facts_t *facts);
#endif
