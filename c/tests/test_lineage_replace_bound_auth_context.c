#include "auth/lineage_replace_bound_auth_context.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define VECTOR_PATH "../rust/test-vectors/replay/lineage-replace-bound-auth-context-v1.txt"
static bool bit(const char *v){ if(strcmp(v,"1")==0)return true; assert(strcmp(v,"0")==0); return false; }
static void fill(uint8_t *out,size_t n,uint8_t v){ memset(out,v,n); }
static lineage_replace_authorization_decision_t expected(const char *v){
 if(strcmp(v,"AUTHORIZED_REPLACEMENT")==0)return LINEAGE_REPLACE_AUTHORIZED_REPLACEMENT;
 if(strcmp(v,"REJECT_CURRENT_CREDENTIAL_CONTROL")==0)return LINEAGE_REPLACE_REJECT_CURRENT_CREDENTIAL_CONTROL;
 if(strcmp(v,"REJECT_SUCCESSOR_KEY_CONTROL")==0)return LINEAGE_REPLACE_REJECT_SUCCESSOR_KEY_CONTROL;
 if(strcmp(v,"REJECT_SESSION_AUTHENTICATION")==0)return LINEAGE_REPLACE_REJECT_SESSION_AUTHENTICATION;
 if(strcmp(v,"REJECT_SESSION_AUTHORIZATION")==0)return LINEAGE_REPLACE_REJECT_SESSION_AUTHORIZATION;
 if(strcmp(v,"REJECT_PREDECESSOR_BINDING")==0)return LINEAGE_REPLACE_REJECT_PREDECESSOR_BINDING;
 assert(strcmp(v,"REJECT_PRIVILEGE_EXPANSION")==0); return LINEAGE_REPLACE_REJECT_PRIVILEGE_EXPANSION;
}
static auth_v3_iot_core_authorization_context_v1_t base_authorization(void){ auth_v3_iot_core_authorization_context_v1_t c={{0},{0},7u,AUTH_V3_IOT_CORE_SCOPE_SECURE_ASSOCIATION,9u,3u,4u}; fill(c.holder_binding,32u,0x11u); fill(c.audience_id,32u,0x22u); return c; }
static auth_v3_iot_core_attribution_record_v1_t base_record(const auth_v3_iot_core_authorization_context_v1_t *c){ auth_v3_iot_core_attribution_record_v1_t r={{0},{0},{0},{0},0u,0u,0u,0u,0u}; fill(r.credential_reference,32u,0x33u); fill(r.peer_identity,32u,0x44u); memcpy(r.holder_binding,c->holder_binding,32u); memcpy(r.audience_id,c->audience_id,32u); r.role_policy_id=c->role_policy_id; r.scope_bits=c->scope_bits; r.authorization_generation=c->authorization_generation; r.policy_epoch=c->policy_epoch; r.revocation_epoch=c->revocation_epoch; return r; }
static lineage_replace_bound_auth_context_evidence_t base_evidence(void){ lineage_replace_bound_auth_context_evidence_t e={true,true,{0},{0},AUTH_V3_IOT_CORE_SCOPE_SECURE_ASSOCIATION}; fill(e.expected_peer_identity,32u,0x44u); fill(e.predecessor_credential_reference,32u,0x33u); return e; }
int main(void){ FILE *fp=fopen(VECTOR_PATH,"r"); char line[512]; unsigned cases=0u; int version=0; assert(fp!=NULL);
 while(fgets(line,sizeof(line),fp)!=NULL){ char *f[11]={0}; char *p; unsigned i=0u; auth_v3_iot_core_authorization_context_v1_t authorization; auth_v3_iot_core_attribution_record_v1_t record; lineage_replace_bound_auth_context_evidence_t evidence; auth_v3_context_t context; lineage_replace_session_binding_expectation_t expectation;
  line[strcspn(line,"\r\n")]='\0'; if(strcmp(line,"version=1")==0){version=1;continue;} if(strncmp(line,"case=",5u)!=0)continue;
  p=strtok(line+5u,"|"); while(p!=NULL&&i<11u){f[i++]=p;p=strtok(NULL,"|");} assert(i==11u&&p==NULL);
  authorization=base_authorization(); record=base_record(&authorization); evidence=base_evidence(); memset(&context,0,sizeof(context)); memset(&expectation,0,sizeof(expectation));
  context.protocol_version=3u; context.suite_id=(auth_suite_t)1; context.profile_id=1u; fill(context.session_id,sizeof(context.session_id),0x51u); fill(context.authz_context_hash,sizeof(context.authz_context_hash),0x52u); fill(context.channel_binding_hash,sizeof(context.channel_binding_hash),0x54u);
  expectation.auth_completion_verified=bit(f[3]); expectation.expected_protocol_version=3u; expectation.expected_suite_id=(auth_suite_t)1; expectation.expected_profile_id=1u; memcpy(expectation.expected_session_id,context.session_id,sizeof(context.session_id)); memcpy(expectation.expected_authz_context_hash,context.authz_context_hash,sizeof(context.authz_context_hash)); memcpy(expectation.expected_channel_binding_hash,context.channel_binding_hash,sizeof(context.channel_binding_hash));
  evidence.current_credential_control_valid=bit(f[1]); evidence.successor_key_control_valid=bit(f[2]); if(!bit(f[4]))expectation.expected_session_id[0]^=1u; if(!bit(f[5]))expectation.expected_authz_context_hash[0]^=1u; if(!bit(f[6]))expectation.expected_channel_binding_hash[0]^=1u; if(!bit(f[7]))record.policy_epoch+=1u; if(!bit(f[8]))evidence.predecessor_credential_reference[0]^=1u; if(!bit(f[9]))evidence.requested_successor_scope_bits=2u;
  assert(lineage_replace_authorization_from_bound_iot_core(&context,&expectation,&authorization,&record,&evidence)==expected(f[10])); cases+=1u;
 }
 fclose(fp); assert(version==1&&cases==12u); puts("lineage-replace bound-auth-context corpus: ok"); return EXIT_SUCCESS; }
