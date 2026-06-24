/*
 * test_e2e.c — end-to-end in-process test of the setup + auth state
 * machines with no network involved.
 *
 * Demonstrates that client state machine + server state machine together
 * produce a successful enrollment and authentication, without UDP/TCP
 * or file I/O. The transport layer only has to pipe byte buffers between
 * them; this test does that explicitly.
 */

#include "auth/iot_auth.h"
#include "auth/auth_crypto.h"
#include "auth/auth_proto.h"

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

/* ---- Registry implementation for this test ----
 *
 * Holds one enrolled device. The lookup callback returns it. */

typedef struct registry_entry {
    uint8_t device_id [32];
    uint8_t device_pub[32];
    uint8_t role_c    [32];
    int     present;
} registry_entry_t;

static auth_err_t registry_lookup(
    void *ctx,
    const uint8_t device_id[32],
    uint8_t device_pub[32],
    uint8_t role_commitment[32])
{
    (void)device_id; /* callback variant: scan-and-return-one */
    registry_entry_t *e = (registry_entry_t *)ctx;
    if (!e || !e->present) return AUTH_ERR_UNKNOWN_DEVICE;
    memcpy(device_pub,      e->device_pub, 32);
    memcpy(role_commitment, e->role_c,     32);
    return AUTH_OK;
}

/* ---- One full setup + auth round ---- */

