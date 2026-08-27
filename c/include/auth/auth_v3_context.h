#ifndef ZK_ARCHE_AUTH_V3_CONTEXT_H
#define ZK_ARCHE_AUTH_V3_CONTEXT_H

#include <stddef.h>
#include <stdint.h>

#define AUTH_V3_CONTEXT_MAGIC_LEN 5u
#define AUTH_V3_CONTEXT_ENCODING_VERSION 1u

typedef enum auth_v3_context_kind {
    AUTH_V3_CONTEXT_AUTHORIZATION = 1,
    AUTH_V3_CONTEXT_CRITICAL_EXTENSIONS = 2,
    AUTH_V3_CONTEXT_CHANNEL_BINDING = 3
} auth_v3_context_kind_t;

typedef struct auth_v3_context_entry {
    uint16_t id;
    const uint8_t *value;
    uint16_t value_len;
} auth_v3_context_entry_t;

typedef enum auth_v3_context_result {
    AUTH_V3_CONTEXT_OK = 0,
    AUTH_V3_CONTEXT_INVALID_ARGUMENT = -1,
    AUTH_V3_CONTEXT_INVALID_ID = -2,
    AUTH_V3_CONTEXT_NON_CANONICAL_ORDER = -3,
    AUTH_V3_CONTEXT_INVALID_CRITICAL_ID = -4,
    AUTH_V3_CONTEXT_BUFFER_TOO_SMALL = -5
} auth_v3_context_result_t;

int auth_v3_context_encode(auth_v3_context_kind_t kind,
                           const auth_v3_context_entry_t *entries,
                           size_t entry_count,
                           uint8_t *out,
                           size_t out_capacity,
                           size_t *out_len);

int auth_v3_context_hash(auth_v3_context_kind_t kind,
                         const auth_v3_context_entry_t *entries,
                         size_t entry_count,
                         uint8_t out_hash[32]);

#endif
