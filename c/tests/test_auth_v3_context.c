#include "auth/auth_v3_context.h"

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
        out[i] = (uint8_t)((hex_nibble(hex[i * 2u]) << 4) | hex_nibble(hex[i * 2u + 1u]));
    }
    return len / 2u;
}

static void assert_case(auth_v3_context_kind_t kind,
                        const auth_v3_context_entry_t *entries,
                        size_t entry_count,
                        const char *expected_hex,
                        const char *expected_hash_hex) {
    uint8_t encoded[256];
    uint8_t expected[256];
    uint8_t hash[32];
    uint8_t expected_hash[32];
    size_t encoded_len = 0u;
    size_t expected_len = decode_hex(expected_hex, expected, sizeof(expected));

    assert(auth_v3_context_encode(kind, entries, entry_count,
                                  encoded, sizeof(encoded), &encoded_len) == AUTH_V3_CONTEXT_OK);
    assert(encoded_len == expected_len);
    assert(memcmp(encoded, expected, expected_len) == 0);

    assert(auth_v3_context_hash(kind, entries, entry_count, hash) == AUTH_V3_CONTEXT_OK);
    assert(decode_hex(expected_hash_hex, expected_hash, sizeof(expected_hash)) == sizeof(expected_hash));
    assert(memcmp(hash, expected_hash, sizeof(hash)) == 0);
}

int main(void) {
    static const uint8_t authz_v1[] = {0x01};
    static const uint8_t authz_v2[] = {'e','d','g','e','-','a'};
    static const uint8_t authz_v3[] = {0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff};
    static const uint8_t crit_v1[] = {0xaa,0xbb};
    static const uint8_t crit_v2[] = {0x01,0x02,0x03};
    static const uint8_t cb_v[] = "tls-exporter-example";

    const auth_v3_context_entry_t authz[] = {
        {1u, authz_v1, (uint16_t)sizeof(authz_v1)},
        {2u, authz_v2, (uint16_t)sizeof(authz_v2)},
        {3u, authz_v3, (uint16_t)sizeof(authz_v3)}
    };
    const auth_v3_context_entry_t critical[] = {
        {0x8001u, crit_v1, (uint16_t)sizeof(crit_v1)},
        {0x8004u, crit_v2, (uint16_t)sizeof(crit_v2)}
    };
    const auth_v3_context_entry_t channel[] = {
        {1u, cb_v, (uint16_t)(sizeof(cb_v) - 1u)}
    };
    const auth_v3_context_entry_t zero_id[] = {{0u, NULL, 0u}};
    const auth_v3_context_entry_t duplicate[] = {
        {1u, authz_v1, 1u}, {1u, crit_v2, 1u}
    };
    const auth_v3_context_entry_t descending[] = {
        {2u, authz_v1, 1u}, {1u, crit_v2, 1u}
    };
    const auth_v3_context_entry_t bad_critical[] = {{1u, authz_v1, 1u}};

    assert_case(AUTH_V3_CONTEXT_AUTHORIZATION, NULL, 0u,
                "5a4b43545801010000",
                "505121c6096720d111eab443818cc974bb66f3339e06de742f69e4692dd2717a");
    assert_case(AUTH_V3_CONTEXT_CRITICAL_EXTENSIONS, NULL, 0u,
                "5a4b43545801020000",
                "ef8116870a7dc594749827eae3c9a5346057612b0d93ed3d1f0cea3d6ff0f3ed");
    assert_case(AUTH_V3_CONTEXT_CHANNEL_BINDING, NULL, 0u,
                "5a4b43545801030000",
                "7f724afa7e3e7a6c13e0fe167fc48a034888d10c523abd7864671c68aaea5fa8");

    assert_case(AUTH_V3_CONTEXT_AUTHORIZATION, authz, 3u,
                "5a4b435458010103000100000100010200000600656467652d61030000100000112233445566778899aabbccddeeff",
                "3f85c714dfca2070fcfa909bfa31442c5828f4f91834f62e140db0320fcfcb69");
    assert_case(AUTH_V3_CONTEXT_CRITICAL_EXTENSIONS, critical, 2u,
                "5a4b435458010202000180000200aabb0480000300010203",
                "3f4424631680740d286af85cd2eb397e89e32a035eca62b8d9498aa970d4e36c");
    assert_case(AUTH_V3_CONTEXT_CHANNEL_BINDING, channel, 1u,
                "5a4b435458010301000100001400746c732d6578706f727465722d6578616d706c65",
                "a8916d7d0e1cac4884319dd7149e76937e654e51e233edfb641232ccd0a5118a");

    assert(auth_v3_context_encode(AUTH_V3_CONTEXT_AUTHORIZATION, zero_id, 1u,
                                  (uint8_t[32]){0}, 32u, &(size_t){0}) == AUTH_V3_CONTEXT_INVALID_ID);
    assert(auth_v3_context_encode(AUTH_V3_CONTEXT_AUTHORIZATION, duplicate, 2u,
                                  (uint8_t[32]){0}, 32u, &(size_t){0}) == AUTH_V3_CONTEXT_NON_CANONICAL_ORDER);
    assert(auth_v3_context_encode(AUTH_V3_CONTEXT_AUTHORIZATION, descending, 2u,
                                  (uint8_t[32]){0}, 32u, &(size_t){0}) == AUTH_V3_CONTEXT_NON_CANONICAL_ORDER);
    assert(auth_v3_context_encode(AUTH_V3_CONTEXT_CRITICAL_EXTENSIONS, bad_critical, 1u,
                                  (uint8_t[32]){0}, 32u, &(size_t){0}) == AUTH_V3_CONTEXT_INVALID_CRITICAL_ID);

    puts("AUTH v3 canonical context vectors: ok");
    return 0;
}
