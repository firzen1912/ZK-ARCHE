#include "auth/data_release_authorization.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VECTOR_PATH "../rust/test-vectors/state/data-release-authorization-v3.txt"

static bool bit(const char *v) {
    assert(v);
    if (!strcmp(v, "0")) return false;
    assert(!strcmp(v, "1"));
    return true;
}

static data_release_action_t action(const char *v) {
    if (!strcmp(v, "RELEASE")) return DATA_RELEASE_ACTION_RELEASE;
    if (!strcmp(v, "FRESH_AUTH_REQUIRED")) return DATA_RELEASE_ACTION_FRESH_AUTH_REQUIRED;
    assert(!strcmp(v, "DENY"));
    return DATA_RELEASE_ACTION_DENY;
}

static data_release_reason_t reason(const char *v) {
    static const char *const names[] = {
        "CURRENT", "ROLLBACK_SUSPECTED", "UNAUTHENTICATED",
        "DEVICE_RELEASE_AUTHORITY_MISSING", "DEVICE_RELEASE_AUTHORITY_STALE",
        "PROTECTED_DATA_NOT_ENCRYPTED", "RELEASE_KEY_SCOPE_MISMATCH",
        "AUTHORIZATION_MISSING", "AUTHORIZATION_STALE", "AUTHORIZATION_GENERATION_STALE",
        "REVOCATION_STALE", "REVOKED", "LINEAGE_STALE", "HOLDER_MISMATCH",
        "AUDIENCE_MISMATCH", "PURPOSE_MISMATCH", "DATA_TYPE_MISMATCH", "POLICY_MISMATCH",
        "EPOCH_MISMATCH", "CHANNEL_BINDING_MISSING_OR_INVALID", "RELEASE_REPLAY_DETECTED"
    };
    size_t i;
    for (i = 0u; i < sizeof(names) / sizeof(names[0]); ++i)
        if (!strcmp(v, names[i])) return (data_release_reason_t)i;
    assert(!"unknown DATA release reason");
    return DATA_RELEASE_REASON_AUTHORIZATION_MISSING;
}

int main(void) {
    FILE *fp = fopen(VECTOR_PATH, "r");
    char line[1024];
    unsigned cases = 0u;
    assert(fp);

    while (fgets(line, sizeof(line), fp)) {
        char *fields[24];
        size_t i;
        data_release_facts_t f;
        data_release_decision_t got;

        line[strcspn(line, "\r\n")] = '\0';
        if (strncmp(line, "case=", 5u)) continue;
        fields[0] = strtok(line + 5u, "|");
        for (i = 1u; i < 24u; ++i) fields[i] = strtok(NULL, "|");
        assert(fields[23] && strtok(NULL, "|") == NULL);

        f = (data_release_facts_t){
            bit(fields[1]), bit(fields[2]), bit(fields[3]), bit(fields[4]), bit(fields[5]),
            bit(fields[6]), bit(fields[7]), bit(fields[8]), bit(fields[9]), bit(fields[10]),
            bit(fields[11]), bit(fields[12]), bit(fields[13]), bit(fields[14]), bit(fields[15]),
            bit(fields[16]), bit(fields[17]), bit(fields[18]), bit(fields[19]), bit(fields[20]),
            bit(fields[21])
        };
        got = data_release_authorization_classify(&f);
        assert(got.action == action(fields[22]));
        assert(got.reason == reason(fields[23]));
        cases += 1u;
    }

    fclose(fp);
    assert(cases == 21u);
    {
        data_release_decision_t got = data_release_authorization_classify(NULL);
        assert(got.action == DATA_RELEASE_ACTION_DENY);
    }
    puts("data release authorization corpus v3: ok cases=21");
    return EXIT_SUCCESS;
}
