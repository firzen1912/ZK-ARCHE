#include "auth/auth_v3_context_parser.h"

#include <sodium.h>
#include <string.h>

static const uint8_t CONTEXT_MAGIC[AUTH_V3_CONTEXT_MAGIC_LEN] = {'Z','K','C','T','X'};

static int parse_impl(const uint8_t *input,
                      size_t input_len,
                      auth_v3_context_kind_t *kind_out,
                      auth_v3_context_entry_t *entries_out,
                      size_t entries_capacity,
                      size_t *entry_count_out,
                      int capture_entries) {
    auth_v3_context_kind_t kind;
    uint16_t entry_count;
    uint16_t previous_id = 0u;
    size_t offset = 9u;
    size_t i;

    if (input == NULL || input_len < 9u) {
        return input == NULL ? AUTH_V3_CONTEXT_PARSE_INVALID_ARGUMENT
                             : AUTH_V3_CONTEXT_PARSE_TRUNCATED;
    }
    if (memcmp(input, CONTEXT_MAGIC, AUTH_V3_CONTEXT_MAGIC_LEN) != 0) {
        return AUTH_V3_CONTEXT_PARSE_INVALID_MAGIC;
    }
    if (input[5] != AUTH_V3_CONTEXT_ENCODING_VERSION) {
        return AUTH_V3_CONTEXT_PARSE_UNSUPPORTED_VERSION;
    }
    if (input[6] < (uint8_t)AUTH_V3_CONTEXT_AUTHORIZATION ||
        input[6] > (uint8_t)AUTH_V3_CONTEXT_CHANNEL_BINDING) {
        return AUTH_V3_CONTEXT_PARSE_UNKNOWN_KIND;
    }
    kind = (auth_v3_context_kind_t)input[6];
    entry_count = (uint16_t)((uint16_t)input[7] | ((uint16_t)input[8] << 8));

    if ((size_t)entry_count > (input_len - 9u) / 5u) {
        return AUTH_V3_CONTEXT_PARSE_TRUNCATED;
    }
    if (capture_entries != 0 && (size_t)entry_count > entries_capacity) {
        return AUTH_V3_CONTEXT_PARSE_ENTRY_BUFFER_TOO_SMALL;
    }
    if (capture_entries != 0 && entry_count > 0u && entries_out == NULL) {
        return AUTH_V3_CONTEXT_PARSE_INVALID_ARGUMENT;
    }

    for (i = 0u; i < (size_t)entry_count; ++i) {
        uint16_t id;
        uint8_t flags;
        uint16_t value_len;

        if (offset > input_len || input_len - offset < 5u) {
            return AUTH_V3_CONTEXT_PARSE_TRUNCATED;
        }

        id = (uint16_t)((uint16_t)input[offset] |
                        ((uint16_t)input[offset + 1u] << 8));
        flags = input[offset + 2u];
        value_len = (uint16_t)((uint16_t)input[offset + 3u] |
                               ((uint16_t)input[offset + 4u] << 8));
        offset += 5u;

        if (id == 0u) {
            return AUTH_V3_CONTEXT_PARSE_INVALID_ID;
        }
        if (i > 0u && id <= previous_id) {
            return AUTH_V3_CONTEXT_PARSE_NON_CANONICAL_ORDER;
        }
        if (kind == AUTH_V3_CONTEXT_CRITICAL_EXTENSIONS &&
            (((id & 0x8000u) == 0u) || ((id & 0x7fffu) == 0u))) {
            return AUTH_V3_CONTEXT_PARSE_INVALID_CRITICAL_ID;
        }
        if (flags != 0u) {
            return AUTH_V3_CONTEXT_PARSE_NONZERO_FLAGS;
        }
        if (offset > input_len || (size_t)value_len > input_len - offset) {
            return AUTH_V3_CONTEXT_PARSE_TRUNCATED;
        }

        if (capture_entries != 0) {
            entries_out[i].id = id;
            entries_out[i].value = input + offset;
            entries_out[i].value_len = value_len;
        }
        offset += (size_t)value_len;
        previous_id = id;
    }

    if (offset != input_len) {
        return AUTH_V3_CONTEXT_PARSE_TRAILING_BYTES;
    }

    if (kind_out != NULL) {
        *kind_out = kind;
    }
    if (entry_count_out != NULL) {
        *entry_count_out = (size_t)entry_count;
    }
    return AUTH_V3_CONTEXT_PARSE_OK;
}

int auth_v3_context_parse_bytes(
    const uint8_t *input,
    size_t input_len,
    auth_v3_context_kind_t *kind_out,
    auth_v3_context_entry_t *entries_out,
    size_t entries_capacity,
    size_t *entry_count_out) {
    if (kind_out == NULL || entry_count_out == NULL) {
        return AUTH_V3_CONTEXT_PARSE_INVALID_ARGUMENT;
    }
    return parse_impl(input, input_len, kind_out, entries_out,
                      entries_capacity, entry_count_out, 1);
}

int auth_v3_context_hash_bytes(
    const uint8_t *input,
    size_t input_len,
    uint8_t out_hash[32]) {
    int rc;

    if (out_hash == NULL) {
        return AUTH_V3_CONTEXT_PARSE_INVALID_ARGUMENT;
    }
    rc = parse_impl(input, input_len, NULL, NULL, 0u, NULL, 0);
    if (rc != AUTH_V3_CONTEXT_PARSE_OK) {
        return rc;
    }
    if (crypto_hash_sha256(out_hash, input, (unsigned long long)input_len) != 0) {
        return AUTH_V3_CONTEXT_PARSE_INVALID_ARGUMENT;
    }
    return AUTH_V3_CONTEXT_PARSE_OK;
}
