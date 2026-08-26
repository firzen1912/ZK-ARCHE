/*
 * Production-path replay edge cases for AUTH_1.
 *
 * FT-022: replay the exact accepted AUTH_1 payload under a fresh outer
 *          session_id/sequence.  Replay identity is derived from the
 *          authenticated AUTH_1 payload, not the transport-neutral header.
 *
 * FT-023: deliver the same valid AUTH_1 concurrently through two workers.
 *          Exactly one worker may create new accepted state; the other must
 *          observe the shared replay transaction and reject the duplicate.
 *
 * This includes the real server translation unit so both scenarios exercise
 * c/bin/server.c::dispatch() and its shared replay/session state.
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

static void init_server_state(
    server_state_t *S,
    const auth_client_ctx_t *client,
    const uint8_t server_sk[AUTH_SCALAR_LEN],
    const uint8_t server_pub[AUTH_POINT_LEN])
{
    memset(S, 0, sizeof *S);
    auth_session_table_init(&S->sessions);
    auth_replay_cache_init(&S->replay_cache);
    CHECK(pthread_mutex_init(&S->mu, NULL) == 0, "server state mutex initializes");
    CHECK(pthread_mutex_init(&S->replay_mu, NULL) == 0, "replay mutex initializes");

    memcpy(S->server_sk, server_sk, AUTH_SCALAR_LEN);
    memcpy(S->server_pub, server_pub, AUTH_POINT_LEN);

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

static void destroy_server_state(server_state_t *S)
{
    CHECK(pthread_mutex_destroy(&S->replay_mu) == 0,
          "replay mutex destroys cleanly");
    CHECK(pthread_mutex_destroy(&S->mu) == 0,
          "server state mutex destroys cleanly");
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

static int classify_response(
    const uint8_t *buf,
    size_t len,
    auth_err_t *error_code_out)
{
    auth_header_t hdr;
    const uint8_t *payload = NULL;
    size_t payload_len = 0;
    auth_err_t err = auth_header_decode(&hdr, buf, len, &payload, &payload_len);
    if (err != AUTH_OK) return -1;

    if (hdr.pkt_type == AUTH_PKT_AUTH_2) return 1;
    if (hdr.pkt_type != AUTH_PKT_ERROR) return -1;

    const char *msg = NULL;
    size_t msg_len = 0;
    auth_err_t code = AUTH_OK;
    err = auth_packet_parse_error(payload, payload_len, &code, &msg, &msg_len);
    (void)msg;
    (void)msg_len;
    if (err != AUTH_OK) return -1;
    if (error_code_out != NULL) *error_code_out = code;
    return 0;
}

static void make_valid_auth1(
    auth_client_ctx_t *client,
    uint8_t *buf,
    size_t buf_cap,
    size_t *len_out)
{
    uint64_t allowed[] = {1u, 2u, 3u};
    OK(auth_client_build_auth1(
        client, allowed, 3u, buf, buf_cap, len_out));
}

static void test_fresh_outer_session_replay(void)
{
    uint8_t server_sk[AUTH_SCALAR_LEN];
    uint8_t server_pub[AUTH_POINT_LEN];
    auth_random_scalar(server_sk);
    OK(auth_scalarmult_base(server_pub, server_sk));

    auth_client_ctx_t client;
    prepare_client(&client, server_pub);

    server_state_t S;
    init_server_state(&S, &client, server_sk, server_pub);

    uint8_t auth1[AUTH_MAX_DATAGRAM];
    size_t auth1_len = 0;
    make_valid_auth1(&client, auth1, sizeof auth1, &auth1_len);

    auth_header_t original_hdr;
    const uint8_t *payload = NULL;
    size_t payload_len = 0;
    OK(auth_header_decode(
        &original_hdr, auth1, auth1_len, &payload, &payload_len));

    uint8_t out[AUTH_MAX_DATAGRAM];
    size_t out_len = 0;
    dispatch(&S, auth1, auth1_len, out, sizeof out, &out_len);
    CHECK(out_len > 0u, "first AUTH_1 produces a response");
    CHECK(classify_response(out, out_len, NULL) == 1,
          "first AUTH_1 is accepted as AUTH_2");

    uint8_t fresh_sid[AUTH_SESSION_ID_LEN];
    do {
        randombytes_buf(fresh_sid, sizeof fresh_sid);
    } while (memcmp(fresh_sid, original_hdr.session_id, sizeof fresh_sid) == 0);

    uint8_t replay_packet[AUTH_MAX_DATAGRAM];
    size_t replay_packet_len = 0;
    OK(auth_packet_build(
        AUTH_PKT_AUTH_1,
        fresh_sid,
        original_hdr.seq + 17u,
        payload,
        payload_len,
        replay_packet,
        sizeof replay_packet,
        &replay_packet_len));

    memset(out, 0, sizeof out);
    out_len = 0;
    dispatch(&S, replay_packet, replay_packet_len, out, sizeof out, &out_len);
    CHECK(out_len > 0u, "fresh-session replay produces a response");

    auth_err_t replay_code = AUTH_OK;
    CHECK(classify_response(out, out_len, &replay_code) == 0,
          "fresh-session replay returns protocol ERROR");
    CHECK(replay_code == AUTH_ERR_REPLAY_DETECTED,
          "fresh-session replay is rejected by payload replay identity");
    CHECK(S.replay_cache.n == 1u,
          "fresh outer session does not create a second replay entry");
    CHECK(count_slot_state(&S.sessions, AUTH_SESSION_SLOT_AUTH) == 1u,
          "fresh-session replay does not create a second AUTH session");
    CHECK(count_slot_state(&S.sessions, AUTH_SESSION_SLOT_RESERVED) == 0u,
          "fresh-session replay releases its temporary reservation");

    destroy_server_state(&S);
}

typedef struct dispatch_thread_args {
    server_state_t *S;
    const uint8_t *packet;
    size_t packet_len;
    uint8_t out[AUTH_MAX_DATAGRAM];
    size_t out_len;
} dispatch_thread_args_t;

static void *dispatch_thread(void *vp)
{
    dispatch_thread_args_t *a = (dispatch_thread_args_t *)vp;
    a->out_len = 0u;
    dispatch(a->S, a->packet, a->packet_len,
             a->out, sizeof a->out, &a->out_len);
    return NULL;
}

static void test_concurrent_duplicate_auth1(void)
{
    uint8_t server_sk[AUTH_SCALAR_LEN];
    uint8_t server_pub[AUTH_POINT_LEN];
    auth_random_scalar(server_sk);
    OK(auth_scalarmult_base(server_pub, server_sk));

    auth_client_ctx_t client;
    prepare_client(&client, server_pub);

    server_state_t S;
    init_server_state(&S, &client, server_sk, server_pub);

    uint8_t auth1[AUTH_MAX_DATAGRAM];
    size_t auth1_len = 0;
    make_valid_auth1(&client, auth1, sizeof auth1, &auth1_len);

    dispatch_thread_args_t args[2];
    memset(args, 0, sizeof args);
    for (size_t i = 0; i < 2u; ++i) {
        args[i].S = &S;
        args[i].packet = auth1;
        args[i].packet_len = auth1_len;
    }

    pthread_t threads[2];
    int rc0 = pthread_create(&threads[0], NULL, dispatch_thread, &args[0]);
    int rc1 = pthread_create(&threads[1], NULL, dispatch_thread, &args[1]);
    CHECK(rc0 == 0, "first duplicate worker starts");
    CHECK(rc1 == 0, "second duplicate worker starts");
    if (rc0 == 0) CHECK(pthread_join(threads[0], NULL) == 0,
                        "first duplicate worker joins");
    if (rc1 == 0) CHECK(pthread_join(threads[1], NULL) == 0,
                        "second duplicate worker joins");

    int accepted = 0;
    int replay_rejected = 0;
    for (size_t i = 0; i < 2u; ++i) {
        CHECK(args[i].out_len > 0u, "duplicate worker produces a response");
        auth_err_t code = AUTH_OK;
        int kind = classify_response(args[i].out, args[i].out_len, &code);
        if (kind == 1) {
            accepted++;
        } else if (kind == 0 && code == AUTH_ERR_REPLAY_DETECTED) {
            replay_rejected++;
        } else {
            CHECK(0, "duplicate worker returns only AUTH_2 or replay ERROR");
        }
    }

    CHECK(accepted == 1,
          "exactly one concurrent duplicate creates accepted AUTH state");
    CHECK(replay_rejected == 1,
          "exactly one concurrent duplicate is rejected as replay");
    CHECK(S.replay_cache.n == 1u,
          "concurrent duplicates retain exactly one replay key");
    CHECK(count_slot_state(&S.sessions, AUTH_SESSION_SLOT_AUTH) == 1u,
          "concurrent duplicates create exactly one AUTH session");
    CHECK(count_slot_state(&S.sessions, AUTH_SESSION_SLOT_RESERVED) == 0u,
          "concurrent duplicate loser releases its reservation");

    destroy_server_state(&S);
}

int main(void)
{
    if (auth_init() != AUTH_OK) {
        fprintf(stderr, "auth_init failed\n");
        return 1;
    }

    printf("== production replay edge cases ==\n");
    test_fresh_outer_session_replay();
    test_concurrent_duplicate_auth1();
    printf("\n%s: %d failure(s)\n", failures ? "FAIL" : "PASS", failures);
    return failures ? 1 : 0;
}
