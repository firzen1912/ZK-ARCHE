#ifndef ZK_ARCHE_AUTH_V3_IOT_CORE_AUTHZ_H
#define ZK_ARCHE_AUTH_V3_IOT_CORE_AUTHZ_H

#include <stddef.h>
#include <stdint.h>

#define AUTH_V3_IOT_CORE_AUTHZ_HOLDER_BINDING_ID 0x0001u
#define AUTH_V3_IOT_CORE_AUTHZ_AUDIENCE_ID 0x0002u
#define AUTH_V3_IOT_CORE_AUTHZ_ROLE_POLICY_ID 0x0003u
#define AUTH_V3_IOT_CORE_AUTHZ_SCOPE_BITS_ID 0x0004u
#define AUTH_V3_IOT_CORE_AUTHZ_GENERATION_ID 0x0005u
#define AUTH_V3_IOT_CORE_AUTHZ_POLICY_EPOCH_ID 0x0006u
#define AUTH_V3_IOT_CORE_AUTHZ_REVOCATION_EPOCH_ID 0x0007u

#define AUTH_V3_IOT_CORE_SCOPE_SECURE_ASSOCIATION 1u
#define AUTH_V3_IOT_CORE_AUTHZ_ENTRY_COUNT 7u
#define AUTH_V3_IOT_CORE_AUTHZ_CANONICAL_LEN 148u

typedef struct auth_v3_iot_core_authorization_context_v1 {
    uint8_t holder_binding[32];
    uint8_t audience_id[32];
    uint64_t role_policy_id;
    uint64_t scope_bits;
    uint64_t authorization_generation;
    uint64_t policy_epoch;
    uint64_t revocation_epoch;
} auth_v3_iot_core_authorization_context_v1_t;

typedef enum auth_v3_iot_core_authz_result {
    AUTH_V3_IOT_CORE_AUTHZ_OK = 0,
    AUTH_V3_IOT_CORE_AUTHZ_INVALID_ARGUMENT = -100,
    AUTH_V3_IOT_CORE_AUTHZ_INVALID_HOLDER = -101,
    AUTH_V3_IOT_CORE_AUTHZ_INVALID_AUDIENCE = -102,
    AUTH_V3_IOT_CORE_AUTHZ_INVALID_ROLE_POLICY = -103,
    AUTH_V3_IOT_CORE_AUTHZ_INVALID_SCOPE = -104,
    AUTH_V3_IOT_CORE_AUTHZ_INVALID_GENERATION = -105,
    AUTH_V3_IOT_CORE_AUTHZ_INVALID_POLICY_EPOCH = -106,
    AUTH_V3_IOT_CORE_AUTHZ_INVALID_REVOCATION_EPOCH = -107,
    AUTH_V3_IOT_CORE_AUTHZ_INVALID_ENCODING_LENGTH = -108,
    AUTH_V3_IOT_CORE_AUTHZ_INVALID_CONTEXT_KIND = -109,
    AUTH_V3_IOT_CORE_AUTHZ_INVALID_ENTRY_SCHEMA = -110,
    AUTH_V3_IOT_CORE_AUTHZ_ENTRY_LIMIT_EXCEEDED = -111,
    AUTH_V3_IOT_CORE_AUTHZ_INVALID_ENCODING = -112
} auth_v3_iot_core_authz_result_t;

int auth_v3_iot_core_authz_validate(
    const auth_v3_iot_core_authorization_context_v1_t *context);

int auth_v3_iot_core_authz_encode(
    const auth_v3_iot_core_authorization_context_v1_t *context,
    uint8_t *out,
    size_t out_capacity,
    size_t *out_len);

int auth_v3_iot_core_authz_hash(
    const auth_v3_iot_core_authorization_context_v1_t *context,
    uint8_t out_hash[32]);

int auth_v3_iot_core_authz_decode_bytes(
    const uint8_t *input,
    size_t input_len,
    auth_v3_iot_core_authorization_context_v1_t *context_out);

int auth_v3_iot_core_authz_hash_bytes(
    const uint8_t *input,
    size_t input_len,
    uint8_t out_hash[32]);

#endif
