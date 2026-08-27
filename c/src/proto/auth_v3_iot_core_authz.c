#include "auth/auth_v3_iot_core_authz.h"

#include "auth/auth_v3_context.h"

#include <string.h>

static int all_zero(const uint8_t *value, size_t len) {
    size_t i;
    uint8_t acc = 0u;
    for (i = 0u; i < len; ++i) {
        acc |= value[i];
    }
    return acc == 0u;
}

static void store_u64_le(uint8_t out[8], uint64_t value) {
    size_t i;
    for (i = 0u; i < 8u; ++i) {
        out[i] = (uint8_t)((value >> (8u * i)) & 0xffu);
    }
}

int auth_v3_iot_core_authz_validate(
    const auth_v3_iot_core_authorization_context_v1_t *context) {
    if (context == NULL) {
        return AUTH_V3_IOT_CORE_AUTHZ_INVALID_ARGUMENT;
    }
    if (all_zero(context->holder_binding, sizeof(context->holder_binding))) {
        return AUTH_V3_IOT_CORE_AUTHZ_INVALID_HOLDER;
    }
    if (all_zero(context->audience_id, sizeof(context->audience_id))) {
        return AUTH_V3_IOT_CORE_AUTHZ_INVALID_AUDIENCE;
    }
    if (context->role_policy_id == 0u) {
        return AUTH_V3_IOT_CORE_AUTHZ_INVALID_ROLE_POLICY;
    }
    if (context->scope_bits != AUTH_V3_IOT_CORE_SCOPE_SECURE_ASSOCIATION) {
        return AUTH_V3_IOT_CORE_AUTHZ_INVALID_SCOPE;
    }
    if (context->authorization_generation == 0u) {
        return AUTH_V3_IOT_CORE_AUTHZ_INVALID_GENERATION;
    }
    if (context->policy_epoch == 0u) {
        return AUTH_V3_IOT_CORE_AUTHZ_INVALID_POLICY_EPOCH;
    }
    if (context->revocation_epoch == 0u) {
        return AUTH_V3_IOT_CORE_AUTHZ_INVALID_REVOCATION_EPOCH;
    }
    return AUTH_V3_IOT_CORE_AUTHZ_OK;
}

static int build_entries(
    const auth_v3_iot_core_authorization_context_v1_t *context,
    auth_v3_context_entry_t entries[7],
    uint8_t role_policy_id[8],
    uint8_t scope_bits[8],
    uint8_t authorization_generation[8],
    uint8_t policy_epoch[8],
    uint8_t revocation_epoch[8]) {
    int rc = auth_v3_iot_core_authz_validate(context);
    if (rc != AUTH_V3_IOT_CORE_AUTHZ_OK) {
        return rc;
    }

    store_u64_le(role_policy_id, context->role_policy_id);
    store_u64_le(scope_bits, context->scope_bits);
    store_u64_le(authorization_generation, context->authorization_generation);
    store_u64_le(policy_epoch, context->policy_epoch);
    store_u64_le(revocation_epoch, context->revocation_epoch);

    entries[0].id = AUTH_V3_IOT_CORE_AUTHZ_HOLDER_BINDING_ID;
    entries[0].value = context->holder_binding;
    entries[0].value_len = 32u;
    entries[1].id = AUTH_V3_IOT_CORE_AUTHZ_AUDIENCE_ID;
    entries[1].value = context->audience_id;
    entries[1].value_len = 32u;
    entries[2].id = AUTH_V3_IOT_CORE_AUTHZ_ROLE_POLICY_ID;
    entries[2].value = role_policy_id;
    entries[2].value_len = 8u;
    entries[3].id = AUTH_V3_IOT_CORE_AUTHZ_SCOPE_BITS_ID;
    entries[3].value = scope_bits;
    entries[3].value_len = 8u;
    entries[4].id = AUTH_V3_IOT_CORE_AUTHZ_GENERATION_ID;
    entries[4].value = authorization_generation;
    entries[4].value_len = 8u;
    entries[5].id = AUTH_V3_IOT_CORE_AUTHZ_POLICY_EPOCH_ID;
    entries[5].value = policy_epoch;
    entries[5].value_len = 8u;
    entries[6].id = AUTH_V3_IOT_CORE_AUTHZ_REVOCATION_EPOCH_ID;
    entries[6].value = revocation_epoch;
    entries[6].value_len = 8u;

    return AUTH_V3_IOT_CORE_AUTHZ_OK;
}

int auth_v3_iot_core_authz_encode(
    const auth_v3_iot_core_authorization_context_v1_t *context,
    uint8_t *out,
    size_t out_capacity,
    size_t *out_len) {
    auth_v3_context_entry_t entries[7];
    uint8_t role_policy_id[8];
    uint8_t scope_bits[8];
    uint8_t authorization_generation[8];
    uint8_t policy_epoch[8];
    uint8_t revocation_epoch[8];
    int rc;

    if (out == NULL || out_len == NULL) {
        return AUTH_V3_IOT_CORE_AUTHZ_INVALID_ARGUMENT;
    }

    rc = build_entries(context, entries, role_policy_id, scope_bits,
                       authorization_generation, policy_epoch, revocation_epoch);
    if (rc != AUTH_V3_IOT_CORE_AUTHZ_OK) {
        return rc;
    }

    rc = auth_v3_context_encode(AUTH_V3_CONTEXT_AUTHORIZATION, entries, 7u,
                                out, out_capacity, out_len);
    if (rc != AUTH_V3_CONTEXT_OK) {
        return rc;
    }
    if (*out_len != AUTH_V3_IOT_CORE_AUTHZ_CANONICAL_LEN) {
        return AUTH_V3_IOT_CORE_AUTHZ_INVALID_ARGUMENT;
    }
    return AUTH_V3_IOT_CORE_AUTHZ_OK;
}

int auth_v3_iot_core_authz_hash(
    const auth_v3_iot_core_authorization_context_v1_t *context,
    uint8_t out_hash[32]) {
    auth_v3_context_entry_t entries[7];
    uint8_t role_policy_id[8];
    uint8_t scope_bits[8];
    uint8_t authorization_generation[8];
    uint8_t policy_epoch[8];
    uint8_t revocation_epoch[8];
    int rc;

    if (out_hash == NULL) {
        return AUTH_V3_IOT_CORE_AUTHZ_INVALID_ARGUMENT;
    }

    rc = build_entries(context, entries, role_policy_id, scope_bits,
                       authorization_generation, policy_epoch, revocation_epoch);
    if (rc != AUTH_V3_IOT_CORE_AUTHZ_OK) {
        return rc;
    }

    return auth_v3_context_hash(AUTH_V3_CONTEXT_AUTHORIZATION, entries, 7u, out_hash);
}
