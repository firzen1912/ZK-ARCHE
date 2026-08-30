#include "auth/lineage_replace_recovery.h"

#include <stddef.h>

lineage_replace_state_t lineage_replace_recover(const lineage_replace_recovery_facts_t *facts) {
    if (facts == NULL || !facts->record_integrity_valid)
        return LINEAGE_REPLACE_STATE_CONTINUITY_BROKEN;

    if (facts->predecessor_active && !facts->replacement_pending && !facts->successor_active &&
        !facts->predecessor_retired && !facts->invalidations_complete)
        return LINEAGE_REPLACE_STATE_ACTIVE_PREDECESSOR;

    if (!facts->predecessor_active && !facts->replacement_pending && facts->successor_active &&
        facts->predecessor_retired && facts->invalidations_complete)
        return LINEAGE_REPLACE_STATE_ACTIVE_SUCCESSOR_PREDECESSOR_RETIRED;

    return LINEAGE_REPLACE_STATE_CONTINUITY_BROKEN;
}
