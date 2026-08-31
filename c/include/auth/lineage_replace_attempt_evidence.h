#ifndef AUTH_LINEAGE_REPLACE_ATTEMPT_EVIDENCE_H
#define AUTH_LINEAGE_REPLACE_ATTEMPT_EVIDENCE_H

#include <stdint.h>

typedef struct {
    uint32_t local_attempt_id;
    uint32_t peer_attempt_id;
    uint32_t local_confirmation_attempt_id;
    uint32_t peer_confirmation_attempt_id;
} lineage_replace_attempt_evidence_facts_t;

typedef enum {
    LINEAGE_REPLACE_ATTEMPT_EVIDENCE_FRESH_CURRENT_ATTEMPT = 0,
    LINEAGE_REPLACE_ATTEMPT_EVIDENCE_MISSING_CURRENT_ATTEMPT,
    LINEAGE_REPLACE_ATTEMPT_EVIDENCE_ATTEMPT_MISMATCH,
    LINEAGE_REPLACE_ATTEMPT_EVIDENCE_LOCAL_CONFIRMATION_MISSING,
    LINEAGE_REPLACE_ATTEMPT_EVIDENCE_PEER_CONFIRMATION_MISSING
} lineage_replace_attempt_evidence_decision_t;

lineage_replace_attempt_evidence_decision_t lineage_replace_classify_attempt_evidence(
    const lineage_replace_attempt_evidence_facts_t *facts);

#endif
