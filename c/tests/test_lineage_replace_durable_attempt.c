#include "auth/lineage_replace_attempt.h"
#include "auth/lineage_replace_freshness.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VECTOR_PATH "../rust/test-vectors/replay/lineage-replace-durable-attempt-v1.txt"

static lineage_replace_attempt_decision_t attempt(const char *name) {
    lineage_replace_attempt_facts_t f = {true, true, true, true, true, true, true, true};
    if (strcmp(name, "AWAITING_CONFIRMATION") == 0) f.peer_confirmation_bound = false;
    else if (strcmp(name, "ATTEMPT_ID_MISMATCH") == 0) f.same_attempt = false;
    else if (strcmp(name, "PREDECESSOR_MISMATCH") == 0) f.same_predecessor_generation = false;
    else if (strcmp(name, "CONTEXT_MISMATCH") == 0) f.same_context = false;
    else assert(strcmp(name, "CONVERGED") == 0);
    return lineage_replace_classify_attempt(&f);
}

static lineage_replace_recovery_facts_t recovery(const char *name) {
    lineage_replace_recovery_facts_t r = {0};
    r.record_integrity_valid = true;
    if (strcmp(name, "PREDECESSOR") == 0) r.predecessor_active = true;
    else if (strcmp(name, "SUCCESSOR") == 0) { r.successor_active = true; r.predecessor_retired = true; r.invalidations_complete = true; }
    else { assert(strcmp(name, "PARTIAL") == 0); r.replacement_pending = true; r.successor_active = true; }
    return r;
}

int main(void) {
    FILE *fp = fopen(VECTOR_PATH, "r");
    char line[512];
    unsigned count = 0u;
    assert(fp != NULL);
    while (fgets(line, sizeof(line), fp) != NULL) {
        char *p[6] = {0}; char *cur; unsigned i = 0u; uint64_t record_generation; uint64_t high_water; lineage_replace_freshness_facts_t fresh; lineage_replace_recovery_facts_t recovery_facts; lineage_replace_state_t state; lineage_replace_attempt_decision_t decision; bool ok;
        line[strcspn(line, "\r\n")] = '\0';
        if (strncmp(line, "case=", 5u) != 0) continue;
        cur = strtok(line + 5u, "|"); while (cur != NULL && i < 6u) { p[i++] = cur; cur = strtok(NULL, "|"); }
        assert(i == 6u && cur == NULL);
        record_generation = (uint64_t)strtoull(p[2], NULL, 10); high_water = (uint64_t)strtoull(p[3], NULL, 10);
        fresh = (lineage_replace_freshness_facts_t){true, true, true, record_generation, high_water};
        recovery_facts = recovery(p[4]); decision = attempt(p[1]); state = lineage_replace_recover_with_freshness(&recovery_facts, &fresh);
        if (strcmp(p[5], "RESUME_SUCCESSOR") == 0) ok = decision == LINEAGE_REPLACE_ATTEMPT_CONVERGED && state == LINEAGE_REPLACE_STATE_ACTIVE_SUCCESSOR_PREDECESSOR_RETIRED;
        else if (strcmp(p[5], "RESUME_PREDECESSOR") == 0) ok = state == LINEAGE_REPLACE_STATE_ACTIVE_PREDECESSOR;
        else { assert(strcmp(p[5], "CONTINUITY_BROKEN") == 0); ok = decision != LINEAGE_REPLACE_ATTEMPT_CONVERGED || state == LINEAGE_REPLACE_STATE_CONTINUITY_BROKEN; }
        assert(ok); count += 1u;
    }
    fclose(fp); assert(count == 10u); puts("lineage-replace durable attempt corpus: ok"); return EXIT_SUCCESS;
}
