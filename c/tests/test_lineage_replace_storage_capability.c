#include "auth/lineage_replace_storage_capability.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VECTOR_PATH "../rust/test-vectors/replay/lineage-replace-storage-capability-v1.txt"

static bool flag(const char *value) {
    assert(strcmp(value, "0") == 0 || strcmp(value, "1") == 0);
    return strcmp(value, "1") == 0;
}

static lineage_replace_storage_capability_decision_t expected(const char *value) {
    if (strcmp(value, "QUALIFIED") == 0) return LINEAGE_REPLACE_STORAGE_CAPABILITY_QUALIFIED;
    if (strcmp(value, "REJECT_DURABILITY") == 0) return LINEAGE_REPLACE_STORAGE_CAPABILITY_REJECT_DURABILITY;
    if (strcmp(value, "REJECT_POWER_LOSS_RECOVERY") == 0) return LINEAGE_REPLACE_STORAGE_CAPABILITY_REJECT_POWER_LOSS_RECOVERY;
    if (strcmp(value, "REJECT_RECORD_INTEGRITY") == 0) return LINEAGE_REPLACE_STORAGE_CAPABILITY_REJECT_RECORD_INTEGRITY;
    if (strcmp(value, "REJECT_REPLAY_PROTECTION") == 0) return LINEAGE_REPLACE_STORAGE_CAPABILITY_REJECT_REPLAY_PROTECTION;
    if (strcmp(value, "REJECT_FRESHNESS_ANCHOR") == 0) return LINEAGE_REPLACE_STORAGE_CAPABILITY_REJECT_FRESHNESS_ANCHOR;
    if (strcmp(value, "REJECT_FRESHNESS_INTEGRITY") == 0) return LINEAGE_REPLACE_STORAGE_CAPABILITY_REJECT_FRESHNESS_INTEGRITY;
    assert(strcmp(value, "REJECT_FRESHNESS_BINDING") == 0);
    return LINEAGE_REPLACE_STORAGE_CAPABILITY_REJECT_FRESHNESS_BINDING;
}

int main(void) {
    FILE *fp = fopen(VECTOR_PATH, "r");
    char line[512];
    unsigned cases = 0u;
    int version = 0;
    assert(fp != NULL);

    while (fgets(line, sizeof(line), fp) != NULL) {
        char *field[9] = {0};
        char *token;
        unsigned i = 0u;
        lineage_replace_storage_capability_t capability;
        lineage_replace_storage_capability_decision_t decision;

        line[strcspn(line, "\r\n")] = '\0';
        if (strcmp(line, "version=1") == 0) { version = 1; continue; }
        if (strncmp(line, "case=", 5u) != 0) continue;

        token = strtok(line + 5u, "|");
        while (token != NULL && i < 9u) { field[i++] = token; token = strtok(NULL, "|"); }
        assert(i == 9u && token == NULL);

        capability.durable_commit_confirmed = flag(field[1]);
        capability.power_loss_recovery_supported = flag(field[2]);
        capability.record_integrity_protected = flag(field[3]);
        capability.replay_protection_supported = flag(field[4]);
        capability.freshness_anchor_available = flag(field[5]);
        capability.freshness_anchor_integrity_valid = flag(field[6]);
        capability.freshness_anchor_lineage_bound = flag(field[7]);

        decision = lineage_replace_classify_storage_capability(&capability);
        assert(decision == expected(field[8]));
        cases += 1u;
    }

    fclose(fp);
    assert(version == 1 && cases == 10u);
    assert(lineage_replace_classify_storage_capability(NULL) ==
           LINEAGE_REPLACE_STORAGE_CAPABILITY_REJECT_DURABILITY);
    puts("lineage-replace storage-capability corpus: ok");
    return EXIT_SUCCESS;
}
