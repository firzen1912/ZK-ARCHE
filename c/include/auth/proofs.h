/*
 * auth_proofs.h — Schnorr proofs, rerandomization, and CDS-OR
 * set-membership. All prove_*() functions have a _with_bytes() variant
 * taking the randomness as an input, for deterministic test vectors
 * and hardware-RNG plumbing.
 */

#ifndef AUTH_PROOFS_H
#define AUTH_PROOFS_H

#include "iot_auth.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Schnorr proof (a, s) ---- */

typedef struct auth_schnorr_proof {
    uint8_t a[AUTH_POINT_LEN];
    uint8_t s[AUTH_SCALAR_LEN];
} auth_schnorr_proof_t;

/* ---- Setup enrollment proofs (device_id is in the transcript) ---- */

auth_err_t auth_prove_setup_client(
    auth_schnorr_proof_t *proof_out,
    const uint8_t x[AUTH_SCALAR_LEN],
    const uint8_t device_id[AUTH_DEVICE_ID_LEN],
    const uint8_t device_pub[AUTH_POINT_LEN],
    const uint8_t server_pub[AUTH_POINT_LEN],
    const uint8_t client_nonce[AUTH_NONCE_LEN],
    const uint8_t server_nonce[AUTH_NONCE_LEN],
    const uint8_t setup_challenge[AUTH_SETUP_CHALLENGE_LEN]);

auth_err_t auth_prove_setup_client_with_bytes(
    auth_schnorr_proof_t *proof_out,
    const uint8_t wide_r[64],
    const uint8_t x[AUTH_SCALAR_LEN],
    const uint8_t device_id[AUTH_DEVICE_ID_LEN],
    const uint8_t device_pub[AUTH_POINT_LEN],
    const uint8_t server_pub[AUTH_POINT_LEN],
    const uint8_t client_nonce[AUTH_NONCE_LEN],
    const uint8_t server_nonce[AUTH_NONCE_LEN],
    const uint8_t setup_challenge[AUTH_SETUP_CHALLENGE_LEN]);

int auth_verify_setup_client(
    const auth_schnorr_proof_t *proof,
    const uint8_t device_id[AUTH_DEVICE_ID_LEN],
    const uint8_t device_pub[AUTH_POINT_LEN],
    const uint8_t server_pub[AUTH_POINT_LEN],
    const uint8_t client_nonce[AUTH_NONCE_LEN],
    const uint8_t server_nonce[AUTH_NONCE_LEN],
    const uint8_t setup_challenge[AUTH_SETUP_CHALLENGE_LEN]);

auth_err_t auth_prove_setup_server(
    auth_schnorr_proof_t *proof_out,
    const uint8_t server_sk[AUTH_SCALAR_LEN],
    const uint8_t device_id[AUTH_DEVICE_ID_LEN],
    const uint8_t device_pub[AUTH_POINT_LEN],
    const uint8_t server_pub[AUTH_POINT_LEN],
    const uint8_t client_nonce[AUTH_NONCE_LEN],
    const uint8_t server_nonce[AUTH_NONCE_LEN],
    const uint8_t setup_challenge[AUTH_SETUP_CHALLENGE_LEN]);

auth_err_t auth_prove_setup_server_with_bytes(
    auth_schnorr_proof_t *proof_out,
    const uint8_t wide_r[64],
    const uint8_t server_sk[AUTH_SCALAR_LEN],
    const uint8_t device_id[AUTH_DEVICE_ID_LEN],
    const uint8_t device_pub[AUTH_POINT_LEN],
    const uint8_t server_pub[AUTH_POINT_LEN],
    const uint8_t client_nonce[AUTH_NONCE_LEN],
    const uint8_t server_nonce[AUTH_NONCE_LEN],
    const uint8_t setup_challenge[AUTH_SETUP_CHALLENGE_LEN]);

int auth_verify_setup_server(
    const auth_schnorr_proof_t *proof,
    const uint8_t server_pub[AUTH_POINT_LEN],
    const uint8_t device_id[AUTH_DEVICE_ID_LEN],
    const uint8_t device_pub[AUTH_POINT_LEN],
    const uint8_t client_nonce[AUTH_NONCE_LEN],
    const uint8_t server_nonce[AUTH_NONCE_LEN],
    const uint8_t setup_challenge[AUTH_SETUP_CHALLENGE_LEN]);

/* ---- Auth v2 Schnorr proofs (bound to pid, not device_id) ---- */

auth_err_t auth_prove_auth_client(
    auth_schnorr_proof_t *proof_out,
    const uint8_t x[AUTH_SCALAR_LEN],
    const uint8_t pid[AUTH_HASH_LEN],
    const uint8_t nonce_c[AUTH_NONCE_LEN],
    const uint8_t eph_c[AUTH_POINT_LEN]);

