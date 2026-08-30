#ifndef AUTH_LINEAGE_REPLACE_FAULTS_H
#define AUTH_LINEAGE_REPLACE_FAULTS_H

#include "auth/lineage_replace_recovery.h"

typedef enum {
    LINEAGE_REPLACE_CUT_BEFORE_BEGIN = 0,
    LINEAGE_REPLACE_CUT_AFTER_PENDING_MARKER,
    LINEAGE_REPLACE_CUT_AFTER_SUCCESSOR_ACTIVATION,
    LINEAGE_REPLACE_CUT_AFTER_PREDECESSOR_RETIREMENT,
    LINEAGE_REPLACE_CUT_AFTER_INVALIDATIONS,
    LINEAGE_REPLACE_CUT_AFTER_COMMIT_MARKER_CLEAR
} lineage_replace_write_cut_t;

lineage_replace_recovery_facts_t lineage_replace_facts_at_write_cut(lineage_replace_write_cut_t cut);

#endif
