#ifndef AUTH_LINEAGE_REPLACE_ATTEMPT_H
#define AUTH_LINEAGE_REPLACE_ATTEMPT_H

#include <stdbool.h>

typedef struct {
    bool local_authorized;
    bool peer_authorized;
    bool same_attempt;
    bool same_predecessor_generation;
    bool same_successor;
    bool same_context;
    bool local_confirmation_bound;
    bool peer_confirmation_bound;
} lineage_replace_attempt_facts_t;

typedef enum {
    LINEAGE_REPLACE_ATTEMPT_CONVERGED = 0,
    LINEAGE_REPLACE_ATTEMPT_AWAITING_CONFIRMATION,
    LINEAGE_REPLACE_ATTEMPT_UNAUTHORIZED,
    LINEAGE_REPLACE_ATTEMPT_ID_MISMATCH,
    LINEAGE_REPLACE_ATTEMPT_PREDECESSOR_MISMATCH,
    LINEAGE_REPLACE_ATTEMPT_SUCCESSOR_MISMATCH,
    LINEAGE_REPLACE_ATTEMPT_CONTEXT_MISMATCH
} lineage_replace_attempt_decision_t;

lineage_replace_attempt_decision_t lineage_replace_classify_attempt(
    const lineage_replace_attempt_facts_t *facts);

#endif
