#include "auth/data_release_authorization.h"

#define RET(a, r) ((data_release_decision_t){(a), (r)})

data_release_decision_t data_release_authorization_classify(const data_release_facts_t *f) {
    if (!f) return RET(DATA_RELEASE_ACTION_DENY, DATA_RELEASE_REASON_AUTHORIZATION_MISSING);
    if (f->rollback_suspected) return RET(DATA_RELEASE_ACTION_DENY, DATA_RELEASE_REASON_ROLLBACK_SUSPECTED);
    if (!f->authenticated) return RET(DATA_RELEASE_ACTION_FRESH_AUTH_REQUIRED, DATA_RELEASE_REASON_UNAUTHENTICATED);
    if (!f->device_release_authority_present) return RET(DATA_RELEASE_ACTION_DENY, DATA_RELEASE_REASON_DEVICE_RELEASE_AUTHORITY_MISSING);
    if (!f->device_release_authority_current) return RET(DATA_RELEASE_ACTION_DENY, DATA_RELEASE_REASON_DEVICE_RELEASE_AUTHORITY_STALE);
    if (!f->protected_data_encrypted) return RET(DATA_RELEASE_ACTION_DENY, DATA_RELEASE_REASON_PROTECTED_DATA_NOT_ENCRYPTED);
    if (!f->release_key_scope_match) return RET(DATA_RELEASE_ACTION_DENY, DATA_RELEASE_REASON_RELEASE_KEY_SCOPE_MISMATCH);
    if (!f->authorization_present) return RET(DATA_RELEASE_ACTION_DENY, DATA_RELEASE_REASON_AUTHORIZATION_MISSING);
    if (!f->authorization_fresh) return RET(DATA_RELEASE_ACTION_DENY, DATA_RELEASE_REASON_AUTHORIZATION_STALE);
    if (!f->authorization_generation_current) return RET(DATA_RELEASE_ACTION_DENY, DATA_RELEASE_REASON_AUTHORIZATION_GENERATION_STALE);
    if (!f->revocation_current) return RET(DATA_RELEASE_ACTION_DENY, DATA_RELEASE_REASON_REVOCATION_STALE);
    if (f->explicitly_revoked) return RET(DATA_RELEASE_ACTION_DENY, DATA_RELEASE_REASON_REVOKED);
    if (!f->lineage_current) return RET(DATA_RELEASE_ACTION_DENY, DATA_RELEASE_REASON_LINEAGE_STALE);
    if (!f->holder_match) return RET(DATA_RELEASE_ACTION_DENY, DATA_RELEASE_REASON_HOLDER_MISMATCH);
    if (!f->audience_match) return RET(DATA_RELEASE_ACTION_DENY, DATA_RELEASE_REASON_AUDIENCE_MISMATCH);
    if (!f->purpose_match) return RET(DATA_RELEASE_ACTION_DENY, DATA_RELEASE_REASON_PURPOSE_MISMATCH);
    if (!f->data_type_match) return RET(DATA_RELEASE_ACTION_DENY, DATA_RELEASE_REASON_DATA_TYPE_MISMATCH);
    if (!f->policy_match) return RET(DATA_RELEASE_ACTION_DENY, DATA_RELEASE_REASON_POLICY_MISMATCH);
    if (!f->epoch_match) return RET(DATA_RELEASE_ACTION_DENY, DATA_RELEASE_REASON_EPOCH_MISMATCH);
    if (f->channel_binding_required && !f->channel_binding_valid) return RET(DATA_RELEASE_ACTION_DENY, DATA_RELEASE_REASON_CHANNEL_BINDING_MISSING_OR_INVALID);
    if (!f->release_operation_unused) return RET(DATA_RELEASE_ACTION_DENY, DATA_RELEASE_REASON_RELEASE_REPLAY_DETECTED);
    return RET(DATA_RELEASE_ACTION_RELEASE, DATA_RELEASE_REASON_CURRENT);
}
