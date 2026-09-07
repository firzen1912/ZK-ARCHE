#include "auth/auth_v3_iot_core_authz.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

static auth_v3_iot_core_authorization_context_v1_t valid_context(void) {
    auth_v3_iot_core_authorization_context_v1_t context;
    memset(&context, 0, sizeof(context));
    memset(context.holder_binding, 0x11, sizeof(context.holder_binding));
    memset(context.audience_id, 0x22, sizeof(context.audience_id));
    context.role_policy_id = 7u;
    context.scope_bits = 1u;
    context.authorization_generation = 9u;
    context.policy_epoch = 11u;
    context.revocation_epoch = 13u;
    return context;
}

static auth_v3_iot_core_attribution_record_v1_t matching_record(
    const auth_v3_iot_core_authorization_context_v1_t *context) {
    auth_v3_iot_core_attribution_record_v1_t record;
    memset(&record, 0, sizeof(record));
    memset(record.credential_reference, 0xa1, sizeof(record.credential_reference));
    memset(record.peer_identity, 0xb1, sizeof(record.peer_identity));
    memcpy(record.holder_binding, context->holder_binding, sizeof(record.holder_binding));
    memcpy(record.audience_id, context->audience_id, sizeof(record.audience_id));
    record.role_policy_id = context->role_policy_id;
    record.scope_bits = context->scope_bits;
    record.authorization_generation = context->authorization_generation;
    record.policy_epoch = context->policy_epoch;
    record.revocation_epoch = context->revocation_epoch;
    return record;
}

static void assert_matching_invalid_context_rejected(
    const auth_v3_iot_core_authorization_context_v1_t *context) {
    auth_v3_iot_core_attribution_record_v1_t record = matching_record(context);
    const auth_v3_iot_core_attribution_record_v1_t *resolved = NULL;

    assert(auth_v3_iot_core_attribution_resolve(
               &record, 1u, record.credential_reference, record.peer_identity, context, &resolved) ==
           AUTH_V3_IOT_CORE_ATTRIBUTION_AUTHORIZATION_MISMATCH);
    assert(resolved == NULL);
}

int main(void) {
    auth_v3_iot_core_authorization_context_v1_t context;

    context = valid_context();
    memset(context.holder_binding, 0, sizeof(context.holder_binding));
    assert_matching_invalid_context_rejected(&context);

    context = valid_context();
    memset(context.audience_id, 0, sizeof(context.audience_id));
    assert_matching_invalid_context_rejected(&context);

    context = valid_context();
    context.role_policy_id = 0u;
    assert_matching_invalid_context_rejected(&context);

    context = valid_context();
    context.scope_bits = 0u;
    assert_matching_invalid_context_rejected(&context);

    context = valid_context();
    context.authorization_generation = 0u;
    assert_matching_invalid_context_rejected(&context);

    context = valid_context();
    context.policy_epoch = 0u;
    assert_matching_invalid_context_rejected(&context);

    context = valid_context();
    context.revocation_epoch = 0u;
    assert_matching_invalid_context_rejected(&context);

    return 0;
}
