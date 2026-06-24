/*
 * auth_client — C client binary.
 *
 * Usage:
 *   auth_client --server HOST:PORT --transport {udp,tcp}
 *                   --state-dir DIR [--setup [--allow-tofu-setup]]
 *                   [--role N] [--allowed-roles R1,R2,...]
 *
 * Defaults: --transport udp, --role 2, --allowed-roles 1,2,3.
 */

#define _POSIX_C_SOURCE 200112L
#define _DEFAULT_SOURCE

#include "auth/iot_auth.h"
#include "auth/auth_crypto.h"
#include "auth/auth_proto.h"
#include "auth/auth_store.h"
#include "auth/auth_transport.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

/* ---- CLI args ---- */

typedef enum { TRANSPORT_UDP, TRANSPORT_TCP } transport_kind_t;

typedef struct {
    const char *server;
    transport_kind_t transport;
    const char *state_dir;
    int    do_setup;
    int    allow_tofu;
    uint64_t role;
    uint64_t allowed[AUTH_MAX_ROLES];
    size_t n_allowed;
    const char *pairing_token;
} args_t;

static void usage(const char *prog) {
    fprintf(stderr,
"Usage:\n"
"  %s --server HOST:PORT [--transport udp|tcp] [--state-dir DIR]\n"
"     [--setup [--allow-tofu-setup]]\n"
"     [--role N] [--allowed-roles R1,R2,...] [--pairing-token T]\n"
"\n"
"  Without --setup, performs online AUTH using credentials from\n"
"  DIR/credentials.bin (which must exist from a prior enrollment).\n",
        prog);
}

static int parse_role_list(const char *s, uint64_t *out, size_t cap, size_t *n_out)
{
    *n_out = 0;
    const char *p = s;
    while (*p && *n_out < cap) {
        char *end = NULL;
        unsigned long long v = strtoull(p, &end, 10);
        if (end == p) return -1;
        out[(*n_out)++] = (uint64_t)v;
        p = end;
        if (*p == ',') p++;
        else if (*p != 0) return -1;
    }
    return 0;
}

static int parse_args(int argc, char **argv, args_t *a) {
    memset(a, 0, sizeof *a);
    a->transport = TRANSPORT_UDP;
    a->role = 2;
    uint64_t def[] = {1, 2};
    memcpy(a->allowed, def, sizeof def);
    a->n_allowed = 2;

    for (int i = 1; i < argc; ++i) {
        const char *k = argv[i];
        if (strcmp(k, "--server") == 0 && i + 1 < argc) a->server = argv[++i];
        else if (strcmp(k, "--transport") == 0 && i + 1 < argc) {
            const char *v = argv[++i];
            if      (strcmp(v, "udp") == 0) a->transport = TRANSPORT_UDP;
            else if (strcmp(v, "tcp") == 0) a->transport = TRANSPORT_TCP;
            else return -1;
        }
        else if (strcmp(k, "--state-dir") == 0 && i + 1 < argc) a->state_dir = argv[++i];
        else if (strcmp(k, "--setup") == 0) a->do_setup = 1;
        else if (strcmp(k, "--allow-tofu-setup") == 0) a->allow_tofu = 1;
        else if (strcmp(k, "--role") == 0 && i + 1 < argc) {
            a->role = strtoull(argv[++i], NULL, 10);
        }
        else if (strcmp(k, "--allowed-roles") == 0 && i + 1 < argc) {
            if (parse_role_list(argv[++i], a->allowed, AUTH_MAX_ROLES, &a->n_allowed) != 0)
                return -1;
        }
        else if (strcmp(k, "--pairing-token") == 0 && i + 1 < argc) {
            a->pairing_token = argv[++i];
        }
        else if (strcmp(k, "-h") == 0 || strcmp(k, "--help") == 0) {
            usage(argv[0]); exit(0);
        }
        else return -1;
    }
    if (!a->server) return -1;
    if (!a->state_dir) a->state_dir = "./client-state";
    return 0;
}

/* ---- Transport wrapper: send packet + read response ---- */

typedef struct {
    transport_kind_t kind;
    auth_udp_t   udp;
    auth_tcp_conn_t tcp;
} client_transport_t;

static auth_err_t transport_connect(client_transport_t *t, const char *server) {
    if (t->kind == TRANSPORT_UDP) {
        return auth_udp_connect(&t->udp, server);
    }
    return auth_tcp_connect(&t->tcp, server);
}

static void transport_close(client_transport_t *t) {
    if (t->kind == TRANSPORT_UDP) auth_udp_close(&t->udp);
    else                          auth_tcp_conn_close(&t->tcp);
}

/* Send `out` and read one response into `in` with a 5-second timeout.
 * Retries once on UDP timeout (simulates the Rust client's behaviour). */
