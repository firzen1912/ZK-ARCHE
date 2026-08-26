#include "auth/session_table.h"

#include <string.h>

static int valid_slot(int slot)
{
    return slot >= 0 && (size_t)slot < AUTH_SESSION_TABLE_CAPACITY;
}

void auth_session_table_init(auth_session_table_t *table)
{
    if (!table) return;
    memset(table, 0, sizeof *table);
}

int auth_session_table_reserve(auth_session_table_t *table)
{
    if (!table) return -1;
    for (size_t i = 0; i < AUTH_SESSION_TABLE_CAPACITY; ++i) {
        if (table->slots[i].state == AUTH_SESSION_SLOT_FREE) {
            memset(&table->slots[i], 0, sizeof table->slots[i]);
            table->slots[i].state = AUTH_SESSION_SLOT_RESERVED;
            return (int)i;
        }
    }
    return -1;
}

auth_err_t auth_session_table_activate_setup(
    auth_session_table_t *table,
    int slot,
    const auth_pending_setup_t *pending)
{
    if (!table || !pending || !valid_slot(slot)) return AUTH_ERR_INVALID_ARGUMENT;
    auth_session_slot_t *entry = &table->slots[(size_t)slot];
    if (entry->state != AUTH_SESSION_SLOT_RESERVED) return AUTH_ERR_INVALID_ARGUMENT;
    entry->pending.setup = *pending;
    entry->state = AUTH_SESSION_SLOT_SETUP;
    return AUTH_OK;
}

auth_err_t auth_session_table_activate_auth(
    auth_session_table_t *table,
    int slot,
    const auth_pending_auth_t *pending)
{
    if (!table || !pending || !valid_slot(slot)) return AUTH_ERR_INVALID_ARGUMENT;
    auth_session_slot_t *entry = &table->slots[(size_t)slot];
    if (entry->state != AUTH_SESSION_SLOT_RESERVED) return AUTH_ERR_INVALID_ARGUMENT;
    entry->pending.auth = *pending;
    entry->state = AUTH_SESSION_SLOT_AUTH;
    return AUTH_OK;
}

int auth_session_table_find_setup(
    const auth_session_table_t *table,
    const uint8_t session_id[AUTH_SESSION_ID_LEN])
{
    if (!table || !session_id) return -1;
    for (size_t i = 0; i < AUTH_SESSION_TABLE_CAPACITY; ++i) {
        const auth_session_slot_t *entry = &table->slots[i];
        if (entry->state == AUTH_SESSION_SLOT_SETUP &&
            memcmp(entry->pending.setup.session_id, session_id,
                   AUTH_SESSION_ID_LEN) == 0) {
            return (int)i;
        }
    }
    return -1;
}

int auth_session_table_find_auth(
    const auth_session_table_t *table,
    const uint8_t session_id[AUTH_SESSION_ID_LEN])
{
    if (!table || !session_id) return -1;
    for (size_t i = 0; i < AUTH_SESSION_TABLE_CAPACITY; ++i) {
        const auth_session_slot_t *entry = &table->slots[i];
        if (entry->state == AUTH_SESSION_SLOT_AUTH &&
            memcmp(entry->pending.auth.session_id, session_id,
                   AUTH_SESSION_ID_LEN) == 0) {
            return (int)i;
        }
    }
    return -1;
}

auth_err_t auth_session_table_release(auth_session_table_t *table, int slot)
{
    if (!table || !valid_slot(slot)) return AUTH_ERR_INVALID_ARGUMENT;
    memset(&table->slots[(size_t)slot], 0, sizeof table->slots[(size_t)slot]);
    return AUTH_OK;
}

auth_session_slot_state_t auth_session_table_state(
    const auth_session_table_t *table,
    int slot)
{
    if (!table || !valid_slot(slot)) return AUTH_SESSION_SLOT_FREE;
    return table->slots[(size_t)slot].state;
}
