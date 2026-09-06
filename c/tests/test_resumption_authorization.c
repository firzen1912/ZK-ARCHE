#include "auth/resumption_authorization.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VECTOR_PATH "../rust/test-vectors/state/resumption-authorization-v5.txt"

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
        "AUTHORIZATION_GENERATION_UNBOUND", "AUTHORIZATION_GENERATION_STALE",
        "REVOCATION_STALE", "REVOKED", "LINEAGE_STALE", "RESTART_CONTINUITY_STALE",
        "CREDENTIAL_EPOCH_STALE", "SESSION_INVALIDATED", "PEER_MISMATCH",
        "DEPLOYMENT_MISMATCH", "AUDIENCE_MISMATCH", "PROFILE_MISMATCH",
        "PRIVACY_IDENTIFIER_STATE_STALE", "REPEATED_IDENTIFIER_LINKABLE"};
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
        char *fields[27];
        size_t i;
        resumption_authorization_facts_t facts;
        resumption_authorization_decision_t got;
        line[strcspn(line, "\r\n")] = '\0';
        if (strcmp(line, "version=5") == 0) { saw_version = 1; continue; }
        if (strncmp(line, "case=", 5u) != 0) continue;
        fields[0] = strtok(line + 5u, "|");
        for (i = 1u; i < 27u; ++i) fields[i] = strtok(NULL, "|");
        assert(fields[26] != NULL && strtok(NULL, "|") == NULL);
        facts = (resumption_authorization_facts_t){
            .credential_present = bit(fields[1]),
            .credential_integrity_valid = bit(fields[2]),
            .binding_valid = bit(fields[3]),
            .expired = bit(fields[4]),
            .usage_count = count(fields[5]),
            .usage_limit = count(fields[6]),
            .usage_counter_continuity_current = bit(fields[7]),
            .authorization_context_present = bit(fields[8]),
            .authorization_context_fresh = bit(fields[9]),
            .authorization_generation_bound = bit(fields[10]),
            .authorization_generation_current = bit(fields[11]),
            .revocation_current = bit(fields[12]),
            .explicitly_revoked = bit(fields[13]),
            .lineage_current = bit(fields[14]),
            .restart_continuity_current = bit(fields[15]),
            .credential_epoch_current = bit(fields[16]),
            .session_invalidated = bit(fields[17]),
            .privacy_identifier_state_current = bit(fields[18]),
            .repeated_identifier_linkable = bit(fields[19]),
            .peer_match = bit(fields[20]),
            .deployment_match = bit(fields[21]),
            .audience_match = bit(fields[22]),
            .profile_match = bit(fields[23]),
            .rollback_suspected = bit(fields[24])};
        got = resumption_authorization_classify(&facts);
        assert(got.action == action(fields[25]));
        assert(got.reason == reason(fields[26]));
        cases += 1u;
    }
    fclose(fp);
    assert(saw_version == 1);
    assert(cases == 24u);
    {
        resumption_authorization_decision_t got = resumption_authorization_classify(NULL);
        assert(got.action == RESUMPTION_ACTION_REJECT);
        assert(got.reason == RESUMPTION_REASON_INVALID_FACTS);
    }
    puts("resumption authorization corpus v5: ok cases=24");
    return EXIT_SUCCESS;
}
