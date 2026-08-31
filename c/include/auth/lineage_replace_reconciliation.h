#ifndef AUTH_LINEAGE_REPLACE_RECONCILIATION_H
#define AUTH_LINEAGE_REPLACE_RECONCILIATION_H

#include <stdbool.h>
#include "auth/lineage_replace.h"
#include "auth/lineage_replace_attempt.h"

typedef struct {
    lineage_replace_attempt_decision_t local_attempt;
    lineage_replace_state_t local_state;
    lineage_replace_attempt_decision_t peer_attempt;
    lineage_replace_state_t peer_state;
    bool same_successor;
} lineage_replace_reconciliation_facts_t;

typedef enum {
    LINEAGE_REPLACE_PAIR_SUCCESSOR_READY = 0,
    LINEAGE_REPLACE_PAIR_PREDECESSOR_READY,
    LINEAGE_REPLACE_RECONCILIATION_REQUIRED,
    LINEAGE_REPLACE_PAIR_CONTINUITY_BROKEN,
    LINEAGE_REPLACE_SUCCESSOR_DIVERGENCE
} lineage_replace_reconciliation_decision_t;

lineage_replace_reconciliation_decision_t lineage_replace_classify_reconciliation(
    const lineage_replace_reconciliation_facts_t *facts);

#endif
