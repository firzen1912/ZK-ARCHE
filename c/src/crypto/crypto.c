/*
 * auth_crypto.c — ristretto255-based primitives built on libsodium.
 */

#include "auth/auth_crypto.h"

#include <sodium.h>
#include <string.h>

/* ---- Global init ---- */

static int g_initialized = 0;

auth_err_t auth_init(void)
{
    if (g_initialized) return AUTH_OK;
    if (sodium_init() < 0) return AUTH_ERR_UNSPECIFIED;
    g_initialized = 1;
    return AUTH_OK;
}

/* ---- Random ---- */

void auth_random_bytes32(uint8_t out[32])
{
    randombytes_buf(out, 32);
}

void auth_random_scalar(uint8_t out[AUTH_SCALAR_LEN])
{
    uint8_t wide[crypto_core_ristretto255_NONREDUCEDSCALARBYTES]; /* 64 */
    randombytes_buf(wide, sizeof wide);
    crypto_core_ristretto255_scalar_reduce(out, wide);
    sodium_memzero(wide, sizeof wide);
}

void auth_scalar_from_wide(const uint8_t wide[64],
                               uint8_t scalar_out[AUTH_SCALAR_LEN])
{
    crypto_core_ristretto255_scalar_reduce(scalar_out, wide);
}

/* ---- Validation ---- */

auth_err_t auth_check_point(const uint8_t point[AUTH_POINT_LEN])
{
    /* is_valid_point returns 1 on valid, 0 otherwise. It also rejects
     * the identity point, which is exactly the check we need (matches
     * Rust reject_identity + decompress_point). */
    if (crypto_core_ristretto255_is_valid_point(point) != 1) {
        /* Distinguish identity from malformed: the all-zero encoding is
         * the canonical identity; any other invalid encoding is a bad
         * point. */
        static const uint8_t identity[AUTH_POINT_LEN] = {0};
        if (sodium_memcmp(point, identity, AUTH_POINT_LEN) == 0) {
            return AUTH_ERR_IDENTITY_POINT;
        }
        return AUTH_ERR_INVALID_POINT;
    }
    return AUTH_OK;
}

auth_err_t auth_check_scalar(const uint8_t scalar[AUTH_SCALAR_LEN])
{
    /* libsodium exposes no direct "is_canonical" check; we do it by
     * reducing and comparing. If scalar == reduce(scalar), it was
     * already canonical. This isn't constant-time w.r.t. pass/fail,
     * but neither is the Rust implementation, and it's called on
     * attacker-supplied scalars where timing doesn't leak secrets. */
    uint8_t wide[64];
    memcpy(wide, scalar, 32);
    memset(wide + 32, 0, 32);
    uint8_t reduced[32];
    crypto_core_ristretto255_scalar_reduce(reduced, wide);
    int ok = (sodium_memcmp(reduced, scalar, 32) == 0);
    sodium_memzero(wide, sizeof wide);
    sodium_memzero(reduced, sizeof reduced);
    return ok ? AUTH_OK : AUTH_ERR_NONCANONICAL_SCALAR;
}

/* ---- Point arithmetic ---- */

auth_err_t auth_scalarmult_base(
    uint8_t out[AUTH_POINT_LEN],
    const uint8_t scalar[AUTH_SCALAR_LEN])
{
    if (crypto_scalarmult_ristretto255_base(out, scalar) != 0) {
        return AUTH_ERR_INVALID_POINT;
    }
    return AUTH_OK;
}

auth_err_t auth_scalarmult(
    uint8_t out[AUTH_POINT_LEN],
    const uint8_t scalar[AUTH_SCALAR_LEN],
    const uint8_t point[AUTH_POINT_LEN])
{
    if (crypto_scalarmult_ristretto255(out, scalar, point) != 0) {
        return AUTH_ERR_INVALID_POINT;
    }
    return AUTH_OK;
}

