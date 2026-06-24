/*
 * auth_server — C server binary.
 *
 * Usage:
 *   auth_server --bind 0.0.0.0:4000 [--transport udp|tcp|both]
 *                   [--state-dir DIR] [--require-pairing-token T]
 *
 * Matches the Rust server CLI: supports UDP, TCP, or both concurrently
 * sharing the same registry / key store / session caches.
 */

#define _POSIX_C_SOURCE 200112L
#define _DEFAULT_SOURCE

#include "auth/iot_auth.h"
#include "auth/auth_crypto.h"
#include "auth/auth_proto.h"
#include "auth/auth_store.h"
#include "auth/auth_transport.h"
#include "auth/auth_wire.h"

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/* ---- CLI ---- */

typedef enum { TRANSPORT_UDP = 1, TRANSPORT_TCP = 2, TRANSPORT_BOTH = 3 } transport_kind_t;

typedef struct {
    const char *bind;
    transport_kind_t transport;
    const char *state_dir;
    const char *require_pairing_token;
} args_t;

static void usage(const char *prog) {
    fprintf(stderr,
"Usage:\n"
"  %s --bind HOST:PORT [--transport udp|tcp|both]\n"
"     [--state-dir DIR] [--require-pairing-token T]\n"
"\n"
"  The same address binds both UDP and TCP when --transport=both.\n",
        prog);
}

static int parse_args(int argc, char **argv, args_t *a) {
    memset(a, 0, sizeof *a);
    a->transport = TRANSPORT_UDP;
    for (int i = 1; i < argc; ++i) {
        const char *k = argv[i];
        if      (strcmp(k, "--bind") == 0 && i+1 < argc)       a->bind = argv[++i];
        else if (strcmp(k, "--state-dir") == 0 && i+1 < argc)  a->state_dir = argv[++i];
        else if (strcmp(k, "--require-pairing-token") == 0 && i+1 < argc) a->require_pairing_token = argv[++i];
        else if (strcmp(k, "--transport") == 0 && i+1 < argc) {
            const char *v = argv[++i];
            if      (strcmp(v, "udp") == 0)  a->transport = TRANSPORT_UDP;
            else if (strcmp(v, "tcp") == 0)  a->transport = TRANSPORT_TCP;
            else if (strcmp(v, "both") == 0) a->transport = TRANSPORT_BOTH;
            else return -1;
        }
        else if (strcmp(k, "-h") == 0 || strcmp(k, "--help") == 0) { usage(argv[0]); exit(0); }
        else return -1;
    }
    if (!a->bind) return -1;
    if (!a->state_dir) a->state_dir = "./server-state";
    return 0;
}

/* ---- Shared server state ---- */

#ifndef MAX_SESSIONS
#define MAX_SESSIONS 64
#endif

/* Session cache entry: either setup OR auth pending, keyed by session_id. */
typedef struct {
    int      in_use;
    int      is_auth;  /* 0 = setup, 1 = auth */
    union {
        auth_pending_setup_t s;
        auth_pending_auth_t  a;
    } u;
} session_slot_t;

typedef struct {
    /* Persistent. */
    auth_registry_t registry;
    uint8_t             server_sk[32];
    uint8_t             server_pub[32];
    char                registry_path[512];

    /* Active sessions. */
    session_slot_t  sessions[MAX_SESSIONS];
    pthread_mutex_t mu;

    /* Policy. */
    const char *require_pairing_token;
    uint64_t    allowed_roles[AUTH_MAX_ROLES];
    size_t      n_allowed;
} server_state_t;

static int session_find(server_state_t *S, const uint8_t sid[16]) {
    for (int i = 0; i < MAX_SESSIONS; ++i) {
        if (S->sessions[i].in_use) {
            const uint8_t *k = S->sessions[i].is_auth
                ? S->sessions[i].u.a.session_id
                : S->sessions[i].u.s.session_id;
            if (memcmp(k, sid, 16) == 0) return i;
        }
    }
    return -1;
}

static int session_alloc(server_state_t *S) {
    for (int i = 0; i < MAX_SESSIONS; ++i) {
        if (!S->sessions[i].in_use) return i;
    }
    return -1;
}

/* ---- AUTH_1 candidate-scan ----
 *
 * The server doesn't know which enrolled device sent this AUTH_1 until
 * it finds one whose registered device_pub makes the Schnorr proof
 * verify and whose stored role_commitment makes the rerand proof
 * verify. We try each in order.
 *
 * We implement this by writing a small "scan + dispatch" loop that
 * feeds each candidate into auth_server_handle_auth1 via a
 * scan-state lookup callback.
 */

