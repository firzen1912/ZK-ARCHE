#include "auth/transport_continuation.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VECTOR_PATH "../rust/test-vectors/state/transport-continuation-v2.txt"

static bool bit(const char *value) {
    assert(value != NULL);
    if (strcmp(value, "0") == 0) return false;
    assert(strcmp(value, "1") == 0);
    return true;
}

static transport_continuation_action_t action(const char *value) {
    if (strcmp(value, "CONTINUE") == 0) return TRANSPORT_CONTINUATION_CONTINUE;
    if (strcmp(value, "FULL_AUTH_REQUIRED") == 0)
        return TRANSPORT_CONTINUATION_FULL_AUTH_REQUIRED;
    assert(strcmp(value, "REJECT") == 0);
    return TRANSPORT_CONTINUATION_REJECT;
}

static transport_continuation_reason_t reason(const char *value) {
    static const char *const names[] = {
        "CURRENT", "INVALID_FACTS", "TRANSPORT_ADDRESS_AS_IDENTITY",
        "TRANSPORT_METADATA_AS_AUTHORITY", "ASSOCIATION_INVALIDATED",
        "REPLAY_CONTINUITY_STALE", "USAGE_COUNTER_CONTINUITY_STALE",
        "AUTHORIZATION_GENERATION_STALE", "PEER_CONTEXT_MISMATCH",
        "PROFILE_CONTEXT_MISMATCH", "BINDING_INVALID",
        "RESUMPTION_NOT_AUTHORIZED", "CONTINUATION_NOT_REQUESTED"};
    size_t i;
    for (i = 0u; i < sizeof(names) / sizeof(names[0]); ++i)
        if (strcmp(value, names[i]) == 0) return (transport_continuation_reason_t)i;
    assert(!"unknown transport continuation reason");
    return TRANSPORT_CONTINUATION_REASON_INVALID_FACTS;
}

int main(void) {
    FILE *fp = fopen(VECTOR_PATH, "r");
    char line[896];
    unsigned cases = 0u;
    int saw_version = 0;
    assert(fp != NULL);

    while (fgets(line, sizeof(line), fp) != NULL) {
        char *fields[17];
        size_t i;
        transport_continuation_facts_t facts;
        transport_continuation_decision_t got;

        line[strcspn(line, "\r\n")] = '\0';
        if (strcmp(line, "version=2") == 0) { saw_version = 1; continue; }
        if (strncmp(line, "case=", 5u) != 0) continue;

        fields[0] = strtok(line + 5u, "|");
        for (i = 1u; i < 17u; ++i) fields[i] = strtok(NULL, "|");
        assert(fields[16] != NULL && strtok(NULL, "|") == NULL);

        facts = (transport_continuation_facts_t){
            bit(fields[1]), bit(fields[2]), bit(fields[3]), bit(fields[4]),
            bit(fields[5]), bit(fields[6]), bit(fields[7]), bit(fields[8]),
            bit(fields[9]), bit(fields[10]), bit(fields[11]), bit(fields[12]),
            bit(fields[13]), bit(fields[14])};

        got = transport_continuation_classify(&facts);
        assert(got.action == action(fields[15]));
        assert(got.reason == reason(fields[16]));
        cases += 1u;
    }

    fclose(fp);
    assert(saw_version == 1);
    assert(cases == 15u);

    {
        transport_continuation_decision_t got = transport_continuation_classify(NULL);
        assert(got.action == TRANSPORT_CONTINUATION_REJECT);
        assert(got.reason == TRANSPORT_CONTINUATION_REASON_INVALID_FACTS);
    }

    puts("transport continuation corpus v2: ok cases=15");
    return EXIT_SUCCESS;
}
