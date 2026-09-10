#include "auth/auth_v3_iot_core_authz.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#define CORPUS_PATH "../rust/test-vectors/auth-v3/iot-core-authorization-freshness-v1.txt"

static auth_v3_iot_core_authorization_context_v1_t context_fixture(void) {
    auth_v3_iot_core_authorization_context_v1_t context;
    memset(&context, 0, sizeof(context));
    memset(context.holder_binding, 0x11, sizeof(context.holder_binding));
    memset(context.audience_id, 0x22, sizeof(context.audience_id));
    context.role_policy_id = 7u;
    context.scope_bits = AUTH_V3_IOT_CORE_SCOPE_SECURE_ASSOCIATION;
    context.authorization_generation = 9u;
    context.policy_epoch = 11u;
    context.revocation_epoch = 13u;
    return context;
}

static auth_v3_iot_core_attribution_record_v1_t authority_fixture(
    const auth_v3_iot_core_authorization_context_v1_t *context) {
    auth_v3_iot_core_attribution_record_v1_t record;
    memset(&record, 0, sizeof(record));
    memset(record.credential_reference, 0xa1, sizeof(record.credential_reference));
    memset(record.peer_identity, 0xb1, sizeof(record.peer_identity));
    memcpy(record.holder_binding, context->holder_binding, sizeof(record.holder_binding));
    memcpy(record.audience_id, context->audience_id, sizeof(record.audience_id));
    record.role_policy_id = context->role_policy_id;
    record.scope_bits = context->scope_bits;
    record.authorization_generation = context->authorization_generation;
    record.policy_epoch = context->policy_epoch;
    record.revocation_epoch = context->revocation_epoch;
    return record;
}

int main(void) {
    FILE *fp = fopen(CORPUS_PATH, "r");
    char line[256];
    unsigned case_count = 0u;
    int saw_version = 0;
    assert(fp != NULL);

    while (fgets(line, sizeof(line), fp) != NULL) {
        char *name;
        char *mutation;
        char *expected;
        char *extra;
        auth_v3_iot_core_authorization_context_v1_t context;
        auth_v3_iot_core_attribution_record_v1_t authority;
        const auth_v3_iot_core_attribution_record_v1_t *resolved = NULL;
        int actual;

        line[strcspn(line, "\r\n")] = '\0';
        if (strcmp(line, "version=1") == 0) {
            saw_version = 1;
            continue;
        }
        if (strncmp(line, "case=", 5u) != 0) continue;

        name = strtok(line + 5u, "|");
        mutation = strtok(NULL, "|");
        expected = strtok(NULL, "|");
        extra = strtok(NULL, "|");
        assert(name != NULL && mutation != NULL && expected != NULL && extra == NULL);
        assert(strcmp(expected, "AUTHORIZATION_MISMATCH") == 0);

        context = context_fixture();
        authority = authority_fixture(&context);
        if (strcmp(mutation, "record_policy_epoch_minus_1") == 0) {
            authority.policy_epoch -= 1u;
        } else if (strcmp(mutation, "record_revocation_epoch_minus_1") == 0) {
            authority.revocation_epoch -= 1u;
        } else {
            fprintf(stderr, "unknown freshness mutation for %s: %s\n", name, mutation);
            assert(0);
        }

        actual = auth_v3_iot_core_attribution_resolve(
            &authority,
            1u,
            authority.credential_reference,
            authority.peer_identity,
            &context,
            &resolved);
        assert(actual == AUTH_V3_IOT_CORE_ATTRIBUTION_AUTHORIZATION_MISMATCH);
        assert(resolved == NULL);
        case_count += 1u;
    }

    fclose(fp);
    assert(saw_version == 1);
    assert(case_count == 2u);
    puts("AUTH v3 iot-core authorization freshness corpus: ok cases=2");
    return 0;
}