typedef struct {
    server_state_t *S;
    size_t          next_idx;
    /* The candidate we report this time. */
    uint8_t         candidate_pub[32];
    uint8_t         candidate_rc [32];
    int             have_candidate;
} scan_state_t;

static auth_err_t scan_lookup(
    void *ctx,
    const uint8_t device_id[AUTH_DEVICE_ID_LEN],
    uint8_t device_pub[32],
    uint8_t role_commitment[32])
{
    (void)device_id;
    scan_state_t *s = (scan_state_t *)ctx;
    if (!s->have_candidate) return AUTH_ERR_UNKNOWN_DEVICE;
    memcpy(device_pub,      s->candidate_pub, 32);
    memcpy(role_commitment, s->candidate_rc,  32);
    return AUTH_OK;
}

/* Try each registry entry as the device. Returns the slot index of the
 * filled pending auth (>=0) or -1 on failure. On failure the last error
 * is returned via *err_out. */
static int try_handle_auth1(
    server_state_t *S,
    const uint8_t sid[16], uint32_t seq,
    const uint8_t *payload, size_t payload_len,
    uint8_t *out, size_t out_cap, size_t *out_len,
    auth_err_t *err_out)
{
    pthread_mutex_lock(&S->mu);
    int slot = session_alloc(S);
    pthread_mutex_unlock(&S->mu);
    if (slot < 0) { *err_out = AUTH_ERR_TOO_MANY_ACTIVE; return -1; }

    scan_state_t scan = { .S = S, .next_idx = 0, .have_candidate = 0 };
    auth_err_t last = AUTH_ERR_UNKNOWN_DEVICE;

    pthread_mutex_lock(&S->mu);
    size_t reg_n = S->registry.n;
    pthread_mutex_unlock(&S->mu);

    for (size_t i = 0; i < reg_n; ++i) {
        pthread_mutex_lock(&S->mu);
        memcpy(scan.candidate_pub, S->registry.entries[i].device_pub,      32);
        memcpy(scan.candidate_rc,  S->registry.entries[i].role_commitment, 32);
        pthread_mutex_unlock(&S->mu);
        scan.have_candidate = 1;

        auth_pending_auth_t pending = {0};
        auth_err_t err = auth_server_handle_auth1(
            sid, seq, payload, payload_len,
            S->server_sk, S->server_pub,
            scan_lookup, &scan,
            S->allowed_roles, S->n_allowed,
            &pending, out, out_cap, out_len);
        if (err == AUTH_OK) {
            pthread_mutex_lock(&S->mu);
            S->sessions[slot].in_use = 1;
            S->sessions[slot].is_auth = 1;
            S->sessions[slot].u.a = pending;
            pthread_mutex_unlock(&S->mu);
            return slot;
        }
        last = err;
        /* Keep scanning unless it's clearly a non-device-match error. */
        if (err != AUTH_ERR_PROOF_VERIFY &&
            err != AUTH_ERR_ROLE_NOT_PERMITTED) {
            break;
        }
    }
    *err_out = last;
    return -1;
}

/* ---- Dispatch one incoming packet ---- */

