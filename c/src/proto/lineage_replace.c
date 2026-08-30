#include "auth/lineage_replace.h"

#include <stddef.h>

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
