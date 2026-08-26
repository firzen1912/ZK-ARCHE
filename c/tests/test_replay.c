#include "auth/replay.h"

#include <sodium.h>
#include <stdio.h>
#include <string.h>

static int failures = 0;

#define CHECK(cond, msg) do { \
    if (!(cond)) { printf("  FAIL: %s\n", msg); failures++; } \
} while (0)

static void test_replay_key_binding(void)
{
    printf("== replay key binding ==\n");
    uint8_t pid[32], nonce_c[32], eph_c[32];
    uint8_t k1[32], k2[32];
    memset(pid, 0x11, sizeof pid);
    memset(nonce_c, 0x22, sizeof nonce_c);
    memset(eph_c, 0x33, sizeof eph_c);

    static const uint8_t expected[32] = {
        0xcb, 0xb6, 0x7c, 0x70, 0x8a, 0x39, 0x6d, 0x22,
        0x5c, 0x12, 0x63, 0x2b, 0xeb, 0xa9, 0x5a, 0x8d,
        0x73, 0x51, 0xbf, 0xaf, 0xf7, 0x41, 0x6b, 0x2e,
        0x0e, 0xbe, 0xe7, 0x2a, 0xa9, 0x22, 0xed, 0x30
    };

    auth_replay_key(k1, pid, nonce_c, eph_c);
    auth_replay_key(k2, pid, nonce_c, eph_c);
    CHECK(memcmp(k1, k2, sizeof k1) == 0, "replay key deterministic");
    CHECK(memcmp(k1, expected, sizeof k1) == 0,
          "replay key matches Rust canonical construction vector");

    pid[0] ^= 1u;
    auth_replay_key(k2, pid, nonce_c, eph_c);
    CHECK(memcmp(k1, k2, sizeof k1) != 0, "pid mutation changes replay key");
    pid[0] ^= 1u;

    nonce_c[0] ^= 1u;
    auth_replay_key(k2, pid, nonce_c, eph_c);
    CHECK(memcmp(k1, k2, sizeof k1) != 0, "nonce mutation changes replay key");
    nonce_c[0] ^= 1u;

    eph_c[0] ^= 1u;
    auth_replay_key(k2, pid, nonce_c, eph_c);
    CHECK(memcmp(k1, k2, sizeof k1) != 0, "ephemeral mutation changes replay key");
}

static void make_key(uint8_t key[AUTH_REPLAY_KEY_LEN], size_t value)
{
    memset(key, 0, AUTH_REPLAY_KEY_LEN);
    key[0] = (uint8_t)value;
    key[1] = (uint8_t)(value >> 8);
    key[2] = (uint8_t)(value >> 16);
    key[3] = (uint8_t)(value >> 24);
}

static void test_bounded_cache(void)
{
    printf("== bounded replay cache ==\n");
    auth_replay_cache_t cache;
    uint8_t key[AUTH_REPLAY_KEY_LEN];
    auth_replay_cache_init(&cache);
    memset(key, 0, sizeof key);

    CHECK(!auth_replay_cache_contains(&cache, key), "fresh cache misses");
    CHECK(auth_replay_cache_insert(&cache, key), "first insert succeeds");
    CHECK(auth_replay_cache_contains(&cache, key), "inserted key found");
    CHECK(!auth_replay_cache_insert(&cache, key), "duplicate insert rejected");

    for (size_t i = 1; i <= AUTH_REPLAY_CACHE_CAPACITY; ++i) {
        make_key(key, i);
        CHECK(auth_replay_cache_insert(&cache, key), "distinct insert succeeds");
    }
    CHECK(cache.n == AUTH_REPLAY_CACHE_CAPACITY, "cache remains bounded");
}

static void test_fifo_eviction_reopens_oldest_key(void)
{
    printf("== replay cache eviction boundary ==\n");
    auth_replay_cache_t cache;
    uint8_t oldest[AUTH_REPLAY_KEY_LEN];
    uint8_t key[AUTH_REPLAY_KEY_LEN];
    auth_replay_cache_init(&cache);

    make_key(oldest, 0x10000u);
    CHECK(auth_replay_cache_insert(&cache, oldest), "oldest key inserted");

    for (size_t i = 1; i < AUTH_REPLAY_CACHE_CAPACITY; ++i) {
        make_key(key, i);
        CHECK(auth_replay_cache_insert(&cache, key), "fill insert succeeds");
    }
    CHECK(cache.n == AUTH_REPLAY_CACHE_CAPACITY, "cache reaches exact capacity");
    CHECK(cache.next_evict == 0u, "FIFO cursor starts at oldest slot");
    CHECK(auth_replay_cache_contains(&cache, oldest), "oldest retained before pressure");

    make_key(key, AUTH_REPLAY_CACHE_CAPACITY + 1u);
    CHECK(auth_replay_cache_insert(&cache, key), "pressure insert succeeds");
    CHECK(cache.next_evict == 1u, "FIFO cursor advances after eviction");
    CHECK(!auth_replay_cache_contains(&cache, oldest),
          "oldest key is no longer protected after capacity eviction");
    CHECK(auth_replay_cache_insert(&cache, oldest),
          "evicted key is accepted as new cache state");
    CHECK(auth_replay_cache_contains(&cache, oldest),
          "reinserted evicted key becomes protected again");
}

static void test_restart_forgets_replay_state(void)
{
    printf("== replay cache restart state loss ==\n");
    auth_replay_cache_t cache;
    uint8_t key[AUTH_REPLAY_KEY_LEN];
    make_key(key, 0x20000u);

    auth_replay_cache_init(&cache);
    CHECK(auth_replay_cache_insert(&cache, key), "pre-restart insert succeeds");
    CHECK(auth_replay_cache_contains(&cache, key), "pre-restart key retained");

    auth_replay_cache_init(&cache);
    CHECK(cache.n == 0u, "restart resets replay entry count");
    CHECK(cache.next_evict == 0u, "restart resets FIFO cursor");
    CHECK(!auth_replay_cache_contains(&cache, key),
          "restart loses previously accepted replay key");
    CHECK(auth_replay_cache_insert(&cache, key),
          "post-restart key is accepted as new cache state");
}

int main(void)
{
    if (sodium_init() < 0) return 1;
    test_replay_key_binding();
    test_bounded_cache();
    test_fifo_eviction_reopens_oldest_key();
    test_restart_forgets_replay_state();
    printf("\n%s: %d failure(s)\n", failures ? "FAIL" : "PASS", failures);
    return failures ? 1 : 0;
}
