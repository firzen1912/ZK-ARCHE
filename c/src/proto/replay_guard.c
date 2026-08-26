#include "auth/replay_guard.h"

#include <string.h>

auth_err_t auth_server_handle_auth1_guarded(
    auth_replay_cache_t *replay_cache,
    const uint8_t session_id[AUTH_SESSION_ID_LEN],
    uint32_t seq,
    const uint8_t *in_payload, size_t in_len,
    const uint8_t server_sk[AUTH_SCALAR_LEN],
    const uint8_t server_pub[AUTH_POINT_LEN],
    auth_registry_lookup_fn lookup_fn, void *lookup_ctx,
    const uint64_t *allowed_roles, size_t n_allowed,
    auth_pending_auth_t *pending_out,
    uint8_t *out, size_t out_cap, size_t *out_len)
{
    if (!replay_cache || !session_id || !in_payload || !server_sk ||
        !server_pub || !lookup_fn || !allowed_roles || !pending_out ||
        !out || !out_len) {
        return AUTH_ERR_INVALID_ARGUMENT;
    }

    auth_auth1_t a1;
    auth_err_t err = auth_auth1_decode(&a1, in_payload, in_len);
    if (err) return err;

    uint8_t replay_key[AUTH_REPLAY_KEY_LEN];
    auth_replay_key(replay_key, a1.pid, a1.nonce_c, a1.eph_c);

    if (auth_replay_cache_contains(replay_cache, replay_key)) {
        memset(replay_key, 0, sizeof replay_key);
        return AUTH_ERR_REPLAY_DETECTED;
    }

    err = auth_server_handle_auth1(
        session_id, seq,
        in_payload, in_len,
        server_sk, server_pub,
        lookup_fn, lookup_ctx,
        allowed_roles, n_allowed,
        pending_out,
        out, out_cap, out_len);
    if (err) {
        memset(replay_key, 0, sizeof replay_key);
        return err;
    }

    /* Insert only after full AUTH_1 verification succeeds. If another
     * serialized claimant already inserted the key, fail closed as replay. */
    if (!auth_replay_cache_insert(replay_cache, replay_key)) {
        memset(replay_key, 0, sizeof replay_key);
        memset(pending_out, 0, sizeof *pending_out);
        *out_len = 0;
        return AUTH_ERR_REPLAY_DETECTED;
    }

    memset(replay_key, 0, sizeof replay_key);
    return AUTH_OK;
}
