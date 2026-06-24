/*
 * auth_proto.h — transport-agnostic state machines for setup and
 * online auth. These are pure, deterministic transitions that take
 * a packet in and produce a packet out (or a decision like OK/ERROR).
 * No network I/O, no file I/O; callers wire those up via transport.h
 * and store.h.
 */

#ifndef AUTH_PROTO_H
#define AUTH_PROTO_H

#include "iot_auth.h"
#include "auth_crypto.h"
#include "auth_proofs.h"
#include "auth_payloads.h"
#include "auth_wire.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Client-side state ---- */

/*
 * auth_client_ctx: held by the client across a single setup OR auth
 * session. Allocated by the caller (e.g. on the stack).
 */
typedef struct auth_client_ctx {
    /* Device identity (persistent, loaded from CredentialStore). */
    uint8_t  device_root  [AUTH_DEVICE_ROOT_LEN];
    uint8_t  device_id    [AUTH_DEVICE_ID_LEN];
    uint8_t  device_sk    [AUTH_SCALAR_LEN];    /* x */
    uint8_t  device_pub   [AUTH_POINT_LEN];     /* g*x */

    /* Pinned server pubkey (TOFU or provisioned). Filled in during
     * setup on a blank state dir, then persisted by the caller. */
    uint8_t  server_pub_pinned[AUTH_POINT_LEN];
    int      has_pinned_server;

    /* Role commitment (stored_c) and its blinding factor. */
    uint8_t  role_commitment[AUTH_POINT_LEN];
    uint8_t  role_blind     [AUTH_SCALAR_LEN];
    uint64_t role_code;
    int      has_role;

    /* Per-session transient state. */
    uint8_t  session_id   [AUTH_SESSION_ID_LEN];
    uint32_t seq;
    uint8_t  client_nonce [AUTH_NONCE_LEN];
    uint8_t  eph_secret   [AUTH_SCALAR_LEN];
    uint8_t  eph_c        [AUTH_POINT_LEN];

    /* The actual Schnorr proof the client sent in AUTH_1. Needed for
     * computing the KC transcript hash in handle_auth2(). */
    auth_schnorr_proof_t pending_client_proof;

    /* Captured during setup: pending enrollment data awaiting SETUP_3. */
    uint8_t  pending_server_pub[AUTH_POINT_LEN];
    uint8_t  pending_server_nonce[AUTH_NONCE_LEN];
    uint8_t  pending_setup_challenge[AUTH_SETUP_CHALLENGE_LEN];

    /* Captured during auth: derived session key and KC state. */
    uint8_t  session_key  [AUTH_SESSION_KEY_LEN];
    uint8_t  kc_th        [AUTH_HASH_LEN];
    uint8_t  kc_k_s2c     [AUTH_MAC_KEY_LEN];
    uint8_t  kc_k_c2s     [AUTH_MAC_KEY_LEN];
    int      auth_complete;
} auth_client_ctx_t;

/*
 * Initialize a client context with a fresh session id + seq = 0.
 * The device identity (root/id/sk/pub) must be filled in by the caller
 * before invoking any state-machine function.
 */
auth_err_t auth_client_ctx_init(auth_client_ctx_t *ctx);

/* ---- Client: SETUP ---- */

/*
 * Build SETUP_1 packet. Caller supplies an optional pairing token.
 * Draws a fresh client_nonce and stores it in the context.
 */
auth_err_t auth_client_build_setup1(
    auth_client_ctx_t *ctx,
    const uint8_t *pairing_token, size_t pairing_token_len,
    uint8_t *out, size_t out_cap, size_t *out_len);

/*
 * Handle SETUP_2 from server: verify the server Schnorr proof, stash
 * server_pub/server_nonce/setup_challenge. If TOFU is allowed and no
 * server is pinned yet, the server pubkey is accepted and the caller
 * should persist it after successful SETUP_3 completion.
 *
 * `allow_tofu` controls whether an unpinned server is acceptable.
 */
auth_err_t auth_client_handle_setup2(
    auth_client_ctx_t *ctx,
    const uint8_t *in,  size_t in_len,
    int allow_tofu);

/*
 * Build SETUP_3 packet: produce the client Schnorr proof.
 */
auth_err_t auth_client_build_setup3(
    auth_client_ctx_t *ctx,
    uint8_t *out, size_t out_cap, size_t *out_len);

/*
 * Handle SETUP_ACK from server. Expects a single 0x01 byte payload.
 */
auth_err_t auth_client_handle_setup_ack(
    auth_client_ctx_t *ctx,
    const uint8_t *in, size_t in_len);

/* ---- Client: AUTH ---- */

/*
 * Build AUTH_1 packet: sample fresh nonce_c, fresh eph_c, rerandomize
 * role commitment, produce client Schnorr + rerand + CDS-OR proofs.
 *
 * `allowed_roles` and `n_allowed` describe the role set the verifier
 * will accept (passed explicitly so the client doesn't need to know
 * the whole registry).
 */
auth_err_t auth_client_build_auth1(
    auth_client_ctx_t *ctx,
    const uint64_t *allowed_roles, size_t n_allowed,
    uint8_t *out, size_t out_cap, size_t *out_len);

/*
 * Handle AUTH_2 from server: verify server's Schnorr proof, derive
 * session key via ECDHE, compute KC transcript hash, derive KC keys,
 * and verify `tag_s`.
 */
auth_err_t auth_client_handle_auth2(
    auth_client_ctx_t *ctx,
    const uint8_t *in, size_t in_len);

/*
 * Build AUTH_3 packet: compute `tag_c` over the KC transcript hash.
 */
auth_err_t auth_client_build_auth3(
    auth_client_ctx_t *ctx,
    uint8_t *out, size_t out_cap, size_t *out_len);