static auth_err_t send_then_recv(
    client_transport_t *t,
    const uint8_t *out, size_t out_len,
    uint8_t *in, size_t in_cap, size_t *in_len)
{
    const int TIMEOUT_MS = 5000;
    const int N_ATTEMPTS = (t->kind == TRANSPORT_UDP) ? 2 : 1;

    for (int attempt = 0; attempt < N_ATTEMPTS; ++attempt) {
        auth_err_t err;
        if (t->kind == TRANSPORT_UDP) {
            err = auth_udp_send(&t->udp, NULL, out, out_len);
        } else {
            err = auth_tcp_send(&t->tcp, out, out_len);
        }
        if (err) return err;

        if (t->kind == TRANSPORT_UDP) {
            err = auth_udp_recv(&t->udp, in, in_cap, in_len, NULL, TIMEOUT_MS);
        } else {
            err = auth_tcp_recv(&t->tcp, in, in_cap, in_len, TIMEOUT_MS);
        }
        if (err == AUTH_OK) return AUTH_OK;
        if (err != AUTH_ERR_TIMEOUT) return err;
    }
    return AUTH_ERR_TIMEOUT;
}

/* ---- Paths ---- */

static void path_join(char *out, size_t cap, const char *dir, const char *file) {
    snprintf(out, cap, "%s/%s", dir, file);
}

static int mkdir_p(const char *path) {
    struct stat st;
    if (stat(path, &st) == 0) return S_ISDIR(st.st_mode) ? 0 : -1;
    return mkdir(path, 0700);
}

/* ---- SETUP ---- */

static int do_setup(const args_t *a, client_transport_t *t)
{
    char cred_path[512];
    path_join(cred_path, sizeof cred_path, a->state_dir, "credentials.bin");

    auth_credentials_t creds;
    auth_err_t err = auth_creds_load(&creds, cred_path);
    if (err == AUTH_ERR_CREDENTIAL_MISSING) {
        auth_creds_init_fresh(&creds);
        fprintf(stderr, "  [SETUP] Creating fresh credentials in %s\n", cred_path);
    } else if (err != AUTH_OK) {
        fprintf(stderr, "Load creds failed: %s\n", auth_strerror(err));
        return 1;
    }

    /* Prepare client context. */
    auth_client_ctx_t ctx = {0};
    memcpy(ctx.device_root, creds.device_root, 32);
    auth_derive_device_id(ctx.device_id, creds.device_root);
    auth_derive_device_scalar(ctx.device_sk, creds.device_root);
    if (auth_scalarmult_base(ctx.device_pub, ctx.device_sk) != AUTH_OK) {
        fprintf(stderr, "device pub derivation failed\n"); return 1;
    }
    if ((err = auth_client_ctx_init(&ctx)) != AUTH_OK) return 1;
    ctx.has_pinned_server = creds.has_pinned_server;
    memcpy(ctx.server_pub_pinned, creds.server_pub_pinned, 32);
    ctx.has_role = creds.has_role;
    memcpy(ctx.role_commitment, creds.role_commitment, 32);
    memcpy(ctx.role_blind,      creds.role_blind,      32);
    ctx.role_code = creds.has_role ? creds.role_code : a->role;

    uint8_t out[AUTH_MAX_DATAGRAM], in[AUTH_MAX_DATAGRAM];
    size_t out_len = 0, in_len = 0;

    /* SETUP_1 */
    size_t token_len = a->pairing_token ? strlen(a->pairing_token) : 0;
    err = auth_client_build_setup1(&ctx,
        (const uint8_t *)a->pairing_token, token_len,
        out, sizeof out, &out_len);
    if (err) { fprintf(stderr, "build_setup1: %s\n", auth_strerror(err)); return 1; }

    err = send_then_recv(t, out, out_len, in, sizeof in, &in_len);
    if (err) { fprintf(stderr, "SETUP_1 roundtrip: %s\n", auth_strerror(err)); return 1; }

    err = auth_client_handle_setup2(&ctx, in, in_len, a->allow_tofu);
    if (err) { fprintf(stderr, "handle_setup2: %s\n", auth_strerror(err)); return 1; }

    /* SETUP_3 */
    err = auth_client_build_setup3(&ctx, out, sizeof out, &out_len);
    if (err) { fprintf(stderr, "build_setup3: %s\n", auth_strerror(err)); return 1; }
    err = send_then_recv(t, out, out_len, in, sizeof in, &in_len);
    if (err) { fprintf(stderr, "SETUP_3 roundtrip: %s\n", auth_strerror(err)); return 1; }
    err = auth_client_handle_setup_ack(&ctx, in, in_len);
    if (err) { fprintf(stderr, "handle_setup_ack: %s\n", auth_strerror(err)); return 1; }

    /* Persist. */
    creds.has_pinned_server = 1;
    memcpy(creds.server_pub_pinned, ctx.server_pub_pinned, 32);
    creds.has_role = 1;
    memcpy(creds.role_commitment, ctx.role_commitment, 32);
    memcpy(creds.role_blind,      ctx.role_blind,      32);
    creds.role_code = ctx.role_code;
    err = auth_creds_save(&creds, cred_path);
    if (err) { fprintf(stderr, "save creds: %s\n", auth_strerror(err)); return 1; }

    const char *tname = (t->kind == TRANSPORT_UDP) ? "UDP" : "TCP";
    printf("Client[SETUP]: Enrollment OK over %s.\n", tname);
    return 0;
}

