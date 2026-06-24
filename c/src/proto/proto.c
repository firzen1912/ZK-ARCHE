/*
 * auth_proto.c — setup and auth state machines.
 *
 * Each function is a pure, transport-independent transformation from
 * (incoming payload + context) to (outgoing packet + updated context).
 * Callers drive these via UDP, TCP, or any other framing.
 */

#include "auth/auth_proto.h"
#include "auth/auth_crypto.h"
#include "auth/auth_proofs.h"
#include "auth/auth_payloads.h"
#include "auth/auth_transcript.h"
#include "auth/auth_wire.h"

#include <sodium.h>
#include <string.h>

/* ======================================================================
 * Client: init
 * ====================================================================== */

auth_err_t auth_client_ctx_init(auth_client_ctx_t *ctx)
{
    if (!ctx) return AUTH_ERR_INVALID_ARGUMENT;
    randombytes_buf(ctx->session_id, AUTH_SESSION_ID_LEN);
    ctx->seq = 0;
    ctx->has_pinned_server = 0;
    ctx->has_role = 0;
    ctx->auth_complete = 0;
    return AUTH_OK;
}

/* ======================================================================
 * Client: SETUP
 * ====================================================================== */

auth_err_t auth_client_build_setup1(
    auth_client_ctx_t *ctx,
    const uint8_t *pairing_token, size_t pairing_token_len,
    uint8_t *out, size_t out_cap, size_t *out_len)
{
    if (!ctx || !out) return AUTH_ERR_INVALID_ARGUMENT;
    if (pairing_token_len > AUTH_MAX_PAIRING_TOKEN)
        return AUTH_ERR_INVALID_ARGUMENT;
    if (pairing_token_len && !pairing_token)
        return AUTH_ERR_INVALID_ARGUMENT;

    /* Fresh client nonce. */
    auth_random_bytes32(ctx->client_nonce);

    /* Sample role_blind if we don't have one yet. */
    if (!ctx->has_role) {
        if (ctx->role_code == 0) ctx->role_code = 1; /* "member" */
        auth_random_scalar(ctx->role_blind);
        auth_err_t err = auth_make_role_commitment(
            ctx->role_commitment, ctx->role_code, ctx->role_blind);
        if (err) return err;
        ctx->has_role = 1;
    }

    auth_setup1_t s1 = {0};
    if (pairing_token_len) {
        memcpy(s1.pairing_token, pairing_token, pairing_token_len);
        s1.pairing_token_len = pairing_token_len;
    }
    memcpy(s1.device_id,       ctx->device_id,       32);
    memcpy(s1.device_pub,      ctx->device_pub,      32);
    memcpy(s1.client_nonce,    ctx->client_nonce,    32);
    memcpy(s1.role_commitment, ctx->role_commitment, 32);

    uint8_t payload[AUTH_MAX_PAYLOAD];
    size_t  payload_len = 0;
    auth_err_t err = auth_setup1_encode(&s1, payload, sizeof payload, &payload_len);
    if (err) return err;

    ctx->seq += 1;
    return auth_packet_build(AUTH_PKT_SETUP_1, ctx->session_id, ctx->seq,
                                 payload, payload_len, out, out_cap, out_len);
}

