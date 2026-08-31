#include "auth/lineage_replace_verified_commit.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VECTOR_PATH "../rust/test-vectors/replay/lineage-replace-verified-commit-v1.txt"

typedef struct {
    lineage_replace_storage_step_t fail_step;
    bool has_fail_step;
    char trace[192];
} test_adapter_t;

static void fill(uint8_t *out, size_t n, uint8_t v) { memset(out, v, n); }
static const char *step_name(lineage_replace_storage_step_t v) {
    switch (v) {
        case LINEAGE_REPLACE_STORAGE_STEP_PERSIST_PENDING: return "PERSIST_PENDING";
        case LINEAGE_REPLACE_STORAGE_STEP_ACTIVATE_SUCCESSOR: return "ACTIVATE_SUCCESSOR";
        case LINEAGE_REPLACE_STORAGE_STEP_RETIRE_PREDECESSOR: return "RETIRE_PREDECESSOR";
        case LINEAGE_REPLACE_STORAGE_STEP_INVALIDATE_DEPENDENT_STATE: return "INVALIDATE_DEPENDENT_STATE";
        case LINEAGE_REPLACE_STORAGE_STEP_CLEAR_PENDING: return "CLEAR_PENDING";
    }
    assert(false); return "";
}
static lineage_replace_storage_step_t step(const char *v) {
    if (strcmp(v, "PERSIST_PENDING") == 0) return LINEAGE_REPLACE_STORAGE_STEP_PERSIST_PENDING;
    assert(strcmp(v, "ACTIVATE_SUCCESSOR") == 0); return LINEAGE_REPLACE_STORAGE_STEP_ACTIVATE_SUCCESSOR;
}
static bool apply_step(void *context, lineage_replace_storage_step_t current) {
    test_adapter_t *adapter = (test_adapter_t *)context;
    const char *name = step_name(current);
    size_t used = strlen(adapter->trace);
    if (used != 0u) { assert(used + 1u < sizeof(adapter->trace)); adapter->trace[used++] = ','; adapter->trace[used] = '\0'; }
    assert(used + strlen(name) < sizeof(adapter->trace)); strcat(adapter->trace, name);
    return !(adapter->has_fail_step && adapter->fail_step == current);
}
static auth_v3_iot_core_authorization_context_v1_t base_authorization(void) {
    auth_v3_iot_core_authorization_context_v1_t c = {{0},{0},7u,AUTH_V3_IOT_CORE_SCOPE_SECURE_ASSOCIATION,9u,3u,4u};
    fill(c.holder_binding, 32u, 0x11u); fill(c.audience_id, 32u, 0x22u); return c;
}
static auth_v3_iot_core_attribution_record_v1_t base_record(const auth_v3_iot_core_authorization_context_v1_t *c) {
    auth_v3_iot_core_attribution_record_v1_t r = {{0},{0},{0},{0},0u,0u,0u,0u,0u};
    fill(r.credential_reference,32u,0x33u); fill(r.peer_identity,32u,0x44u);
    memcpy(r.holder_binding,c->holder_binding,32u); memcpy(r.audience_id,c->audience_id,32u);
    r.role_policy_id=c->role_policy_id; r.scope_bits=c->scope_bits; r.authorization_generation=c->authorization_generation;
    r.policy_epoch=c->policy_epoch; r.revocation_epoch=c->revocation_epoch; return r;
}
static lineage_replace_facts_t base_facts(void) {
    lineage_replace_facts_t f = {0};
    f.trigger=LINEAGE_REPLACE_TRIGGER_LIFECYCLE; f.predecessor_valid=true; f.successor_valid=true;
    f.context_valid=true; f.freshness_valid=true; f.replay_free=true; f.concurrent_free=true;
    f.rollback_clear=true; f.storage_safe=true; f.dependent_state_safe=true; return f;
}
static lineage_replace_verified_commit_phase_t phase(const char *v) {
    if(strcmp(v,"STORAGE_COMMITTED")==0)return LINEAGE_REPLACE_VERIFIED_COMMIT_STORAGE_COMMITTED;
    if(strcmp(v,"STORAGE_FAILED")==0)return LINEAGE_REPLACE_VERIFIED_COMMIT_STORAGE_FAILED;
    if(strcmp(v,"REJECT_DECISION")==0)return LINEAGE_REPLACE_VERIFIED_COMMIT_REJECT_DECISION;
    assert(strcmp(v,"REJECT_AUTHORIZATION")==0); return LINEAGE_REPLACE_VERIFIED_COMMIT_REJECT_AUTHORIZATION;
}
static lineage_replace_authorization_decision_t auth_expected(const char *v) {
    if(strcmp(v,"AUTHORIZED_REPLACEMENT")==0)return LINEAGE_REPLACE_AUTHORIZED_REPLACEMENT;
    if(strcmp(v,"REJECT_CURRENT_CREDENTIAL_CONTROL")==0)return LINEAGE_REPLACE_REJECT_CURRENT_CREDENTIAL_CONTROL;
    if(strcmp(v,"REJECT_SESSION_AUTHENTICATION")==0)return LINEAGE_REPLACE_REJECT_SESSION_AUTHENTICATION;
    assert(strcmp(v,"REJECT_PRIVILEGE_EXPANSION")==0); return LINEAGE_REPLACE_REJECT_PRIVILEGE_EXPANSION;
}
static lineage_replace_decision_t decision_expected(const char *v) {
    if(strcmp(v,"ACCEPT_SUCCESSOR")==0)return LINEAGE_REPLACE_ACCEPT_SUCCESSOR;
    if(strcmp(v,"REJECT_FRESHNESS")==0)return LINEAGE_REPLACE_REJECT_FRESHNESS;
    if(strcmp(v,"REJECT_REPLAY")==0)return LINEAGE_REPLACE_REJECT_REPLAY;
    assert(strcmp(v,"REJECT_AUTHORITY")==0); return LINEAGE_REPLACE_REJECT_AUTHORITY;
}
static lineage_replace_storage_transaction_result_t storage_expected(const char *v) {
    if(strcmp(v,"COMMITTED")==0)return LINEAGE_REPLACE_STORAGE_TRANSACTION_COMMITTED;
    if(strcmp(v,"FAIL_PENDING")==0)return LINEAGE_REPLACE_STORAGE_TRANSACTION_FAIL_PENDING;
    if(strcmp(v,"FAIL_SUCCESSOR")==0)return LINEAGE_REPLACE_STORAGE_TRANSACTION_FAIL_SUCCESSOR;
    assert(strcmp(v,"REJECT_PLAN")==0); return LINEAGE_REPLACE_STORAGE_TRANSACTION_REJECT_PLAN;
}

