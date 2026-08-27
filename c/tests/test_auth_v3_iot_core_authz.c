#include "auth/auth_v3_iot_core_authz.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VECTOR_PATH "../rust/test-vectors/auth-v3/iot-core-authorization-v1.txt"

static int read_vector_value(const char *key, char *out, size_t out_capacity) {
    FILE *fp = fopen(VECTOR_PATH, "r");
    char line[1024];
    size_t key_len = strlen(key);
    if (fp == NULL) {
        return -1;
    }
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (strncmp(line, key, key_len) == 0 && line[key_len] == '=') {
            size_t len = strcspn(line + key_len + 1u, "\r\n");
            if (len + 1u > out_capacity) {
                fclose(fp);
                return -1;
            }
            memcpy(out, line + key_len + 1u, len);
            out[len] = '\0';
            fclose(fp);
            return 0;
        }
    }
    fclose(fp);
    return -1;
}

static uint8_t hex_nibble(char c) {
    if (c >= '0' && c <= '9') {
        return (uint8_t)(c - '0');
    }
    if (c >= 'a' && c <= 'f') {
        return (uint8_t)(10 + (c - 'a'));
    }
    if (c >= 'A' && c <= 'F') {
        return (uint8_t)(10 + (c - 'A'));
    }
    abort();
}

static void decode_hex_exact(const char *hex, uint8_t *out, size_t out_len) {
    size_t i;
    assert(strlen(hex) == out_len * 2u);
    for (i = 0u; i < out_len; ++i) {
        out[i] = (uint8_t)((hex_nibble(hex[i * 2u]) << 4) |
                           hex_nibble(hex[i * 2u + 1u]));
    }
}

static uint64_t read_u64(const char *key) {
    char value[64];
    char *end = NULL;
    unsigned long long parsed;
    assert(read_vector_value(key, value, sizeof(value)) == 0);
    parsed = strtoull(value, &end, 10);
    assert(end != NULL && *end == '\0');
    return (uint64_t)parsed;
}

static auth_v3_iot_core_authorization_context_v1_t fixture(void) {
    auth_v3_iot_core_authorization_context_v1_t context;
    char value[256];
    memset(&context, 0, sizeof(context));

    assert(read_vector_value("holder_binding", value, sizeof(value)) == 0);
    decode_hex_exact(value, context.holder_binding, sizeof(context.holder_binding));
    assert(read_vector_value("audience_id", value, sizeof(value)) == 0);
    decode_hex_exact(value, context.audience_id, sizeof(context.audience_id));

    context.role_policy_id = read_u64("role_policy_id");
    context.scope_bits = read_u64("scope_bits");
    context.authorization_generation = read_u64("authorization_generation");
    context.policy_epoch = read_u64("policy_epoch");
    context.revocation_epoch = read_u64("revocation_epoch");
    return context;
}

static void test_shared_vector(void) {
    auth_v3_iot_core_authorization_context_v1_t context = fixture();
    uint8_t encoded[AUTH_V3_IOT_CORE_AUTHZ_CANONICAL_LEN];
    uint8_t expected_encoded[AUTH_V3_IOT_CORE_AUTHZ_CANONICAL_LEN];
    uint8_t hash[32];
    uint8_t expected_hash[32];
    char value[512];
    size_t encoded_len = 0u;

    assert(auth_v3_iot_core_authz_encode(&context, encoded, sizeof(encoded),
                                         &encoded_len) == AUTH_V3_IOT_CORE_AUTHZ_OK);
    assert(encoded_len == AUTH_V3_IOT_CORE_AUTHZ_CANONICAL_LEN);
    assert(read_vector_value("encoded", value, sizeof(value)) == 0);
    decode_hex_exact(value, expected_encoded, sizeof(expected_encoded));
    assert(memcmp(encoded, expected_encoded, sizeof(encoded)) == 0);

    assert(auth_v3_iot_core_authz_hash(&context, hash) == AUTH_V3_IOT_CORE_AUTHZ_OK);
    assert(read_vector_value("sha256", value, sizeof(value)) == 0);
    decode_hex_exact(value, expected_hash, sizeof(expected_hash));
    assert(memcmp(hash, expected_hash, sizeof(hash)) == 0);
}

static void test_semantic_rejections(void) {
    auth_v3_iot_core_authorization_context_v1_t base = fixture();
    auth_v3_iot_core_authorization_context_v1_t case_context;

    case_context = base;
    memset(case_context.holder_binding, 0, sizeof(case_context.holder_binding));
    assert(auth_v3_iot_core_authz_validate(&case_context) == AUTH_V3_IOT_CORE_AUTHZ_INVALID_HOLDER);

    case_context = base;
    memset(case_context.audience_id, 0, sizeof(case_context.audience_id));
    assert(auth_v3_iot_core_authz_validate(&case_context) == AUTH_V3_IOT_CORE_AUTHZ_INVALID_AUDIENCE);

    case_context = base;
    case_context.role_policy_id = 0u;
    assert(auth_v3_iot_core_authz_validate(&case_context) == AUTH_V3_IOT_CORE_AUTHZ_INVALID_ROLE_POLICY);

    case_context = base;
    case_context.scope_bits = 0u;
    assert(auth_v3_iot_core_authz_validate(&case_context) == AUTH_V3_IOT_CORE_AUTHZ_INVALID_SCOPE);
    case_context.scope_bits = 2u;
    assert(auth_v3_iot_core_authz_validate(&case_context) == AUTH_V3_IOT_CORE_AUTHZ_INVALID_SCOPE);

    case_context = base;
    case_context.authorization_generation = 0u;
    assert(auth_v3_iot_core_authz_validate(&case_context) == AUTH_V3_IOT_CORE_AUTHZ_INVALID_GENERATION);

    case_context = base;
    case_context.policy_epoch = 0u;
    assert(auth_v3_iot_core_authz_validate(&case_context) == AUTH_V3_IOT_CORE_AUTHZ_INVALID_POLICY_EPOCH);

    case_context = base;
    case_context.revocation_epoch = 0u;
    assert(auth_v3_iot_core_authz_validate(&case_context) == AUTH_V3_IOT_CORE_AUTHZ_INVALID_REVOCATION_EPOCH);
}

int main(void) {
    test_shared_vector();
    test_semantic_rejections();
    puts("AUTH v3 iot-core authorization context: ok");
    return 0;
}
