#include "auth/auth_v3_context.h"

#include <sodium.h>
#include <string.h>

static const uint8_t CONTEXT_MAGIC[AUTH_V3_CONTEXT_MAGIC_LEN] = {'Z','K','C','T','X'};

static int validate_entries(auth_v3_context_kind_t kind,
                            const auth_v3_context_entry_t *entries,
                            size_t entry_count,
                            size_t *encoded_len) {
    size_t total = 9u;
    uint16_t previous = 0u;
    size_t i;

    if (encoded_len == NULL) {
        return AUTH_V3_CONTEXT_INVALID_ARGUMENT;
    }
    if (entry_count > UINT16_MAX) {
        return AUTH_V3_CONTEXT_INVALID_ARGUMENT;
    }
    if (entry_count > 0u && entries == NULL) {
        return AUTH_V3_CONTEXT_INVALID_ARGUMENT;
    }
    if (kind != AUTH_V3_CONTEXT_AUTHORIZATION &&
        kind != AUTH_V3_CONTEXT_CRITICAL_EXTENSIONS &&
        kind != AUTH_V3_CONTEXT_CHANNEL_BINDING) {
        return AUTH_V3_CONTEXT_INVALID_ARGUMENT;
    }

    for (i = 0u; i < entry_count; ++i) {
        const auth_v3_context_entry_t *entry = &entries[i];
        if (entry->id == 0u) {
            return AUTH_V3_CONTEXT_INVALID_ID;
        }
        if (i > 0u && entry->id <= previous) {
            return AUTH_V3_CONTEXT_NON_CANONICAL_ORDER;
        }
        if (kind == AUTH_V3_CONTEXT_CRITICAL_EXTENSIONS &&
            (((entry->id & 0x8000u) == 0u) || ((entry->id & 0x7fffu) == 0u))) {
            return AUTH_V3_CONTEXT_INVALID_CRITICAL_ID;
        }
        if (entry->value_len > 0u && entry->value == NULL) {
            return AUTH_V3_CONTEXT_INVALID_ARGUMENT;
        }
        if (total > SIZE_MAX - 5u - (size_t)entry->value_len) {
            return AUTH_V3_CONTEXT_INVALID_ARGUMENT;
        }
        total += 5u + (size_t)entry->value_len;
        previous = entry->id;
    }

    *encoded_len = total;
    return AUTH_V3_CONTEXT_OK;
}

int auth_v3_context_encode(auth_v3_context_kind_t kind,
                           const auth_v3_context_entry_t *entries,
                           size_t entry_count,
                           uint8_t *out,
                           size_t out_capacity,
                           size_t *out_len) {
    size_t needed = 0u;
    size_t offset = 0u;
    size_t i;
    int rc = validate_entries(kind, entries, entry_count, &needed);

    if (rc != AUTH_V3_CONTEXT_OK) {
        return rc;
    }
    if (out == NULL || out_len == NULL) {
        return AUTH_V3_CONTEXT_INVALID_ARGUMENT;
    }
    if (out_capacity < needed) {
        return AUTH_V3_CONTEXT_BUFFER_TOO_SMALL;
    }

    memcpy(out + offset, CONTEXT_MAGIC, AUTH_V3_CONTEXT_MAGIC_LEN);
    offset += AUTH_V3_CONTEXT_MAGIC_LEN;
    out[offset++] = AUTH_V3_CONTEXT_ENCODING_VERSION;
    out[offset++] = (uint8_t)kind;
    out[offset++] = (uint8_t)(entry_count & 0xffu);
    out[offset++] = (uint8_t)((entry_count >> 8) & 0xffu);

    for (i = 0u; i < entry_count; ++i) {
        const auth_v3_context_entry_t *entry = &entries[i];
        out[offset++] = (uint8_t)(entry->id & 0xffu);
        out[offset++] = (uint8_t)((entry->id >> 8) & 0xffu);
        out[offset++] = 0u;
        out[offset++] = (uint8_t)(entry->value_len & 0xffu);
        out[offset++] = (uint8_t)((entry->value_len >> 8) & 0xffu);
        if (entry->value_len > 0u) {
            memcpy(out + offset, entry->value, entry->value_len);
            offset += entry->value_len;
        }
    }

    *out_len = offset;
    return AUTH_V3_CONTEXT_OK;
}

int auth_v3_context_hash(auth_v3_context_kind_t kind,
                         const auth_v3_context_entry_t *entries,
                         size_t entry_count,
                         uint8_t out_hash[32]) {
    crypto_hash_sha256_state state;
    uint8_t header[9];
    size_t i;
    size_t ignored_len = 0u;
    int rc = validate_entries(kind, entries, entry_count, &ignored_len);

    if (rc != AUTH_V3_CONTEXT_OK) {
        return rc;
    }
    if (out_hash == NULL) {
        return AUTH_V3_CONTEXT_INVALID_ARGUMENT;
    }

    memcpy(header, CONTEXT_MAGIC, AUTH_V3_CONTEXT_MAGIC_LEN);
    header[5] = AUTH_V3_CONTEXT_ENCODING_VERSION;
    header[6] = (uint8_t)kind;
    header[7] = (uint8_t)(entry_count & 0xffu);
    header[8] = (uint8_t)((entry_count >> 8) & 0xffu);

    crypto_hash_sha256_init(&state);
    crypto_hash_sha256_update(&state, header, sizeof(header));
    for (i = 0u; i < entry_count; ++i) {
        const auth_v3_context_entry_t *entry = &entries[i];
        uint8_t entry_header[5];
        entry_header[0] = (uint8_t)(entry->id & 0xffu);
        entry_header[1] = (uint8_t)((entry->id >> 8) & 0xffu);
        entry_header[2] = 0u;
        entry_header[3] = (uint8_t)(entry->value_len & 0xffu);
        entry_header[4] = (uint8_t)((entry->value_len >> 8) & 0xffu);
        crypto_hash_sha256_update(&state, entry_header, sizeof(entry_header));
        if (entry->value_len > 0u) {
            crypto_hash_sha256_update(&state, entry->value, entry->value_len);
        }
    }
    crypto_hash_sha256_final(&state, out_hash);
    return AUTH_V3_CONTEXT_OK;
}
