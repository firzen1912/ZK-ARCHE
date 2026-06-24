/*
 * auth_store.h — filesystem-backed credential, registry, and
 * server-key stores. All store operations are atomic (write + rename).
 *
 * On embedded targets without a filesystem, replace this backend with
 * a flash-sector-based one implementing the same call signatures.
 */

#ifndef AUTH_STORE_H
#define AUTH_STORE_H

#include "iot_auth.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Client: CredentialStore ----
 *
 * Stores the device root secret (persistent), plus the pinned server
 * pubkey, stored role_commitment, and role_blind.
 *
 * File layout: one binary file, version-tagged, little-endian fields.
 *
 *   magic         [8]    "IACRED\0\1"
 *   version       u8     = 1
 *   flags         u8     bit 0: has_pinned_server, bit 1: has_role
 *   device_root   [32]
 *   server_pub    [32]   zeroed if !has_pinned_server
 *   role_commit   [32]   zeroed if !has_role
 *   role_blind    [32]   zeroed if !has_role
 *   role_code     u64    LE; zero if !has_role
 */

typedef struct auth_credentials {
    uint8_t  device_root     [AUTH_DEVICE_ROOT_LEN];
    int      has_pinned_server;
    uint8_t  server_pub_pinned[AUTH_POINT_LEN];
    int      has_role;
    uint8_t  role_commitment [AUTH_POINT_LEN];
    uint8_t  role_blind      [AUTH_SCALAR_LEN];
    uint64_t role_code;
} auth_credentials_t;

/*
 * Load credentials from `path`. If the file does not exist, returns
 * AUTH_ERR_CREDENTIAL_MISSING and leaves `out` zeroed so the
 * caller can start a fresh enrollment.
 */
auth_err_t auth_creds_load(
    auth_credentials_t *out, const char *path);

auth_err_t auth_creds_save(
    const auth_credentials_t *in, const char *path);

/*
 * Create a fresh credential set: generates device_root from the
 * system RNG; leaves server/role fields zeroed.
 */
void auth_creds_init_fresh(auth_credentials_t *out);

/* ---- Server: RegistryStore ----
 *
 * Maps device_id -> (device_pub, role_commitment). Single binary file.
 *
 *   magic     [8]   "IAREG\0\0\1"
 *   version   u8    = 1
 *   count     u32   LE, number of entries
 *   repeat `count` times:
 *       device_id  [32]
 *       device_pub [32]
 *       role_c     [32]
 */

typedef struct auth_registry_entry {
    uint8_t device_id       [AUTH_DEVICE_ID_LEN];
    uint8_t device_pub      [AUTH_POINT_LEN];
    uint8_t role_commitment [AUTH_POINT_LEN];
} auth_registry_entry_t;

#ifndef AUTH_REGISTRY_MAX_ENTRIES
#define AUTH_REGISTRY_MAX_ENTRIES 256
#endif

typedef struct auth_registry {
    auth_registry_entry_t entries[AUTH_REGISTRY_MAX_ENTRIES];
    size_t                    n;
    char                      path[256];
} auth_registry_t;

/* Load from disk; returns OK with empty registry if file missing. */
auth_err_t auth_registry_load(
    auth_registry_t *reg, const char *path);

/* Atomic-rename save (writes to path + ".tmp" first). */
auth_err_t auth_registry_save(const auth_registry_t *reg);

/*
 * Insert or update by device_id. Returns AUTH_ERR_TOO_MANY_ACTIVE
 * if the registry is full.
 */
auth_err_t auth_registry_put(
    auth_registry_t *reg,
    const uint8_t device_id       [AUTH_DEVICE_ID_LEN],
    const uint8_t device_pub      [AUTH_POINT_LEN],
    const uint8_t role_commitment [AUTH_POINT_LEN]);

/* Lookup by device_id. Returns AUTH_ERR_UNKNOWN_DEVICE on miss. */
auth_err_t auth_registry_get(
    const auth_registry_t *reg,
    const uint8_t device_id       [AUTH_DEVICE_ID_LEN],
    uint8_t device_pub            [AUTH_POINT_LEN],
    uint8_t role_commitment       [AUTH_POINT_LEN]);

/*
 * Scan callback: invoked for each enrolled device until it returns
 * non-zero, which stops iteration. Intended use: AUTH_1 handler tries
 * each candidate device to find the one whose Schnorr proof verifies.
 * `found_device_id` is set by the callback when it accepts the match.
 */
typedef int (*auth_registry_scan_fn)(
    void *ctx,
    const uint8_t device_id        [AUTH_DEVICE_ID_LEN],
    const uint8_t device_pub       [AUTH_POINT_LEN],
    const uint8_t role_commitment  [AUTH_POINT_LEN]);

int auth_registry_scan(
    const auth_registry_t *reg,
    auth_registry_scan_fn fn, void *ctx);

/* ---- Server: ServerKeyStore ----
 *
 * Holds the server's long-lived secret scalar on disk. Created on
 * first use; subsequently read-only.
 *
 *   magic    [8]   "IASKEY\0\1"
 *   version  u8    = 1
 *   server_sk [32]
 */

auth_err_t auth_server_key_load_or_create(
    uint8_t server_sk[AUTH_SCALAR_LEN],
    const char *path);

#ifdef __cplusplus
}
#endif
#endif /* AUTH_STORE_H */