auth_err_t auth_client_handle_setup2(
    auth_client_ctx_t *ctx,
    const uint8_t *in,  size_t in_len,
    int allow_tofu)
{
    if (!ctx || !in) return AUTH_ERR_INVALID_ARGUMENT;

    auth_header_t hdr;
    const uint8_t *payload;
    size_t payload_len;
    auth_err_t err = auth_header_decode(&hdr, in, in_len, &payload, &payload_len);
    if (err) return err;
    if (hdr.pkt_type == AUTH_PKT_ERROR) {
        auth_err_t code = AUTH_ERR_UNSPECIFIED;
        auth_packet_parse_error(payload, payload_len, &code, NULL, NULL);
        return code;
    }
    if (hdr.pkt_type != AUTH_PKT_SETUP_2) return AUTH_ERR_UNKNOWN_PKT_TYPE;
    if (memcmp(hdr.session_id, ctx->session_id, AUTH_SESSION_ID_LEN) != 0)
        return AUTH_ERR_UNKNOWN_SESSION;

    auth_setup2_t s2;
    err = auth_setup2_decode(&s2, payload, payload_len);
    if (err) return err;

    err = auth_check_point(s2.server_pub);
    if (err) return err;

    /* TOFU / pinning check. */
    if (ctx->has_pinned_server) {
        if (memcmp(ctx->server_pub_pinned, s2.server_pub, 32) != 0)
            return AUTH_ERR_PEER_KEY_MISMATCH;
    } else if (!allow_tofu) {
        return AUTH_ERR_PEER_KEY_MISMATCH;
    } else {
        memcpy(ctx->server_pub_pinned, s2.server_pub, 32);
        ctx->has_pinned_server = 1;
    }

    if (!auth_verify_setup_server(
            &s2.server_proof, s2.server_pub,
            ctx->device_id, ctx->device_pub,
            ctx->client_nonce, s2.server_nonce,
            s2.setup_challenge)) {
        return AUTH_ERR_PROOF_VERIFY;
    }

    memcpy(ctx->pending_server_pub,      s2.server_pub,      32);
    memcpy(ctx->pending_server_nonce,    s2.server_nonce,    32);
    memcpy(ctx->pending_setup_challenge, s2.setup_challenge, AUTH_SETUP_CHALLENGE_LEN);
    return AUTH_OK;
}

auth_err_t auth_client_build_setup3(
    auth_client_ctx_t *ctx,
    uint8_t *out, size_t out_cap, size_t *out_len)
{
    if (!ctx) return AUTH_ERR_INVALID_ARGUMENT;

    auth_setup3_t s3 = {0};
    auth_err_t err = auth_prove_setup_client(
        &s3.client_proof,
        ctx->device_sk,
        ctx->device_id,
        ctx->device_pub,
        ctx->pending_server_pub,
        ctx->client_nonce,
        ctx->pending_server_nonce,
        ctx->pending_setup_challenge);
    if (err) return err;

    uint8_t payload[AUTH_MAX_PAYLOAD];
    size_t payload_len = 0;
    err = auth_setup3_encode(&s3, payload, sizeof payload, &payload_len);
    if (err) return err;

    ctx->seq += 1;
    return auth_packet_build(AUTH_PKT_SETUP_3, ctx->session_id, ctx->seq,
                                 payload, payload_len, out, out_cap, out_len);
}

auth_err_t auth_client_handle_setup_ack(
    auth_client_ctx_t *ctx,
    const uint8_t *in, size_t in_len)
{
    if (!ctx || !in) return AUTH_ERR_INVALID_ARGUMENT;
    auth_header_t hdr;
    const uint8_t *payload;
    size_t payload_len;
    auth_err_t err = auth_header_decode(&hdr, in, in_len, &payload, &payload_len);
    if (err) return err;
    if (hdr.pkt_type == AUTH_PKT_ERROR) {
        auth_err_t code = AUTH_ERR_UNSPECIFIED;
        auth_packet_parse_error(payload, payload_len, &code, NULL, NULL);
        return code;
    }
    if (hdr.pkt_type != AUTH_PKT_SETUP_ACK) return AUTH_ERR_UNKNOWN_PKT_TYPE;
    return auth_ack_decode(payload, payload_len);
}

/* ======================================================================
 * Client: AUTH
 * ====================================================================== */

