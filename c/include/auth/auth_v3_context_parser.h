#ifndef ZK_ARCHE_AUTH_V3_CONTEXT_PARSER_H
#define ZK_ARCHE_AUTH_V3_CONTEXT_PARSER_H

#include "auth/auth_v3_context.h"

#include <stddef.h>
#include <stdint.h>

typedef enum auth_v3_context_parse_result {
    AUTH_V3_CONTEXT_PARSE_OK = 0,
    AUTH_V3_CONTEXT_PARSE_INVALID_ARGUMENT = -20,
    AUTH_V3_CONTEXT_PARSE_TRUNCATED = -21,
    AUTH_V3_CONTEXT_PARSE_INVALID_MAGIC = -22,
    AUTH_V3_CONTEXT_PARSE_UNSUPPORTED_VERSION = -23,
    AUTH_V3_CONTEXT_PARSE_UNKNOWN_KIND = -24,
    AUTH_V3_CONTEXT_PARSE_INVALID_ID = -25,
    AUTH_V3_CONTEXT_PARSE_NON_CANONICAL_ORDER = -26,
    AUTH_V3_CONTEXT_PARSE_INVALID_CRITICAL_ID = -27,
    AUTH_V3_CONTEXT_PARSE_NONZERO_FLAGS = -28,
    AUTH_V3_CONTEXT_PARSE_TRAILING_BYTES = -29,
    AUTH_V3_CONTEXT_PARSE_ENTRY_BUFFER_TOO_SMALL = -30
} auth_v3_context_parse_result_t;

/*
 * Parse one byte-exact ZKCTX v1 value.
 *
 * Parsed entry values point into input and remain valid only while input remains
 * alive. The parser never sorts, deduplicates, or rewrites attacker-controlled
 * bytes. entries_out may be NULL only when the encoded entry count is zero.
 */
int auth_v3_context_parse_bytes(
    const uint8_t *input,
    size_t input_len,
    auth_v3_context_kind_t *kind_out,
    auth_v3_context_entry_t *entries_out,
    size_t entries_capacity,
    size_t *entry_count_out);

/* Validate the raw representation, then hash the exact canonical bytes. */
int auth_v3_context_hash_bytes(
    const uint8_t *input,
    size_t input_len,
    uint8_t out_hash[32]);

#endif