auth_err_t auth_prove_auth_client_with_bytes(
    auth_schnorr_proof_t *proof_out,
    const uint8_t wide_r[64],
    const uint8_t x[AUTH_SCALAR_LEN],
    const uint8_t pid[AUTH_HASH_LEN],
    const uint8_t nonce_c[AUTH_NONCE_LEN],
    const uint8_t eph_c[AUTH_POINT_LEN]);

int auth_verify_auth_client(
    const auth_schnorr_proof_t *proof,
    const uint8_t device_pub[AUTH_POINT_LEN],
    const uint8_t pid[AUTH_HASH_LEN],
    const uint8_t nonce_c[AUTH_NONCE_LEN],
    const uint8_t eph_c[AUTH_POINT_LEN]);

auth_err_t auth_prove_auth_server(
    auth_schnorr_proof_t *proof_out,
    const uint8_t server_sk[AUTH_SCALAR_LEN],
    const uint8_t nonce_s[AUTH_NONCE_LEN],
    const uint8_t eph_s[AUTH_POINT_LEN]);

auth_err_t auth_prove_auth_server_with_bytes(
    auth_schnorr_proof_t *proof_out,
    const uint8_t wide_r[64],
    const uint8_t server_sk[AUTH_SCALAR_LEN],
    const uint8_t nonce_s[AUTH_NONCE_LEN],
    const uint8_t eph_s[AUTH_POINT_LEN]);

int auth_verify_auth_server(
    const auth_schnorr_proof_t *proof,
    const uint8_t server_pub[AUTH_POINT_LEN],
    const uint8_t nonce_s[AUTH_NONCE_LEN],
    const uint8_t eph_s[AUTH_POINT_LEN]);

/* ---- Role-commitment helpers ---- */

/*
 * commitment = g*role + h*blind.
 */
auth_err_t auth_make_role_commitment(
    uint8_t commitment_out[AUTH_POINT_LEN],
    uint64_t role_code,
    const uint8_t blind[AUTH_SCALAR_LEN]);

/*
 * Rerandomize: c_prime = stored_c + h*delta, blind_prime = blind + delta.
 * Caller provides `delta`.
 */
auth_err_t auth_rerandomize_commitment(
    uint8_t c_prime_out[AUTH_POINT_LEN],
    uint8_t blind_prime_out[AUTH_SCALAR_LEN],
    const uint8_t stored_c[AUTH_POINT_LEN],
    const uint8_t stored_blind[AUTH_SCALAR_LEN],
    const uint8_t delta[AUTH_SCALAR_LEN]);

/* ---- Rerandomization proof ---- */

auth_err_t auth_prove_rerandomization(
    auth_schnorr_proof_t *proof_out,
    const uint8_t stored_c[AUTH_POINT_LEN],
    const uint8_t c_prime[AUTH_POINT_LEN],
    const uint8_t delta[AUTH_SCALAR_LEN],
    const uint8_t pid[AUTH_HASH_LEN],
    const uint8_t nonce_c[AUTH_NONCE_LEN],
    const uint8_t eph_c[AUTH_POINT_LEN]);

auth_err_t auth_prove_rerandomization_with_bytes(
    auth_schnorr_proof_t *proof_out,
    const uint8_t wide_r[64],
    const uint8_t stored_c[AUTH_POINT_LEN],
    const uint8_t c_prime[AUTH_POINT_LEN],
    const uint8_t delta[AUTH_SCALAR_LEN],
    const uint8_t pid[AUTH_HASH_LEN],
    const uint8_t nonce_c[AUTH_NONCE_LEN],
    const uint8_t eph_c[AUTH_POINT_LEN]);

int auth_verify_rerandomization(
    const auth_schnorr_proof_t *proof,
    const uint8_t stored_c[AUTH_POINT_LEN],
    const uint8_t c_prime[AUTH_POINT_LEN],
    const uint8_t pid[AUTH_HASH_LEN],
    const uint8_t nonce_c[AUTH_NONCE_LEN],
    const uint8_t eph_c[AUTH_POINT_LEN]);

/* ---- CDS-OR set-membership proof ---- */

typedef struct auth_set_branch {
    uint8_t a[AUTH_POINT_LEN];
    uint8_t c[AUTH_SCALAR_LEN];
    uint8_t s[AUTH_SCALAR_LEN];
} auth_set_branch_t;