auth_err_t auth_client_build_auth1(
    auth_client_ctx_t *ctx,
    const uint64_t *allowed_roles, size_t n_allowed,
    uint8_t *out, size_t out_cap, size_t *out_len)
{
    if (!ctx || !allowed_roles) return AUTH_ERR_INVALID_ARGUMENT;
    if (!ctx->has_pinned_server) return AUTH_ERR_DEVICE_NOT_ENROLLED;
    if (!ctx->has_role) return AUTH_ERR_ROLE_NOT_PERMITTED;
    if (n_allowed == 0 || n_allowed > AUTH_MAX_ROLES)
        return AUTH_ERR_INVALID_ARGUMENT;

    auth_random_bytes32(ctx->client_nonce);
    auth_random_scalar(ctx->eph_secret);
    auth_err_t err = auth_scalarmult_base(ctx->eph_c, ctx->eph_secret);
    if (err) return err;

    uint8_t pid[32];
    err = auth_compute_pid(pid,
        ctx->device_pub, ctx->client_nonce, ctx->eph_c, ctx->server_pub_pinned);
    if (err) return err;

    uint8_t delta[32];
    auth_random_scalar(delta);
    uint8_t c_prime[32], blind_prime[32];
    err = auth_rerandomize_commitment(c_prime, blind_prime,
        ctx->role_commitment, ctx->role_blind, delta);
    if (err) return err;

    auth_auth1_t a1 = {0};
    memcpy(a1.pid,     pid,              32);
    memcpy(a1.nonce_c, ctx->client_nonce, 32);
    memcpy(a1.eph_c,   ctx->eph_c,       32);
    memcpy(a1.c_prime, c_prime,          32);

    /* Produce client Schnorr proof AND cache it for KC transcript hash. */
    err = auth_prove_auth_client(&a1.client_proof,
        ctx->device_sk, pid, ctx->client_nonce, ctx->eph_c);
    if (err) { auth_zeroize(delta, 32); auth_zeroize(blind_prime, 32); return err; }
    memcpy(&ctx->pending_client_proof, &a1.client_proof, sizeof a1.client_proof);

    err = auth_prove_rerandomization(&a1.rerand_proof,
        ctx->role_commitment, c_prime, delta,
        pid, ctx->client_nonce, ctx->eph_c);
    if (err) { auth_zeroize(delta, 32); auth_zeroize(blind_prime, 32); return err; }

    size_t n_branches_out = 0;
    err = auth_prove_role_set_membership(
        a1.branches, &n_branches_out,
        allowed_roles, n_allowed,
        c_prime, ctx->role_code, blind_prime,
        pid, ctx->client_nonce, ctx->eph_c);
    auth_zeroize(delta,       32);
    auth_zeroize(blind_prime, 32);
    if (err) return err;
    a1.n_branches = n_branches_out;

    uint8_t payload[AUTH_MAX_PAYLOAD];
    size_t payload_len = 0;
    err = auth_auth1_encode(&a1, payload, sizeof payload, &payload_len);
    if (err) return err;

    ctx->seq += 1;
    return auth_packet_build(AUTH_PKT_AUTH_1, ctx->session_id, ctx->seq,
                                 payload, payload_len, out, out_cap, out_len);
}

auth_err_t auth_client_handle_auth2(
    auth_client_ctx_t *ctx,
    const uint8_t *in, size_t in_len)
{
    if (!ctx || !in) return AUTH_ERR_INVALID_ARGUMENT;

    auth_header_t hdr;
    const uint8_t *payload;
    size_t payload_len;
    auth_err_t err = auth_header_decode(&hdr, in, in_len, &payload, &payload_len);
    if (err) return err;
    if (hdr.pkt_type == AUTH_PKT_ERROR) {
        auth_err_t code = AUTH_ERR_UNSPECIFIED;
        auth_packet_parse_error(payload, payload_len, &code, NULL, NULL);
        return code;
    }
    if (hdr.pkt_type != AUTH_PKT_AUTH_2) return AUTH_ERR_UNKNOWN_PKT_TYPE;
    if (memcmp(hdr.session_id, ctx->session_id, AUTH_SESSION_ID_LEN) != 0)
        return AUTH_ERR_UNKNOWN_SESSION;

    auth_auth2_t a2;
    err = auth_auth2_decode(&a2, payload, payload_len);
    if (err) return err;

    err = auth_check_point(a2.server_pub);
    if (err) return err;
    err = auth_check_point(a2.eph_s);
    if (err) return err;

    if (memcmp(a2.server_pub, ctx->server_pub_pinned, 32) != 0)
        return AUTH_ERR_PEER_KEY_MISMATCH;

    if (!auth_verify_auth_server(&a2.server_proof, a2.server_pub,
                                     a2.nonce_s, a2.eph_s)) {
        return AUTH_ERR_PROOF_VERIFY;
    }

    /* Derive session key. */
    uint8_t pid[32];
    err = auth_compute_pid(pid,
        ctx->device_pub, ctx->client_nonce, ctx->eph_c, ctx->server_pub_pinned);
    if (err) return err;
    err = auth_derive_session_key(ctx->session_key,
        ctx->eph_secret, a2.eph_s,
        ctx->client_nonce, a2.nonce_s, pid, ctx->eph_c, a2.eph_s);
    if (err) return err;

    /* KC transcript: binds ACTUAL (a_c, s_c) we sent + server's (a_s, s_s). */
    err = auth_kc_transcript_hash(ctx->kc_th,
        pid,
        &ctx->pending_client_proof,
        ctx->client_nonce, ctx->eph_c,
        a2.server_pub,
        &a2.server_proof,
        a2.nonce_s, a2.eph_s);
    if (err) return err;

    auth_derive_kc_keys(ctx->kc_k_s2c, ctx->kc_k_c2s,
                            ctx->session_key, ctx->kc_th);

    /* Verify tag_s = HMAC(k_s2c, "server finished" || kc_th). */
    uint8_t expect_tag[AUTH_MAC_TAG_LEN];
    auth_hmac_tag(expect_tag, ctx->kc_k_s2c,
                      (const uint8_t *)"server finished", 15, ctx->kc_th);
    if (sodium_memcmp(expect_tag, a2.tag_s, AUTH_MAC_TAG_LEN) != 0) {
        return AUTH_ERR_KEY_CONFIRM;
    }
    return AUTH_OK;
}

