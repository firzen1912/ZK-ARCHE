#include "auth/auth_v3_context_parser.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint8_t hex_nibble(char c) {
    if (c >= '0' && c <= '9') return (uint8_t)(c - '0');
    if (c >= 'a' && c <= 'f') return (uint8_t)(c - 'a' + 10);
    fputs("invalid test hex\n", stderr);
    abort();
}

static size_t decode_hex(const char *hex, uint8_t *out, size_t out_cap) {
    size_t len = strlen(hex);
    size_t i;
    assert((len % 2u) == 0u);
    assert(out_cap >= len / 2u);
    for (i = 0u; i < len / 2u; ++i) {
        out[i] = (uint8_t)((hex_nibble(hex[i * 2u]) << 4) |
                           hex_nibble(hex[i * 2u + 1u]));
    }
    return len / 2u;
}

static void assert_parse_error(const char *hex, int expected) {
    uint8_t input[256];
    auth_v3_context_kind_t kind = AUTH_V3_CONTEXT_AUTHORIZATION;
    auth_v3_context_entry_t entries[8];
    size_t entry_count = 0u;
    size_t input_len = decode_hex(hex, input, sizeof(input));

    assert(auth_v3_context_parse_bytes(input, input_len, &kind,
                                       entries, 8u, &entry_count) == expected);
    assert(auth_v3_context_hash_bytes(input, input_len,
                                      (uint8_t[32]){0}) == expected);
}

static void test_canonical_parse_and_hash(void) {
    static const char *encoded_hex =
        "5a4b435458010103000100000100010200000600656467652d61030000100000112233445566778899aabbccddeeff";
    static const char *hash_hex =
        "3f85c714dfca2070fcfa909bfa31442c5828f4f91834f62e140db0320fcfcb69";
    uint8_t input[256];
    uint8_t expected_hash[32];
    uint8_t actual_hash[32];
    auth_v3_context_kind_t kind = AUTH_V3_CONTEXT_CHANNEL_BINDING;
    auth_v3_context_entry_t entries[3];
    size_t entry_count = 0u;
    size_t input_len = decode_hex(encoded_hex, input, sizeof(input));

    assert(auth_v3_context_parse_bytes(input, input_len, &kind,
                                       entries, 3u, &entry_count) == AUTH_V3_CONTEXT_PARSE_OK);
    assert(kind == AUTH_V3_CONTEXT_AUTHORIZATION);
    assert(entry_count == 3u);
    assert(entries[0].id == 1u);
    assert(entries[0].value_len == 1u);
    assert(entries[0].value[0] == 0x01u);
    assert(entries[1].id == 2u);
    assert(entries[1].value_len == 6u);
    assert(memcmp(entries[1].value, "edge-a", 6u) == 0);
    assert(entries[2].id == 3u);
    assert(entries[2].value_len == 16u);

    assert(auth_v3_context_hash_bytes(input, input_len, actual_hash) == AUTH_V3_CONTEXT_PARSE_OK);
    assert(decode_hex(hash_hex, expected_hash, sizeof(expected_hash)) == sizeof(expected_hash));
    assert(memcmp(actual_hash, expected_hash, sizeof(actual_hash)) == 0);
}

static void test_empty_context(void) {
    uint8_t input[16];
    auth_v3_context_kind_t kind = AUTH_V3_CONTEXT_AUTHORIZATION;
    size_t entry_count = 99u;
    size_t input_len = decode_hex("5a4b43545801030000", input, sizeof(input));

    assert(auth_v3_context_parse_bytes(input, input_len, &kind,
                                       NULL, 0u, &entry_count) == AUTH_V3_CONTEXT_PARSE_OK);
    assert(kind == AUTH_V3_CONTEXT_CHANNEL_BINDING);
    assert(entry_count == 0u);
}

static void test_buffer_limit_is_fail_closed(void) {
    uint8_t input[64];
    auth_v3_context_kind_t kind = AUTH_V3_CONTEXT_AUTHORIZATION;
    auth_v3_context_entry_t entries[8];
    size_t entry_count = 0u;
    size_t input_len = decode_hex(
        "5a4b4354580101080001000000000200000000030000000004000000000500000000060000000007000000000800000000",
        input, sizeof(input));

    assert(auth_v3_context_parse_bytes(input, input_len, &kind,
                                       entries, 7u, &entry_count) ==
           AUTH_V3_CONTEXT_PARSE_ENTRY_BUFFER_TOO_SMALL);
    assert(auth_v3_context_parse_bytes(input, input_len, &kind,
                                       entries, 8u, &entry_count) == AUTH_V3_CONTEXT_PARSE_OK);
    assert(entry_count == 8u);
}

int main(void) {
    test_canonical_parse_and_hash();
    test_empty_context();
    test_buffer_limit_is_fail_closed();

    assert_parse_error("5a4b435458010100", AUTH_V3_CONTEXT_PARSE_TRUNCATED);
    assert_parse_error("004b43545801010000", AUTH_V3_CONTEXT_PARSE_INVALID_MAGIC);
    assert_parse_error("5a4b43545802010000", AUTH_V3_CONTEXT_PARSE_UNSUPPORTED_VERSION);
    assert_parse_error("5a4b43545801040000", AUTH_V3_CONTEXT_PARSE_UNKNOWN_KIND);
    assert_parse_error("5a4b435458010101000000000000", AUTH_V3_CONTEXT_PARSE_INVALID_ID);
    assert_parse_error("5a4b4354580101020001000000000100000000",
                       AUTH_V3_CONTEXT_PARSE_NON_CANONICAL_ORDER);
    assert_parse_error("5a4b4354580101020002000000000100000000",
                       AUTH_V3_CONTEXT_PARSE_NON_CANONICAL_ORDER);
    assert_parse_error("5a4b435458010201000100000000",
                       AUTH_V3_CONTEXT_PARSE_INVALID_CRITICAL_ID);
    assert_parse_error("5a4b435458010101000100010000",
                       AUTH_V3_CONTEXT_PARSE_NONZERO_FLAGS);
    assert_parse_error("5a4b435458010101000100", AUTH_V3_CONTEXT_PARSE_TRUNCATED);
    assert_parse_error("5a4b435458010101000100000200aa",
                       AUTH_V3_CONTEXT_PARSE_TRUNCATED);
    assert_parse_error("5a4b4354580101000000", AUTH_V3_CONTEXT_PARSE_TRAILING_BYTES);

    puts("AUTH v3 canonical context parser: ok");
    return 0;
}