static void dispatch(
    server_state_t *S,
    const uint8_t *in, size_t in_len,
    uint8_t *out, size_t out_cap, size_t *out_len)
{
    *out_len = 0;
    auth_header_t hdr;
    const uint8_t *payload;
    size_t payload_len;
    auth_err_t err = auth_header_decode(&hdr, in, in_len, &payload, &payload_len);
    if (err) {
        fprintf(stderr, "  [wire] bad header: %s\n", auth_strerror(err));
        return;
    }

    switch (hdr.pkt_type) {
    case AUTH_PKT_SETUP_1: {
        pthread_mutex_lock(&S->mu);
        int slot = session_alloc(S);
        pthread_mutex_unlock(&S->mu);
        if (slot < 0) { err = AUTH_ERR_TOO_MANY_ACTIVE; goto error_reply; }

        auth_pending_setup_t pending = {0};
        err = auth_server_handle_setup1(
            hdr.session_id, hdr.seq, payload, payload_len,
            S->server_sk, S->server_pub,
            S->require_pairing_token,
            &pending, out, out_cap, out_len);
        if (err) goto error_reply;

        pthread_mutex_lock(&S->mu);
        S->sessions[slot].in_use  = 1;
        S->sessions[slot].is_auth = 0;
        S->sessions[slot].u.s = pending;
        pthread_mutex_unlock(&S->mu);
        return;
    }

    case AUTH_PKT_SETUP_3: {
        pthread_mutex_lock(&S->mu);
        int slot = session_find(S, hdr.session_id);
        if (slot < 0 || S->sessions[slot].is_auth) {
            pthread_mutex_unlock(&S->mu);
            err = AUTH_ERR_UNKNOWN_SESSION; goto error_reply;
        }
        auth_pending_setup_t pending = S->sessions[slot].u.s;
        pthread_mutex_unlock(&S->mu);

        err = auth_server_handle_setup3(&pending,
            hdr.session_id, hdr.seq, payload, payload_len,
            out, out_cap, out_len);
        if (err) goto error_reply;

        /* Persist to registry. */
        pthread_mutex_lock(&S->mu);
        err = auth_registry_put(&S->registry,
            pending.device_id, pending.device_pub, pending.role_commitment);
        if (!err) err = auth_registry_save(&S->registry);
        S->sessions[slot].in_use = 0;
        pthread_mutex_unlock(&S->mu);
        if (err) goto error_reply;
        return;
    }

    case AUTH_PKT_AUTH_1: {
        int slot = try_handle_auth1(S,
            hdr.session_id, hdr.seq, payload, payload_len,
            out, out_cap, out_len, &err);
        if (slot < 0) goto error_reply;
        return;
    }

    case AUTH_PKT_AUTH_3: {
        pthread_mutex_lock(&S->mu);
        int slot = session_find(S, hdr.session_id);
        if (slot < 0 || !S->sessions[slot].is_auth) {
            pthread_mutex_unlock(&S->mu);
            err = AUTH_ERR_UNKNOWN_SESSION; goto error_reply;
        }
        auth_pending_auth_t pending = S->sessions[slot].u.a;
        pthread_mutex_unlock(&S->mu);

        err = auth_server_handle_auth3(&pending,
            hdr.session_id, hdr.seq, payload, payload_len,
            out, out_cap, out_len);
        pthread_mutex_lock(&S->mu);
        S->sessions[slot].in_use = 0;
        pthread_mutex_unlock(&S->mu);
        if (err) goto error_reply;
        return;
    }

    default:
        err = AUTH_ERR_UNKNOWN_PKT_TYPE;
        goto error_reply;
    }

error_reply:
    fprintf(stderr, "  [proto] %s\n", auth_strerror(err));
    auth_packet_build_error(hdr.session_id, hdr.seq, err,
        auth_strerror(err), out, out_cap, out_len);
}

/* ---- UDP driver ---- */

typedef struct { server_state_t *S; const char *bind; } udp_args_t;

static void *run_udp(void *vp) {
    udp_args_t *ua = (udp_args_t *)vp;
    auth_udp_t u;
    auth_err_t err = auth_udp_bind(&u, ua->bind);
    if (err) {
        fprintf(stderr, "udp bind: %s\n", auth_strerror(err));
        return NULL;
    }
    auth_addr_t local;
    if (auth_udp_local_addr(&u, &local) == AUTH_OK) {
        char lb[128]; auth_addr_format(&local, lb, sizeof lb);
        printf("  [UDP] listening on %s\n", lb);
    }

    uint8_t in[AUTH_MAX_DATAGRAM], out[AUTH_MAX_DATAGRAM];
    for (;;) {
        size_t in_len = 0;
        auth_addr_t peer;
        err = auth_udp_recv(&u, in, sizeof in, &in_len, &peer, -1);
        if (err == AUTH_ERR_TIMEOUT) continue;
        if (err) { fprintf(stderr, "udp recv: %s\n", auth_strerror(err)); continue; }

        size_t out_len = 0;
        dispatch(ua->S, in, in_len, out, sizeof out, &out_len);
        if (out_len) {
            auth_udp_send(&u, &peer, out, out_len);
        }
    }
}

/* ---- TCP driver ---- */

typedef struct {
    server_state_t *S;
    int             fd;
    auth_addr_t peer;
} tcp_conn_args_t;

static void *handle_tcp_conn(void *vp) {
    tcp_conn_args_t *ca = (tcp_conn_args_t *)vp;
    auth_tcp_conn_t conn;
    auth_tcp_conn_wrap(&conn, ca->fd);

    uint8_t in[AUTH_MAX_DATAGRAM], out[AUTH_MAX_DATAGRAM];
    const int IO_TIMEOUT_MS = 30000;

    for (;;) {
        size_t in_len = 0;
        auth_err_t err = auth_tcp_recv(&conn, in, sizeof in, &in_len, IO_TIMEOUT_MS);
        if (err == AUTH_ERR_IO) break;      /* clean EOF */
        if (err == AUTH_ERR_TIMEOUT) break; /* idle */
        if (err) { fprintf(stderr, "tcp recv: %s\n", auth_strerror(err)); break; }

        size_t out_len = 0;
        dispatch(ca->S, in, in_len, out, sizeof out, &out_len);
        if (out_len) {
            err = auth_tcp_send(&conn, out, out_len);
            if (err) { fprintf(stderr, "tcp send: %s\n", auth_strerror(err)); break; }
        }
    }
    auth_tcp_conn_close(&conn);
    free(ca);
    return NULL;
}

