#ifndef AUTH_REPLAY_H
#define AUTH_REPLAY_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define AUTH_REPLAY_KEY_LEN 32u

#ifndef AUTH_REPLAY_CACHE_CAPACITY
#define AUTH_REPLAY_CACHE_CAPACITY 64u
#endif

typedef struct auth_replay_cache {
    uint8_t keys[AUTH_REPLAY_CACHE_CAPACITY][AUTH_REPLAY_KEY_LEN];
    size_t n;
    size_t next_evict;
} auth_replay_cache_t;

void auth_replay_key(
    uint8_t out[AUTH_REPLAY_KEY_LEN],
    const uint8_t pid[32],
    const uint8_t nonce_c[32],
    const uint8_t eph_c[32]);

void auth_replay_cache_init(auth_replay_cache_t *cache);

int auth_replay_cache_contains(
    const auth_replay_cache_t *cache,
    const uint8_t key[AUTH_REPLAY_KEY_LEN]);

int auth_replay_cache_insert(
    auth_replay_cache_t *cache,
    const uint8_t key[AUTH_REPLAY_KEY_LEN]);

#ifdef __cplusplus
}
#endif
#endif /* AUTH_REPLAY_H */
