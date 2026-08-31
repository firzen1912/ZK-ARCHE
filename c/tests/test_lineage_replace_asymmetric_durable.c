#include "auth/lineage_replace_attempt.h"
#include "auth/lineage_replace_freshness.h"
#include "auth/lineage_replace_reconciliation.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VECTOR_PATH "../rust/test-vectors/replay/lineage-replace-asymmetric-durable-v1.txt"

static lineage_replace_attempt_decision_t attempt(const char *name) {
    lineage_replace_attempt_facts_t facts = {true, true, true, true, true, true, true, true};
    if (strcmp(name, "AWAITING_CONFIRMATION") == 0) facts.peer_confirmation_bound = false;
    else if (strcmp(name, "ATTEMPT_ID_MISMATCH") == 0) facts.same_attempt = false;
    else assert(strcmp(name, "CONVERGED") == 0);
    return lineage_replace_classify_attempt(&facts);
}

static lineage_replace_recovery_facts_t recovery(const char *name) {
    lineage_replace_recovery_facts_t facts = {0};
    facts.record_integrity_valid = true;
    if (strcmp(name, "PREDECESSOR") == 0) facts.predecessor_active = true;
    else if (strcmp(name, "SUCCESSOR") == 0) {
        facts.successor_active = true;
        facts.predecessor_retired = true;
        facts.invalidations_complete = true;
    } else {
        assert(strcmp(name, "PARTIAL") == 0);
        facts.replacement_pending = true;
        facts.successor_active = true;
    }
    return facts;
}

static lineage_replace_state_t durable_state(const char *kind, uint64_t generation, uint64_t high_water) {
    lineage_replace_recovery_facts_t recovery_facts = recovery(kind);
    lineage_replace_freshness_facts_t freshness = {true, true, true, generation, high_water};
    return lineage_replace_recover_with_freshness(&recovery_facts, &freshness);
}

static lineage_replace_reconciliation_decision_t expected(const char *name) {
    if (strcmp(name, "PAIR_SUCCESSOR_READY") == 0) return LINEAGE_REPLACE_PAIR_SUCCESSOR_READY;
    if (strcmp(name, "PAIR_PREDECESSOR_READY") == 0) return LINEAGE_REPLACE_PAIR_PREDECESSOR_READY;
    if (strcmp(name, "RECONCILIATION_REQUIRED") == 0)
        return LINEAGE_REPLACE_RECONCILIATION_REQUIRED;
    if (strcmp(name, "CONTINUITY_BROKEN") == 0) return LINEAGE_REPLACE_PAIR_CONTINUITY_BROKEN;
    assert(strcmp(name, "SUCCESSOR_DIVERGENCE") == 0);
    return LINEAGE_REPLACE_SUCCESSOR_DIVERGENCE;
}

int main(void) {
    FILE *fp = fopen(VECTOR_PATH, "r");
    char line[768];
    unsigned count = 0u;
    int saw_version = 0;

    assert(fp != NULL);
    while (fgets(line, sizeof(line), fp) != NULL) {
        char *p[11] = {0};
        char *cur;
        unsigned i = 0u;
        uint64_t local_generation;
        uint64_t local_high_water;
        uint64_t peer_generation;
        uint64_t peer_high_water;
        lineage_replace_reconciliation_facts_t facts;

        line[strcspn(line, "\r\n")] = '\0';
        if (strcmp(line, "version=1") == 0) {
            saw_version = 1;
            continue;
        }
        if (strncmp(line, "case=", 5u) != 0) continue;

        cur = strtok(line + 5u, "|");
        while (cur != NULL && i < 11u) {
            p[i++] = cur;
            cur = strtok(NULL, "|");
        }
        assert(i == 11u && cur == NULL);

        local_generation = (uint64_t)strtoull(p[2], NULL, 10);
        local_high_water = (uint64_t)strtoull(p[3], NULL, 10);
        peer_generation = (uint64_t)strtoull(p[6], NULL, 10);
        peer_high_water = (uint64_t)strtoull(p[7], NULL, 10);
        assert(strcmp(p[9], "0") == 0 || strcmp(p[9], "1") == 0);

        facts = (lineage_replace_reconciliation_facts_t){
            attempt(p[1]),
            durable_state(p[4], local_generation, local_high_water),
            attempt(p[5]),
            durable_state(p[8], peer_generation, peer_high_water),
            strcmp(p[9], "1") == 0};
        assert(lineage_replace_classify_reconciliation(&facts) == expected(p[10]));
        count += 1u;
    }

    fclose(fp);
    assert(saw_version == 1);
    assert(count == 14u);
    assert(lineage_replace_classify_reconciliation(NULL) == LINEAGE_REPLACE_PAIR_CONTINUITY_BROKEN);
    puts("lineage-replace asymmetric durable corpus: ok");
    return EXIT_SUCCESS;
}
