#include "auth/lineage_replace_attempt_evidence.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define VECTOR_PATH "../rust/test-vectors/replay/lineage-replace-attempt-evidence-v1.txt"
static lineage_replace_attempt_evidence_decision_t expected(const char *v) {
    if (strcmp(v, "FRESH_CURRENT_ATTEMPT") == 0) return LINEAGE_REPLACE_ATTEMPT_EVIDENCE_FRESH_CURRENT_ATTEMPT;
    if (strcmp(v, "MISSING_CURRENT_ATTEMPT") == 0) return LINEAGE_REPLACE_ATTEMPT_EVIDENCE_MISSING_CURRENT_ATTEMPT;
    if (strcmp(v, "ATTEMPT_MISMATCH") == 0) return LINEAGE_REPLACE_ATTEMPT_EVIDENCE_ATTEMPT_MISMATCH;
    if (strcmp(v, "LOCAL_CONFIRMATION_MISSING") == 0) return LINEAGE_REPLACE_ATTEMPT_EVIDENCE_LOCAL_CONFIRMATION_MISSING;
    assert(strcmp(v, "PEER_CONFIRMATION_MISSING") == 0); return LINEAGE_REPLACE_ATTEMPT_EVIDENCE_PEER_CONFIRMATION_MISSING;
}
static uint32_t id(const char *v) { char *end = NULL; unsigned long x = strtoul(v, &end, 10); assert(end != v && *end == '\0' && x <= 0xffffffffUL); return (uint32_t)x; }
int main(void) {
    FILE *fp = fopen(VECTOR_PATH, "r"); char line[512]; unsigned n = 0u; int ver = 0; assert(fp != NULL);
    while (fgets(line, sizeof(line), fp) != NULL) {
        char *f[6] = {0}; char *p; unsigned i = 0u; lineage_replace_attempt_evidence_facts_t facts;
        line[strcspn(line, "\r\n")] = '\0'; if (strcmp(line, "version=1") == 0) { ver = 1; continue; } if (strncmp(line, "case=", 5u) != 0) continue;
        p = strtok(line + 5u, "|"); while (p != NULL && i < 6u) { f[i++] = p; p = strtok(NULL, "|"); } assert(i == 6u && p == NULL);
        facts = (lineage_replace_attempt_evidence_facts_t){id(f[1]), id(f[2]), id(f[3]), id(f[4])};
        assert(lineage_replace_classify_attempt_evidence(&facts) == expected(f[5])); n += 1u;
    }
    fclose(fp); assert(ver == 1 && n == 10u);
    assert(lineage_replace_classify_attempt_evidence(NULL) == LINEAGE_REPLACE_ATTEMPT_EVIDENCE_MISSING_CURRENT_ATTEMPT);
    puts("lineage-replace attempt evidence corpus: ok"); return EXIT_SUCCESS;
}
