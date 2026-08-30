#ifndef AUTH_LINEAGE_REPLACE_RECOVERY_H
#define AUTH_LINEAGE_REPLACE_RECOVERY_H

#include <stdbool.h>
#include "auth/lineage_replace.h"

typedef struct {
    bool record_integrity_valid;
    bool predecessor_active;
    bool replacement_pending;
    bool successor_active;
    bool predecessor_retired;
    bool invalidations_complete;
} lineage_replace_recovery_facts_t;

lineage_replace_state_t lineage_replace_recover(const lineage_replace_recovery_facts_t *facts);

#endif
