#include "auth/lineage_replace_storage_transaction.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VECTOR_PATH "../rust/test-vectors/replay/lineage-replace-storage-transaction-v1.txt"

typedef struct {
    lineage_replace_storage_step_t fail_step;
    bool has_fail_step;
    char trace[192];
} test_adapter_t;

static bool bit(const char *v) { if (strcmp(v, "1") == 0) return true; assert(strcmp(v, "0") == 0); return false; }
static lineage_replace_storage_step_t step(const char *v) {
    if (strcmp(v, "PERSIST_PENDING") == 0) return LINEAGE_REPLACE_STORAGE_STEP_PERSIST_PENDING;
    if (strcmp(v, "ACTIVATE_SUCCESSOR") == 0) return LINEAGE_REPLACE_STORAGE_STEP_ACTIVATE_SUCCESSOR;
    if (strcmp(v, "RETIRE_PREDECESSOR") == 0) return LINEAGE_REPLACE_STORAGE_STEP_RETIRE_PREDECESSOR;
    if (strcmp(v, "INVALIDATE_DEPENDENT_STATE") == 0) return LINEAGE_REPLACE_STORAGE_STEP_INVALIDATE_DEPENDENT_STATE;
    assert(strcmp(v, "CLEAR_PENDING") == 0); return LINEAGE_REPLACE_STORAGE_STEP_CLEAR_PENDING;
}
static const char *step_name(lineage_replace_storage_step_t v) {
    switch (v) {
        case LINEAGE_REPLACE_STORAGE_STEP_PERSIST_PENDING: return "PERSIST_PENDING";
        case LINEAGE_REPLACE_STORAGE_STEP_ACTIVATE_SUCCESSOR: return "ACTIVATE_SUCCESSOR";
        case LINEAGE_REPLACE_STORAGE_STEP_RETIRE_PREDECESSOR: return "RETIRE_PREDECESSOR";
        case LINEAGE_REPLACE_STORAGE_STEP_INVALIDATE_DEPENDENT_STATE: return "INVALIDATE_DEPENDENT_STATE";
        case LINEAGE_REPLACE_STORAGE_STEP_CLEAR_PENDING: return "CLEAR_PENDING";
    }
    assert(false); return "";
}
static lineage_replace_storage_transaction_result_t expected(const char *v) {
    if (strcmp(v, "COMMITTED") == 0) return LINEAGE_REPLACE_STORAGE_TRANSACTION_COMMITTED;
    if (strcmp(v, "REJECT_PLAN") == 0) return LINEAGE_REPLACE_STORAGE_TRANSACTION_REJECT_PLAN;
    if (strcmp(v, "FAIL_PENDING") == 0) return LINEAGE_REPLACE_STORAGE_TRANSACTION_FAIL_PENDING;
    if (strcmp(v, "FAIL_SUCCESSOR") == 0) return LINEAGE_REPLACE_STORAGE_TRANSACTION_FAIL_SUCCESSOR;
    if (strcmp(v, "FAIL_PREDECESSOR") == 0) return LINEAGE_REPLACE_STORAGE_TRANSACTION_FAIL_PREDECESSOR;
    if (strcmp(v, "FAIL_INVALIDATIONS") == 0) return LINEAGE_REPLACE_STORAGE_TRANSACTION_FAIL_INVALIDATIONS;
    assert(strcmp(v, "FAIL_CLEAR") == 0); return LINEAGE_REPLACE_STORAGE_TRANSACTION_FAIL_CLEAR;
}
static bool apply_step(void *context, lineage_replace_storage_step_t current) {
    test_adapter_t *adapter = (test_adapter_t *)context;
    const char *name = step_name(current);
    size_t used = strlen(adapter->trace);
    if (used != 0u) { assert(used + 1u < sizeof(adapter->trace)); adapter->trace[used++] = ','; adapter->trace[used] = '\0'; }
    assert(used + strlen(name) < sizeof(adapter->trace)); strcat(adapter->trace, name);
    return !(adapter->has_fail_step && adapter->fail_step == current);
}
static lineage_replace_plan_t canonical_plan(void) {
    lineage_replace_plan_t plan = {0};
    plan.retire_predecessor = true; plan.activate_successor = true;
    plan.invalidate_session_keys = true; plan.invalidate_resumption = true;
    plan.invalidate_authorization_cache = true; plan.invalidate_attribution_cache = true;
    plan.invalidate_channel_binding = true; plan.invalidate_replay_state = true;
    return plan;
}

int main(void) {
    FILE *fp = fopen(VECTOR_PATH, "r"); char line[512]; unsigned cases = 0u; int version = 0; assert(fp != NULL);
    while (fgets(line, sizeof(line), fp) != NULL) {
        char *f[5] = {0}; char *p; unsigned i = 0u; lineage_replace_plan_t plan; test_adapter_t adapter = {0};
        line[strcspn(line, "\r\n")] = '\0'; if (strcmp(line, "version=1") == 0) { version = 1; continue; } if (strncmp(line, "case=", 5u) != 0) continue;
        p = strtok(line + 5u, "|"); while (p != NULL && i < 5u) { f[i++] = p; p = strtok(NULL, "|"); } assert(i == 5u && p == NULL);
        plan = canonical_plan(); if (!bit(f[1])) plan.invalidate_replay_state = false;
        if (strcmp(f[2], "NONE") != 0) { adapter.has_fail_step = true; adapter.fail_step = step(f[2]); }
        assert(lineage_replace_execute_storage_transaction(&plan, apply_step, &adapter) == expected(f[3]));
        assert(strcmp(adapter.trace[0] == '\0' ? "NONE" : adapter.trace, f[4]) == 0); cases += 1u;
    }
    fclose(fp); assert(version == 1 && cases == 7u); puts("lineage-replace storage-transaction corpus: ok"); return EXIT_SUCCESS;
}
