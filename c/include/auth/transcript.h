/*
 * auth_transcript.h — canonical transcript builder.
 *
 * Matches the Rust `auth_proto::transcript` module byte-for-byte.
 * The transcript is an append-only byte buffer with this encoding:
 *
 *   transcript := u8(len(domain)) | domain
 *   each append: u8(len(label)) | label | u32_LE(len(msg)) | msg
 *
 * Points are appended as 32-byte compressed Ristretto255.
 * Scalars are appended as 32-byte canonical encodings.
 * u64 values are appended as 8 little-endian bytes.
 *
 * The buffer is fixed-size (embedded-friendly); the upper bound of
 * AUTH_TRANSCRIPT_CAP bytes fits every transcript produced by this
 * protocol (~4 KiB is safe even for AUTH_MAX_ROLES branches).
 */

#ifndef AUTH_TRANSCRIPT_H
#define AUTH_TRANSCRIPT_H

#include "iot_auth.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef AUTH_TRANSCRIPT_CAP
#define AUTH_TRANSCRIPT_CAP  4096
#endif

/* ---- Domain separators (wire-stable ASCII, no trailing NUL) ---- */

/* Use auth_ds_*() accessors below rather than string literals in
 * call sites, so the domain lengths are not accidentally broken. */
const uint8_t *auth_ds_setup       (size_t *len_out);
const uint8_t *auth_ds_setup_server(size_t *len_out);
const uint8_t *auth_ds_server      (size_t *len_out);
const uint8_t *auth_ds_pid         (size_t *len_out);
const uint8_t *auth_ds_client_v2   (size_t *len_out);
const uint8_t *auth_ds_kc_v2       (size_t *len_out);
const uint8_t *auth_ds_role_set    (size_t *len_out);
const uint8_t *auth_ds_role_rerand (size_t *len_out);

/* ---- Transcript type ---- */

typedef struct auth_transcript {
    uint8_t  buf[AUTH_TRANSCRIPT_CAP];
    size_t   len;
    int      overflow;  /* 1 if any append ever exceeded CAP */
} auth_transcript_t;

/*
 * Initialize a transcript with a domain separator. `domain_len` must
 * be <= 255. Returns AUTH_OK or AUTH_ERR_INVALID_ARGUMENT.
 */
auth_err_t auth_transcript_init(
    auth_transcript_t *t,
    const uint8_t *domain, size_t domain_len);

/*
 * Append (label, msg). `label_len` <= 255, `msg_len` <= UINT32_MAX.
 * Returns AUTH_OK on success, AUTH_ERR_INVALID_ARGUMENT on
 * label too long, AUTH_ERR_BUFFER_TOO_SMALL if CAP would be
 * exceeded (and sets t->overflow = 1).
 */
auth_err_t auth_transcript_append(
    auth_transcript_t *t,
    const uint8_t *label, size_t label_len,
    const uint8_t *msg,   size_t msg_len);

/* Convenience wrappers for the common append types. */

auth_err_t auth_transcript_append_u8(
    auth_transcript_t *t,
    const uint8_t *label, size_t label_len,
    uint8_t v);

auth_err_t auth_transcript_append_u64(
    auth_transcript_t *t,
    const uint8_t *label, size_t label_len,
    uint64_t v);

auth_err_t auth_transcript_append_point(
    auth_transcript_t *t,
    const uint8_t *label, size_t label_len,
    const uint8_t point[AUTH_POINT_LEN]);

auth_err_t auth_transcript_append_scalar(
    auth_transcript_t *t,
    const uint8_t *label, size_t label_len,
    const uint8_t scalar[AUTH_SCALAR_LEN]);

/*
 * Finalize as a Schnorr challenge scalar: reduce SHA-512(transcript)
 * mod L via 64-byte wide reduction.
 */
auth_err_t auth_transcript_challenge(
    const auth_transcript_t *t,
    uint8_t scalar_out[AUTH_SCALAR_LEN]);

/*
 * Finalize as a 32-byte SHA-256 hash of the transcript.
 */
auth_err_t auth_transcript_hash_sha256(
    const auth_transcript_t *t,
    uint8_t hash_out[AUTH_HASH_LEN]);

/* ---- PID derivation ---- */

/*
 * pid = SHA-256( u32_LE(len(T_PID)) || T_PID || device_pub || nonce_c ||
 *                eph_c || server_pub )
 *
 * Note: this is NOT a Transcript hash; it is a direct SHA-256. This
 * matches the Rust `compute_pid()` exactly and is tested against
 * test-vectors/0x0001/pid.json.
 */
auth_err_t auth_compute_pid(
    uint8_t pid_out[AUTH_HASH_LEN],
    const uint8_t device_pub[AUTH_POINT_LEN],
    const uint8_t nonce_c[AUTH_NONCE_LEN],
    const uint8_t eph_c[AUTH_POINT_LEN],
    const uint8_t server_pub[AUTH_POINT_LEN]);

#ifdef __cplusplus
}
#endif
#endif /* AUTH_TRANSCRIPT_H */
