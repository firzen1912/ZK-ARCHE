/*
 * auth_crypto.h — cryptographic primitives.
 *
 * Every function is pure (no I/O, no heap) and matches the Rust
 * `auth_proto::crypto` module byte-for-byte given identical inputs.
 * Test vectors under test-vectors/0x0001/ validate this interoperability.
 *
 * Points are 32-byte compressed Ristretto255. Scalars are 32-byte
 * canonical encodings (mod L = 2^252 + 27742317777372353535851937790883648493).
 *
 * All buffers passed in are const-qualified; output buffers are caller-
 * provided and have explicit fixed sizes documented at each function.
 */

#ifndef AUTH_CRYPTO_H
#define AUTH_CRYPTO_H

#include "iot_auth.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Random sampling ---- */

/*
 * Generate a uniformly random 32-byte buffer. Uses the platform CSPRNG
 * via libsodium's randombytes_buf(). Constant-time with respect to the
 * output bytes.
 */
void auth_random_bytes32(uint8_t out[32]);

/*
 * Sample a uniformly random Ristretto255 scalar.
 * Implementation: draws 64 uniform bytes then reduces mod L.
 */
void auth_random_scalar(uint8_t scalar_out[AUTH_SCALAR_LEN]);

/*
 * Deterministic scalar derivation from 64 input bytes, matching
 * Rust's Scalar::from_bytes_mod_order_wide. Used by the test-vector
 * harness and by any DRBG-driven reproduction.
 */
void auth_scalar_from_wide(const uint8_t wide[64],
                               uint8_t scalar_out[AUTH_SCALAR_LEN]);

/* ---- Point / scalar validation ---- */

/*
 * Verify that `point` is a valid encoded Ristretto255 point AND not
 * the identity. Returns AUTH_OK, AUTH_ERR_INVALID_POINT, or
 * AUTH_ERR_IDENTITY_POINT.
 */
auth_err_t auth_check_point(const uint8_t point[AUTH_POINT_LEN]);

/*
 * Verify that `scalar` is in canonical form (< L). Returns AUTH_OK
 * or AUTH_ERR_NONCANONICAL_SCALAR.
 */
auth_err_t auth_check_scalar(const uint8_t scalar[AUTH_SCALAR_LEN]);

/* ---- Point arithmetic (thin wrappers, used by proofs) ---- */

/* Ristretto255 basepoint times scalar. */
auth_err_t auth_scalarmult_base(
    uint8_t point_out[AUTH_POINT_LEN],
    const uint8_t scalar[AUTH_SCALAR_LEN]);

/* Scalar multiplication on an arbitrary point. */
auth_err_t auth_scalarmult(
    uint8_t point_out[AUTH_POINT_LEN],
    const uint8_t scalar[AUTH_SCALAR_LEN],
    const uint8_t point[AUTH_POINT_LEN]);

auth_err_t auth_point_add(
    uint8_t point_out[AUTH_POINT_LEN],
    const uint8_t a[AUTH_POINT_LEN],
    const uint8_t b[AUTH_POINT_LEN]);

auth_err_t auth_point_sub(
    uint8_t point_out[AUTH_POINT_LEN],
    const uint8_t a[AUTH_POINT_LEN],
    const uint8_t b[AUTH_POINT_LEN]);

/* ---- Scalar arithmetic (mod L) ---- */

void auth_scalar_add(
    uint8_t out[AUTH_SCALAR_LEN],
    const uint8_t a[AUTH_SCALAR_LEN],
    const uint8_t b[AUTH_SCALAR_LEN]);

void auth_scalar_sub(
    uint8_t out[AUTH_SCALAR_LEN],
    const uint8_t a[AUTH_SCALAR_LEN],
    const uint8_t b[AUTH_SCALAR_LEN]);

void auth_scalar_mul(
    uint8_t out[AUTH_SCALAR_LEN],
    const uint8_t a[AUTH_SCALAR_LEN],
    const uint8_t b[AUTH_SCALAR_LEN]);

/* Encode a u64 role_code as a scalar (little-endian into first 8 bytes,
 * rest zero). Matches Scalar::from(role_code) in the Rust impl. */
void auth_scalar_from_u64(
    uint8_t out[AUTH_SCALAR_LEN],
    uint64_t value);

/* ---- Hash-to-point ---- */

/*
 * Domain-separated hash-to-point for ristretto255. Matches the Rust
 * `hash_to_point(label)` function:
 *
 *    wide  = SHA-512("ristretto-hash-to-point-v1" || label)
 *    point = ristretto255_from_hash(wide)   // 64-byte uniform reduction
 */
auth_err_t auth_hash_to_point(
    uint8_t point_out[AUTH_POINT_LEN],
    const uint8_t *label,
    size_t label_len);

/*
 * The canonical blinding generator h for role commitments:
 *    h = hash_to_point("iot-auth/attr-h/v1")
 *
 * Callers that need h repeatedly should cache the output.
 */
auth_err_t auth_attr_h(uint8_t point_out[AUTH_POINT_LEN]);

/* ---- HKDF-SHA256 ---- */

/*
 * HKDF-Extract then HKDF-Expand to `okm_len` bytes.
 *   salt: may be NULL (treated as zeros of hash-output length).
 *   info: may be NULL if info_len == 0.
 * Returns AUTH_OK or AUTH_ERR_INVALID_ARGUMENT.
 */
auth_err_t auth_hkdf_sha256(
    uint8_t *okm_out, size_t okm_len,
    const uint8_t *salt, size_t salt_len,
    const uint8_t *ikm,  size_t ikm_len,
    const uint8_t *info, size_t info_len);

/* ---- HMAC-SHA256 ---- */

/*
 * HMAC-SHA256 of (label || transcript_hash) under `key`. Matches the
 * Rust `hmac_tag()` function.
 */
void auth_hmac_tag(
    uint8_t tag_out[AUTH_MAC_TAG_LEN],
    const uint8_t key[AUTH_MAC_KEY_LEN],
    const uint8_t *label, size_t label_len,
    const uint8_t th[AUTH_HASH_LEN]);

/* ---- Blake2b device-root derivations ---- */

/*
 * device_id = Blake2b-256("device-id" || root)
 * Matches the Rust `derive_device_id()` function.
 */
void auth_derive_device_id(
    uint8_t device_id_out[AUTH_DEVICE_ID_LEN],
    const uint8_t root[AUTH_DEVICE_ROOT_LEN]);

/*
 * x = reduce_mod_L( Blake2b-512("device-auth-v1" || root) )
 * Matches the Rust `derive_device_scalar()` function.
 */
void auth_derive_device_scalar(
    uint8_t scalar_out[AUTH_SCALAR_LEN],
    const uint8_t root[AUTH_DEVICE_ROOT_LEN]);

/* ---- Zeroization ---- */

/*
 * Securely erase `buf`. Not optimized-away by the compiler.
 * Uses sodium_memzero().
 */
void auth_zeroize(void *buf, size_t len);

#ifdef __cplusplus
}
#endif
#endif /* AUTH_CRYPTO_H */