/*
 * Prove c_prime commits to some role in `allowed_roles` without
 * revealing which. The caller provides the true role_code and the
 * rerandomized blind_prime. The branches are returned in the same
 * order as `allowed_roles`.
 *
 * The number of branches equals `n_roles` and must be <= AUTH_MAX_ROLES.
 */
auth_err_t auth_prove_role_set_membership(
    auth_set_branch_t *branches_out,
    size_t *n_branches_out,
    const uint64_t *allowed_roles, size_t n_roles,
    const uint8_t c_prime[AUTH_POINT_LEN],
    uint64_t role_code,
    const uint8_t blind_prime[AUTH_SCALAR_LEN],
    const uint8_t pid[AUTH_HASH_LEN],
    const uint8_t nonce_c[AUTH_NONCE_LEN],
    const uint8_t eph_c[AUTH_POINT_LEN]);

/*
 * Deterministic variant. `wide_bytes` must be a buffer of length
 * 64 * (n_roles + 1) bytes. The first 64 bytes produce the random
 * witness w for the true branch; each of the n_roles-1 simulated
 * branches consumes 64 bytes for c_i then 64 bytes for s_i — but
 * the *true branch* only consumes 64 bytes (w), so the total is
 * 64 bytes for w + 2*64 bytes per simulated branch =
 * 64 + 128*(n_roles-1) bytes. For simplicity, the caller must pass a
 * buffer of at least 64 * (2*n_roles - 1) bytes.
 *
 * For the CDS-OR proof, the consumption order matches the Rust
 * implementation for interop with deterministic test vectors:
 *   for i in 0..n:
 *     if i == true_index: consume 64 bytes for w
 *     else:               consume 64 bytes for c_i then 64 for s_i
 */
auth_err_t auth_prove_role_set_membership_with_bytes(
    auth_set_branch_t *branches_out,
    size_t *n_branches_out,
    const uint8_t *wide_bytes, size_t wide_bytes_len,
    const uint64_t *allowed_roles, size_t n_roles,
    const uint8_t c_prime[AUTH_POINT_LEN],
    uint64_t role_code,
    const uint8_t blind_prime[AUTH_SCALAR_LEN],
    const uint8_t pid[AUTH_HASH_LEN],
    const uint8_t nonce_c[AUTH_NONCE_LEN],
    const uint8_t eph_c[AUTH_POINT_LEN]);

int auth_verify_role_set_membership(
    const auth_set_branch_t *branches, size_t n_branches,
    const uint64_t *allowed_roles, size_t n_roles,
    const uint8_t c_prime[AUTH_POINT_LEN],
    const uint8_t pid[AUTH_HASH_LEN],
    const uint8_t nonce_c[AUTH_NONCE_LEN],
    const uint8_t eph_c[AUTH_POINT_LEN]);

/* ---- Session key and key-confirmation ---- */

/*
 * session_key = HKDF-SHA256(
 *     salt = nonce_c || nonce_s,
 *     ikm  = ECDH(eph_secret, peer_eph_pub),
 *     info = "session key v2" || pid || eph_c || eph_s )
 */
auth_err_t auth_derive_session_key(
    uint8_t session_key_out[AUTH_SESSION_KEY_LEN],
    const uint8_t eph_secret[AUTH_SCALAR_LEN],
    const uint8_t peer_eph_pub[AUTH_POINT_LEN],
    const uint8_t nonce_c[AUTH_NONCE_LEN],
    const uint8_t nonce_s[AUTH_NONCE_LEN],
    const uint8_t pid[AUTH_HASH_LEN],
    const uint8_t eph_c[AUTH_POINT_LEN],
    const uint8_t eph_s[AUTH_POINT_LEN]);

/*
 * th = SHA-256(kc-transcript); see spec §6.
 */
auth_err_t auth_kc_transcript_hash(
    uint8_t th_out[AUTH_HASH_LEN],
    const uint8_t pid[AUTH_HASH_LEN],
    const auth_schnorr_proof_t *client_proof,
    const uint8_t nonce_c[AUTH_NONCE_LEN],
    const uint8_t eph_c[AUTH_POINT_LEN],
    const uint8_t server_pub[AUTH_POINT_LEN],
    const auth_schnorr_proof_t *server_proof,
    const uint8_t nonce_s[AUTH_NONCE_LEN],
    const uint8_t eph_s[AUTH_POINT_LEN]);

void auth_derive_kc_keys(
    uint8_t k_s2c_out[AUTH_MAC_KEY_LEN],
    uint8_t k_c2s_out[AUTH_MAC_KEY_LEN],
    const uint8_t session_key[AUTH_SESSION_KEY_LEN],
    const uint8_t th[AUTH_HASH_LEN]);

#ifdef __cplusplus
}
#endif
#endif /* AUTH_PROOFS_H */
