#include "auth/lineage_replace_convergence.h"
#include <stddef.h>

lineage_replace_convergence_decision_t lineage_replace_classify_convergence(
    const lineage_replace_convergence_facts_t *facts) {
    if (facts == NULL) return LINEAGE_REPLACE_UNAUTHORIZED;
    if (!facts->local_authorized || !facts->peer_authorized)
        return LINEAGE_REPLACE_UNAUTHORIZED;
    if (!facts->same_successor) return LINEAGE_REPLACE_SUCCESSOR_CONFLICT;
    if (facts->local_confirmed && facts->peer_confirmed) return LINEAGE_REPLACE_CONVERGED;
    return LINEAGE_REPLACE_AWAITING_CONFIRMATION;
}
