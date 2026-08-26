#include "auth/iot_auth.h"
#include "auth/auth_crypto.h"
#include "auth/auth_proto.h"
#include "auth/replay_guard.h"

#include <sodium.h>
#include <stdio.h>
#include <string.h>

static int failures = 0;
#define CHECK(cond, msg) do { \
    if (!(cond)) { printf("  FAIL: %s\n", msg); failures++; } \
} while (0)

#define OK(expr) do { \
    auth_err_t _e = (expr); \
    if (_e != AUTH_OK) { \
        printf("  FAIL: %s -> 0x%04x %s\n", #expr, _e, auth_strerror(_e)); \
        failures++; return; \
    } \
} while (0)

typedef struct test_registry_entry {
    uint8_t device_pub[32];
    uint8_t role_c[32];
} test_registry_entry_t;

static auth_err_t registry_lookup(
    void *ctx,
    const uint8_t device_id[AUTH_DEVICE_ID_LEN],
    uint8_t device_pub[AUTH_POINT_LEN],
    uint8_t role_commitment[AUTH_POINT_LEN])
{
    (void)device_id;
    test_registry_entry_t *e = (test_registry_entry_t *)ctx;
    if (!e) return AUTH_ERR_UNKNOWN_DEVICE;
    memcpy(device_pub, e->device_pub, 32);
    memcpy(role_commitment, e->role_c, 32);
    return AUTH_OK;
}

static void prepare_client(
    auth_client_ctx_t *client,
    const uint8_t server_pub[32],
    test_registry_entry_t *reg)
{
    memset(client, 0, sizeof *client);
    randombytes_buf(client->device_root, 32);
    auth_derive_device_id(client->device_id, client->device_root);
    auth_derive_device_scalar(client->device_sk, client->device_root);
    OK(auth_scalarmult_base(client->device_pub, client->device_sk));
    OK(auth_client_ctx_init(client));

    memcpy(client->server_pub_pinned, server_pub, 32);
    client->has_pinned_server = 1;
    client->role_code = 2;
    auth_random_scalar(client->role_blind);
    OK(auth_make_role_commitment(
        client->role_commitment, client->role_code, client->role_blind));
    client->has_role = 1;

    memcpy(reg->device_pub, client->device_pub, 32);
    memcpy(reg->role_c, client->role_commitment, 32);
}

static void test_success_then_replay(void)
{
    uint8_t server_sk[32], server_pub[32];
    auth_random_scalar(server_sk);
    OK(auth_scalarmult_base(server_pub, server_sk));

    auth_client_ctx_t client;
    test_registry_entry_t reg;
    prepare_client(&client, server_pub, &reg);

    uint64_t allowed[] = {1, 2, 3};
    uint8_t a1_pkt[AUTH_MAX_DATAGRAM];
    size_t a1_len = 0;
    OK(auth_client_build_auth1(
        &client, allowed, 3, a1_pkt, sizeof a1_pkt, &a1_len));

    auth_header_t hdr;
    const uint8_t *payload;
    size_t payload_len;
    OK(auth_header_decode(&hdr, a1_pkt, a1_len, &payload, &payload_len));

    auth_replay_cache_t cache;
    auth_replay_cache_init(&cache);

    auth_pending_auth_t pending = {0};
    uint8_t a2_pkt[AUTH_MAX_DATAGRAM];
    size_t a2_len = 0;
    OK(auth_server_handle_auth1_guarded(
        &cache,
        hdr.session_id, hdr.seq,
        payload, payload_len,
        server_sk, server_pub,
        registry_lookup, &reg,
        allowed, 3,
        &pending,
        a2_pkt, sizeof a2_pkt, &a2_len));
    CHECK(cache.n == 1u, "successful AUTH_1 is retained in replay cache");

    auth_pending_auth_t replay_pending = {0};
    uint8_t replay_out[AUTH_MAX_DATAGRAM];
    size_t replay_out_len = 99u;
    auth_err_t replay_err = auth_server_handle_auth1_guarded(
        &cache,
        hdr.session_id, hdr.seq,
        payload, payload_len,
        server_sk, server_pub,
        registry_lookup, &reg,
        allowed, 3,
        &replay_pending,
        replay_out, sizeof replay_out, &replay_out_len);
    CHECK(replay_err == AUTH_ERR_REPLAY_DETECTED,
          "accepted AUTH_1 replay is rejected before repeated auth work");
}

static void test_invalid_auth_does_not_poison(void)
{
    uint8_t server_sk[32], server_pub[32];
    auth_random_scalar(server_sk);
    OK(auth_scalarmult_base(server_pub, server_sk));

    auth_client_ctx_t client;
    test_registry_entry_t reg;
    prepare_client(&client, server_pub, &reg);

    uint64_t allowed[] = {1, 2, 3};
    uint8_t a1_pkt[AUTH_MAX_DATAGRAM];
    size_t a1_len = 0;
    OK(auth_client_build_auth1(
        &client, allowed, 3, a1_pkt, sizeof a1_pkt, &a1_len));

    auth_header_t hdr;
    const uint8_t *payload_const;
    size_t payload_len;
    OK(auth_header_decode(&hdr, a1_pkt, a1_len, &payload_const, &payload_len));
    CHECK(payload_len > 0u, "AUTH_1 payload is non-empty");
    uint8_t *payload = a1_pkt + AUTH_HEADER_LEN;
    payload[payload_len - 1u] ^= 0x01u;

    auth_replay_cache_t cache;
    auth_replay_cache_init(&cache);

    for (int attempt = 0; attempt < 2; ++attempt) {
        auth_pending_auth_t pending = {0};
        uint8_t out[AUTH_MAX_DATAGRAM];
        size_t out_len = 0;
        auth_err_t err = auth_server_handle_auth1_guarded(
            &cache,
            hdr.session_id, hdr.seq,
            payload, payload_len,
            server_sk, server_pub,
            registry_lookup, &reg,
            allowed, 3,
            &pending,
            out, sizeof out, &out_len);
        CHECK(err != AUTH_OK, "corrupted AUTH_1 is rejected");
        CHECK(err != AUTH_ERR_REPLAY_DETECTED,
              "invalid AUTH_1 does not poison replay cache");
        CHECK(cache.n == 0u, "replay cache remains empty after invalid AUTH_1");
    }
}

int main(void)
{
    if (auth_init() != AUTH_OK) {
        fprintf(stderr, "auth_init failed\n");
        return 1;
    }

    printf("== replay-guarded AUTH_1 ==\n");
    test_success_then_replay();
    test_invalid_auth_does_not_poison();
    printf("\n%s: %d failure(s)\n", failures ? "FAIL" : "PASS", failures);
    return failures ? 1 : 0;
}
