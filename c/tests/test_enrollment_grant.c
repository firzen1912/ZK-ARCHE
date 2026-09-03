#include "auth/enrollment_grant.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VECTOR_PATH "../rust/test-vectors/state/enrollment-grant-v4.txt"

static bool bit(const char *v) {
    assert(v != NULL);
    if (strcmp(v, "0") == 0) return false;
    assert(strcmp(v, "1") == 0);
    return true;
}

static zk_enrollment_grant_action_t action(const char *v) {
    if (strcmp(v, "ISSUE") == 0) return ZK_ENROLLMENT_GRANT_ISSUE;
    assert(strcmp(v, "DENY") == 0);
    return ZK_ENROLLMENT_GRANT_DENY;
}

static zk_enrollment_grant_reason_t reason(const char *v) {
    static const char *const names[] = {
        "CURRENT", "ROLLBACK_SUSPECTED", "NORMAL_AUTH_FORBIDDEN",
        "EXPLICIT_ENROLL_REQUIRED", "COMMISSIONER_UNAUTHENTICATED",
        "COMMISSIONER_UNAUTHORIZED", "COMMISSIONER_AUTHORIZATION_STALE",
        "COMMISSIONER_AUTHORIZATION_GENERATION_UNBOUND",
        "COMMISSIONER_AUTHORIZATION_GENERATION_STALE", "COMMISSIONER_REVOKED",
        "ENROLLMENT_REPLAY_DETECTED", "SUBJECT_POSSESSION_MISSING",
        "AUTHORITY_ESCALATION", "SCOPE_UNBOUNDED", "AUDIENCE_UNBOUND",
        "DEPLOYMENT_UNBOUND", "VALIDITY_UNBOUNDED", "EPOCH_STALE",
        "REVOCATION_STALE", "LINEAGE_STALE", "DELEGATION_DEPTH_EXCEEDED"
    };
    size_t i;
    for (i = 0u; i < sizeof(names) / sizeof(names[0]); ++i)
        if (strcmp(v, names[i]) == 0) return (zk_enrollment_grant_reason_t)i;
    assert(!"unknown enrollment grant reason");
    return ZK_ENROLLMENT_GRANT_REASON_EXPLICIT_ENROLL_REQUIRED;
}

int main(void) {
    FILE *fp = fopen(VECTOR_PATH, "r");
    char line[1024];
    unsigned cases = 0u;
    int saw_version = 0;
    assert(fp != NULL);

    while (fgets(line, sizeof(line), fp) != NULL) {
        char *f[23];
        size_t i;
        zk_enrollment_grant_facts_t facts;
        zk_enrollment_grant_decision_t got;

        line[strcspn(line, "\r\n")] = '\0';
        if (strcmp(line, "version=4") == 0) {
            saw_version = 1;
            continue;
        }
        if (strncmp(line, "case=", 5u) != 0) continue;

        f[0] = strtok(line + 5u, "|");
        for (i = 1u; i < 23u; ++i) f[i] = strtok(NULL, "|");
        assert(f[22] != NULL && strtok(NULL, "|") == NULL);

        facts = (zk_enrollment_grant_facts_t){
            bit(f[1]), bit(f[2]), bit(f[3]), bit(f[4]), bit(f[5]), bit(f[6]),
            bit(f[7]), bit(f[8]), bit(f[9]), bit(f[10]), bit(f[11]), bit(f[12]),
            bit(f[13]), bit(f[14]), bit(f[15]), bit(f[16]), bit(f[17]), bit(f[18]),
            bit(f[19]), bit(f[20])
        };

        got = zk_enrollment_grant_classify(&facts);
        assert(got.action == action(f[21]));
        assert(got.reason == reason(f[22]));
        cases += 1u;
    }

    fclose(fp);
    assert(saw_version == 1);
    assert(cases == 21u);

    {
        zk_enrollment_grant_decision_t got = zk_enrollment_grant_classify(NULL);
        assert(got.action == ZK_ENROLLMENT_GRANT_DENY);
    }

    puts("enrollment grant corpus v4: ok cases=21");
    return EXIT_SUCCESS;
}