auth_err_t auth_client_build_auth3(
    auth_client_ctx_t *ctx,
    uint8_t *out, size_t out_cap, size_t *out_len)
{
    if (!ctx) return AUTH_ERR_INVALID_ARGUMENT;

    auth_auth3_t a3 = {0};
    auth_hmac_tag(a3.tag_c, ctx->kc_k_c2s,
                      (const uint8_t *)"client finished", 15, ctx->kc_th);

    uint8_t payload[AUTH_MAX_PAYLOAD];
    size_t payload_len = 0;
    auth_err_t err = auth_auth3_encode(&a3, payload, sizeof payload, &payload_len);
    if (err) return err;

    ctx->seq += 1;
    return auth_packet_build(AUTH_PKT_AUTH_3, ctx->session_id, ctx->seq,
                                 payload, payload_len, out, out_cap, out_len);
}

auth_err_t auth_client_handle_auth_ack(
    auth_client_ctx_t *ctx,
    const uint8_t *in, size_t in_len)
{
    if (!ctx || !in) return AUTH_ERR_INVALID_ARGUMENT;
    auth_header_t hdr;
    const uint8_t *payload;
    size_t payload_len;
    auth_err_t err = auth_header_decode(&hdr, in, in_len, &payload, &payload_len);
    if (err) return err;
    if (hdr.pkt_type == AUTH_PKT_ERROR) {
        auth_err_t code = AUTH_ERR_UNSPECIFIED;
        auth_packet_parse_error(payload, payload_len, &code, NULL, NULL);
        return code;
    }
    if (hdr.pkt_type != AUTH_PKT_AUTH_ACK) return AUTH_ERR_UNKNOWN_PKT_TYPE;
    err = auth_ack_decode(payload, payload_len);
    if (err) return err;
    ctx->auth_complete = 1;
    return AUTH_OK;
}

/* ======================================================================
 * Server: SETUP
 * ====================================================================== */

