#include "auth/lineage_replace_recovery.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RECOVERY_VECTOR_PATH "../rust/test-vectors/replay/lineage-replace-recovery-v1.txt"

static bool bit(const char *value) {
    assert(value != NULL);
    if (strcmp(value, "0") == 0) return false;
    assert(strcmp(value, "1") == 0);
    return true;
}

static lineage_replace_state_t expected_state(const char *name) {
    if (strcmp(name, "ACTIVE_PREDECESSOR") == 0)
        return LINEAGE_REPLACE_STATE_ACTIVE_PREDECESSOR;
    if (strcmp(name, "ACTIVE_SUCCESSOR_PREDECESSOR_RETIRED") == 0)
        return LINEAGE_REPLACE_STATE_ACTIVE_SUCCESSOR_PREDECESSOR_RETIRED;
    assert(strcmp(name, "CONTINUITY_BROKEN") == 0);
    return LINEAGE_REPLACE_STATE_CONTINUITY_BROKEN;
}

int main(void) {
    FILE *fp = fopen(RECOVERY_VECTOR_PATH, "r");
    char line[320];
    unsigned case_count = 0u;
    int saw_version = 0;
    assert(fp != NULL);

    while (fgets(line, sizeof(line), fp) != NULL) {
        char *name, *integrity, *predecessor, *pending, *successor, *retired, *invalidations;
        char *expected, *extra;
        lineage_replace_recovery_facts_t facts;

        line[strcspn(line, "\r\n")] = '\0';
        if (strcmp(line, "version=1") == 0) {
            saw_version = 1;
            continue;
        }
        if (strncmp(line, "case=", 5u) != 0) continue;

        name = strtok(line + 5u, "|");
        integrity = strtok(NULL, "|");
        predecessor = strtok(NULL, "|");
        pending = strtok(NULL, "|");
        successor = strtok(NULL, "|");
        retired = strtok(NULL, "|");
        invalidations = strtok(NULL, "|");
        expected = strtok(NULL, "|");
        extra = strtok(NULL, "|");
        assert(name != NULL && integrity != NULL && predecessor != NULL && pending != NULL);
        assert(successor != NULL && retired != NULL && invalidations != NULL && expected != NULL);
        assert(extra == NULL);

        facts = (lineage_replace_recovery_facts_t){bit(integrity), bit(predecessor), bit(pending),
                                                   bit(successor), bit(retired), bit(invalidations)};
        assert(lineage_replace_recover(&facts) == expected_state(expected));
        case_count += 1u;
    }

    fclose(fp);
    assert(saw_version == 1);
    assert(case_count == 12u);
    assert(lineage_replace_recover(NULL) == LINEAGE_REPLACE_STATE_CONTINUITY_BROKEN);
    puts("lineage-replace recovery corpus: ok");
    return EXIT_SUCCESS;
}