typedef struct { server_state_t *S; const char *bind; } tcp_args_t;

static void *run_tcp(void *vp) {
    tcp_args_t *ta = (tcp_args_t *)vp;
    auth_tcp_listener_t l;
    auth_err_t err = auth_tcp_listener_bind(&l, ta->bind, 64);
    if (err) {
        fprintf(stderr, "tcp bind: %s\n", auth_strerror(err));
        return NULL;
    }
    auth_addr_t local;
    if (auth_tcp_listener_local_addr(&l, &local) == AUTH_OK) {
        char lb[128]; auth_addr_format(&local, lb, sizeof lb);
        printf("  [TCP] listening on %s\n", lb);
    }

    for (;;) {
        int peer_fd = -1;
        auth_addr_t peer;
        err = auth_tcp_listener_accept(&l, &peer_fd, &peer);
        if (err) { fprintf(stderr, "accept: %s\n", auth_strerror(err)); continue; }

        tcp_conn_args_t *ca = (tcp_conn_args_t *)malloc(sizeof *ca);
        if (!ca) { close(peer_fd); continue; }
        ca->S = ta->S; ca->fd = peer_fd; ca->peer = peer;
        pthread_t th;
        if (pthread_create(&th, NULL, handle_tcp_conn, ca) != 0) {
            fprintf(stderr, "pthread_create: %s\n", strerror(errno));
            close(peer_fd);
            free(ca);
            continue;
        }
        pthread_detach(th);
    }
}

/* ---- Main ---- */

static int mkdir_p(const char *path) {
    struct stat st;
    if (stat(path, &st) == 0) return S_ISDIR(st.st_mode) ? 0 : -1;
    return mkdir(path, 0700);
}

int main(int argc, char **argv)
{
    args_t a;
    if (parse_args(argc, argv, &a) != 0) { usage(argv[0]); return 2; }
    if (auth_init() != AUTH_OK) { fprintf(stderr, "sodium init failed\n"); return 1; }
    if (mkdir_p(a.state_dir) != 0) {
        fprintf(stderr, "mkdir %s: %s\n", a.state_dir, strerror(errno)); return 1;
    }

    server_state_t S;
    memset(&S, 0, sizeof S);
    pthread_mutex_init(&S.mu, NULL);
    S.require_pairing_token = a.require_pairing_token;
    uint64_t default_roles[2] = {1, 2};
    memcpy(S.allowed_roles, default_roles, sizeof default_roles);
    S.n_allowed = 2;

    char reg_path[512], sk_path[512];
    snprintf(reg_path, sizeof reg_path, "%s/registry.bin", a.state_dir);
    snprintf(sk_path,  sizeof sk_path,  "%s/server_sk.bin", a.state_dir);
    memcpy(S.registry_path, reg_path, strlen(reg_path) + 1);

    auth_err_t err = auth_registry_load(&S.registry, reg_path);
    if (err) { fprintf(stderr, "registry load: %s\n", auth_strerror(err)); return 1; }
    err = auth_server_key_load_or_create(S.server_sk, sk_path);
    if (err) { fprintf(stderr, "server key: %s\n", auth_strerror(err)); return 1; }
    if (auth_scalarmult_base(S.server_pub, S.server_sk) != AUTH_OK) return 1;

    printf("iot-auth-server (C) v%d.%d.%d starting. server_pub=",
           AUTH_LIB_VERSION_MAJOR, AUTH_LIB_VERSION_MINOR,
           AUTH_LIB_VERSION_PATCH);
    for (int i = 0; i < 32; ++i) printf("%02x", S.server_pub[i]);
    printf("\n");

    pthread_t tu = 0, tt = 0;
    udp_args_t ua = { &S, a.bind };
    tcp_args_t ta = { &S, a.bind };
    if (a.transport & TRANSPORT_UDP) pthread_create(&tu, NULL, run_udp, &ua);
    if (a.transport & TRANSPORT_TCP) pthread_create(&tt, NULL, run_tcp, &ta);

    if (tu) pthread_join(tu, NULL);
    if (tt) pthread_join(tt, NULL);
    return 0;
}
