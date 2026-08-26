/*
 * Production-path regression coverage for AUTH_1 replay/session handling.
 *
 * This intentionally includes the server translation unit so the test can
 * exercise the real static dispatch() path without creating a second server
 * implementation solely for tests. The server main symbol is renamed here;
 * all protocol behavior still comes from c/bin/server.c plus libauth.
 */
#define main zk_arche_server_embedded_main
#include "../bin/server.c"
#undef main

#include <sodium.h>

static int failures = 0;

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        printf("  FAIL: %s\n", msg); \
        failures++; \
    } \
} while (0)

#define OK(expr) do { \
    auth_err_t _e = (expr); \
    if (_e != AUTH_OK) { \
        printf("  FAIL: %s -> 0x%04x %s\n", #expr, _e, auth_strerror(_e)); \
        failures++; \
        return; \
    } \
} while (0)

static void prepare_client(
    auth_client_ctx_t *client,
    const uint8_t server_pub[AUTH_POINT_LEN])
{
    memset(client, 0, sizeof *client);
    randombytes_buf(client->device_root, AUTH_DEVICE_ROOT_LEN);
    auth_derive_device_id(client->device_id, client->device_root);
    auth_derive_device_scalar(client->device_sk, client->device_root);
    OK(auth_scalarmult_base(client->device_pub, client->device_sk));
    OK(auth_client_ctx_init(client));

    memcpy(client->server_pub_pinned, server_pub, AUTH_POINT_LEN);
    client->has_pinned_server = 1;
    client->role_code = 2;
    auth_random_scalar(client->role_blind);
    OK(auth_make_role_commitment(
        client->role_commitment, client->role_code, client->role_blind));
    client->has_role = 1;
}

static size_t count_slot_state(
    const auth_session_table_t *table,
    auth_session_slot_state_t state)
{
    size_t count = 0;
    for (size_t i = 0; i < AUTH_SESSION_TABLE_CAPACITY; ++i) {
        if (auth_session_table_state(table, (int)i) == state) count++;
    }
    return count;
}

static void init_server_state(
    server_state_t *S,
    const auth_client_ctx_t *client)
{
    memset(S, 0, sizeof *S);
    auth_session_table_init(&S->sessions);
    auth_replay_cache_init(&S->replay_cache);
    CHECK(pthread_mutex_init(&S->mu, NULL) == 0, "server state mutex initializes");
    CHECK(pthread_mutex_init(&S->replay_mu, NULL) == 0, "replay mutex initializes");

    auth_random_scalar(S->server_sk);
    OK(auth_scalarmult_base(S->server_pub, S->server_sk));

    S->registry.n = 1u;
    memcpy(S->registry.entries[0].device_id,
           client->device_id, AUTH_DEVICE_ID_LEN);
    memcpy(S->registry.entries[0].device_pub,
           client->device_pub, AUTH_POINT_LEN);
    memcpy(S->registry.entries[0].role_commitment,
           client->role_commitment, AUTH_POINT_LEN);

    S->allowed_roles[0] = 1u;
    S->allowed_roles[1] = 2u;
    S->allowed_roles[2] = 3u;
    S->n_allowed = 3u;
}

