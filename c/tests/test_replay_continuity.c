#include "auth/replay_continuity.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures = 0;

#define CHECK(cond, msg) do { \
    if (!(cond)) { printf("  FAIL: %s\n", msg); failures++; } \
} while (0)

static FILE *open_corpus(void)
{
    static const char *paths[] = {
        "../rust/test-vectors/replay-continuity/state-v1.txt",
        "rust/test-vectors/replay-continuity/state-v1.txt"
    };

    for (size_t i = 0; i < sizeof paths / sizeof paths[0]; ++i) {
        FILE *f = fopen(paths[i], "rb");
        if (f != NULL) return f;
    }
    return NULL;
}

static auth_replay_continuity_state_t parse_state(const char *value)
{
    if (strcmp(value, "TRUSTED") == 0) return AUTH_REPLAY_CONTINUITY_TRUSTED;
    if (strcmp(value, "RESTORING") == 0) return AUTH_REPLAY_CONTINUITY_RESTORING;
    if (strcmp(value, "CONTINUITY_BROKEN") == 0) return AUTH_REPLAY_CONTINUITY_BROKEN;
    CHECK(0, "unknown replay continuity state");
    return AUTH_REPLAY_CONTINUITY_BROKEN;
}

static auth_replay_continuity_event_t parse_event(const char *value)
{
    if (strcmp(value, "restart") == 0) return AUTH_REPLAY_EVENT_RESTART;
    if (strcmp(value, "restored_trusted_window") == 0) {
        return AUTH_REPLAY_EVENT_RESTORED_TRUSTED_WINDOW;
    }
    if (strcmp(value, "restore_missing") == 0) return AUTH_REPLAY_EVENT_RESTORE_MISSING;
    if (strcmp(value, "restore_corrupt") == 0) return AUTH_REPLAY_EVENT_RESTORE_CORRUPT;
    if (strcmp(value, "restore_stale") == 0) return AUTH_REPLAY_EVENT_RESTORE_STALE;
    if (strcmp(value, "rollback_suspected") == 0) {
        return AUTH_REPLAY_EVENT_ROLLBACK_SUSPECTED;
    }
    if (strcmp(value, "empty_cache_reset") == 0) return AUTH_REPLAY_EVENT_EMPTY_CACHE_RESET;
    if (strcmp(value, "fresh_outer_session") == 0) {
        return AUTH_REPLAY_EVENT_FRESH_OUTER_SESSION;
    }
    if (strcmp(value, "failed_auth") == 0) return AUTH_REPLAY_EVENT_FAILED_AUTH;
    CHECK(0, "unknown replay continuity event");
    return AUTH_REPLAY_EVENT_FAILED_AUTH;
}

static void test_shared_corpus(void)
{
    printf("== shared replay continuity state corpus ==\n");
    FILE *f = open_corpus();
    CHECK(f != NULL, "open shared replay continuity corpus");
    if (f == NULL) return;

    char line[256];
    CHECK(fgets(line, sizeof line, f) != NULL, "read corpus format");
    CHECK(strcmp(line, "format=ZKREPLAYCONTINUITY/1\n") == 0, "corpus format");
    CHECK(fgets(line, sizeof line, f) != NULL, "read corpus header");
    CHECK(strcmp(line, "id|initial|event|expected|apply_ok|auth_allowed\n") == 0,
          "corpus header");

    unsigned int cases = 0u;
    while (fgets(line, sizeof line, f) != NULL) {
        size_t len = strlen(line);
        if (len > 0u && line[len - 1u] == '\n') line[len - 1u] = '\0';
        CHECK(line[0] != '\0', "blank corpus row");
        if (line[0] == '\0') continue;

        char *fields[6];
        char *cursor = line;
        for (size_t i = 0; i < 6u; ++i) {
            fields[i] = cursor;
            char *sep = strchr(cursor, '|');
            if (i < 5u) {
                CHECK(sep != NULL, "corpus row has six fields");
                if (sep == NULL) break;
                *sep = '\0';
                cursor = sep + 1;
            } else {
                CHECK(sep == NULL, "corpus row has no extra fields");
            }
        }

        auth_replay_continuity_state_t initial = parse_state(fields[1]);
        auth_replay_continuity_state_t expected = parse_state(fields[3]);
        auth_replay_continuity_event_t event = parse_event(fields[2]);
        int apply_ok = strcmp(fields[4], "1") == 0 ? 1 : 0;
        int auth_allowed = strcmp(fields[5], "1") == 0 ? 1 : 0;

        auth_replay_continuity_t continuity;
        auth_replay_continuity_init(&continuity, initial);
        int rc = auth_replay_continuity_apply(&continuity, event);
        CHECK((rc == 0 ? 1 : 0) == apply_ok, fields[0]);
        CHECK(continuity.state == expected, fields[0]);
        CHECK(auth_replay_continuity_auth_allowed(&continuity) == auth_allowed, fields[0]);
        cases++;
    }

    fclose(f);
    CHECK(cases >= 12u, "replay continuity corpus unexpectedly small");
}

int main(void)
{
    test_shared_corpus();
    printf("\n%s: %d failure(s)\n", failures ? "FAIL" : "PASS", failures);
    return failures ? 1 : 0;
}
