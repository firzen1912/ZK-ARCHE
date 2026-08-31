#include "auth/lineage_replace_storage_transaction.h"

static bool plan_complete(const lineage_replace_plan_t *plan) {
    return plan != NULL &&
           plan->retire_predecessor &&
           plan->activate_successor &&
           plan->invalidate_session_keys &&
           plan->invalidate_resumption &&
           plan->invalidate_authorization_cache &&
           plan->invalidate_attribution_cache &&
           plan->invalidate_channel_binding &&
           plan->invalidate_replay_state;
}

lineage_replace_storage_transaction_result_t lineage_replace_execute_storage_transaction(
    const lineage_replace_plan_t *plan,
    lineage_replace_storage_apply_fn apply,
    void *context) {
    if (!plan_complete(plan) || apply == NULL) {
        return LINEAGE_REPLACE_STORAGE_TRANSACTION_REJECT_PLAN;
    }
    if (!apply(context, LINEAGE_REPLACE_STORAGE_STEP_PERSIST_PENDING)) {
        return LINEAGE_REPLACE_STORAGE_TRANSACTION_FAIL_PENDING;
    }
    if (!apply(context, LINEAGE_REPLACE_STORAGE_STEP_ACTIVATE_SUCCESSOR)) {
        return LINEAGE_REPLACE_STORAGE_TRANSACTION_FAIL_SUCCESSOR;
    }
    if (!apply(context, LINEAGE_REPLACE_STORAGE_STEP_RETIRE_PREDECESSOR)) {
        return LINEAGE_REPLACE_STORAGE_TRANSACTION_FAIL_PREDECESSOR;
    }
    if (!apply(context, LINEAGE_REPLACE_STORAGE_STEP_INVALIDATE_DEPENDENT_STATE)) {
        return LINEAGE_REPLACE_STORAGE_TRANSACTION_FAIL_INVALIDATIONS;
    }
    if (!apply(context, LINEAGE_REPLACE_STORAGE_STEP_CLEAR_PENDING)) {
        return LINEAGE_REPLACE_STORAGE_TRANSACTION_FAIL_CLEAR;
    }
    return LINEAGE_REPLACE_STORAGE_TRANSACTION_COMMITTED;
}
