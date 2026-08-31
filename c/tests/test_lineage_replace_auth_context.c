#include "auth/lineage_replace_auth_context.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define VECTOR_PATH "../rust/test-vectors/replay/lineage-replace-auth-context-v1.txt"
static bool bit(const char *v) { if (strcmp(v, "1") == 0) return true; assert(strcmp(v, "0") == 0); return false; }
static void fill(uint8_t out[32], uint8_t value) { memset(out, value, 32u); }
static lineage_replace_authorization_decision_t expected(const char *v) {
    if (strcmp(v, "AUTHORIZED_REPLACEMENT") == 0) return LINEAGE_REPLACE_AUTHORIZED_REPLACEMENT;
    if (strcmp(v, "REJECT_CURRENT_CREDENTIAL_CONTROL") == 0) return LINEAGE_REPLACE_REJECT_CURRENT_CREDENTIAL_CONTROL;
    if (strcmp(v, "REJECT_SUCCESSOR_KEY_CONTROL") == 0) return LINEAGE_REPLACE_REJECT_SUCCESSOR_KEY_CONTROL;
    if (strcmp(v, "REJECT_SESSION_AUTHENTICATION") == 0) return LINEAGE_REPLACE_REJECT_SESSION_AUTHENTICATION;
    if (strcmp(v, "REJECT_SESSION_AUTHORIZATION") == 0) return LINEAGE_REPLACE_REJECT_SESSION_AUTHORIZATION;
    if (strcmp(v, "REJECT_CONTEXT_BINDING") == 0) return LINEAGE_REPLACE_REJECT_CONTEXT_BINDING;
    if (strcmp(v, "REJECT_PREDECESSOR_BINDING") == 0) return LINEAGE_REPLACE_REJECT_PREDECESSOR_BINDING;
    assert(strcmp(v, "REJECT_PRIVILEGE_EXPANSION") == 0); return LINEAGE_REPLACE_REJECT_PRIVILEGE_EXPANSION;
}
static auth_v3_iot_core_authorization_context_v1_t base_context(void) {
    auth_v3_iot_core_authorization_context_v1_t c = {{0}, {0}, 7u, AUTH_V3_IOT_CORE_SCOPE_SECURE_ASSOCIATION, 9u, 3u, 4u};
    fill(c.holder_binding, 0x11u); fill(c.audience_id, 0x22u); return c;
}
static auth_v3_iot_core_attribution_record_v1_t base_record(const auth_v3_iot_core_authorization_context_v1_t *c) {
    auth_v3_iot_core_attribution_record_v1_t r = {{0}, {0}, {0}, {0}, 0u, 0u, 0u, 0u, 0u};
    fill(r.credential_reference, 0x33u); fill(r.peer_identity, 0x44u);
    memcpy(r.holder_binding, c->holder_binding, 32u); memcpy(r.audience_id, c->audience_id, 32u);
    r.role_policy_id = c->role_policy_id; r.scope_bits = c->scope_bits;
    r.authorization_generation = c->authorization_generation; r.policy_epoch = c->policy_epoch;
    r.revocation_epoch = c->revocation_epoch; return r;
}
static lineage_replace_auth_context_evidence_t base_evidence(void) {
    lineage_replace_auth_context_evidence_t e = {true, true, true, {0}, {0}, AUTH_V3_IOT_CORE_SCOPE_SECURE_ASSOCIATION};
    fill(e.expected_peer_identity, 0x44u); fill(e.predecessor_credential_reference, 0x33u); return e;
}
int main(void) {
    FILE *fp = fopen(VECTOR_PATH, "r"); char line[512]; unsigned cases = 0u; int version = 0; assert(fp != NULL);
    while (fgets(line, sizeof(line), fp) != NULL) {
        char *f[10] = {0}; char *p; unsigned i = 0u;
        auth_v3_iot_core_authorization_context_v1_t c; auth_v3_iot_core_attribution_record_v1_t r;
        lineage_replace_auth_context_evidence_t e;
        line[strcspn(line, "\r\n")] = '\0'; if (strcmp(line, "version=1") == 0) { version = 1; continue; }
        if (strncmp(line, "case=", 5u) != 0) continue;
        p = strtok(line + 5u, "|"); while (p != NULL && i < 10u) { f[i++] = p; p = strtok(NULL, "|"); } assert(i == 10u && p == NULL);
        c = base_context(); r = base_record(&c); e = base_evidence();
        e.current_credential_control_valid = bit(f[1]); e.successor_key_control_valid = bit(f[2]); e.current_session_authenticated = bit(f[3]);
        if (!bit(f[4])) c.authorization_generation = 0u;
        if (!bit(f[5])) r.policy_epoch += 1u;
        if (!bit(f[6])) r.peer_identity[0] ^= 1u;
        if (!bit(f[7])) e.predecessor_credential_reference[0] ^= 1u;
        if (!bit(f[8])) e.requested_successor_scope_bits = 2u;
        assert(lineage_replace_authorization_from_iot_core(&c, &r, &e) == expected(f[9])); cases += 1u;
    }
    fclose(fp); assert(version == 1 && cases == 12u);
    { auth_v3_iot_core_authorization_context_v1_t c = base_context(); auth_v3_iot_core_attribution_record_v1_t r = base_record(&c); lineage_replace_auth_context_evidence_t e = base_evidence();
      assert(lineage_replace_authorization_from_iot_core(NULL, &r, &e) == LINEAGE_REPLACE_REJECT_SESSION_AUTHORIZATION);
      assert(lineage_replace_authorization_from_iot_core(&c, NULL, &e) == LINEAGE_REPLACE_REJECT_SESSION_AUTHORIZATION);
      assert(lineage_replace_authorization_from_iot_core(&c, &r, NULL) == LINEAGE_REPLACE_REJECT_CURRENT_CREDENTIAL_CONTROL); }
    puts("lineage-replace auth-context corpus: ok"); return EXIT_SUCCESS;
}
