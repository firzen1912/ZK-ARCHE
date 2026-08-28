#include "auth/auth_v3_iot_core_authz.h"

#include "auth/auth_v3_context.h"
#include "auth/auth_v3_context_parser.h"

#include <sodium.h>
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

static uint64_t load_u64_le(const uint8_t in[8]) {
    size_t i;
    uint64_t value = 0u;
    for (i = 0u; i < 8u; ++i) {
        value |= ((uint64_t)in[i]) << (8u * i);
    }
    return value;
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

int auth_v3_iot_core_authz_decode_bytes(
    const uint8_t *input,
    size_t input_len,
    auth_v3_iot_core_authorization_context_v1_t *context_out) {
    static const uint16_t expected_ids[AUTH_V3_IOT_CORE_AUTHZ_ENTRY_COUNT] = {
        AUTH_V3_IOT_CORE_AUTHZ_HOLDER_BINDING_ID,
        AUTH_V3_IOT_CORE_AUTHZ_AUDIENCE_ID,
        AUTH_V3_IOT_CORE_AUTHZ_ROLE_POLICY_ID,
        AUTH_V3_IOT_CORE_AUTHZ_SCOPE_BITS_ID,
        AUTH_V3_IOT_CORE_AUTHZ_GENERATION_ID,
        AUTH_V3_IOT_CORE_AUTHZ_POLICY_EPOCH_ID,
        AUTH_V3_IOT_CORE_AUTHZ_REVOCATION_EPOCH_ID
    };
    static const size_t expected_lengths[AUTH_V3_IOT_CORE_AUTHZ_ENTRY_COUNT] = {
        32u, 32u, 8u, 8u, 8u, 8u, 8u
    };
    auth_v3_context_entry_t entries[AUTH_V3_IOT_CORE_AUTHZ_ENTRY_COUNT];
    auth_v3_context_kind_t kind;
    size_t entry_count = 0u;
    size_t i;
    int rc;

    if (input == NULL || context_out == NULL) {
        return AUTH_V3_IOT_CORE_AUTHZ_INVALID_ARGUMENT;
    }
    if (input_len != AUTH_V3_IOT_CORE_AUTHZ_CANONICAL_LEN) {
        return AUTH_V3_IOT_CORE_AUTHZ_INVALID_ENCODING_LENGTH;
    }

    rc = auth_v3_context_parse_bytes(input, input_len, &kind, entries,
                                     AUTH_V3_IOT_CORE_AUTHZ_ENTRY_COUNT,
                                     &entry_count);
    if (rc == AUTH_V3_CONTEXT_PARSE_ENTRY_BUFFER_TOO_SMALL) {
        return AUTH_V3_IOT_CORE_AUTHZ_ENTRY_LIMIT_EXCEEDED;
    }
    if (rc != AUTH_V3_CONTEXT_PARSE_OK) {
        return AUTH_V3_IOT_CORE_AUTHZ_INVALID_ENCODING;
    }
    if (kind != AUTH_V3_CONTEXT_AUTHORIZATION) {
        return AUTH_V3_IOT_CORE_AUTHZ_INVALID_CONTEXT_KIND;
    }
    if (entry_count != AUTH_V3_IOT_CORE_AUTHZ_ENTRY_COUNT) {
        return AUTH_V3_IOT_CORE_AUTHZ_INVALID_ENTRY_SCHEMA;
    }
    for (i = 0u; i < AUTH_V3_IOT_CORE_AUTHZ_ENTRY_COUNT; ++i) {
        if (entries[i].id != expected_ids[i] ||
            entries[i].value_len != expected_lengths[i]) {
            return AUTH_V3_IOT_CORE_AUTHZ_INVALID_ENTRY_SCHEMA;
        }
    }

    memset(context_out, 0, sizeof(*context_out));
    memcpy(context_out->holder_binding, entries[0].value,
           sizeof(context_out->holder_binding));
    memcpy(context_out->audience_id, entries[1].value,
           sizeof(context_out->audience_id));
    context_out->role_policy_id = load_u64_le(entries[2].value);
    context_out->scope_bits = load_u64_le(entries[3].value);
    context_out->authorization_generation = load_u64_le(entries[4].value);
    context_out->policy_epoch = load_u64_le(entries[5].value);
    context_out->revocation_epoch = load_u64_le(entries[6].value);

    return auth_v3_iot_core_authz_validate(context_out);
}

int auth_v3_iot_core_authz_hash_bytes(
    const uint8_t *input,
    size_t input_len,
    uint8_t out_hash[32]) {
    auth_v3_iot_core_authorization_context_v1_t context;
    int rc;

    if (out_hash == NULL) {
        return AUTH_V3_IOT_CORE_AUTHZ_INVALID_ARGUMENT;
    }
    rc = auth_v3_iot_core_authz_decode_bytes(input, input_len, &context);
    if (rc != AUTH_V3_IOT_CORE_AUTHZ_OK) {
        return rc;
    }
    if (crypto_hash_sha256(out_hash, input, (unsigned long long)input_len) != 0) {
        return AUTH_V3_IOT_CORE_AUTHZ_INVALID_ARGUMENT;
    }
    return AUTH_V3_IOT_CORE_AUTHZ_OK;
}
