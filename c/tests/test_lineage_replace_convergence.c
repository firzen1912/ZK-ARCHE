#include "auth/lineage_replace_convergence.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CONVERGENCE_VECTOR_PATH "../rust/test-vectors/replay/lineage-replace-convergence-v1.txt"

static bool bit(const char *value) {
    assert(value != NULL);
    if (strcmp(value, "0") == 0) return false;
    assert(strcmp(value, "1") == 0);
    return true;
}

static lineage_replace_convergence_decision_t expected(const char *value) {
    assert(value != NULL);
    if (strcmp(value, "CONVERGED") == 0) return LINEAGE_REPLACE_CONVERGED;
    if (strcmp(value, "AWAITING_CONFIRMATION") == 0)
        return LINEAGE_REPLACE_AWAITING_CONFIRMATION;
    if (strcmp(value, "UNAUTHORIZED") == 0) return LINEAGE_REPLACE_UNAUTHORIZED;
    assert(strcmp(value, "SUCCESSOR_CONFLICT") == 0);
    return LINEAGE_REPLACE_SUCCESSOR_CONFLICT;
}

int main(void) {
    FILE *fp = fopen(CONVERGENCE_VECTOR_PATH, "r");
    char line[512];
    unsigned case_count = 0u;
    int saw_version = 0;
    assert(fp != NULL);

    while (fgets(line, sizeof(line), fp) != NULL) {
        char *name, *local_authorized, *peer_authorized, *same_successor;
        char *local_confirmed, *peer_confirmed, *decision, *extra;
        lineage_replace_convergence_facts_t facts;

        line[strcspn(line, "\r\n")] = '\0';
        if (strcmp(line, "version=1") == 0) {
            saw_version = 1;
            continue;
        }
        if (strncmp(line, "case=", 5u) != 0) continue;

        name = strtok(line + 5u, "|");
        local_authorized = strtok(NULL, "|");
        peer_authorized = strtok(NULL, "|");
        same_successor = strtok(NULL, "|");
        local_confirmed = strtok(NULL, "|");
        peer_confirmed = strtok(NULL, "|");
        decision = strtok(NULL, "|");
        extra = strtok(NULL, "|");
        assert(name != NULL && local_authorized != NULL && peer_authorized != NULL);
        assert(same_successor != NULL && local_confirmed != NULL && peer_confirmed != NULL);
        assert(decision != NULL && extra == NULL);

        facts = (lineage_replace_convergence_facts_t){
            bit(local_authorized), bit(peer_authorized), bit(same_successor),
            bit(local_confirmed), bit(peer_confirmed)};
        assert(lineage_replace_classify_convergence(&facts) == expected(decision));
        case_count += 1u;
    }

    fclose(fp);
    assert(saw_version == 1);
    assert(case_count == 8u);
    assert(lineage_replace_classify_convergence(NULL) == LINEAGE_REPLACE_UNAUTHORIZED);
    puts("lineage-replace convergence corpus: ok");
    return EXIT_SUCCESS;
}