auth_err_t auth_server_handle_setup1(
    const uint8_t session_id[AUTH_SESSION_ID_LEN],
    uint32_t seq,
    const uint8_t *in_payload, size_t in_len,
    const uint8_t server_sk[AUTH_SCALAR_LEN],
    const uint8_t server_pub[AUTH_POINT_LEN],
    const char *require_pairing_token,
    auth_pending_setup_t *pending,
    uint8_t *out, size_t out_cap, size_t *out_len)
{
    if (!session_id || !in_payload || !server_sk || !server_pub ||
        !pending || !out)
        return AUTH_ERR_INVALID_ARGUMENT;

    auth_setup1_t s1;
    auth_err_t err = auth_setup1_decode(&s1, in_payload, in_len);
    if (err) return err;

    /* Pairing token check. */
    if (require_pairing_token) {
        size_t expect_len = strlen(require_pairing_token);
        if (s1.pairing_token_len != expect_len ||
            memcmp(s1.pairing_token, require_pairing_token, expect_len) != 0) {
            return AUTH_ERR_PAIRING_TOKEN_BAD;
        }
    }

    err = auth_check_point(s1.device_pub);
    if (err) return err;
    /* role_commitment can be any valid point (including basis
     * combinations); still reject malformed encodings. */
    err = auth_check_point(s1.role_commitment);
    if (err && err != AUTH_ERR_IDENTITY_POINT) return err;

    /* Verify device_id matches Blake2b-256("device-id" || root). The
     * server doesn't know the root, so we can only verify that the
     * device_id is a plausible shape (32 bytes). Deeper binding
     * happens on AUTH via the Schnorr proof against device_pub. */

    /* Build pending state. */
    memset(pending, 0, sizeof *pending);
    memcpy(pending->session_id,      session_id,         AUTH_SESSION_ID_LEN);
    memcpy(pending->device_id,       s1.device_id,       32);
    memcpy(pending->device_pub,      s1.device_pub,      32);
    memcpy(pending->role_commitment, s1.role_commitment, 32);
    memcpy(pending->client_nonce,    s1.client_nonce,    32);
    memcpy(pending->server_pub,      server_pub,         32);
    auth_random_bytes32(pending->server_nonce);
    randombytes_buf(pending->setup_challenge, AUTH_SETUP_CHALLENGE_LEN);

    /* Produce SETUP_2 payload. */
    auth_setup2_t s2 = {0};
    memcpy(s2.server_nonce,    pending->server_nonce,    32);
    memcpy(s2.setup_challenge, pending->setup_challenge, AUTH_SETUP_CHALLENGE_LEN);
    memcpy(s2.server_pub,      server_pub,               32);

    err = auth_prove_setup_server(&s2.server_proof,
        server_sk,
        s1.device_id, s1.device_pub, server_pub,
        s1.client_nonce, pending->server_nonce, pending->setup_challenge);
    if (err) return err;

    uint8_t payload[AUTH_MAX_PAYLOAD];
    size_t payload_len = 0;
    err = auth_setup2_encode(&s2, payload, sizeof payload, &payload_len);
    if (err) return err;

    err = auth_packet_build(AUTH_PKT_SETUP_2, session_id, seq,
                                payload, payload_len, out, out_cap, out_len);
    if (err) return err;

    /* Cache the response for idempotent retransmits. */
    if (*out_len <= sizeof pending->response_packet) {
        memcpy(pending->response_packet, out, *out_len);
        pending->response_packet_len = *out_len;
    }
    return AUTH_OK;
}

auth_err_t auth_server_handle_setup3(
    const auth_pending_setup_t *pending,
    const uint8_t session_id[AUTH_SESSION_ID_LEN],
    uint32_t seq,
    const uint8_t *in_payload, size_t in_len,
    uint8_t *out, size_t out_cap, size_t *out_len)
{
    if (!pending || !session_id || !in_payload || !out)
        return AUTH_ERR_INVALID_ARGUMENT;
    if (memcmp(session_id, pending->session_id, AUTH_SESSION_ID_LEN) != 0)
        return AUTH_ERR_UNKNOWN_SESSION;

    auth_setup3_t s3;
    auth_err_t err = auth_setup3_decode(&s3, in_payload, in_len);
    if (err) return err;

    if (!auth_verify_setup_client(&s3.client_proof,
        pending->device_id, pending->device_pub, pending->server_pub,
        pending->client_nonce, pending->server_nonce, pending->setup_challenge)) {
        return AUTH_ERR_PROOF_VERIFY;
    }

    uint8_t ack[1];
    size_t ack_len = 0;
    err = auth_ack_encode(ack, sizeof ack, &ack_len);
    if (err) return err;
    return auth_packet_build(AUTH_PKT_SETUP_ACK, session_id, seq,
                                 ack, ack_len, out, out_cap, out_len);
}

/* ======================================================================
 * Server: AUTH
 * ====================================================================== */

