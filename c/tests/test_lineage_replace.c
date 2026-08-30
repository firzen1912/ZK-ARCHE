#include "auth/lineage_replace.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#define VECTOR_PATH "../rust/test-vectors/replay/lineage-replace-decisions-v1.txt"

static lineage_replace_facts_t fixture(void) {
    lineage_replace_facts_t facts = {
        LINEAGE_REPLACE_TRIGGER_LIFECYCLE,
        true, true, true, true, true, true, true, true, true, true
    };
    return facts;
}

static lineage_replace_decision_t expected_decision(const char *name) {
    if (strcmp(name, "ACCEPT_SUCCESSOR") == 0) return LINEAGE_REPLACE_ACCEPT_SUCCESSOR;
    if (strcmp(name, "REJECT_AUTHORITY") == 0) return LINEAGE_REPLACE_REJECT_AUTHORITY;
    if (strcmp(name, "REJECT_PREDECESSOR") == 0) return LINEAGE_REPLACE_REJECT_PREDECESSOR;
    if (strcmp(name, "REJECT_SUCCESSOR") == 0) return LINEAGE_REPLACE_REJECT_SUCCESSOR;
    if (strcmp(name, "REJECT_CONTEXT") == 0) return LINEAGE_REPLACE_REJECT_CONTEXT;
    if (strcmp(name, "REJECT_FRESHNESS") == 0) return LINEAGE_REPLACE_REJECT_FRESHNESS;
    if (strcmp(name, "REJECT_REPLAY") == 0) return LINEAGE_REPLACE_REJECT_REPLAY;
    if (strcmp(name, "REJECT_CONCURRENT") == 0) return LINEAGE_REPLACE_REJECT_CONCURRENT;
    if (strcmp(name, "REJECT_ROLLBACK") == 0) return LINEAGE_REPLACE_REJECT_ROLLBACK;
    if (strcmp(name, "REJECT_STORAGE") == 0) return LINEAGE_REPLACE_REJECT_STORAGE;
    assert(0 && "unknown lineage-replace decision");
    return LINEAGE_REPLACE_REJECT_STORAGE;
}

static void apply_mutation(lineage_replace_facts_t *facts, const char *mutation) {
    if (strcmp(mutation, "none") == 0) return;
    if (strcmp(mutation, "unauthorized_authority") == 0) facts->authority_valid = false;
    else if (strcmp(mutation, "restart_trigger") == 0) facts->trigger = LINEAGE_REPLACE_TRIGGER_RESTART;
    else if (strcmp(mutation, "transport_change_trigger") == 0) facts->trigger = LINEAGE_REPLACE_TRIGGER_TRANSPORT_CHANGE;
    else if (strcmp(mutation, "auth_trigger") == 0) facts->trigger = LINEAGE_REPLACE_TRIGGER_AUTH;
    else if (strcmp(mutation, "replayed_transition") == 0 || strcmp(mutation, "predecessor_domain_not_retired") == 0) facts->replay_free = false;
    else if (strcmp(mutation, "storage_partial") == 0 || strcmp(mutation, "storage_ambiguous") == 0) facts->storage_safe = false;
    else if (strcmp(mutation, "rollback_suspected") == 0) facts->rollback_clear = false;
    else if (strcmp(mutation, "stale_freshness") == 0 || strcmp(mutation, "stale_revocation") == 0) facts->freshness_valid = false;
    else if (strcmp(mutation, "dependent_state_not_revalidated") == 0) facts->dependent_state_safe = false;
    else if (strcmp(mutation, "competing_successor") == 0) facts->concurrent_free = false;
    else if (strcmp(mutation, "retired_predecessor") == 0 || strcmp(mutation, "wrong_predecessor_generation") == 0) facts->predecessor_valid = false;
    else if (strcmp(mutation, "successor_binding_mismatch") == 0) facts->successor_valid = false;
    else if (strcmp(mutation, "downgrade_context") == 0 || strcmp(mutation, "wrong_domain_context") == 0) facts->context_valid = false;
    else assert(0 && "unknown lineage-replace mutation");
}

int main(void) {
    FILE *fp = fopen(VECTOR_PATH, "r");
    char line[256];
    unsigned case_count = 0u;
    int saw_version = 0;
    assert(fp != NULL);

    while (fgets(line, sizeof(line), fp) != NULL) {
        char *name, *mutation, *expected, *extra;
        lineage_replace_facts_t facts;
        line[strcspn(line, "\r\n")] = '\0';
        if (strcmp(line, "version=1") == 0) {
            saw_version = 1;
            continue;
        }
        if (strncmp(line, "case=", 5u) != 0) continue;
        name = strtok(line + 5u, "|");
        mutation = strtok(NULL, "|");
        expected = strtok(NULL, "|");
        extra = strtok(NULL, "|");
        assert(name != NULL && mutation != NULL && expected != NULL && extra == NULL);

        facts = fixture();
        apply_mutation(&facts, mutation);
        assert(lineage_replace_evaluate(&facts) == expected_decision(expected));
        case_count += 1u;
    }

    fclose(fp);
    assert(saw_version == 1);
    assert(case_count == 20u);
    puts("lineage-replace shared corpus: ok");
    return 0;
}