static void run_e2e(void)
{
    printf("== end-to-end setup + auth ==\n");

    /* --- Server keys --- */
    uint8_t server_sk[32], server_pub[32];
    auth_random_scalar(server_sk);
    OK(auth_scalarmult_base(server_pub, server_sk));

    /* --- Client keys --- */
    auth_client_ctx_t client = {0};
    randombytes_buf(client.device_root, 32);
    auth_derive_device_id(client.device_id, client.device_root);
    auth_derive_device_scalar(client.device_sk, client.device_root);
    OK(auth_scalarmult_base(client.device_pub, client.device_sk));
    OK(auth_client_ctx_init(&client));
    client.role_code = 2;  /* "operator" */

    /* --- SETUP flow --- */
    uint8_t pkt[AUTH_MAX_DATAGRAM];
    size_t pkt_len = 0;

    /* Client -> SETUP_1 */
    OK(auth_client_build_setup1(&client, NULL, 0, pkt, sizeof pkt, &pkt_len));
    printf("  SETUP_1 packet: %zu bytes\n", pkt_len);

    /* Server <- SETUP_1 -> SETUP_2 */
    auth_header_t hdr_in;
    const uint8_t *payload_in;
    size_t payload_len_in;
    OK(auth_header_decode(&hdr_in, pkt, pkt_len, &payload_in, &payload_len_in));
    CHECK(hdr_in.pkt_type == AUTH_PKT_SETUP_1, "is SETUP_1");

    auth_pending_setup_t pending_s = {0};
    uint8_t s2_pkt[AUTH_MAX_DATAGRAM];
    size_t s2_len = 0;
    OK(auth_server_handle_setup1(
        hdr_in.session_id, hdr_in.seq,
        payload_in, payload_len_in,
        server_sk, server_pub,
        NULL /* no pairing token required */,
        &pending_s,
        s2_pkt, sizeof s2_pkt, &s2_len));
    printf("  SETUP_2 packet: %zu bytes\n", s2_len);

    /* Client <- SETUP_2 */
    OK(auth_client_handle_setup2(&client, s2_pkt, s2_len, 1 /* allow TOFU */));
    CHECK(client.has_pinned_server, "server pinned");

    /* Client -> SETUP_3 */
    uint8_t s3_pkt[AUTH_MAX_DATAGRAM];
    size_t s3_len = 0;
    OK(auth_client_build_setup3(&client, s3_pkt, sizeof s3_pkt, &s3_len));
    printf("  SETUP_3 packet: %zu bytes\n", s3_len);

    /* Server <- SETUP_3 -> SETUP_ACK */
    OK(auth_header_decode(&hdr_in, s3_pkt, s3_len, &payload_in, &payload_len_in));
    CHECK(hdr_in.pkt_type == AUTH_PKT_SETUP_3, "is SETUP_3");
    uint8_t sack_pkt[AUTH_MAX_DATAGRAM];
    size_t sack_len = 0;
    OK(auth_server_handle_setup3(
        &pending_s, hdr_in.session_id, hdr_in.seq,
        payload_in, payload_len_in,
        sack_pkt, sizeof sack_pkt, &sack_len));
    printf("  SETUP_ACK packet: %zu bytes\n", sack_len);

    /* Client <- SETUP_ACK */
    OK(auth_client_handle_setup_ack(&client, sack_pkt, sack_len));
    printf("  [SETUP] OK\n");

    /* Simulate registry persistence after successful setup. */
    registry_entry_t reg = {0};
    memcpy(reg.device_id,  client.device_id, 32);
    memcpy(reg.device_pub, client.device_pub, 32);
    memcpy(reg.role_c,     client.role_commitment, 32);
    reg.present = 1;

    /* --- AUTH flow --- */
    auth_client_ctx_t client_auth = client;  /* reuse device identity */
    OK(auth_client_ctx_init(&client_auth)); /* fresh session id */
    /* Preserve persistent state that init() cleared. */
    client_auth.has_pinned_server = 1;
    memcpy(client_auth.server_pub_pinned, client.server_pub_pinned, 32);
    client_auth.has_role = 1;
    memcpy(client_auth.role_commitment, client.role_commitment, 32);
    memcpy(client_auth.role_blind,      client.role_blind,      32);
    client_auth.role_code = client.role_code;

    uint64_t allowed[3] = {1, 2, 3};

    /* Client -> AUTH_1 */
    uint8_t a1_pkt[AUTH_MAX_DATAGRAM];
    size_t a1_len = 0;
    OK(auth_client_build_auth1(&client_auth, allowed, 3,
                                   a1_pkt, sizeof a1_pkt, &a1_len));
    printf("  AUTH_1 packet: %zu bytes\n", a1_len);

    /* Server <- AUTH_1 -> AUTH_2 */
    OK(auth_header_decode(&hdr_in, a1_pkt, a1_len, &payload_in, &payload_len_in));
    CHECK(hdr_in.pkt_type == AUTH_PKT_AUTH_1, "is AUTH_1");

    auth_pending_auth_t pending_a = {0};
    uint8_t a2_pkt[AUTH_MAX_DATAGRAM];
    size_t a2_len = 0;
    OK(auth_server_handle_auth1(
        hdr_in.session_id, hdr_in.seq,
        payload_in, payload_len_in,
        server_sk, server_pub,
        registry_lookup, &reg,
        allowed, 3,
        &pending_a,
        a2_pkt, sizeof a2_pkt, &a2_len));
    printf("  AUTH_2 packet: %zu bytes\n", a2_len);

    /* Client <- AUTH_2 */
    OK(auth_client_handle_auth2(&client_auth, a2_pkt, a2_len));

    /* Client -> AUTH_3 */
    uint8_t a3_pkt[AUTH_MAX_DATAGRAM];
    size_t a3_len = 0;
    OK(auth_client_build_auth3(&client_auth, a3_pkt, sizeof a3_pkt, &a3_len));
    printf("  AUTH_3 packet: %zu bytes\n", a3_len);

    /* Server <- AUTH_3 -> AUTH_ACK */
    OK(auth_header_decode(&hdr_in, a3_pkt, a3_len, &payload_in, &payload_len_in));
    CHECK(hdr_in.pkt_type == AUTH_PKT_AUTH_3, "is AUTH_3");
    uint8_t aack_pkt[AUTH_MAX_DATAGRAM];
    size_t aack_len = 0;
    OK(auth_server_handle_auth3(
        &pending_a, hdr_in.session_id, hdr_in.seq,
        payload_in, payload_len_in,
        aack_pkt, sizeof aack_pkt, &aack_len));
    printf("  AUTH_ACK packet: %zu bytes\n", aack_len);

    /* Client <- AUTH_ACK */
    OK(auth_client_handle_auth_ack(&client_auth, aack_pkt, aack_len));

    /* Both sides must have matching session keys. */
    if (memcmp(client_auth.session_key, pending_a.session_key,
               AUTH_SESSION_KEY_LEN) != 0) {
        CHECK(0, "session keys don't match");
    } else {
        printf("  session key agreement: OK (");
        for (int i = 0; i < 8; ++i) printf("%02x", client_auth.session_key[i]);
        printf("...)\n");
    }
    CHECK(client_auth.auth_complete, "auth_complete");
    printf("  [AUTH] OK\n");
}

/* ---- Main ---- */

int main(void)
{
    if (auth_init() != AUTH_OK) { fprintf(stderr, "init\n"); return 1; }
    run_e2e();
    printf("\n%s: %d failure(s)\n", failures ? "FAIL" : "PASS", failures);
    return failures ? 1 : 0;
}