auth_err_t auth_point_add(
    uint8_t out[AUTH_POINT_LEN],
    const uint8_t a[AUTH_POINT_LEN],
    const uint8_t b[AUTH_POINT_LEN])
{
    if (crypto_core_ristretto255_add(out, a, b) != 0) {
        return AUTH_ERR_INVALID_POINT;
    }
    return AUTH_OK;
}

auth_err_t auth_point_sub(
    uint8_t out[AUTH_POINT_LEN],
    const uint8_t a[AUTH_POINT_LEN],
    const uint8_t b[AUTH_POINT_LEN])
{
    if (crypto_core_ristretto255_sub(out, a, b) != 0) {
        return AUTH_ERR_INVALID_POINT;
    }
    return AUTH_OK;
}

/* ---- Scalar arithmetic (mod L) ---- */

void auth_scalar_add(
    uint8_t out[AUTH_SCALAR_LEN],
    const uint8_t a[AUTH_SCALAR_LEN],
    const uint8_t b[AUTH_SCALAR_LEN])
{
    crypto_core_ristretto255_scalar_add(out, a, b);
}

void auth_scalar_sub(
    uint8_t out[AUTH_SCALAR_LEN],
    const uint8_t a[AUTH_SCALAR_LEN],
    const uint8_t b[AUTH_SCALAR_LEN])
{
    crypto_core_ristretto255_scalar_sub(out, a, b);
}

void auth_scalar_mul(
    uint8_t out[AUTH_SCALAR_LEN],
    const uint8_t a[AUTH_SCALAR_LEN],
    const uint8_t b[AUTH_SCALAR_LEN])
{
    crypto_core_ristretto255_scalar_mul(out, a, b);
}

void auth_scalar_from_u64(
    uint8_t out[AUTH_SCALAR_LEN],
    uint64_t value)
{
    /* Little-endian into first 8 bytes, rest zero. Matches Scalar::from(u64). */
    memset(out, 0, AUTH_SCALAR_LEN);
    for (size_t i = 0; i < 8; ++i) {
        out[i] = (uint8_t)(value >> (8 * i));
    }
}

/* ---- Hash-to-point ---- */

auth_err_t auth_hash_to_point(
    uint8_t point_out[AUTH_POINT_LEN],
    const uint8_t *label,
    size_t label_len)
{
    /*
     * Matches Rust hash_to_point():
     *   wide = SHA-512("ristretto-hash-to-point-v1" || label)
     *   return ristretto255_from_hash(wide)
     */
    static const uint8_t DOM[] = "ristretto-hash-to-point-v1";
    crypto_hash_sha512_state st;
    uint8_t wide[crypto_hash_sha512_BYTES]; /* 64 */

    crypto_hash_sha512_init(&st);
    crypto_hash_sha512_update(&st, DOM, sizeof DOM - 1);
    if (label_len) crypto_hash_sha512_update(&st, label, label_len);
    crypto_hash_sha512_final(&st, wide);

    if (crypto_core_ristretto255_from_hash(point_out, wide) != 0) {
        sodium_memzero(wide, sizeof wide);
        return AUTH_ERR_INVALID_POINT;
    }
    sodium_memzero(wide, sizeof wide);
    return AUTH_OK;
}

auth_err_t auth_attr_h(uint8_t out[AUTH_POINT_LEN])
{
    static const uint8_t LABEL[] = "iot-auth/attr-h/v1";
    return auth_hash_to_point(out, LABEL, sizeof LABEL - 1);
}

/* ---- HKDF-SHA256 (libsodium 1.0.18 has no built-in; we compose HMAC) ---- */