static void test_dispatch_success_then_replay(void)
{
    server_state_t S;
    memset(&S, 0, sizeof S);

    uint8_t server_sk[AUTH_SCALAR_LEN];
    uint8_t server_pub[AUTH_POINT_LEN];
    auth_random_scalar(server_sk);
    OK(auth_scalarmult_base(server_pub, server_sk));

    auth_client_ctx_t client;
    prepare_client(&client, server_pub);

    /* Initialize the real server state, then retain the key pair the client
     * pinned so this is a valid production-path AUTH exchange. */
    init_server_state(&S, &client);
    memcpy(S.server_sk, server_sk, sizeof server_sk);
    memcpy(S.server_pub, server_pub, sizeof server_pub);

    uint64_t allowed[] = {1u, 2u, 3u};
    uint8_t auth1[AUTH_MAX_DATAGRAM];
    size_t auth1_len = 0;
    OK(auth_client_build_auth1(
        &client, allowed, 3u, auth1, sizeof auth1, &auth1_len));

    auth_header_t auth1_hdr;
    const uint8_t *auth1_payload = NULL;
    size_t auth1_payload_len = 0;
    OK(auth_header_decode(
        &auth1_hdr, auth1, auth1_len,
        &auth1_payload, &auth1_payload_len));
    (void)auth1_payload;
    (void)auth1_payload_len;

    uint8_t out[AUTH_MAX_DATAGRAM];
    size_t out_len = 0;
    dispatch(&S, auth1, auth1_len, out, sizeof out, &out_len);
    CHECK(out_len > 0u, "accepted AUTH_1 produces AUTH_2");

    auth_header_t first_hdr;
    const uint8_t *first_payload = NULL;
    size_t first_payload_len = 0;
    OK(auth_header_decode(
        &first_hdr, out, out_len,
        &first_payload, &first_payload_len));
    CHECK(first_hdr.pkt_type == AUTH_PKT_AUTH_2,
          "first production dispatch returns AUTH_2");
    CHECK(memcmp(first_hdr.session_id, auth1_hdr.session_id,
                 AUTH_SESSION_ID_LEN) == 0,
          "AUTH_2 remains bound to the original session");
    CHECK(S.replay_cache.n == 1u,
          "successful production dispatch retains one replay key");
    CHECK(count_slot_state(&S.sessions, AUTH_SESSION_SLOT_AUTH) == 1u,
          "successful production dispatch activates one AUTH session");
    CHECK(count_slot_state(&S.sessions, AUTH_SESSION_SLOT_RESERVED) == 0u,
          "successful production dispatch leaves no reservation behind");

    memset(out, 0, sizeof out);
    out_len = 0;
    dispatch(&S, auth1, auth1_len, out, sizeof out, &out_len);
    CHECK(out_len > 0u, "replayed AUTH_1 produces an error response");

    auth_header_t replay_hdr;
    const uint8_t *replay_payload = NULL;
    size_t replay_payload_len = 0;
    OK(auth_header_decode(
        &replay_hdr, out, out_len,
        &replay_payload, &replay_payload_len));
    CHECK(replay_hdr.pkt_type == AUTH_PKT_ERROR,
          "production replay is returned as protocol ERROR");

    auth_err_t replay_code = AUTH_OK;
    const char *replay_msg = NULL;
    size_t replay_msg_len = 0;
    OK(auth_packet_parse_error(
        replay_payload, replay_payload_len,
        &replay_code, &replay_msg, &replay_msg_len));
    (void)replay_msg;
    (void)replay_msg_len;
    CHECK(replay_code == AUTH_ERR_REPLAY_DETECTED,
          "production dispatch reports AUTH_ERR_REPLAY_DETECTED");
    CHECK(S.replay_cache.n == 1u,
          "rejected replay does not create another replay entry");
    CHECK(count_slot_state(&S.sessions, AUTH_SESSION_SLOT_AUTH) == 1u,
          "rejected replay does not disturb the accepted session");
    CHECK(count_slot_state(&S.sessions, AUTH_SESSION_SLOT_RESERVED) == 0u,
          "rejected replay releases its temporary reservation");

    CHECK(pthread_mutex_destroy(&S.replay_mu) == 0,
          "replay mutex destroys cleanly");
    CHECK(pthread_mutex_destroy(&S.mu) == 0,
          "server state mutex destroys cleanly");
}

int main(void)
{
    if (auth_init() != AUTH_OK) {
        fprintf(stderr, "auth_init failed\n");
        return 1;
    }

    printf("== production server dispatch replay lifecycle ==\n");
    test_dispatch_success_then_replay();
    printf("\n%s: %d failure(s)\n", failures ? "FAIL" : "PASS", failures);
    return failures ? 1 : 0;
}