auth_err_t auth_server_handle_auth1(
    const uint8_t session_id[AUTH_SESSION_ID_LEN],
    uint32_t seq,
    const uint8_t *in_payload, size_t in_len,
    const uint8_t server_sk[AUTH_SCALAR_LEN],
    const uint8_t server_pub[AUTH_POINT_LEN],
    auth_registry_lookup_fn lookup_fn, void *lookup_ctx,
    const uint64_t *allowed_roles, size_t n_allowed,
    auth_pending_auth_t *pending,
    uint8_t *out, size_t out_cap, size_t *out_len)
{
    if (!session_id || !in_payload || !server_sk || !server_pub ||
        !lookup_fn || !allowed_roles || !pending || !out)
        return AUTH_ERR_INVALID_ARGUMENT;

    auth_auth1_t a1;
    auth_err_t err = auth_auth1_decode(&a1, in_payload, in_len);
    if (err) return err;

    /* Sanity-check points. */
    err = auth_check_point(a1.eph_c);
    if (err) return err;
    /* c_prime may be near-identity only via malicious delta; validate shape. */
    err = auth_check_point(a1.c_prime);
    if (err && err != AUTH_ERR_IDENTITY_POINT) return err;

    /* Registry lookup. We need device_pub and stored role_commitment.
     * device_id is derived from pid context but not transmitted in
     * AUTH_1, so we match by device_pub. This requires the lookup_fn
     * to be pubkey-indexed OR we iterate the registry. For now, we
     * compute device_id from (unknown root) -- we can't. So the
     * registry lookup must be by device_pub, not device_id. Update
     * the signature expectation in store/fs.c accordingly. */

    /* Actually, the client sends pid which was computed as
     * SHA256(T_PID || device_pub || nonce_c || eph_c || server_pub).
     * The server knows nonce_c, eph_c, server_pub. To find device_pub
     * we must enumerate the registry and look for one whose Schnorr
     * proof verifies. That's the price of unlinkable auth. */

    /* For the current scope: we require a helper that walks the
     * registry to try each device. That's what the Rust impl does
     * inside handle_auth_1 via registry.for_each_device. We simulate
     * it with a small callback protocol: lookup_fn(ctx, NULL, out_dp,
     * out_rc) returns AUTH_ERR_UNKNOWN_DEVICE when there is no
     * matching device; otherwise it returns the device that verifies
     * this AUTH_1. The caller owns the scan. */

    uint8_t device_pub[32];
    uint8_t stored_c[32];
    err = lookup_fn(lookup_ctx, NULL, device_pub, stored_c);
    if (err) return err;
    /* Pass NULL device_id slot: the lookup impl will try devices
     * against our a1 via a side-channel; to keep the C impl simple
     * we wrap a retry loop in the server main that invokes this
     * handler once per candidate. See bin/auth_server.c.
     * For unit-test purposes here, lookup_fn returns exactly one
     * candidate and we verify below. */

    /* Verify client Schnorr against the candidate device_pub. */
    if (!auth_verify_auth_client(&a1.client_proof, device_pub,
                                     a1.pid, a1.nonce_c, a1.eph_c)) {
        return AUTH_ERR_PROOF_VERIFY;
    }

    /* Verify pid binds to (device_pub, nonce_c, eph_c, server_pub). */
    uint8_t expect_pid[32];
    err = auth_compute_pid(expect_pid,
        device_pub, a1.nonce_c, a1.eph_c, server_pub);
    if (err) return err;
    if (memcmp(expect_pid, a1.pid, 32) != 0)
        return AUTH_ERR_PROOF_VERIFY;

    /* Rerandomization proof. */
    if (!auth_verify_rerandomization(&a1.rerand_proof,
            stored_c, a1.c_prime, a1.pid, a1.nonce_c, a1.eph_c)) {
        return AUTH_ERR_PROOF_VERIFY;
    }

    /* Role set-membership. */
    if (!auth_verify_role_set_membership(a1.branches, a1.n_branches,
            allowed_roles, n_allowed, a1.c_prime, a1.pid, a1.nonce_c, a1.eph_c)) {
        return AUTH_ERR_ROLE_NOT_PERMITTED;
    }

    /* Server ephemeral. */
    uint8_t eph_s_secret[32];
    auth_random_scalar(eph_s_secret);
    uint8_t eph_s[32];
    err = auth_scalarmult_base(eph_s, eph_s_secret);
    if (err) { auth_zeroize(eph_s_secret, 32); return err; }

    /* Server nonce. */
    uint8_t nonce_s[32];
    auth_random_bytes32(nonce_s);

    /* Server Schnorr proof. */
    auth_schnorr_proof_t server_proof;
    err = auth_prove_auth_server(&server_proof, server_sk, nonce_s, eph_s);
    if (err) { auth_zeroize(eph_s_secret, 32); return err; }

    /* Session key. */
    uint8_t session_key[AUTH_SESSION_KEY_LEN];
    err = auth_derive_session_key(session_key,
        eph_s_secret, a1.eph_c,
        a1.nonce_c, nonce_s, a1.pid, a1.eph_c, eph_s);
    auth_zeroize(eph_s_secret, 32);
    if (err) return err;

    /* KC transcript + keys. */
    uint8_t kc_th[AUTH_HASH_LEN];
    err = auth_kc_transcript_hash(kc_th,
        a1.pid,
        &a1.client_proof,
        a1.nonce_c, a1.eph_c,
        server_pub,
        &server_proof,
        nonce_s, eph_s);
    if (err) return err;
    uint8_t k_s2c[AUTH_MAC_KEY_LEN], k_c2s[AUTH_MAC_KEY_LEN];
    auth_derive_kc_keys(k_s2c, k_c2s, session_key, kc_th);

    /* tag_s = HMAC(k_s2c, "server finished" || kc_th) */
    uint8_t tag_s[AUTH_MAC_TAG_LEN];
    auth_hmac_tag(tag_s, k_s2c,
                      (const uint8_t *)"server finished", 15, kc_th);

    /* Build AUTH_2 packet. */
    auth_auth2_t a2 = {0};
    memcpy(a2.server_pub,     server_pub,   32);
    memcpy(&a2.server_proof,  &server_proof, sizeof server_proof);
    memcpy(a2.nonce_s,        nonce_s,      32);
    memcpy(a2.eph_s,          eph_s,        32);
    memcpy(a2.tag_s,          tag_s,        AUTH_MAC_TAG_LEN);

    uint8_t payload[AUTH_MAX_PAYLOAD];
    size_t payload_len = 0;
    err = auth_auth2_encode(&a2, payload, sizeof payload, &payload_len);
    if (err) return err;

    err = auth_packet_build(AUTH_PKT_AUTH_2, session_id, seq,
                                payload, payload_len, out, out_cap, out_len);
    if (err) return err;

    /* Fill pending state. */
    memset(pending, 0, sizeof *pending);
    memcpy(pending->session_id, session_id, AUTH_SESSION_ID_LEN);
    memcpy(pending->device_pub, device_pub, 32);
    memcpy(pending->pid,        a1.pid,     32);
    memcpy(pending->nonce_c,    a1.nonce_c, 32);
    memcpy(pending->nonce_s,    nonce_s,    32);
    memcpy(pending->eph_c,      a1.eph_c,   32);
    memcpy(pending->eph_s,      eph_s,      32);
    memcpy(pending->session_key,session_key,AUTH_SESSION_KEY_LEN);
    memcpy(pending->kc_th,      kc_th,      AUTH_HASH_LEN);
    memcpy(pending->kc_k_s2c,   k_s2c,      AUTH_MAC_KEY_LEN);
    memcpy(pending->kc_k_c2s,   k_c2s,      AUTH_MAC_KEY_LEN);
    if (*out_len <= sizeof pending->response_packet) {
        memcpy(pending->response_packet, out, *out_len);
        pending->response_packet_len = *out_len;
    }
    auth_zeroize(session_key, sizeof session_key);
    auth_zeroize(k_s2c,       sizeof k_s2c);
    auth_zeroize(k_c2s,       sizeof k_c2s);
    return AUTH_OK;
}

