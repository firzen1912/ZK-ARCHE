#include "auth/replay.h"

#include <sodium.h>
#include <string.h>

static const unsigned char AUTH_REPLAY_DOMAIN[] = "iot-auth/replay-key/v2";

void auth_replay_key(
    uint8_t out[AUTH_REPLAY_KEY_LEN],
    const uint8_t pid[32],
    const uint8_t nonce_c[32],
    const uint8_t eph_c[32])
{
    crypto_hash_sha256_state st;
    crypto_hash_sha256_init(&st);
    crypto_hash_sha256_update(
        &st, AUTH_REPLAY_DOMAIN, sizeof AUTH_REPLAY_DOMAIN - 1u);
    crypto_hash_sha256_update(&st, pid, 32u);
    crypto_hash_sha256_update(&st, nonce_c, 32u);
    crypto_hash_sha256_update(&st, eph_c, 32u);
    crypto_hash_sha256_final(&st, out);
}

void auth_replay_cache_init(auth_replay_cache_t *cache)
{
    if (!cache) return;
    sodium_memzero(cache, sizeof *cache);
}

int auth_replay_cache_contains(
    const auth_replay_cache_t *cache,
    const uint8_t key[AUTH_REPLAY_KEY_LEN])
{
    if (!cache || !key) return 0;
    for (size_t i = 0; i < cache->n; ++i) {
        if (sodium_memcmp(cache->keys[i], key, AUTH_REPLAY_KEY_LEN) == 0)
            return 1;
    }
    return 0;
}

int auth_replay_cache_insert(
    auth_replay_cache_t *cache,
    const uint8_t key[AUTH_REPLAY_KEY_LEN])
{
    if (!cache || !key) return 0;
    if (auth_replay_cache_contains(cache, key)) return 0;

    if (cache->n < AUTH_REPLAY_CACHE_CAPACITY) {
        memcpy(cache->keys[cache->n], key, AUTH_REPLAY_KEY_LEN);
        cache->n += 1u;
        return 1;
    }

    memcpy(cache->keys[cache->next_evict], key, AUTH_REPLAY_KEY_LEN);
    cache->next_evict = (cache->next_evict + 1u) % AUTH_REPLAY_CACHE_CAPACITY;
    return 1;
}
