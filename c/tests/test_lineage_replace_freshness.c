#include "auth/lineage_replace_freshness.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FRESHNESS_VECTOR_PATH "../rust/test-vectors/replay/lineage-replace-freshness-v1.txt"

static bool bit(const char *value) {
    assert(value != NULL);
    if (strcmp(value, "0") == 0) return false;
    assert(strcmp(value, "1") == 0);
    return true;
}

static uint64_t generation(const char *value) {
    char *end = NULL;
    unsigned long long parsed;
    assert(value != NULL);
    errno = 0;
    parsed = strtoull(value, &end, 10);
    assert(errno == 0 && end != value && *end == '\0');
    return (uint64_t)parsed;
}

static lineage_replace_recovery_facts_t recovery_fixture(const char *name) {
    assert(name != NULL);
    if (strcmp(name, "PREDECESSOR") == 0)
        return (lineage_replace_recovery_facts_t){true, true, false, false, false, false};
    if (strcmp(name, "SUCCESSOR") == 0)
        return (lineage_replace_recovery_facts_t){true, false, false, true, true, true};
    assert(strcmp(name, "PARTIAL") == 0);
    return (lineage_replace_recovery_facts_t){true, false, true, true, true, true};
}

static lineage_replace_freshness_decision_t expected_decision(const char *name) {
    assert(name != NULL);
    if (strcmp(name, "CURRENT") == 0) return LINEAGE_REPLACE_FRESHNESS_CURRENT;
    if (strcmp(name, "ANCHOR_UNAVAILABLE") == 0)
        return LINEAGE_REPLACE_FRESHNESS_ANCHOR_UNAVAILABLE;
    if (strcmp(name, "ANCHOR_INVALID") == 0) return LINEAGE_REPLACE_FRESHNESS_ANCHOR_INVALID;
    if (strcmp(name, "BINDING_MISMATCH") == 0)
        return LINEAGE_REPLACE_FRESHNESS_BINDING_MISMATCH;
    if (strcmp(name, "ROLLBACK_DETECTED") == 0)
        return LINEAGE_REPLACE_FRESHNESS_ROLLBACK_DETECTED;
    assert(strcmp(name, "GENERATION_AHEAD") == 0);
    return LINEAGE_REPLACE_FRESHNESS_GENERATION_AHEAD;
}

static lineage_replace_state_t expected_state(const char *name) {
    assert(name != NULL);
    if (strcmp(name, "ACTIVE_PREDECESSOR") == 0)
        return LINEAGE_REPLACE_STATE_ACTIVE_PREDECESSOR;
    if (strcmp(name, "ACTIVE_SUCCESSOR_PREDECESSOR_RETIRED") == 0)
        return LINEAGE_REPLACE_STATE_ACTIVE_SUCCESSOR_PREDECESSOR_RETIRED;
    assert(strcmp(name, "CONTINUITY_BROKEN") == 0);
    return LINEAGE_REPLACE_STATE_CONTINUITY_BROKEN;
}

int main(void) {
    FILE *fp = fopen(FRESHNESS_VECTOR_PATH, "r");
    char line[512];
    unsigned case_count = 0u;
    int saw_version = 0;
    assert(fp != NULL);

    while (fgets(line, sizeof(line), fp) != NULL) {
        char *name, *fixture, *available, *integrity, *binding, *record_generation;
        char *high_water_generation, *decision, *state, *extra;
        lineage_replace_recovery_facts_t recovery;
        lineage_replace_freshness_facts_t freshness;

        line[strcspn(line, "\r\n")] = '\0';
        if (strcmp(line, "version=1") == 0) {
            saw_version = 1;
            continue;
        }
        if (strncmp(line, "case=", 5u) != 0) continue;

        name = strtok(line + 5u, "|");
        fixture = strtok(NULL, "|");
        available = strtok(NULL, "|");
        integrity = strtok(NULL, "|");
        binding = strtok(NULL, "|");
        record_generation = strtok(NULL, "|");
        high_water_generation = strtok(NULL, "|");
        decision = strtok(NULL, "|");
        state = strtok(NULL, "|");
        extra = strtok(NULL, "|");
        assert(name != NULL && fixture != NULL && available != NULL && integrity != NULL);
        assert(binding != NULL && record_generation != NULL && high_water_generation != NULL);
        assert(decision != NULL && state != NULL && extra == NULL);

        recovery = recovery_fixture(fixture);
        freshness = (lineage_replace_freshness_facts_t){
            bit(available), bit(integrity), bit(binding), generation(record_generation),
            generation(high_water_generation)};
        assert(lineage_replace_classify_freshness(&freshness) == expected_decision(decision));
        assert(lineage_replace_recover_with_freshness(&recovery, &freshness) ==
               expected_state(state));
        case_count += 1u;
    }

    fclose(fp);
    assert(saw_version == 1);
    assert(case_count == 10u);
    assert(lineage_replace_classify_freshness(NULL) == LINEAGE_REPLACE_FRESHNESS_ANCHOR_INVALID);
    assert(lineage_replace_recover_with_freshness(NULL, NULL) ==
           LINEAGE_REPLACE_STATE_CONTINUITY_BROKEN);
    puts("lineage-replace freshness corpus: ok");
    return EXIT_SUCCESS;
}