auth_err_t auth_server_handle_auth3(
    const auth_pending_auth_t *pending,
    const uint8_t session_id[AUTH_SESSION_ID_LEN],
    uint32_t seq,
    const uint8_t *in_payload, size_t in_len,
    uint8_t *out, size_t out_cap, size_t *out_len)
{
    if (!pending || !session_id || !in_payload || !out)
        return AUTH_ERR_INVALID_ARGUMENT;
    if (memcmp(session_id, pending->session_id, AUTH_SESSION_ID_LEN) != 0)
        return AUTH_ERR_UNKNOWN_SESSION;

    auth_auth3_t a3;
    auth_err_t err = auth_auth3_decode(&a3, in_payload, in_len);
    if (err) return err;

    /* Verify tag_c = HMAC(k_c2s, "client finished" || kc_th). */
    uint8_t expect_tag[AUTH_MAC_TAG_LEN];
    auth_hmac_tag(expect_tag, pending->kc_k_c2s,
                      (const uint8_t *)"client finished", 15, pending->kc_th);
    if (sodium_memcmp(expect_tag, a3.tag_c, AUTH_MAC_TAG_LEN) != 0)
        return AUTH_ERR_KEY_CONFIRM;

    uint8_t ack[1];
    size_t ack_len = 0;
    err = auth_ack_encode(ack, sizeof ack, &ack_len);
    if (err) return err;
    return auth_packet_build(AUTH_PKT_AUTH_ACK, session_id, seq,
                                 ack, ack_len, out, out_cap, out_len);
}
