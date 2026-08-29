#include "auth/auth_v3.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <sodium.h>

static void decode_hex32(const char *hex_value, uint8_t out[32])
{
    size_t decoded_len = 0u;
    assert(strlen(hex_value) == 64u);
    assert(sodium_hex2bin(out, 32u, hex_value, 64u, NULL, &decoded_len, NULL) == 0);
    assert(decoded_len == 32u);
}

static void test_auth_v3_finished_rejects_cross_direction_reflection(void)
{
    assert(auth_init() == AUTH_OK);

    uint8_t session_key[AUTH_SESSION_KEY_LEN];
    uint8_t th[AUTH_HASH_LEN];
    uint8_t expected_server[AUTH_MAC_TAG_LEN];
    uint8_t expected_client[AUTH_MAC_TAG_LEN];
    decode_hex32("5cea979c840f9cb1302db41f7dcfe91c4f8b22f7019b0586db183219e27ef348",
                 session_key);
    decode_hex32("e2b85befd4f3f58b5e880673ce1b27e81de875bf4443d7af6971d811e10439d2",
                 th);
    decode_hex32("453edbad5976c5c08e720e6bffb0e111eb117501f26afb0f55b54cc719e38096",
                 expected_server);
    decode_hex32("6c748a2e93e297095be36ba757c66bf886ec11fb9833ffdab7cbdcf5aa75d819",
                 expected_client);

    uint8_t k_s2c[AUTH_MAC_KEY_LEN];
    uint8_t k_c2s[AUTH_MAC_KEY_LEN];
    uint8_t k_complete[AUTH_MAC_KEY_LEN];
    assert(auth_v3_derive_kc_keys(k_s2c, k_c2s, k_complete, session_key, th) == AUTH_OK);

    static const uint8_t SERVER_FINISHED[] = "server finished v3";
    static const uint8_t CLIENT_FINISHED[] = "client finished v3";
    uint8_t server_tag[AUTH_MAC_TAG_LEN];
    uint8_t client_tag[AUTH_MAC_TAG_LEN];
    auth_v3_finished_tag(server_tag, k_s2c, SERVER_FINISHED,
                         sizeof SERVER_FINISHED - 1u, th);
    auth_v3_finished_tag(client_tag, k_c2s, CLIENT_FINISHED,
                         sizeof CLIENT_FINISHED - 1u, th);

    assert(sodium_memcmp(server_tag, expected_server, sizeof server_tag) == 0);
    assert(sodium_memcmp(client_tag, expected_client, sizeof client_tag) == 0);

    /* A verbatim Finished value from either direction must not satisfy the
     * opposite-direction expectation. */
    assert(sodium_memcmp(server_tag, expected_client, sizeof server_tag) != 0);
    assert(sodium_memcmp(client_tag, expected_server, sizeof client_tag) != 0);

    /* Keep both independent direction separators covered: the target role
     * label paired with the source-direction key still cannot produce the
     * target-direction Finished value. */
    uint8_t server_key_with_client_label[AUTH_MAC_TAG_LEN];
    uint8_t client_key_with_server_label[AUTH_MAC_TAG_LEN];
    auth_v3_finished_tag(server_key_with_client_label, k_s2c, CLIENT_FINISHED,
                         sizeof CLIENT_FINISHED - 1u, th);
    auth_v3_finished_tag(client_key_with_server_label, k_c2s, SERVER_FINISHED,
                         sizeof SERVER_FINISHED - 1u, th);
    assert(sodium_memcmp(server_key_with_client_label, expected_client,
                         sizeof server_key_with_client_label) != 0);
    assert(sodium_memcmp(client_key_with_server_label, expected_server,
                         sizeof client_key_with_server_label) != 0);

    sodium_memzero(session_key, sizeof session_key);
    sodium_memzero(k_s2c, sizeof k_s2c);
    sodium_memzero(k_c2s, sizeof k_c2s);
    sodium_memzero(k_complete, sizeof k_complete);
}

int main(void)
{
    test_auth_v3_finished_rejects_cross_direction_reflection();
    puts("AUTH v3 Finished reflection negatives: ok");
    return 0;
}
