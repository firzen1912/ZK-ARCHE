#ifndef AUTH_LINEAGE_REPLACE_FRESHNESS_H
#define AUTH_LINEAGE_REPLACE_FRESHNESS_H

#include <stdbool.h>
#include <stdint.h>
#include "auth/lineage_replace_recovery.h"

typedef struct {
    bool anchor_available;
    bool anchor_integrity_valid;
    bool anchor_binding_valid;
    uint64_t record_generation;
    uint64_t trusted_high_water_generation;
} lineage_replace_freshness_facts_t;

typedef enum {
    LINEAGE_REPLACE_FRESHNESS_CURRENT = 0,
    LINEAGE_REPLACE_FRESHNESS_ANCHOR_UNAVAILABLE = 1,
    LINEAGE_REPLACE_FRESHNESS_ANCHOR_INVALID = 2,
    LINEAGE_REPLACE_FRESHNESS_BINDING_MISMATCH = 3,
    LINEAGE_REPLACE_FRESHNESS_ROLLBACK_DETECTED = 4,
    LINEAGE_REPLACE_FRESHNESS_GENERATION_AHEAD = 5
} lineage_replace_freshness_decision_t;

lineage_replace_freshness_decision_t lineage_replace_classify_freshness(
    const lineage_replace_freshness_facts_t *facts);
lineage_replace_state_t lineage_replace_recover_with_freshness(
    const lineage_replace_recovery_facts_t *recovery,
    const lineage_replace_freshness_facts_t *freshness);

#endif
