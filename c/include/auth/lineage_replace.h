#ifndef AUTH_LINEAGE_REPLACE_H
#define AUTH_LINEAGE_REPLACE_H

#include <stdbool.h>
#include "auth/lineage_replace_authorization.h"

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

typedef struct {
    bool retire_predecessor;
    bool activate_successor;
    bool invalidate_session_keys;
    bool invalidate_resumption;
    bool invalidate_authorization_cache;
    bool invalidate_attribution_cache;
    bool invalidate_channel_binding;
    bool invalidate_replay_state;
} lineage_replace_plan_t;

typedef enum {
    LINEAGE_REPLACE_STATE_ACTIVE_PREDECESSOR = 0,
    LINEAGE_REPLACE_STATE_REPLACEMENT_PENDING,
    LINEAGE_REPLACE_STATE_ACTIVE_SUCCESSOR_PREDECESSOR_RETIRED,
    LINEAGE_REPLACE_STATE_CONTINUITY_BROKEN
} lineage_replace_state_t;

typedef enum {
    LINEAGE_REPLACE_EVENT_BEGIN = 0,
    LINEAGE_REPLACE_EVENT_COMMIT,
    LINEAGE_REPLACE_EVENT_INTERRUPT
} lineage_replace_event_t;

/* Normalized predicate retained for decision-corpus compatibility. Lifecycle
 * request handlers MUST derive authority through lineage_replace_classify_authorization()
 * and enter through lineage_replace_evaluate_authorized(). */
lineage_replace_decision_t lineage_replace_evaluate(const lineage_replace_facts_t *facts);
lineage_replace_decision_t lineage_replace_evaluate_authorized(
    lineage_replace_authorization_decision_t authorization,
    const lineage_replace_facts_t *facts);
bool lineage_replace_plan(lineage_replace_decision_t decision, lineage_replace_plan_t *out_plan);
bool lineage_replace_advance(lineage_replace_state_t *state, lineage_replace_event_t event,
                             const lineage_replace_plan_t *plan);

#endif
