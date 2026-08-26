#ifndef AUTH_REPLAY_GUARD_H
#define AUTH_REPLAY_GUARD_H

#include "auth/proto.h"
#include "auth/replay.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Replay-protected AUTH_1 entry point.
 *
 * The caller owns replay_cache for the lifetime of the replay window and
 * MUST serialize access when the same cache is shared across threads.
 * The guard computes the canonical replay key before registry/proof work,
 * rejects keys already accepted, delegates full AUTH_1 verification to the
 * existing state machine, and records the key only after that verification
 * succeeds. Invalid unauthenticated packets therefore cannot poison the
 * replay cache.
 */
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
    uint8_t *out, size_t out_cap, size_t *out_len);

#ifdef __cplusplus
}
#endif
#endif /* AUTH_REPLAY_GUARD_H */
