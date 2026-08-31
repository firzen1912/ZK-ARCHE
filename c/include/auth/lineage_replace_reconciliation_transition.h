#ifndef AUTH_LINEAGE_REPLACE_RECONCILIATION_TRANSITION_H
#define AUTH_LINEAGE_REPLACE_RECONCILIATION_TRANSITION_H

#include <stdbool.h>
#include "auth/lineage_replace_attempt_evidence.h"
#include "auth/lineage_replace_reconciliation.h"

typedef struct {
    lineage_replace_reconciliation_decision_t prior;
    lineage_replace_reconciliation_decision_t current;
    lineage_replace_attempt_evidence_decision_t attempt_evidence;
    bool explicit_clean_retry;
} lineage_replace_reconciliation_transition_facts_t;

typedef enum {
    LINEAGE_REPLACE_RECONCILIATION_HOLD = 0,
    LINEAGE_REPLACE_RECONCILIATION_ACTIVATE_SUCCESSOR,
    LINEAGE_REPLACE_RECONCILIATION_RESUME_PREDECESSOR,
    LINEAGE_REPLACE_RECONCILIATION_REJECT_DIVERGENCE,
    LINEAGE_REPLACE_RECONCILIATION_CONTINUITY_BROKEN
} lineage_replace_reconciliation_transition_t;

lineage_replace_reconciliation_transition_t lineage_replace_classify_reconciliation_transition(
    const lineage_replace_reconciliation_transition_facts_t *facts);

#endif
