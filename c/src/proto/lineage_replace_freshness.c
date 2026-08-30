#include "auth/lineage_replace_freshness.h"

#include <stddef.h>

lineage_replace_freshness_decision_t lineage_replace_classify_freshness(
    const lineage_replace_freshness_facts_t *facts) {
    if (facts == NULL || !facts->anchor_integrity_valid)
        return LINEAGE_REPLACE_FRESHNESS_ANCHOR_INVALID;
    if (!facts->anchor_available)
        return LINEAGE_REPLACE_FRESHNESS_ANCHOR_UNAVAILABLE;
    if (!facts->anchor_binding_valid)
        return LINEAGE_REPLACE_FRESHNESS_BINDING_MISMATCH;
    if (facts->record_generation < facts->trusted_high_water_generation)
        return LINEAGE_REPLACE_FRESHNESS_ROLLBACK_DETECTED;
    if (facts->record_generation > facts->trusted_high_water_generation)
        return LINEAGE_REPLACE_FRESHNESS_GENERATION_AHEAD;
    return LINEAGE_REPLACE_FRESHNESS_CURRENT;
}

lineage_replace_state_t lineage_replace_recover_with_freshness(
    const lineage_replace_recovery_facts_t *recovery,
    const lineage_replace_freshness_facts_t *freshness) {
    if (lineage_replace_classify_freshness(freshness) != LINEAGE_REPLACE_FRESHNESS_CURRENT)
        return LINEAGE_REPLACE_STATE_CONTINUITY_BROKEN;
    return lineage_replace_recover(recovery);
}
