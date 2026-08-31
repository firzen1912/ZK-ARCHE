#include "auth/lineage_replace_attempt.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ATTEMPT_VECTOR_PATH "../rust/test-vectors/replay/lineage-replace-attempt-binding-v1.txt"

static bool bit(const char *value) {
    assert(value != NULL);
    if (strcmp(value, "0") == 0) return false;
    assert(strcmp(value, "1") == 0);
    return true;
}

static lineage_replace_attempt_decision_t expected(const char *value) {
    assert(value != NULL);
    if (strcmp(value, "CONVERGED") == 0) return LINEAGE_REPLACE_ATTEMPT_CONVERGED;
    if (strcmp(value, "AWAITING_CONFIRMATION") == 0)
        return LINEAGE_REPLACE_ATTEMPT_AWAITING_CONFIRMATION;
    if (strcmp(value, "UNAUTHORIZED") == 0)
        return LINEAGE_REPLACE_ATTEMPT_UNAUTHORIZED;
    if (strcmp(value, "ATTEMPT_ID_MISMATCH") == 0)
        return LINEAGE_REPLACE_ATTEMPT_ID_MISMATCH;
    if (strcmp(value, "PREDECESSOR_MISMATCH") == 0)
        return LINEAGE_REPLACE_ATTEMPT_PREDECESSOR_MISMATCH;
    if (strcmp(value, "SUCCESSOR_MISMATCH") == 0)
        return LINEAGE_REPLACE_ATTEMPT_SUCCESSOR_MISMATCH;
    assert(strcmp(value, "CONTEXT_MISMATCH") == 0);
    return LINEAGE_REPLACE_ATTEMPT_CONTEXT_MISMATCH;
}

int main(void) {
    FILE *fp = fopen(ATTEMPT_VECTOR_PATH, "r");
    char line[768];
    unsigned case_count = 0u;
    int saw_version = 0;

    assert(fp != NULL);
    while (fgets(line, sizeof(line), fp) != NULL) {
        char *fields[10] = {0};
        char *cursor;
        unsigned i;
        lineage_replace_attempt_facts_t facts;

        line[strcspn(line, "\r\n")] = '\0';
        if (strcmp(line, "version=1") == 0) {
            saw_version = 1;
            continue;
        }
        if (strncmp(line, "case=", 5u) != 0) continue;

        cursor = strtok(line + 5u, "|");
        for (i = 0u; i < 10u && cursor != NULL; i += 1u) {
            fields[i] = cursor;
            cursor = strtok(NULL, "|");
        }
        assert(i == 10u && cursor == NULL);

        facts = (lineage_replace_attempt_facts_t){
            bit(fields[1]), bit(fields[2]), bit(fields[3]), bit(fields[4]), bit(fields[5]),
            bit(fields[6]), bit(fields[7]), bit(fields[8])};
        assert(lineage_replace_classify_attempt(&facts) == expected(fields[9]));
        case_count += 1u;
    }

    fclose(fp);
    assert(saw_version == 1);
    assert(case_count == 12u);
    assert(lineage_replace_classify_attempt(NULL) == LINEAGE_REPLACE_ATTEMPT_UNAUTHORIZED);
    puts("lineage-replace attempt binding corpus: ok");
    return EXIT_SUCCESS;
}
