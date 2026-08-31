#include "auth/lineage_replace_reconciliation.h"

#include <stddef.h>

lineage_replace_reconciliation_decision_t lineage_replace_classify_reconciliation(
    const lineage_replace_reconciliation_facts_t *facts) {
    bool local_successor;
    bool peer_successor;

    if (facts == NULL || facts->local_state == LINEAGE_REPLACE_STATE_CONTINUITY_BROKEN ||
        facts->peer_state == LINEAGE_REPLACE_STATE_CONTINUITY_BROKEN)
        return LINEAGE_REPLACE_PAIR_CONTINUITY_BROKEN;

    local_successor =
        facts->local_state == LINEAGE_REPLACE_STATE_ACTIVE_SUCCESSOR_PREDECESSOR_RETIRED;
    peer_successor =
        facts->peer_state == LINEAGE_REPLACE_STATE_ACTIVE_SUCCESSOR_PREDECESSOR_RETIRED;

    if (local_successor && peer_successor) {
        if (!facts->same_successor) return LINEAGE_REPLACE_SUCCESSOR_DIVERGENCE;
        if (facts->local_attempt == LINEAGE_REPLACE_ATTEMPT_CONVERGED &&
            facts->peer_attempt == LINEAGE_REPLACE_ATTEMPT_CONVERGED)
            return LINEAGE_REPLACE_PAIR_SUCCESSOR_READY;
        return LINEAGE_REPLACE_RECONCILIATION_REQUIRED;
    }

    if (facts->local_state == LINEAGE_REPLACE_STATE_ACTIVE_PREDECESSOR &&
        facts->peer_state == LINEAGE_REPLACE_STATE_ACTIVE_PREDECESSOR &&
        facts->local_attempt != LINEAGE_REPLACE_ATTEMPT_CONVERGED &&
        facts->peer_attempt != LINEAGE_REPLACE_ATTEMPT_CONVERGED)
        return LINEAGE_REPLACE_PAIR_PREDECESSOR_READY;

    return LINEAGE_REPLACE_RECONCILIATION_REQUIRED;
}
