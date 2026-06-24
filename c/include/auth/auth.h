/*
 * iot_auth.h — public API for the Auth ZK-ARCHE v2 protocol (C).
 *
 * Wire-compatible with the Rust reference implementation. See
 * spec/iot-auth-wire-spec.docx for the normative specification and
 * test-vectors/0x0001/ for cross-language conformance fixtures.
 *
 * Design notes for embedded targets:
 *
 *   - All crypto buffers are fixed-size: 32 bytes for points and scalars,
 *     32 bytes for hashes, 16 bytes for session ids.
 *   - Wire packets are bounded by AUTH_MAX_DATAGRAM (2048 bytes).
 *   - Callers always provide the output buffer; the library never
 *     allocates. Stateful objects (stores, sessions) are allocated once
 *     at init and re-used.
 *   - Every function returns auth_err_t. Error strings are available
 *     via auth_strerror().
 *   - This file is self-contained: including it does not pull libsodium
 *     into the consumer's translation unit.
 */

#ifndef AUTH_H
#define AUTH_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Version ---- */

#define AUTH_PROTOCOL_VERSION     0x02
#define AUTH_MIN_SUPPORTED_VERSION 0x02

#define AUTH_LIB_VERSION_MAJOR 0
#define AUTH_LIB_VERSION_MINOR 2
#define AUTH_LIB_VERSION_PATCH 0

/* ---- Fixed sizes ---- */

#define AUTH_POINT_LEN            32
#define AUTH_SCALAR_LEN           32
#define AUTH_NONCE_LEN            32
#define AUTH_HASH_LEN             32  /* SHA-256 / Blake2b-256 output */
#define AUTH_DEVICE_ID_LEN        32
#define AUTH_DEVICE_ROOT_LEN      32
#define AUTH_SETUP_CHALLENGE_LEN  16
#define AUTH_SESSION_ID_LEN       16
#define AUTH_SESSION_KEY_LEN      32
#define AUTH_MAC_KEY_LEN          32
#define AUTH_MAC_TAG_LEN          32
#define AUTH_HEADER_LEN           24
#define AUTH_MAX_DATAGRAM         2048
#define AUTH_MAX_PAYLOAD          (AUTH_MAX_DATAGRAM - AUTH_HEADER_LEN)
#define AUTH_MAX_PAIRING_TOKEN    128

/* Maximum branches in the role-set CDS-OR proof. Embedded builds may
 * override at compile time; default is small enough for ~2k-byte packets. */
#ifndef AUTH_MAX_ROLES
#define AUTH_MAX_ROLES            8
#endif

/* ---- Suite identifiers ---- */

typedef uint16_t auth_suite_t;
#define AUTH_SUITE_RISTRETTO255_SHA256  ((auth_suite_t)0x0001)

/* ---- Capability bitmap (u64) ---- */

typedef uint64_t auth_caps_t;
#define AUTH_CAP_AUTH_V2               (1ULL << 0)
#define AUTH_CAP_ROLE_RERAND           (1ULL << 1)
#define AUTH_CAP_ROLE_SET_MEMBERSHIP   (1ULL << 2)
#define AUTH_CAP_PAIRING_TOKEN         (1ULL << 3)
#define AUTH_CAP_TOFU_SETUP            (1ULL << 4)
#define AUTH_CAP_PROFILE_MINIMAL       (1ULL << 8)
#define AUTH_CAP_PROFILE_STANDARD      (1ULL << 9)
#define AUTH_CAP_PROFILE_GATEWAY       (1ULL << 10)
#define AUTH_CAP_CBOR_FRAMING          (1ULL << 16)

#define AUTH_CAPS_BASELINE ( \
        AUTH_CAP_AUTH_V2 | \
        AUTH_CAP_ROLE_RERAND | \
        AUTH_CAP_ROLE_SET_MEMBERSHIP | \
        AUTH_CAP_PROFILE_STANDARD)

/* ---- Structured error codes (wire-stable u16) ---- */

typedef enum auth_err {
    AUTH_OK                     = 0x0000,

    /* Local-only codes */
    AUTH_ERR_INVALID_ARGUMENT   = 0x0001,
    AUTH_ERR_BUFFER_TOO_SMALL   = 0x0002,
    AUTH_ERR_NOT_INITIALIZED    = 0x0003,
    AUTH_ERR_IO                 = 0x0004,
    AUTH_ERR_TIMEOUT            = 0x0005,

    /* Version / capability (0x0100-0x01FF) */
    AUTH_ERR_UNSUPPORTED_VERSION= 0x0101,
    AUTH_ERR_UNSUPPORTED_SUITE  = 0x0102,
    AUTH_ERR_CAP_MISMATCH       = 0x0103,

    /* Packet framing (0x0200-0x02FF) */
    AUTH_ERR_MALFORMED_PACKET   = 0x0201,
    AUTH_ERR_UNKNOWN_PKT_TYPE   = 0x0202,
    AUTH_ERR_PAYLOAD_TOO_LARGE  = 0x0203,
    AUTH_ERR_PAYLOAD_TOO_SHORT  = 0x0204,
    AUTH_ERR_INVALID_ENCODING   = 0x0205,

    /* Cryptographic validation (0x0300-0x03FF) */
    AUTH_ERR_INVALID_POINT      = 0x0301,
    AUTH_ERR_NONCANONICAL_SCALAR= 0x0302,
    AUTH_ERR_IDENTITY_POINT     = 0x0303,
    AUTH_ERR_PROOF_VERIFY       = 0x0304,
    AUTH_ERR_KEY_CONFIRM        = 0x0305,
    AUTH_ERR_PEER_KEY_MISMATCH  = 0x0306,

    /* Session / replay (0x0400-0x04FF) */
    AUTH_ERR_UNKNOWN_SESSION    = 0x0401,
    AUTH_ERR_SESSION_EXPIRED    = 0x0402,
    AUTH_ERR_REPLAY_DETECTED    = 0x0403,
    AUTH_ERR_SEQ_OUT_OF_ORDER   = 0x0404,

    /* Authorization (0x0500-0x05FF) */
    AUTH_ERR_UNKNOWN_DEVICE     = 0x0501,
    AUTH_ERR_DEVICE_NOT_ENROLLED= 0x0502,
    AUTH_ERR_ROLE_NOT_PERMITTED = 0x0503,
    AUTH_ERR_PAIRING_TOKEN_BAD  = 0x0504,

    /* Rate / resource (0x0600-0x06FF) */
    AUTH_ERR_RATE_LIMITED       = 0x0601,
    AUTH_ERR_SERVER_BUSY        = 0x0602,
    AUTH_ERR_TOO_MANY_ACTIVE    = 0x0603,

    /* Storage (0x0700-0x07FF) */
    AUTH_ERR_STORAGE_FAILURE    = 0x0701,
    AUTH_ERR_CREDENTIAL_MISSING = 0x0702,
    AUTH_ERR_REGISTRY_CORRUPT   = 0x0703,

    AUTH_ERR_UNSPECIFIED        = 0x7FFF
} auth_err_t;

/* Return a static human-readable name for an error. Does NOT allocate. */
const char *auth_strerror(auth_err_t err);

/* True if `err` represents a wire-transmittable condition (category >= 0x01). */
int auth_err_is_wire(auth_err_t err);

/* ---- Library init ---- */

/*
 * Initialize libsodium and any global state. Must be called once per
 * process before any other auth_* function. Returns AUTH_OK on
 * success; thread-safe and idempotent.
 */
auth_err_t auth_init(void);

#ifdef __cplusplus
}
#endif
#endif /* AUTH_H */
