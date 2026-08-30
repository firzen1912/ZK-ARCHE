#include "auth/lineage_replace.h"

#include <stddef.h>

static bool lineage_replace_plan_complete(const lineage_replace_plan_t *plan) {
    return plan != NULL && plan->retire_predecessor && plan->activate_successor &&
           plan->invalidate_session_keys && plan->invalidate_resumption &&
           plan->invalidate_authorization_cache && plan->invalidate_attribution_cache &&
           plan->invalidate_channel_binding && plan->invalidate_replay_state;
}

lineage_replace_decision_t lineage_replace_evaluate(const lineage_replace_facts_t *facts) {
    if (facts == NULL) return LINEAGE_REPLACE_REJECT_STORAGE;
    if (facts->trigger != LINEAGE_REPLACE_TRIGGER_LIFECYCLE || !facts->authority_valid)
        return LINEAGE_REPLACE_REJECT_AUTHORITY;
    if (!facts->predecessor_valid) return LINEAGE_REPLACE_REJECT_PREDECESSOR;
    if (!facts->successor_valid) return LINEAGE_REPLACE_REJECT_SUCCESSOR;
    if (!facts->context_valid || !facts->dependent_state_safe) return LINEAGE_REPLACE_REJECT_CONTEXT;
    if (!facts->freshness_valid) return LINEAGE_REPLACE_REJECT_FRESHNESS;
    if (!facts->replay_free) return LINEAGE_REPLACE_REJECT_REPLAY;
    if (!facts->concurrent_free) return LINEAGE_REPLACE_REJECT_CONCURRENT;
    if (!facts->rollback_clear) return LINEAGE_REPLACE_REJECT_ROLLBACK;
    if (!facts->storage_safe) return LINEAGE_REPLACE_REJECT_STORAGE;
    return LINEAGE_REPLACE_ACCEPT_SUCCESSOR;
}

bool lineage_replace_plan(lineage_replace_decision_t decision, lineage_replace_plan_t *out_plan) {
    if (out_plan == NULL) return false;

    *out_plan = (lineage_replace_plan_t){false, false, false, false, false, false, false, false};
    if (decision != LINEAGE_REPLACE_ACCEPT_SUCCESSOR) return false;

    *out_plan = (lineage_replace_plan_t){true, true, true, true, true, true, true, true};
    return true;
}

bool lineage_replace_advance(lineage_replace_state_t *state, lineage_replace_event_t event,
                             const lineage_replace_plan_t *plan) {
    if (state == NULL) return false;

    if (*state == LINEAGE_REPLACE_STATE_ACTIVE_PREDECESSOR &&
        event == LINEAGE_REPLACE_EVENT_BEGIN && lineage_replace_plan_complete(plan)) {
        *state = LINEAGE_REPLACE_STATE_REPLACEMENT_PENDING;
        return true;
    }

    if (*state == LINEAGE_REPLACE_STATE_REPLACEMENT_PENDING &&
        event == LINEAGE_REPLACE_EVENT_COMMIT && lineage_replace_plan_complete(plan)) {
        *state = LINEAGE_REPLACE_STATE_ACTIVE_SUCCESSOR_PREDECESSOR_RETIRED;
        return true;
    }

    if (*state == LINEAGE_REPLACE_STATE_REPLACEMENT_PENDING &&
        event == LINEAGE_REPLACE_EVENT_INTERRUPT) {
        *state = LINEAGE_REPLACE_STATE_CONTINUITY_BROKEN;
        return true;
    }

    return false;
}
