#include "auth/transport_binding.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#define VECTOR_PATH "../rust/test-vectors/state/transport-binding-v1.txt"

static bool bit(const char *value) {
    assert(value != NULL);
    if (strcmp(value, "0") == 0) return false;
    assert(strcmp(value, "1") == 0);
    return true;
}

static transport_binding_disposition_t disposition(const char *value) {
    if (strcmp(value, "BOUND") == 0) return TRANSPORT_BINDING_BOUND;
    if (strcmp(value, "UNBOUND_ALLOWED") == 0) return TRANSPORT_BINDING_UNBOUND_ALLOWED;
    assert(strcmp(value, "REJECT") == 0);
    return TRANSPORT_BINDING_REJECT;
}

static transport_binding_reason_t reason(const char *value) {
    static const char *const names[] = {
        "CURRENT", "INVALID_FACTS", "TRANSPORT_ADDRESS_AS_IDENTITY",
        "TRANSPORT_METADATA_AS_AUTHORITY", "BINDING_MISSING", "BINDING_NOT_REQUIRED",
        "BINDING_INVALID", "BINDING_STALE", "AUTH_INSTANCE_MISMATCH",
        "PEER_CONTEXT_MISMATCH", "PROFILE_CONTEXT_MISMATCH"};
    size_t i;
    for (i = 0u; i < sizeof(names) / sizeof(names[0]); ++i)
        if (strcmp(value, names[i]) == 0) return (transport_binding_reason_t)i;
    assert(!"unknown reason");
    return TRANSPORT_BINDING_REASON_INVALID_FACTS;
}

int main(void) {
    FILE *fp = fopen(VECTOR_PATH, "r");
    char line[768];
    unsigned cases = 0u;
    assert(fp != NULL);
    while (fgets(line, sizeof(line), fp) != NULL) {
        char *fields[12];
        char *cursor;
        size_t n = 0u;
        transport_binding_facts_t facts;
        transport_binding_decision_t got;
        if (strncmp(line, "case=", 5u) != 0) continue;
        cursor = line + 5;
        cursor[strcspn(cursor, "\r\n")] = '\0';
        fields[n++] = strtok(cursor, "|");
        while (n < 12u && (fields[n] = strtok(NULL, "|")) != NULL) ++n;
        assert(n == 12u);
        facts = (transport_binding_facts_t){
            bit(fields[1]), bit(fields[2]), bit(fields[3]), bit(fields[4]), bit(fields[5]),
            bit(fields[6]), bit(fields[7]), bit(fields[8]), bit(fields[9])};
        got = transport_binding_classify(&facts);
        assert(got.disposition == disposition(fields[10]));
        assert(got.reason == reason(fields[11]));
        ++cases;
    }
    fclose(fp);
    assert(cases == 12u);
    puts("transport binding corpus: ok");
    return 0;
}