/* ---- AUTH ---- */

static int do_auth(const args_t *a, client_transport_t *t)
{
    char cred_path[512];
    path_join(cred_path, sizeof cred_path, a->state_dir, "credentials.bin");

    auth_credentials_t creds;
    auth_err_t err = auth_creds_load(&creds, cred_path);
    if (err != AUTH_OK) {
        fprintf(stderr, "Load creds failed: %s (run --setup first)\n",
                auth_strerror(err));
        return 1;
    }
    if (!creds.has_pinned_server || !creds.has_role) {
        fprintf(stderr, "Credentials incomplete; run --setup first\n");
        return 1;
    }

    auth_client_ctx_t ctx = {0};
    memcpy(ctx.device_root, creds.device_root, 32);
    auth_derive_device_id(ctx.device_id, creds.device_root);
    auth_derive_device_scalar(ctx.device_sk, creds.device_root);
    if (auth_scalarmult_base(ctx.device_pub, ctx.device_sk) != AUTH_OK) {
        fprintf(stderr, "device pub derivation failed\n"); return 1;
    }
    if ((err = auth_client_ctx_init(&ctx)) != AUTH_OK) return 1;
    ctx.has_pinned_server = 1;
    memcpy(ctx.server_pub_pinned, creds.server_pub_pinned, 32);
    ctx.has_role = 1;
    memcpy(ctx.role_commitment, creds.role_commitment, 32);
    memcpy(ctx.role_blind,      creds.role_blind,      32);
    ctx.role_code = creds.role_code;

    uint8_t out[AUTH_MAX_DATAGRAM], in[AUTH_MAX_DATAGRAM];
    size_t out_len = 0, in_len = 0;

    /* AUTH_1 */
    err = auth_client_build_auth1(&ctx, a->allowed, a->n_allowed,
                                      out, sizeof out, &out_len);
    if (err) { fprintf(stderr, "build_auth1: %s\n", auth_strerror(err)); return 1; }

    err = send_then_recv(t, out, out_len, in, sizeof in, &in_len);
    if (err) { fprintf(stderr, "AUTH_1 roundtrip: %s\n", auth_strerror(err)); return 1; }

    err = auth_client_handle_auth2(&ctx, in, in_len);
    if (err) { fprintf(stderr, "handle_auth2: %s\n", auth_strerror(err)); return 1; }

    /* AUTH_3 */
    err = auth_client_build_auth3(&ctx, out, sizeof out, &out_len);
    if (err) { fprintf(stderr, "build_auth3: %s\n", auth_strerror(err)); return 1; }
    err = send_then_recv(t, out, out_len, in, sizeof in, &in_len);
    if (err) { fprintf(stderr, "AUTH_3 roundtrip: %s\n", auth_strerror(err)); return 1; }
    err = auth_client_handle_auth_ack(&ctx, in, in_len);
    if (err) { fprintf(stderr, "handle_auth_ack: %s\n", auth_strerror(err)); return 1; }

    const char *tname = (t->kind == TRANSPORT_UDP) ? "UDP" : "TCP";
    printf("Client[AUTH]: OK over %s. session_key=", tname);
    for (int i = 0; i < 8; ++i) printf("%02x", ctx.session_key[i]);
    printf("\n");
    return 0;
}

/* ---- Main ---- */

int main(int argc, char **argv)
{
    args_t a;
    if (parse_args(argc, argv, &a) != 0) { usage(argv[0]); return 2; }
    if (auth_init() != AUTH_OK) { fprintf(stderr, "sodium init failed\n"); return 1; }
    if (mkdir_p(a.state_dir) != 0) {
        fprintf(stderr, "mkdir %s: %s\n", a.state_dir, strerror(errno));
        return 1;
    }

    client_transport_t t;
    t.kind = a.transport;
    auth_err_t err = transport_connect(&t, a.server);
    if (err) {
        fprintf(stderr, "connect to %s: %s\n", a.server, auth_strerror(err));
        return 1;
    }

    int rc = a.do_setup ? do_setup(&a, &t) : do_auth(&a, &t);
    transport_close(&t);
    return rc;
}