auth_err_t auth_hkdf_sha256(
    uint8_t *okm, size_t okm_len,
    const uint8_t *salt, size_t salt_len,
    const uint8_t *ikm,  size_t ikm_len,
    const uint8_t *info, size_t info_len)
{
    if (!okm || okm_len == 0) return AUTH_ERR_INVALID_ARGUMENT;
    if (!ikm && ikm_len > 0)  return AUTH_ERR_INVALID_ARGUMENT;
    if (!info && info_len > 0) return AUTH_ERR_INVALID_ARGUMENT;

    /* RFC 5869, hash = SHA-256, HashLen = 32. Max okm_len = 255*32 = 8160. */
    if (okm_len > 255u * 32u) return AUTH_ERR_INVALID_ARGUMENT;

    /* Default salt is 32 zero bytes if none provided. */
    uint8_t default_salt[32] = {0};
    if (salt == NULL || salt_len == 0) {
        salt = default_salt;
        salt_len = 32;
    }

    /* Extract: PRK = HMAC-SHA256(salt, ikm) */
    uint8_t prk[32];
    {
        crypto_auth_hmacsha256_state st;
        crypto_auth_hmacsha256_init(&st, salt, salt_len);
        if (ikm_len) crypto_auth_hmacsha256_update(&st, ikm, ikm_len);
        crypto_auth_hmacsha256_final(&st, prk);
    }

    /* Expand: T(0)=empty; T(i) = HMAC-SHA256(PRK, T(i-1) || info || i) */
    uint8_t t[32];
    size_t   t_len = 0;
    size_t   written = 0;
    uint8_t  counter = 1;
    while (written < okm_len) {
        crypto_auth_hmacsha256_state st;
        crypto_auth_hmacsha256_init(&st, prk, sizeof prk);
        if (t_len) crypto_auth_hmacsha256_update(&st, t, t_len);
        if (info_len) crypto_auth_hmacsha256_update(&st, info, info_len);
        crypto_auth_hmacsha256_update(&st, &counter, 1);
        crypto_auth_hmacsha256_final(&st, t);
        t_len = 32;

        size_t to_copy = (okm_len - written) < 32 ? (okm_len - written) : 32;
        memcpy(okm + written, t, to_copy);
        written += to_copy;
        counter++;
    }

    sodium_memzero(prk, sizeof prk);
    sodium_memzero(t,   sizeof t);
    return AUTH_OK;
}

/* ---- HMAC tag ---- */

void auth_hmac_tag(
    uint8_t tag_out[AUTH_MAC_TAG_LEN],
    const uint8_t key[AUTH_MAC_KEY_LEN],
    const uint8_t *label, size_t label_len,
    const uint8_t th[AUTH_HASH_LEN])
{
    crypto_auth_hmacsha256_state st;
    crypto_auth_hmacsha256_init(&st, key, AUTH_MAC_KEY_LEN);
    if (label_len) crypto_auth_hmacsha256_update(&st, label, label_len);
    crypto_auth_hmacsha256_update(&st, th, AUTH_HASH_LEN);
    crypto_auth_hmacsha256_final(&st, tag_out);
}

/* ---- Blake2b derivations ---- */

void auth_derive_device_id(
    uint8_t id_out[AUTH_DEVICE_ID_LEN],
    const uint8_t root[AUTH_DEVICE_ROOT_LEN])
{
    /* Matches Rust derive_device_id():
     *   id = Blake2b-256("device-id" || root)
     *
     * Rust used Blake2bVar::new(32); libsodium's generichash defaults
     * to Blake2b. We request a 32-byte output with no key, which
     * matches Blake2b-256 over ("device-id" || root). */
    static const uint8_t DOM[] = "device-id";
    crypto_generichash_state st;
    crypto_generichash_init(&st, NULL, 0, AUTH_DEVICE_ID_LEN);
    crypto_generichash_update(&st, DOM, sizeof DOM - 1);
    crypto_generichash_update(&st, root, AUTH_DEVICE_ROOT_LEN);
    crypto_generichash_final(&st, id_out, AUTH_DEVICE_ID_LEN);
}

