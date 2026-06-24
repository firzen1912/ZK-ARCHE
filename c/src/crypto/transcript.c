/*
 * auth_transcript.c — canonical byte-stable transcript builder.
 * Matches Rust auth_proto::transcript byte-for-byte.
 */

#include "auth/auth_transcript.h"
#include "auth/auth_crypto.h"

#include <sodium.h>
#include <string.h>

/* ---- Domain separators (ASCII, no trailing NUL on the wire) ---- */

#define DOM_ACCESSOR(name, value)                                     \
    const uint8_t *auth_ds_##name(size_t *len_out) {              \
        static const uint8_t v[] = value;                             \
        if (len_out) *len_out = sizeof v - 1;                         \
        return v;                                                     \
    }

DOM_ACCESSOR(setup,        "setup_client_schnorr_v1")
DOM_ACCESSOR(setup_server, "setup_server_schnorr_v1")
DOM_ACCESSOR(server,       "server_schnorr_v1")
DOM_ACCESSOR(pid,          "iot-auth/pid/v1")
DOM_ACCESSOR(client_v2,    "client_schnorr_v2")
DOM_ACCESSOR(kc_v2,        "kc_v2")
DOM_ACCESSOR(role_set,     "client_role_set_v1")
DOM_ACCESSOR(role_rerand,  "client_role_rerand_v1")

/* ---- Init / append ---- */

static int append_bytes(auth_transcript_t *t,
                        const uint8_t *b, size_t n)
{
    if (t->overflow) return 0;
    if (t->len + n > sizeof t->buf) {
        t->overflow = 1;
        return 0;
    }
    memcpy(t->buf + t->len, b, n);
    t->len += n;
    return 1;
}

auth_err_t auth_transcript_init(
    auth_transcript_t *t,
    const uint8_t *domain, size_t domain_len)
{
    if (!t || (!domain && domain_len > 0) || domain_len > 255) {
        return AUTH_ERR_INVALID_ARGUMENT;
    }
    t->len = 0;
    t->overflow = 0;
    uint8_t lb = (uint8_t)domain_len;
    if (!append_bytes(t, &lb, 1)) return AUTH_ERR_BUFFER_TOO_SMALL;
    if (domain_len && !append_bytes(t, domain, domain_len)) {
        return AUTH_ERR_BUFFER_TOO_SMALL;
    }
    return AUTH_OK;
}

auth_err_t auth_transcript_append(
    auth_transcript_t *t,
    const uint8_t *label, size_t label_len,
    const uint8_t *msg,   size_t msg_len)
{
    if (!t || !label || label_len > 255) return AUTH_ERR_INVALID_ARGUMENT;
    if (msg_len > 0xFFFFFFFFu)            return AUTH_ERR_INVALID_ARGUMENT;
    if (!msg && msg_len > 0)              return AUTH_ERR_INVALID_ARGUMENT;

    uint8_t lb = (uint8_t)label_len;
    uint8_t len_le[4];
    len_le[0] = (uint8_t)(msg_len       & 0xff);
    len_le[1] = (uint8_t)((msg_len >>  8) & 0xff);
    len_le[2] = (uint8_t)((msg_len >> 16) & 0xff);
    len_le[3] = (uint8_t)((msg_len >> 24) & 0xff);

    if (!append_bytes(t, &lb, 1))         return AUTH_ERR_BUFFER_TOO_SMALL;
    if (label_len && !append_bytes(t, label, label_len))
                                           return AUTH_ERR_BUFFER_TOO_SMALL;
    if (!append_bytes(t, len_le, 4))      return AUTH_ERR_BUFFER_TOO_SMALL;
    if (msg_len && !append_bytes(t, msg, msg_len))
                                           return AUTH_ERR_BUFFER_TOO_SMALL;
    return AUTH_OK;
}

auth_err_t auth_transcript_append_u8(
    auth_transcript_t *t,
    const uint8_t *label, size_t label_len,
    uint8_t v)
{
    return auth_transcript_append(t, label, label_len, &v, 1);
}

auth_err_t auth_transcript_append_u64(
    auth_transcript_t *t,
    const uint8_t *label, size_t label_len,
    uint64_t v)
{
    uint8_t le[8];
    for (size_t i = 0; i < 8; ++i) le[i] = (uint8_t)(v >> (8 * i));
    return auth_transcript_append(t, label, label_len, le, 8);
}

auth_err_t auth_transcript_append_point(
    auth_transcript_t *t,
    const uint8_t *label, size_t label_len,
    const uint8_t point[AUTH_POINT_LEN])
{
    return auth_transcript_append(t, label, label_len,
                                      point, AUTH_POINT_LEN);
}

auth_err_t auth_transcript_append_scalar(
    auth_transcript_t *t,
    const uint8_t *label, size_t label_len,
    const uint8_t scalar[AUTH_SCALAR_LEN])
{
    return auth_transcript_append(t, label, label_len,
                                      scalar, AUTH_SCALAR_LEN);
}

/* ---- Finalization ---- */

auth_err_t auth_transcript_challenge(
    const auth_transcript_t *t,
    uint8_t scalar_out[AUTH_SCALAR_LEN])
{
    if (!t || t->overflow) return AUTH_ERR_BUFFER_TOO_SMALL;
    uint8_t wide[crypto_hash_sha512_BYTES]; /* 64 */
    crypto_hash_sha512(wide, t->buf, t->len);
    crypto_core_ristretto255_scalar_reduce(scalar_out, wide);
    sodium_memzero(wide, sizeof wide);
    return AUTH_OK;
}

auth_err_t auth_transcript_hash_sha256(
    const auth_transcript_t *t,
    uint8_t hash_out[AUTH_HASH_LEN])
{
    if (!t || t->overflow) return AUTH_ERR_BUFFER_TOO_SMALL;
    crypto_hash_sha256(hash_out, t->buf, t->len);
    return AUTH_OK;
}

/* ---- PID ---- */

auth_err_t auth_compute_pid(
    uint8_t pid_out[AUTH_HASH_LEN],
    const uint8_t device_pub[AUTH_POINT_LEN],
    const uint8_t nonce_c[AUTH_NONCE_LEN],
    const uint8_t eph_c[AUTH_POINT_LEN],
    const uint8_t server_pub[AUTH_POINT_LEN])
{
    /* pid = SHA-256( u32_LE(len(T_PID)) || T_PID ||
     *                device_pub || nonce_c || eph_c || server_pub ) */
    size_t pid_label_len;
    const uint8_t *pid_label = auth_ds_pid(&pid_label_len);

    uint8_t len_le[4];
    len_le[0] = (uint8_t)(pid_label_len      & 0xff);
    len_le[1] = (uint8_t)((pid_label_len >> 8) & 0xff);
    len_le[2] = (uint8_t)((pid_label_len >>16) & 0xff);
    len_le[3] = (uint8_t)((pid_label_len >>24) & 0xff);

    crypto_hash_sha256_state st;
    crypto_hash_sha256_init(&st);
    crypto_hash_sha256_update(&st, len_le, 4);
    crypto_hash_sha256_update(&st, pid_label, pid_label_len);
    crypto_hash_sha256_update(&st, device_pub, AUTH_POINT_LEN);
    crypto_hash_sha256_update(&st, nonce_c,    AUTH_NONCE_LEN);
    crypto_hash_sha256_update(&st, eph_c,      AUTH_POINT_LEN);
    crypto_hash_sha256_update(&st, server_pub, AUTH_POINT_LEN);
    crypto_hash_sha256_final(&st, pid_out);
    return AUTH_OK;
}
