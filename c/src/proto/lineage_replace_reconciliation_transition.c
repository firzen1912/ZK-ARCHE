#include "auth/lineage_replace_reconciliation_transition.h"

#include <stddef.h>

lineage_replace_reconciliation_transition_t lineage_replace_classify_reconciliation_transition(
    const lineage_replace_reconciliation_transition_facts_t *facts) {
    if (facts == NULL ||
        facts->prior == LINEAGE_REPLACE_PAIR_CONTINUITY_BROKEN ||
        facts->current == LINEAGE_REPLACE_PAIR_CONTINUITY_BROKEN)
        return LINEAGE_REPLACE_RECONCILIATION_CONTINUITY_BROKEN;

    if (facts->current == LINEAGE_REPLACE_SUCCESSOR_DIVERGENCE)
        return LINEAGE_REPLACE_RECONCILIATION_REJECT_DIVERGENCE;

    if (facts->prior == LINEAGE_REPLACE_SUCCESSOR_DIVERGENCE && !facts->explicit_clean_retry)
        return LINEAGE_REPLACE_RECONCILIATION_REJECT_DIVERGENCE;

    if (facts->current == LINEAGE_REPLACE_PAIR_SUCCESSOR_READY) {
        if (facts->prior == LINEAGE_REPLACE_PAIR_SUCCESSOR_READY)
            return LINEAGE_REPLACE_RECONCILIATION_ACTIVATE_SUCCESSOR;
        if (facts->fresh_authenticated_attempt_evidence &&
            (facts->prior == LINEAGE_REPLACE_RECONCILIATION_REQUIRED ||
             (facts->prior == LINEAGE_REPLACE_SUCCESSOR_DIVERGENCE &&
              facts->explicit_clean_retry)))
            return LINEAGE_REPLACE_RECONCILIATION_ACTIVATE_SUCCESSOR;
        return LINEAGE_REPLACE_RECONCILIATION_HOLD;
    }

    if (facts->current == LINEAGE_REPLACE_PAIR_PREDECESSOR_READY) {
        if (facts->prior == LINEAGE_REPLACE_PAIR_PREDECESSOR_READY ||
            facts->prior == LINEAGE_REPLACE_RECONCILIATION_REQUIRED ||
            (facts->prior == LINEAGE_REPLACE_SUCCESSOR_DIVERGENCE &&
             facts->explicit_clean_retry))
            return LINEAGE_REPLACE_RECONCILIATION_RESUME_PREDECESSOR;
    }

    return LINEAGE_REPLACE_RECONCILIATION_HOLD;
}
