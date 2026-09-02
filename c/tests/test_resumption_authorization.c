#include "auth/resumption_authorization.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VECTOR_PATH "../rust/test-vectors/state/resumption-authorization-v3.txt"

static bool bit(const char *value) {
    assert(value != NULL);
    if (strcmp(value, "0") == 0) return false;
    assert(strcmp(value, "1") == 0);
    return true;
}

static uint32_t count(const char *value) {
    char *end = NULL;
    unsigned long parsed;
    errno = 0;
    parsed = strtoul(value, &end, 10);
    assert(errno == 0 && end != value && *end == '\0' && parsed <= UINT32_MAX);
    return (uint32_t)parsed;
}

static resumption_action_t action(const char *value) {
    if (strcmp(value, "RESUME") == 0) return RESUMPTION_ACTION_RESUME;
    if (strcmp(value, "FULL_AUTH_REQUIRED") == 0) return RESUMPTION_ACTION_FULL_AUTH_REQUIRED;
    assert(strcmp(value, "REJECT") == 0);
    return RESUMPTION_ACTION_REJECT;
}

static resumption_reason_t reason(const char *value) {
    static const char *const names[] = {
        "CURRENT", "INVALID_FACTS", "ROLLBACK_SUSPECTED", "USAGE_COUNTER_CONTINUITY_STALE",
        "CREDENTIAL_MISSING", "CREDENTIAL_INVALID", "BINDING_MISMATCH", "EXPIRED",
        "REUSE_LIMIT_REACHED", "AUTHORIZATION_CONTEXT_MISSING", "AUTHORIZATION_STALE",
        "AUTHORIZATION_GENERATION_STALE", "REVOCATION_STALE", "REVOKED", "LINEAGE_STALE",
        "RESTART_CONTINUITY_STALE", "CREDENTIAL_EPOCH_STALE", "SESSION_INVALIDATED",
        "PEER_MISMATCH", "DEPLOYMENT_MISMATCH", "AUDIENCE_MISMATCH", "PROFILE_MISMATCH"};
    size_t i;
    for (i = 0u; i < sizeof(names) / sizeof(names[0]); ++i)
        if (strcmp(value, names[i]) == 0) return (resumption_reason_t)i;
    assert(!"unknown reason");
    return RESUMPTION_REASON_INVALID_FACTS;
}

int main(void) {
    FILE *fp = fopen(VECTOR_PATH, "r");
    char line[1024];
    unsigned cases = 0u;
    int saw_version = 0;
    assert(fp != NULL);
    while (fgets(line, sizeof(line), fp) != NULL) {
        char *fields[24];
        size_t i;
        resumption_authorization_facts_t facts;
        resumption_authorization_decision_t got;
        line[strcspn(line, "\r\n")] = '\0';
        if (strcmp(line, "version=3") == 0) { saw_version = 1; continue; }
        if (strncmp(line, "case=", 5u) != 0) continue;
        fields[0] = strtok(line + 5u, "|");
        for (i = 1u; i < 24u; ++i) fields[i] = strtok(NULL, "|");
        assert(fields[23] != NULL && strtok(NULL, "|") == NULL);
        facts = (resumption_authorization_facts_t){
            bit(fields[1]), bit(fields[2]), bit(fields[3]), bit(fields[4]),
            count(fields[5]), count(fields[6]), bit(fields[7]), bit(fields[8]),
            bit(fields[9]), bit(fields[10]), bit(fields[11]), bit(fields[12]),
            bit(fields[13]), bit(fields[14]), bit(fields[15]), bit(fields[16]),
            bit(fields[17]), bit(fields[18]), bit(fields[19]), bit(fields[20]),
            bit(fields[21])};
        got = resumption_authorization_classify(&facts);
        assert(got.action == action(fields[22]));
        assert(got.reason == reason(fields[23]));
        cases += 1u;
    }
    fclose(fp);
    assert(saw_version == 1);
    assert(cases == 21u);
    {
        resumption_authorization_decision_t got = resumption_authorization_classify(NULL);
        assert(got.action == RESUMPTION_ACTION_REJECT);
        assert(got.reason == RESUMPTION_REASON_INVALID_FACTS);
    }
    puts("resumption authorization corpus v3: ok cases=21");
    return EXIT_SUCCESS;
}