void auth_derive_device_scalar(
    uint8_t scalar_out[AUTH_SCALAR_LEN],
    const uint8_t root[AUTH_DEVICE_ROOT_LEN])
{
    /* Matches Rust derive_device_scalar():
     *   wide = Blake2b-512("device-auth-v1" || root)
     *   x    = reduce_mod_L(wide)
     *
     * Blake2b-512 here means requesting a 64-byte digest from the
     * generichash Blake2b implementation. */
    static const uint8_t DOM[] = "device-auth-v1";
    uint8_t wide[64];
    crypto_generichash_state st;
    crypto_generichash_init(&st, NULL, 0, sizeof wide);
    crypto_generichash_update(&st, DOM, sizeof DOM - 1);
    crypto_generichash_update(&st, root, AUTH_DEVICE_ROOT_LEN);
    crypto_generichash_final(&st, wide, sizeof wide);

    crypto_core_ristretto255_scalar_reduce(scalar_out, wide);
    sodium_memzero(wide, sizeof wide);
}

/* ---- Zeroize ---- */

void auth_zeroize(void *buf, size_t len)
{
    sodium_memzero(buf, len);
}

/* ---- Error strings ---- */

const char *auth_strerror(auth_err_t err)
{
    switch (err) {
    case AUTH_OK:                       return "OK";
    case AUTH_ERR_INVALID_ARGUMENT:     return "invalid argument";
    case AUTH_ERR_BUFFER_TOO_SMALL:     return "buffer too small";
    case AUTH_ERR_NOT_INITIALIZED:      return "not initialized";
    case AUTH_ERR_IO:                   return "I/O error";
    case AUTH_ERR_TIMEOUT:              return "timeout";
    case AUTH_ERR_UNSUPPORTED_VERSION:  return "unsupported protocol version";
    case AUTH_ERR_UNSUPPORTED_SUITE:    return "unsupported cipher suite";
    case AUTH_ERR_CAP_MISMATCH:         return "capability mismatch";
    case AUTH_ERR_MALFORMED_PACKET:     return "malformed packet";
    case AUTH_ERR_UNKNOWN_PKT_TYPE:     return "unknown packet type";
    case AUTH_ERR_PAYLOAD_TOO_LARGE:    return "payload too large";
    case AUTH_ERR_PAYLOAD_TOO_SHORT:    return "payload too short";
    case AUTH_ERR_INVALID_ENCODING:     return "invalid encoding";
    case AUTH_ERR_INVALID_POINT:        return "invalid Ristretto255 point";
    case AUTH_ERR_NONCANONICAL_SCALAR:  return "non-canonical scalar";
    case AUTH_ERR_IDENTITY_POINT:       return "identity point rejected";
    case AUTH_ERR_PROOF_VERIFY:         return "proof verification failed";
    case AUTH_ERR_KEY_CONFIRM:          return "key confirmation failed";
    case AUTH_ERR_PEER_KEY_MISMATCH:    return "peer key mismatch";
    case AUTH_ERR_UNKNOWN_SESSION:      return "unknown session";
    case AUTH_ERR_SESSION_EXPIRED:      return "session expired";
    case AUTH_ERR_REPLAY_DETECTED:      return "replay detected";
    case AUTH_ERR_SEQ_OUT_OF_ORDER:     return "sequence number out of order";
    case AUTH_ERR_UNKNOWN_DEVICE:       return "unknown device";
    case AUTH_ERR_DEVICE_NOT_ENROLLED:  return "device not enrolled";
    case AUTH_ERR_ROLE_NOT_PERMITTED:   return "role not permitted";
    case AUTH_ERR_PAIRING_TOKEN_BAD:    return "pairing token invalid";
    case AUTH_ERR_RATE_LIMITED:         return "rate limited";
    case AUTH_ERR_SERVER_BUSY:          return "server busy";
    case AUTH_ERR_TOO_MANY_ACTIVE:      return "too many active sessions";
    case AUTH_ERR_STORAGE_FAILURE:      return "storage failure";
    case AUTH_ERR_CREDENTIAL_MISSING:   return "credential missing";
    case AUTH_ERR_REGISTRY_CORRUPT:     return "registry corrupt";
    case AUTH_ERR_UNSPECIFIED:          return "unspecified";
    }
    return "unknown error code";
}

int auth_err_is_wire(auth_err_t err)
{
    uint16_t code = (uint16_t)err;
    return code >= 0x0100 && code <= 0x07FF;
}
