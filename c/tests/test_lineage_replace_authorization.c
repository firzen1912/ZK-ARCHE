#include "auth/lineage_replace.h"
#include "auth/lineage_replace_authorization.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VECTOR_PATH "../rust/test-vectors/replay/lineage-replace-authorization-v1.txt"

static bool bit(const char *v) { if (strcmp(v, "1") == 0) return true; assert(strcmp(v, "0") == 0); return false; }
static lineage_replace_authorization_decision_t expected_auth(const char *v) {
    if (strcmp(v, "AUTHORIZED_REPLACEMENT") == 0) return LINEAGE_REPLACE_AUTHORIZED_REPLACEMENT;
    if (strcmp(v, "REJECT_CURRENT_CREDENTIAL_CONTROL") == 0) return LINEAGE_REPLACE_REJECT_CURRENT_CREDENTIAL_CONTROL;
    if (strcmp(v, "REJECT_SUCCESSOR_KEY_CONTROL") == 0) return LINEAGE_REPLACE_REJECT_SUCCESSOR_KEY_CONTROL;
    if (strcmp(v, "REJECT_SESSION_AUTHENTICATION") == 0) return LINEAGE_REPLACE_REJECT_SESSION_AUTHENTICATION;
    if (strcmp(v, "REJECT_SESSION_AUTHORIZATION") == 0) return LINEAGE_REPLACE_REJECT_SESSION_AUTHORIZATION;
    if (strcmp(v, "REJECT_CONTEXT_BINDING") == 0) return LINEAGE_REPLACE_REJECT_CONTEXT_BINDING;
    if (strcmp(v, "REJECT_PREDECESSOR_BINDING") == 0) return LINEAGE_REPLACE_REJECT_PREDECESSOR_BINDING;
    assert(strcmp(v, "REJECT_PRIVILEGE_EXPANSION") == 0); return LINEAGE_REPLACE_REJECT_PRIVILEGE_EXPANSION;
}
static lineage_replace_decision_t expected_lifecycle(const char *v) {
    if (strcmp(v, "ACCEPT_SUCCESSOR") == 0) return LINEAGE_REPLACE_ACCEPT_SUCCESSOR;
    assert(strcmp(v, "REJECT_AUTHORITY") == 0); return LINEAGE_REPLACE_REJECT_AUTHORITY;
}
int main(void) {
    FILE *fp = fopen(VECTOR_PATH, "r"); char line[512]; unsigned cases = 0u; int version = 0;
    lineage_replace_facts_t lifecycle = {LINEAGE_REPLACE_TRIGGER_LIFECYCLE, false, true, true, true, true, true, true, true, true, true};
    assert(fp != NULL);
    while (fgets(line, sizeof(line), fp) != NULL) {
        char *f[10] = {0}; char *p; unsigned i = 0u; lineage_replace_authorization_facts_t facts; lineage_replace_authorization_decision_t decision;
        line[strcspn(line, "\r\n")] = '\0'; if (strcmp(line, "version=1") == 0) { version = 1; continue; } if (strncmp(line, "case=", 5u) != 0) continue;
        p = strtok(line + 5u, "|"); while (p != NULL && i < 10u) { f[i++] = p; p = strtok(NULL, "|"); } assert(i == 10u && p == NULL);
        facts = (lineage_replace_authorization_facts_t){bit(f[1]), bit(f[2]), bit(f[3]), bit(f[4]), bit(f[5]), bit(f[6]), bit(f[7])};
        decision = lineage_replace_classify_authorization(&facts); assert(decision == expected_auth(f[8]));
        assert(lineage_replace_evaluate_authorized(decision, &lifecycle) == expected_lifecycle(f[9])); cases += 1u;
    }
    fclose(fp); assert(version == 1 && cases == 10u);
    assert(lineage_replace_classify_authorization(NULL) == LINEAGE_REPLACE_REJECT_CURRENT_CREDENTIAL_CONTROL);
    assert(lineage_replace_evaluate_authorized(LINEAGE_REPLACE_AUTHORIZED_REPLACEMENT, NULL) == LINEAGE_REPLACE_REJECT_STORAGE);
    puts("lineage-replace authorization corpus: ok"); return EXIT_SUCCESS;
}
