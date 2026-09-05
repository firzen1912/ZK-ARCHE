#include "auth/association_admission.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VECTOR_PATH "../rust/test-vectors/state/association-admission-v4.txt"

static bool bit(const char *value) { assert(value != NULL); if (strcmp(value, "0") == 0) return false; assert(strcmp(value, "1") == 0); return true; }
static association_admission_action_t action(const char *value) { if (strcmp(value, "ESTABLISH") == 0) return ASSOCIATION_ADMISSION_ESTABLISH; assert(strcmp(value, "FAIL_CLOSED") == 0); return ASSOCIATION_ADMISSION_FAIL_CLOSED; }
static association_admission_reason_t reason(const char *value) {
    static const char *const names[] = {"CURRENT","INVALID_FACTS","ROLLBACK_SUSPECTED","TRUST_MUTATION_REQUESTED","AUTH_INCOMPLETE","TRUST_RECORD_MISSING","AUTHORIZATION_MISSING","AUTHORIZATION_STALE","REVOCATION_STALE","REVOKED","LINEAGE_STALE","REPLAY_CONTINUITY_STALE","BINDING_INVALID","AUTHORIZATION_GENERATION_STALE","RESTART_CONTINUITY_STALE","AUTHORIZATION_GENERATION_UNBOUND","USAGE_COUNTER_CONTINUITY_STALE"};
    size_t i; for (i = 0u; i < sizeof(names)/sizeof(names[0]); ++i) if (strcmp(value, names[i]) == 0) return (association_admission_reason_t)i; assert(!"unknown reason"); return ASSOCIATION_ADMISSION_REASON_INVALID_FACTS;
}

int main(void) {
    FILE *fp = fopen(VECTOR_PATH, "r"); char line[896]; unsigned cases = 0u; int saw_version = 0; assert(fp != NULL);
    while (fgets(line, sizeof(line), fp) != NULL) {
        char *fields[19]; size_t i; association_admission_facts_t facts; association_admission_decision_t got;
        line[strcspn(line, "\r\n")] = '\0'; if (strcmp(line, "version=4") == 0) { saw_version = 1; continue; } if (strncmp(line, "case=", 5u) != 0) continue;
        fields[0] = strtok(line + 5u, "|"); for (i = 1u; i < 19u; ++i) fields[i] = strtok(NULL, "|"); assert(fields[18] != NULL && strtok(NULL, "|") == NULL);
        facts = (association_admission_facts_t){bit(fields[1]),bit(fields[2]),bit(fields[3]),bit(fields[4]),bit(fields[5]),bit(fields[6]),bit(fields[7]),bit(fields[8]),bit(fields[9]),bit(fields[10]),bit(fields[11]),bit(fields[12]),bit(fields[13]),bit(fields[14]),bit(fields[15]),bit(fields[16])};
        got = association_admission_classify(&facts); assert(got.action == action(fields[17])); assert(got.reason == reason(fields[18])); cases += 1u;
    }
    fclose(fp); assert(saw_version == 1); assert(cases == 18u);
    { association_admission_decision_t got = association_admission_classify(NULL); assert(got.action == ASSOCIATION_ADMISSION_FAIL_CLOSED); assert(got.reason == ASSOCIATION_ADMISSION_REASON_INVALID_FACTS); }
    puts("association admission corpus v4: ok cases=18"); return EXIT_SUCCESS;
}
