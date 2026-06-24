/*
 * auth_payloads.h — encoders/decoders for the per-packet payloads.
 *
 * Every struct here mirrors the byte layout in the Rust
 * auth_proto::proto::payloads module. All layouts are part of the
 * wire-format spec.
 */

#ifndef AUTH_PAYLOADS_H
#define AUTH_PAYLOADS_H

#include "iot_auth.h"
#include "auth_proofs.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ---- SETUP_1 (client -> server) ----
 *
 *   u8  token_len  (0..=128)
 *   token_len bytes pairing_token
 *   32 device_id
 *   32 device_pub
 *   32 client_nonce
 *   32 role_commitment
 */
typedef struct auth_setup1 {
    uint8_t  pairing_token[AUTH_MAX_PAIRING_TOKEN];
    size_t   pairing_token_len;  /* 0 if absent */
    uint8_t  device_id       [AUTH_DEVICE_ID_LEN];
    uint8_t  device_pub      [AUTH_POINT_LEN];
    uint8_t  client_nonce    [AUTH_NONCE_LEN];
    uint8_t  role_commitment [AUTH_POINT_LEN];
} auth_setup1_t;

auth_err_t auth_setup1_encode(
    const auth_setup1_t *m,
    uint8_t *buf, size_t buf_len, size_t *written_out);

auth_err_t auth_setup1_decode(
    auth_setup1_t *m_out,
    const uint8_t *buf, size_t buf_len);

/* ---- SETUP_2 (server -> client) ----
 *
 *   32 server_nonce
 *   16 setup_challenge
 *   32 server_pub
 *   32 a_s
 *   32 s_s
 */
typedef struct auth_setup2 {
    uint8_t server_nonce    [AUTH_NONCE_LEN];
    uint8_t setup_challenge [AUTH_SETUP_CHALLENGE_LEN];
    uint8_t server_pub      [AUTH_POINT_LEN];
    auth_schnorr_proof_t server_proof;
} auth_setup2_t;

auth_err_t auth_setup2_encode(
    const auth_setup2_t *m,
    uint8_t *buf, size_t buf_len, size_t *written_out);

auth_err_t auth_setup2_decode(
    auth_setup2_t *m_out,
    const uint8_t *buf, size_t buf_len);

/* ---- SETUP_3 (client -> server) ----
 *
 *   32 a_c
 *   32 s_c
 */
typedef struct auth_setup3 {
    auth_schnorr_proof_t client_proof;
} auth_setup3_t;

auth_err_t auth_setup3_encode(
    const auth_setup3_t *m,
    uint8_t *buf, size_t buf_len, size_t *written_out);

auth_err_t auth_setup3_decode(
    auth_setup3_t *m_out,
    const uint8_t *buf, size_t buf_len);

/* ---- AUTH_1 (client -> server) ----
 *
 *   32 pid
 *   32 a_c
 *   32 s_c
 *   32 nonce_c
 *   32 eph_c
 *   32 c_prime
 *   32 rerand_a
 *   32 rerand_s
 *   u16_LE branches_len
 *   branches_len * (32 A_i | 32 c_i | 32 s_i)
 */
typedef struct auth_auth1 {
    uint8_t                    pid[AUTH_HASH_LEN];
    auth_schnorr_proof_t   client_proof;
    uint8_t                    nonce_c[AUTH_NONCE_LEN];
    uint8_t                    eph_c[AUTH_POINT_LEN];
    uint8_t                    c_prime[AUTH_POINT_LEN];
    auth_schnorr_proof_t   rerand_proof;
    auth_set_branch_t      branches[AUTH_MAX_ROLES];
    size_t                     n_branches;
} auth_auth1_t;

auth_err_t auth_auth1_encode(
    const auth_auth1_t *m,
    uint8_t *buf, size_t buf_len, size_t *written_out);

auth_err_t auth_auth1_decode(
    auth_auth1_t *m_out,
    const uint8_t *buf, size_t buf_len);

/* ---- AUTH_2 (server -> client) ----
 *
 *   32 server_pub
 *   32 a_s
 *   32 s_s
 *   32 nonce_s
 *   32 eph_s
 *   32 tag_s
 */
typedef struct auth_auth2 {
    uint8_t                    server_pub[AUTH_POINT_LEN];
    auth_schnorr_proof_t   server_proof;
    uint8_t                    nonce_s[AUTH_NONCE_LEN];
    uint8_t                    eph_s[AUTH_POINT_LEN];
    uint8_t                    tag_s[AUTH_MAC_TAG_LEN];
} auth_auth2_t;

auth_err_t auth_auth2_encode(
    const auth_auth2_t *m,
    uint8_t *buf, size_t buf_len, size_t *written_out);

auth_err_t auth_auth2_decode(
    auth_auth2_t *m_out,
    const uint8_t *buf, size_t buf_len);

/* ---- AUTH_3 (client -> server) ----
 *
 *   32 tag_c
 */
typedef struct auth_auth3 {
    uint8_t tag_c[AUTH_MAC_TAG_LEN];
} auth_auth3_t;

auth_err_t auth_auth3_encode(
    const auth_auth3_t *m,
    uint8_t *buf, size_t buf_len, size_t *written_out);

auth_err_t auth_auth3_decode(
    auth_auth3_t *m_out,
    const uint8_t *buf, size_t buf_len);

/* ---- ACKs (single 0x01 byte) ---- */

#define AUTH_ACK_BYTE 0x01u

auth_err_t auth_ack_encode(
    uint8_t *buf, size_t buf_len, size_t *written_out);

auth_err_t auth_ack_decode(const uint8_t *buf, size_t buf_len);

#ifdef __cplusplus
}
#endif
#endif /* AUTH_PAYLOADS_H */
