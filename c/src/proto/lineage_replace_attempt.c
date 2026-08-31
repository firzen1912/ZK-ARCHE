#include "auth/lineage_replace_attempt.h"

#include <stddef.h>

lineage_replace_attempt_decision_t lineage_replace_classify_attempt(
    const lineage_replace_attempt_facts_t *facts) {
    if (facts == NULL) return LINEAGE_REPLACE_ATTEMPT_UNAUTHORIZED;
    if (!facts->local_authorized || !facts->peer_authorized)
        return LINEAGE_REPLACE_ATTEMPT_UNAUTHORIZED;
    if (!facts->same_attempt) return LINEAGE_REPLACE_ATTEMPT_ID_MISMATCH;
    if (!facts->same_predecessor_generation)
        return LINEAGE_REPLACE_ATTEMPT_PREDECESSOR_MISMATCH;
    if (!facts->same_successor) return LINEAGE_REPLACE_ATTEMPT_SUCCESSOR_MISMATCH;
    if (!facts->same_context) return LINEAGE_REPLACE_ATTEMPT_CONTEXT_MISMATCH;
    if (!facts->local_confirmation_bound || !facts->peer_confirmation_bound)
        return LINEAGE_REPLACE_ATTEMPT_AWAITING_CONFIRMATION;
    return LINEAGE_REPLACE_ATTEMPT_CONVERGED;
}
