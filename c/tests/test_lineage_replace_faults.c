#include "auth/lineage_replace_faults.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CUT_VECTOR_PATH "../rust/test-vectors/replay/lineage-replace-write-cuts-v1.txt"

static lineage_replace_write_cut_t cut(const char *name) {
    if (strcmp(name, "BEFORE_BEGIN") == 0) return LINEAGE_REPLACE_CUT_BEFORE_BEGIN;
    if (strcmp(name, "AFTER_PENDING_MARKER") == 0) return LINEAGE_REPLACE_CUT_AFTER_PENDING_MARKER;
    if (strcmp(name, "AFTER_SUCCESSOR_ACTIVATION") == 0)
        return LINEAGE_REPLACE_CUT_AFTER_SUCCESSOR_ACTIVATION;
    if (strcmp(name, "AFTER_PREDECESSOR_RETIREMENT") == 0)
        return LINEAGE_REPLACE_CUT_AFTER_PREDECESSOR_RETIREMENT;
    if (strcmp(name, "AFTER_INVALIDATIONS") == 0) return LINEAGE_REPLACE_CUT_AFTER_INVALIDATIONS;
    assert(strcmp(name, "AFTER_COMMIT_MARKER_CLEAR") == 0);
    return LINEAGE_REPLACE_CUT_AFTER_COMMIT_MARKER_CLEAR;
}

static lineage_replace_state_t state(const char *name) {
    if (strcmp(name, "ACTIVE_PREDECESSOR") == 0)
        return LINEAGE_REPLACE_STATE_ACTIVE_PREDECESSOR;
    if (strcmp(name, "ACTIVE_SUCCESSOR_PREDECESSOR_RETIRED") == 0)
        return LINEAGE_REPLACE_STATE_ACTIVE_SUCCESSOR_PREDECESSOR_RETIRED;
    assert(strcmp(name, "CONTINUITY_BROKEN") == 0);
    return LINEAGE_REPLACE_STATE_CONTINUITY_BROKEN;
}

int main(void) {
    FILE *fp = fopen(CUT_VECTOR_PATH, "r");
    char line[256];
    unsigned case_count = 0u;
    int saw_version = 0;
    assert(fp != NULL);

    while (fgets(line, sizeof(line), fp) != NULL) {
        char *name, *expected, *extra;
        lineage_replace_recovery_facts_t facts;

        line[strcspn(line, "\r\n")] = '\0';
        if (strcmp(line, "version=1") == 0) {
            saw_version = 1;
            continue;
        }
        if (strncmp(line, "case=", 5u) != 0) continue;

        name = strtok(line + 5u, "|");
        expected = strtok(NULL, "|");
        extra = strtok(NULL, "|");
        assert(name != NULL && expected != NULL && extra == NULL);

        facts = lineage_replace_facts_at_write_cut(cut(name));
        assert(lineage_replace_recover(&facts) == state(expected));
        case_count += 1u;
    }

    fclose(fp);
    assert(saw_version == 1);
    assert(case_count == 6u);

    {
        lineage_replace_recovery_facts_t invalid = lineage_replace_facts_at_write_cut((lineage_replace_write_cut_t)99);
        assert(!invalid.record_integrity_valid);
        assert(lineage_replace_recover(&invalid) == LINEAGE_REPLACE_STATE_CONTINUITY_BROKEN);
    }

    puts("lineage-replace write-cut corpus: ok");
    return EXIT_SUCCESS;
}
