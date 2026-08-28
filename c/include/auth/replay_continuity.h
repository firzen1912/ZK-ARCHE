#ifndef AUTH_REPLAY_CONTINUITY_H
#define AUTH_REPLAY_CONTINUITY_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum auth_replay_continuity_state {
    AUTH_REPLAY_CONTINUITY_TRUSTED = 0,
    AUTH_REPLAY_CONTINUITY_RESTORING = 1,
    AUTH_REPLAY_CONTINUITY_BROKEN = 2
} auth_replay_continuity_state_t;

typedef enum auth_replay_continuity_event {
    AUTH_REPLAY_EVENT_RESTART = 0,
    AUTH_REPLAY_EVENT_RESTORED_TRUSTED_WINDOW = 1,
    AUTH_REPLAY_EVENT_RESTORE_MISSING = 2,
    AUTH_REPLAY_EVENT_RESTORE_CORRUPT = 3,
    AUTH_REPLAY_EVENT_RESTORE_STALE = 4,
    AUTH_REPLAY_EVENT_ROLLBACK_SUSPECTED = 5,
    AUTH_REPLAY_EVENT_EMPTY_CACHE_RESET = 6,
    AUTH_REPLAY_EVENT_FRESH_OUTER_SESSION = 7,
    AUTH_REPLAY_EVENT_FAILED_AUTH = 8
} auth_replay_continuity_event_t;

typedef struct auth_replay_continuity {
    auth_replay_continuity_state_t state;
} auth_replay_continuity_t;

void auth_replay_continuity_init(auth_replay_continuity_t *continuity,
                                 auth_replay_continuity_state_t state);

int auth_replay_continuity_apply(auth_replay_continuity_t *continuity,
                                 auth_replay_continuity_event_t event);

int auth_replay_continuity_auth_allowed(const auth_replay_continuity_t *continuity);

#ifdef __cplusplus
}
#endif

#endif
