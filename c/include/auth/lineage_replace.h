#ifndef AUTH_LINEAGE_REPLACE_H
#define AUTH_LINEAGE_REPLACE_H

#include <stdbool.h>

typedef enum {
    LINEAGE_REPLACE_TRIGGER_LIFECYCLE = 0,
    LINEAGE_REPLACE_TRIGGER_RESTART,
    LINEAGE_REPLACE_TRIGGER_TRANSPORT_CHANGE,
    LINEAGE_REPLACE_TRIGGER_AUTH
} lineage_replace_trigger_t;

typedef enum {
    LINEAGE_REPLACE_ACCEPT_SUCCESSOR = 0,
    LINEAGE_REPLACE_REJECT_AUTHORITY,
    LINEAGE_REPLACE_REJECT_PREDECESSOR,
    LINEAGE_REPLACE_REJECT_SUCCESSOR,
    LINEAGE_REPLACE_REJECT_CONTEXT,
    LINEAGE_REPLACE_REJECT_FRESHNESS,
    LINEAGE_REPLACE_REJECT_REPLAY,
    LINEAGE_REPLACE_REJECT_CONCURRENT,
    LINEAGE_REPLACE_REJECT_ROLLBACK,
    LINEAGE_REPLACE_REJECT_STORAGE
} lineage_replace_decision_t;

typedef struct {
    lineage_replace_trigger_t trigger;
    bool authority_valid;
    bool predecessor_valid;
    bool successor_valid;
    bool context_valid;
    bool freshness_valid;
    bool replay_free;
    bool concurrent_free;
    bool rollback_clear;
    bool storage_safe;
    bool dependent_state_safe;
} lineage_replace_facts_t;

lineage_replace_decision_t lineage_replace_evaluate(const lineage_replace_facts_t *facts);

#endif
