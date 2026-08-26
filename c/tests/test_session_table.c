#include "auth/session_table.h"

#include <stdio.h>
#include <string.h>

static int failures = 0;

#define CHECK(cond, msg) do { \
    if (!(cond)) { printf("  FAIL: %s\n", msg); failures++; } \
} while (0)

static void sid_fill(uint8_t sid[AUTH_SESSION_ID_LEN], uint8_t v)
{
    memset(sid, v, AUTH_SESSION_ID_LEN);
}

static void test_reservation_lifecycle(void)
{
    printf("== session reservation lifecycle ==\n");
    auth_session_table_t table;
    auth_session_table_init(&table);

    int a = auth_session_table_reserve(&table);
    int b = auth_session_table_reserve(&table);
    CHECK(a >= 0, "first reservation succeeds");
    CHECK(b >= 0, "second reservation succeeds");
    CHECK(a != b, "distinct reservations never alias");
    CHECK(auth_session_table_state(&table, a) == AUTH_SESSION_SLOT_RESERVED,
          "first slot remains reserved");
    CHECK(auth_session_table_state(&table, b) == AUTH_SESSION_SLOT_RESERVED,
          "second slot remains reserved");

    uint8_t sid_a[AUTH_SESSION_ID_LEN];
    uint8_t sid_b[AUTH_SESSION_ID_LEN];
    sid_fill(sid_a, 0x11);
    sid_fill(sid_b, 0x22);

    CHECK(auth_session_table_find_setup(&table, sid_a) < 0,
          "reserved slot is not visible as setup");
    CHECK(auth_session_table_find_auth(&table, sid_a) < 0,
          "reserved slot is not visible as auth");

    auth_pending_setup_t setup = {0};
    memcpy(setup.session_id, sid_a, sizeof setup.session_id);
    CHECK(auth_session_table_activate_setup(&table, a, &setup) == AUTH_OK,
          "reserved slot promotes to setup");
    CHECK(auth_session_table_find_setup(&table, sid_a) == a,
          "active setup is discoverable");
    CHECK(auth_session_table_find_auth(&table, sid_a) < 0,
          "setup is not discoverable as auth");

    auth_pending_auth_t auth = {0};
    memcpy(auth.session_id, sid_b, sizeof auth.session_id);
    CHECK(auth_session_table_activate_auth(&table, b, &auth) == AUTH_OK,
          "reserved slot promotes to auth");
    CHECK(auth_session_table_find_auth(&table, sid_b) == b,
          "active auth is discoverable");

    CHECK(auth_session_table_release(&table, a) == AUTH_OK,
          "setup slot release succeeds");
    CHECK(auth_session_table_find_setup(&table, sid_a) < 0,
          "released setup is no longer discoverable");
    CHECK(auth_session_table_state(&table, a) == AUTH_SESSION_SLOT_FREE,
          "released setup returns to free");

    int c = auth_session_table_reserve(&table);
    CHECK(c == a, "released slot can be reserved again deterministically");
}

static void test_invalid_transitions(void)
{
    printf("== invalid session transitions ==\n");
    auth_session_table_t table;
    auth_session_table_init(&table);

    auth_pending_auth_t auth = {0};
    auth_pending_setup_t setup = {0};
    int slot = auth_session_table_reserve(&table);
    CHECK(slot >= 0, "reservation succeeds");
    CHECK(auth_session_table_activate_auth(&table, slot, &auth) == AUTH_OK,
          "activation succeeds once");
    CHECK(auth_session_table_activate_auth(&table, slot, &auth) == AUTH_ERR_INVALID_ARGUMENT,
          "active slot cannot be activated twice");
    CHECK(auth_session_table_activate_setup(&table, slot, &setup) == AUTH_ERR_INVALID_ARGUMENT,
          "active auth slot cannot change kind without release");
    CHECK(auth_session_table_release(&table, -1) == AUTH_ERR_INVALID_ARGUMENT,
          "negative slot rejected");
    CHECK(auth_session_table_release(&table, (int)AUTH_SESSION_TABLE_CAPACITY) == AUTH_ERR_INVALID_ARGUMENT,
          "out-of-range slot rejected");
}

static void test_capacity_is_bounded(void)
{
    printf("== bounded session capacity ==\n");
    auth_session_table_t table;
    auth_session_table_init(&table);

    for (size_t i = 0; i < AUTH_SESSION_TABLE_CAPACITY; ++i) {
        CHECK(auth_session_table_reserve(&table) >= 0,
              "reservation succeeds before capacity");
    }
    CHECK(auth_session_table_reserve(&table) < 0,
          "reservation fails closed when full");
}

int main(void)
{
    test_reservation_lifecycle();
    test_invalid_transitions();
    test_capacity_is_bounded();
    printf("\n%s: %d failure(s)\n", failures ? "FAIL" : "PASS", failures);
    return failures ? 1 : 0;
}
