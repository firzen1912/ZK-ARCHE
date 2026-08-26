#ifndef AUTH_SESSION_TABLE_H
#define AUTH_SESSION_TABLE_H

#include "auth_proto.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef AUTH_SESSION_TABLE_CAPACITY
#define AUTH_SESSION_TABLE_CAPACITY 64u
#endif

typedef enum auth_session_slot_state {
    AUTH_SESSION_SLOT_FREE = 0,
    AUTH_SESSION_SLOT_RESERVED = 1,
    AUTH_SESSION_SLOT_SETUP = 2,
    AUTH_SESSION_SLOT_AUTH = 3
} auth_session_slot_state_t;

typedef struct auth_session_slot {
    auth_session_slot_state_t state;
    union {
        auth_pending_setup_t setup;
        auth_pending_auth_t auth;
    } pending;
} auth_session_slot_t;

typedef struct auth_session_table {
    auth_session_slot_t slots[AUTH_SESSION_TABLE_CAPACITY];
} auth_session_table_t;

/*
 * This table is intentionally synchronization-agnostic. Callers that share a
 * table across threads MUST serialize reserve/activate/find/release operations.
 * Reservation is explicit so expensive authentication can happen after a slot
 * has been claimed without allowing another worker to claim the same slot.
 */
void auth_session_table_init(auth_session_table_t *table);

/* Reserve one free slot. Returns the slot index, or -1 when full. */
int auth_session_table_reserve(auth_session_table_t *table);

/* Promote a reserved slot to an active SETUP/AUTH session. */
auth_err_t auth_session_table_activate_setup(
    auth_session_table_t *table,
    int slot,
    const auth_pending_setup_t *pending);

auth_err_t auth_session_table_activate_auth(
    auth_session_table_t *table,
    int slot,
    const auth_pending_auth_t *pending);

/* Find only fully active sessions. Reserved slots are never searchable. */
int auth_session_table_find_setup(
    const auth_session_table_t *table,
    const uint8_t session_id[AUTH_SESSION_ID_LEN]);

int auth_session_table_find_auth(
    const auth_session_table_t *table,
    const uint8_t session_id[AUTH_SESSION_ID_LEN]);

/* Release any reserved or active slot back to FREE. */
auth_err_t auth_session_table_release(auth_session_table_t *table, int slot);

auth_session_slot_state_t auth_session_table_state(
    const auth_session_table_t *table,
    int slot);

#ifdef __cplusplus
}
#endif

#endif /* AUTH_SESSION_TABLE_H */
