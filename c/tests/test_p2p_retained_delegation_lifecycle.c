#include "auth/p2p_delegation.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define VECTOR_PATH "../rust/test-vectors/p2p/retained-delegation-lifecycle-v1.txt"

static p2p_delegation_facts_t baseline(void){return(p2p_delegation_facts_t){true,true,true,true,true,true,true,true,true,true,true,true,true,false,true,true,false,false,false};}
static p2p_delegation_reason_t reason(const char *v){
 if(strcmp(v,"AUTHORIZATION_GENERATION_STALE")==0)return P2P_DELEGATION_REASON_AUTHORIZATION_GENERATION_STALE;
 if(strcmp(v,"REVOCATION_STALE")==0)return P2P_DELEGATION_REASON_REVOCATION_STALE;
 if(strcmp(v,"REVOKED")==0)return P2P_DELEGATION_REASON_REVOKED;
 if(strcmp(v,"LINEAGE_STALE")==0)return P2P_DELEGATION_REASON_LINEAGE_STALE;
 if(strcmp(v,"ISSUER_TRUST_NOT_LOCAL")==0)return P2P_DELEGATION_REASON_ISSUER_TRUST_NOT_LOCAL;
 if(strcmp(v,"EPOCH_STALE")==0)return P2P_DELEGATION_REASON_EPOCH_STALE;
 assert(strcmp(v,"ROLLBACK_SUSPECTED")==0);return P2P_DELEGATION_REASON_ROLLBACK_SUSPECTED;
}
int main(void){FILE *fp=fopen(VECTOR_PATH,"r");char line[512];unsigned cases=0u;assert(fp!=NULL);
 while(fgets(line,sizeof line,fp)!=NULL){char *f[7];size_t i;p2p_delegation_facts_t facts;p2p_delegation_decision_t got;
  line[strcspn(line,"\r\n")]='\0';if(strncmp(line,"case=",5u)!=0)continue;f[0]=strtok(line+5u,"|");for(i=1u;i<7u;++i)f[i]=strtok(NULL,"|");assert(f[6]!=NULL&&strtok(NULL,"|")==NULL);
  assert(strcmp(f[1],"mcu-core")==0||strcmp(f[1],"mcu-plus")==0||strcmp(f[1],"linux-edge")==0||strcmp(f[1],"accelerated-edge")==0);
  assert(strcmp(f[2],"mcu-core")==0||strcmp(f[2],"mcu-plus")==0||strcmp(f[2],"linux-edge")==0||strcmp(f[2],"accelerated-edge")==0);assert(strcmp(f[3],"0")==0||strcmp(f[3],"1")==0);
  facts=baseline();got=p2p_delegation_classify(&facts);assert(got.action==P2P_DELEGATION_ACCEPT&&got.reason==P2P_DELEGATION_REASON_CURRENT);
  if(strcmp(f[4],"authorization_generation_stale")==0)facts.authorization_generation_current=false;
  else if(strcmp(f[4],"revocation_stale")==0)facts.revocation_current=false;else if(strcmp(f[4],"revoked")==0)facts.explicitly_revoked=true;
  else if(strcmp(f[4],"lineage_stale")==0)facts.lineage_current=false;else if(strcmp(f[4],"issuer_trust_not_local")==0)facts.issuer_trust_local=false;
  else if(strcmp(f[4],"epoch_stale")==0)facts.epoch_current=false;else{assert(strcmp(f[4],"rollback_suspected")==0);facts.rollback_suspected=true;}
  got=p2p_delegation_classify(&facts);assert(strcmp(f[5],"DENY")==0);assert(got.action==P2P_DELEGATION_DENY);assert(got.reason==reason(f[6]));++cases;
 }fclose(fp);assert(cases==10u);puts("p2p retained delegation lifecycle v1: ok cases=10");return EXIT_SUCCESS;}
