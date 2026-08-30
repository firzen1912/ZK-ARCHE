#ifndef AUTH_LINEAGE_REPLACE_CONVERGENCE_H
#define AUTH_LINEAGE_REPLACE_CONVERGENCE_H

#include <stdbool.h>

typedef struct {
    bool local_authorized;
    bool peer_authorized;
    bool same_successor;
    bool local_confirmed;
    bool peer_confirmed;
} lineage_replace_convergence_facts_t;

typedef enum {
    LINEAGE_REPLACE_CONVERGED = 0,
    LINEAGE_REPLACE_AWAITING_CONFIRMATION,
    LINEAGE_REPLACE_UNAUTHORIZED,
    LINEAGE_REPLACE_SUCCESSOR_CONFLICT
} lineage_replace_convergence_decision_t;

lineage_replace_convergence_decision_t lineage_replace_classify_convergence(
    const lineage_replace_convergence_facts_t *facts);

#endif
