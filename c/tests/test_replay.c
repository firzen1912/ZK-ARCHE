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

static FILE *open_shared_fifo_corpus(void)
{
    static const char *paths[] = {
        "../rust/test-vectors/replay-cache/fifo-capacity-64.txt",
        "rust/test-vectors/replay-cache/fifo-capacity-64.txt"
    };

    for (size_t i = 0; i < sizeof paths / sizeof paths[0]; ++i) {
        FILE *f = fopen(paths[i], "r");
        if (f != NULL) return f;
    }
    return NULL;
}

static void test_shared_fifo_capacity_64_corpus(void)
{
    printf("== shared Rust/C FIFO replay corpus ==\n");
    FILE *f = open_shared_fifo_corpus();
    CHECK(f != NULL, "open shared replay corpus");
    if (f == NULL) return;

    auth_replay_cache_t cache;
    uint8_t key[AUTH_REPLAY_KEY_LEN];
    char line[160];
    char op[16];
    char expected[16];
    size_t index = 0u;
    size_t declared_capacity = 0u;
    int saw_capacity = 0;

    auth_replay_cache_init(&cache);

    while (fgets(line, sizeof line, f) != NULL) {
        char *p = line;
        while (*p == ' ' || *p == '\t') ++p;
        if (*p == '\0' || *p == '\n' || *p == '#') continue;

        size_t capacity = 0u;
        if (sscanf(p, "capacity=%zu", &capacity) == 1) {
            declared_capacity = capacity;
            saw_capacity = 1;
            CHECK(capacity == AUTH_REPLAY_CACHE_CAPACITY,
                  "shared corpus capacity matches C replay capacity");
            continue;
        }

        if (sscanf(p, "%15s %zu %15s", op, &index, expected) != 3) {
            CHECK(0, "parse shared replay corpus operation");
            continue;
        }

        make_key(key, index);

        if (strcmp(op, "insert") == 0) {
            int actual = auth_replay_cache_insert(&cache, key);
            if (strcmp(expected, "new") == 0) {
                CHECK(actual != 0, "shared corpus insert expected new");
            } else if (strcmp(expected, "duplicate") == 0) {
                CHECK(actual == 0, "shared corpus insert expected duplicate");
            } else {
                CHECK(0, "unknown shared corpus insert expectation");
            }
        } else if (strcmp(op, "contains") == 0) {
            int actual = auth_replay_cache_contains(&cache, key);
            if (strcmp(expected, "present") == 0) {
                CHECK(actual != 0, "shared corpus contains expected present");
            } else if (strcmp(expected, "absent") == 0) {
                CHECK(actual == 0, "shared corpus contains expected absent");
            } else {
                CHECK(0, "unknown shared corpus contains expectation");
            }
        } else {
            CHECK(0, "unknown shared corpus operation");
        }
    }

    fclose(f);
    CHECK(saw_capacity, "shared corpus declares replay capacity");
    CHECK(declared_capacity == 64u, "shared corpus uses matched capacity 64");
    CHECK(cache.n == AUTH_REPLAY_CACHE_CAPACITY,
          "shared corpus leaves cache at bounded capacity");
}

int main(void)
{
    if (sodium_init() < 0) return 1;
    test_replay_key_binding();
    test_bounded_cache();
    test_fifo_eviction_reopens_oldest_key();
    test_restart_forgets_replay_state();
    test_shared_fifo_capacity_64_corpus();
    printf("\n%s: %d failure(s)\n", failures ? "FAIL" : "PASS", failures);
    return failures ? 1 : 0;
}
