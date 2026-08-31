#ifndef AUTH_LINEAGE_REPLACE_STORAGE_TRANSACTION_H
#define AUTH_LINEAGE_REPLACE_STORAGE_TRANSACTION_H

#include <stdbool.h>
#include "auth/lineage_replace.h"

typedef enum {
    LINEAGE_REPLACE_STORAGE_STEP_PERSIST_PENDING = 0,
    LINEAGE_REPLACE_STORAGE_STEP_ACTIVATE_SUCCESSOR,
    LINEAGE_REPLACE_STORAGE_STEP_RETIRE_PREDECESSOR,
    LINEAGE_REPLACE_STORAGE_STEP_INVALIDATE_DEPENDENT_STATE,
    LINEAGE_REPLACE_STORAGE_STEP_CLEAR_PENDING
} lineage_replace_storage_step_t;

typedef enum {
    LINEAGE_REPLACE_STORAGE_TRANSACTION_COMMITTED = 0,
    LINEAGE_REPLACE_STORAGE_TRANSACTION_REJECT_PLAN,
    LINEAGE_REPLACE_STORAGE_TRANSACTION_FAIL_PENDING,
    LINEAGE_REPLACE_STORAGE_TRANSACTION_FAIL_SUCCESSOR,
    LINEAGE_REPLACE_STORAGE_TRANSACTION_FAIL_PREDECESSOR,
    LINEAGE_REPLACE_STORAGE_TRANSACTION_FAIL_INVALIDATIONS,
    LINEAGE_REPLACE_STORAGE_TRANSACTION_FAIL_CLEAR
} lineage_replace_storage_transaction_result_t;

typedef bool (*lineage_replace_storage_apply_fn)(void *context,
                                                  lineage_replace_storage_step_t step);

/* Executes the storage-neutral logical transaction order. A callback returning
 * true asserts only that the adapter made that logical step durable. After the
 * pending marker succeeds, any later failure is fail-closed: this function
 * never compensates, clears pending, or infers completion. */
lineage_replace_storage_transaction_result_t lineage_replace_execute_storage_transaction(
    const lineage_replace_plan_t *plan,
    lineage_replace_storage_apply_fn apply,
    void *context);

#endif
