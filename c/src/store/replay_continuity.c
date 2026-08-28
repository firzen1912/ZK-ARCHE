#include "auth/replay_continuity.h"

void auth_replay_continuity_init(auth_replay_continuity_t *continuity,
                                 auth_replay_continuity_state_t state)
{
    if (continuity == 0) return;
    continuity->state = state;
}

int auth_replay_continuity_apply(auth_replay_continuity_t *continuity,
                                 auth_replay_continuity_event_t event)
{
    if (continuity == 0) return -1;

    switch (continuity->state) {
    case AUTH_REPLAY_CONTINUITY_TRUSTED:
        if (event == AUTH_REPLAY_EVENT_RESTART) {
            continuity->state = AUTH_REPLAY_CONTINUITY_RESTORING;
            return 0;
        }
        if (event == AUTH_REPLAY_EVENT_FAILED_AUTH) return 0;
        return -1;

    case AUTH_REPLAY_CONTINUITY_RESTORING:
        if (event == AUTH_REPLAY_EVENT_RESTORED_TRUSTED_WINDOW) {
            continuity->state = AUTH_REPLAY_CONTINUITY_TRUSTED;
            return 0;
        }
        if (event == AUTH_REPLAY_EVENT_RESTORE_MISSING ||
            event == AUTH_REPLAY_EVENT_RESTORE_CORRUPT ||
            event == AUTH_REPLAY_EVENT_RESTORE_STALE ||
            event == AUTH_REPLAY_EVENT_ROLLBACK_SUSPECTED) {
            continuity->state = AUTH_REPLAY_CONTINUITY_BROKEN;
            return 0;
        }
        if (event == AUTH_REPLAY_EVENT_FAILED_AUTH) return 0;
        return -1;

    case AUTH_REPLAY_CONTINUITY_BROKEN:
        if (event == AUTH_REPLAY_EVENT_EMPTY_CACHE_RESET ||
            event == AUTH_REPLAY_EVENT_FRESH_OUTER_SESSION ||
            event == AUTH_REPLAY_EVENT_FAILED_AUTH) {
            return 0;
        }
        return -1;

    default:
        return -1;
    }
}

int auth_replay_continuity_auth_allowed(const auth_replay_continuity_t *continuity)
{
    if (continuity == 0) return 0;
    return continuity->state == AUTH_REPLAY_CONTINUITY_TRUSTED ? 1 : 0;
}