auth_err_t auth_client_handle_auth_ack(
    auth_client_ctx_t *ctx,
    const uint8_t *in, size_t in_len);

/* ---- Server-side: pending sessions ----
 *
 * The server holds one pending-setup and one pending-auth record per
 * active session_id. These are allocated by the caller (e.g. inside a
 * HashMap keyed by session_id) and passed to the state-machine
 * functions.
 */

typedef struct auth_pending_setup {
    uint8_t  session_id      [AUTH_SESSION_ID_LEN];
    uint8_t  device_id       [AUTH_DEVICE_ID_LEN];
    uint8_t  device_pub      [AUTH_POINT_LEN];
    uint8_t  role_commitment [AUTH_POINT_LEN];
    uint8_t  client_nonce    [AUTH_NONCE_LEN];
    uint8_t  server_nonce    [AUTH_NONCE_LEN];
    uint8_t  setup_challenge [AUTH_SETUP_CHALLENGE_LEN];
    uint8_t  server_pub      [AUTH_POINT_LEN];  /* cached */
    /* Cached SETUP_2 response packet for retransmit idempotency. */
    uint8_t  response_packet [AUTH_MAX_DATAGRAM];
    size_t   response_packet_len;
} auth_pending_setup_t;

typedef struct auth_pending_auth {
    uint8_t  session_id   [AUTH_SESSION_ID_LEN];
    uint8_t  device_id    [AUTH_DEVICE_ID_LEN];
    uint8_t  device_pub   [AUTH_POINT_LEN];
    uint8_t  pid          [AUTH_HASH_LEN];
    uint8_t  nonce_c      [AUTH_NONCE_LEN];
    uint8_t  nonce_s      [AUTH_NONCE_LEN];
    uint8_t  eph_c        [AUTH_POINT_LEN];
    uint8_t  eph_s        [AUTH_POINT_LEN];
    uint8_t  session_key  [AUTH_SESSION_KEY_LEN];
    uint8_t  kc_th        [AUTH_HASH_LEN];
    uint8_t  kc_k_s2c     [AUTH_MAC_KEY_LEN];
    uint8_t  kc_k_c2s     [AUTH_MAC_KEY_LEN];
    /* Cached AUTH_2 response packet for retransmit idempotency. */
    uint8_t  response_packet [AUTH_MAX_DATAGRAM];
    size_t   response_packet_len;
} auth_pending_auth_t;

/* ---- Server state machine: SETUP ---- */

/*
 * Handle SETUP_1 from a client. On success:
 *   - writes SETUP_2 packet into out/out_len
 *   - fills `pending_out` with per-session state to be cached by caller
 *
 * `require_pairing_token` (NULL if not required): if present, the
 * client's pairing_token must match this string exactly, else
 * AUTH_ERR_PAIRING_TOKEN_BAD.
 */
auth_err_t auth_server_handle_setup1(
    const uint8_t session_id[AUTH_SESSION_ID_LEN],
    uint32_t seq,
    const uint8_t *in_payload, size_t in_len,
    const uint8_t server_sk[AUTH_SCALAR_LEN],
    const uint8_t server_pub[AUTH_POINT_LEN],
    const char *require_pairing_token,
    auth_pending_setup_t *pending_out,
    uint8_t *out, size_t out_cap, size_t *out_len);

/*
 * Handle SETUP_3: verify the client Schnorr proof against the pending
 * state, and produce SETUP_ACK. On success, the caller should persist
 * the device to its registry BEFORE sending SETUP_ACK.
 */
auth_err_t auth_server_handle_setup3(
    const auth_pending_setup_t *pending,
    const uint8_t session_id[AUTH_SESSION_ID_LEN],
    uint32_t seq,
    const uint8_t *in_payload, size_t in_len,
    uint8_t *out, size_t out_cap, size_t *out_len);

/* ---- Server state machine: AUTH ---- */

/*
 * Registry-lookup callback: given a device_id, return the enrolled
 * device_pub and role commitment (or AUTH_ERR_UNKNOWN_DEVICE).
 * The implementation is provided by the store layer (store/fs.c).
 */
typedef auth_err_t (*auth_registry_lookup_fn)(
    void *ctx,
    const uint8_t device_id[AUTH_DEVICE_ID_LEN],
    uint8_t device_pub[AUTH_POINT_LEN],
    uint8_t role_commitment[AUTH_POINT_LEN]);

/*
 * Handle AUTH_1: look up device in registry, verify client Schnorr proof,
 * verify rerand and CDS-OR, compute server nonce/eph/proof, derive
 * session key, build AUTH_2 packet.
 *
 * The caller owns the `pending_out` slot; on success it must be stored
 * keyed by session_id to receive AUTH_3.
 */
auth_err_t auth_server_handle_auth1(
    const uint8_t session_id[AUTH_SESSION_ID_LEN],
    uint32_t seq,
    const uint8_t *in_payload, size_t in_len,
    const uint8_t server_sk[AUTH_SCALAR_LEN],
    const uint8_t server_pub[AUTH_POINT_LEN],
    auth_registry_lookup_fn lookup_fn, void *lookup_ctx,
    const uint64_t *allowed_roles, size_t n_allowed,
    auth_pending_auth_t *pending_out,
    uint8_t *out, size_t out_cap, size_t *out_len);

/*
 * Handle AUTH_3: verify tag_c, produce AUTH_ACK.
 */
auth_err_t auth_server_handle_auth3(
    const auth_pending_auth_t *pending,
    const uint8_t session_id[AUTH_SESSION_ID_LEN],
    uint32_t seq,
    const uint8_t *in_payload, size_t in_len,
    uint8_t *out, size_t out_cap, size_t *out_len);

#ifdef __cplusplus
}
#endif
#endif /* AUTH_PROTO_H */
