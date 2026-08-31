#include "auth/lineage_replace_attempt_evidence.h"

#include <stddef.h>

lineage_replace_attempt_evidence_decision_t lineage_replace_classify_attempt_evidence(
    const lineage_replace_attempt_evidence_facts_t *facts) {
    if (facts == NULL || facts->local_attempt_id == 0u || facts->peer_attempt_id == 0u)
        return LINEAGE_REPLACE_ATTEMPT_EVIDENCE_MISSING_CURRENT_ATTEMPT;
    if (facts->local_attempt_id != facts->peer_attempt_id)
        return LINEAGE_REPLACE_ATTEMPT_EVIDENCE_ATTEMPT_MISMATCH;
    if (facts->local_confirmation_attempt_id != facts->local_attempt_id)
        return LINEAGE_REPLACE_ATTEMPT_EVIDENCE_LOCAL_CONFIRMATION_MISSING;
    if (facts->peer_confirmation_attempt_id != facts->peer_attempt_id)
        return LINEAGE_REPLACE_ATTEMPT_EVIDENCE_PEER_CONFIRMATION_MISSING;
    return LINEAGE_REPLACE_ATTEMPT_EVIDENCE_FRESH_CURRENT_ATTEMPT;
}