int main(void) {
    FILE *fp=fopen(VECTOR_PATH,"r"); char line[768]; unsigned cases=0u; int version=0; assert(fp!=NULL);
    while(fgets(line,sizeof(line),fp)!=NULL) {
        char *f[9]={0}; char *p; unsigned i=0u; uint8_t sid[16], ah[32], eh[32], cb[32];
        auth_v3_context_t context; lineage_replace_session_binding_expectation_t expectation;
        auth_v3_iot_core_authorization_context_v1_t authorization; auth_v3_iot_core_attribution_record_v1_t record;
        lineage_replace_verified_bound_auth_context_evidence_t evidence = {0}; lineage_replace_facts_t facts; test_adapter_t adapter={0}; lineage_replace_verified_commit_result_t result;
        line[strcspn(line,"\r\n")]='\0'; if(strcmp(line,"version=1")==0){version=1;continue;} if(strncmp(line,"case=",5u)!=0)continue;
        p=strtok(line+5u,"|"); while(p!=NULL&&i<9u){f[i++]=p;p=strtok(NULL,"|");} assert(i==9u&&p==NULL);
        fill(sid,sizeof(sid),0x51u); fill(ah,sizeof(ah),0x52u); fill(eh,sizeof(eh),0x53u); fill(cb,sizeof(cb),0x54u);
        memset(&context,0,sizeof(context)); context.protocol_version=3u; context.suite_id=(auth_suite_t)1; context.profile_id=1u;
        memcpy(context.session_id,sid,sizeof(sid)); memcpy(context.authz_context_hash,ah,sizeof(ah)); memcpy(context.critical_extensions_hash,eh,sizeof(eh)); memcpy(context.channel_binding_hash,cb,sizeof(cb));
        memset(&expectation,0,sizeof(expectation)); expectation.auth_completion_verified=true; expectation.expected_protocol_version=3u; expectation.expected_suite_id=(auth_suite_t)1; expectation.expected_profile_id=1u;
        memcpy(expectation.expected_session_id,sid,sizeof(sid)); memcpy(expectation.expected_authz_context_hash,ah,sizeof(ah)); memcpy(expectation.expected_channel_binding_hash,cb,sizeof(cb));
        authorization=base_authorization(); record=base_record(&authorization); facts=base_facts();
        evidence.current_credential_proof.verification_valid=true; memcpy(evidence.current_credential_proof.session_id,sid,sizeof(sid)); fill(evidence.current_credential_proof.subject_reference,32u,0x33u);
        evidence.successor_key_proof.verification_valid=true; memcpy(evidence.successor_key_proof.session_id,sid,sizeof(sid)); fill(evidence.successor_key_proof.subject_reference,32u,0x66u);
        fill(evidence.successor_key_reference,32u,0x66u); fill(evidence.expected_peer_identity,32u,0x44u); fill(evidence.predecessor_credential_reference,32u,0x33u); evidence.requested_successor_scope_bits=AUTH_V3_IOT_CORE_SCOPE_SECURE_ASSOCIATION;
        if(strcmp(f[1],"CURRENT_PROOF")==0)evidence.current_credential_proof.verification_valid=false;
        else if(strcmp(f[1],"SESSION")==0)expectation.auth_completion_verified=false;
        else if(strcmp(f[1],"PRIVILEGE")==0)evidence.requested_successor_scope_bits=2u;
        else assert(strcmp(f[1],"NONE")==0);
        if(strcmp(f[2],"FRESHNESS")==0)facts.freshness_valid=false; else if(strcmp(f[2],"REPLAY")==0)facts.replay_free=false; else assert(strcmp(f[2],"NONE")==0);
        if(strcmp(f[3],"NONE")!=0){adapter.has_fail_step=true;adapter.fail_step=step(f[3]);}
        result=lineage_replace_execute_verified_bound_iot_core_commit(&context,&expectation,&authorization,&record,&evidence,&facts,apply_step,&adapter);
        assert(result.phase==phase(f[4])); assert(result.authorization==auth_expected(f[5])); assert(result.decision==decision_expected(f[6])); assert(result.storage_result==storage_expected(f[7]));
        assert(strcmp(adapter.trace[0]=='\0'?"NONE":adapter.trace,f[8])==0); cases+=1u;
    }
    fclose(fp); assert(version==1&&cases==8u); puts("lineage-replace verified-commit corpus: ok"); return EXIT_SUCCESS;
}
