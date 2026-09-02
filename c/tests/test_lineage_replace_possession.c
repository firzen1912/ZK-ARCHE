#include "auth/lineage_replace_possession.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define VECTOR_PATH "../rust/test-vectors/replay/lineage-replace-possession-v1.txt"
static bool bit(const char *v){ if(strcmp(v,"1")==0)return true; assert(strcmp(v,"0")==0); return false; }
static void fill(uint8_t *out,size_t n,uint8_t v){ memset(out,v,n); }
static lineage_replace_possession_decision_t expected(const char *v){
 if(strcmp(v,"VERIFIED")==0)return LINEAGE_REPLACE_POSSESSION_VERIFIED;
 if(strcmp(v,"REJECT_CURRENT_CREDENTIAL_CONTROL")==0)return LINEAGE_REPLACE_POSSESSION_REJECT_CURRENT_CREDENTIAL_CONTROL;
 assert(strcmp(v,"REJECT_SUCCESSOR_KEY_CONTROL")==0); return LINEAGE_REPLACE_POSSESSION_REJECT_SUCCESSOR_KEY_CONTROL;
}
int main(void){ FILE *fp=fopen(VECTOR_PATH,"r"); char line[512]; unsigned cases=0u; int version=0; assert(fp!=NULL);
 while(fgets(line,sizeof(line),fp)!=NULL){ char *f[10]={0}; char *p; unsigned i=0u; uint8_t session[16], predecessor[32], successor_ref[32]; lineage_replace_verified_lifecycle_possession_proof_t current={0}, successor={0};
  line[strcspn(line,"\r\n")]='\0'; if(strcmp(line,"version=1")==0){version=1;continue;} if(strncmp(line,"case=",5u)!=0)continue;
  p=strtok(line+5u,"|"); while(p!=NULL&&i<10u){f[i++]=p;p=strtok(NULL,"|");} assert(i==10u&&p==NULL);
  fill(session,sizeof(session),0x51u); fill(predecessor,sizeof(predecessor),0x33u); fill(successor_ref,sizeof(successor_ref),0x66u);
  current.verification_valid=bit(f[2]); memcpy(current.session_id,session,sizeof(session)); memcpy(current.subject_reference,predecessor,sizeof(predecessor));
  successor.verification_valid=bit(f[6]); memcpy(successor.session_id,session,sizeof(session)); memcpy(successor.subject_reference,successor_ref,sizeof(successor_ref));
  if (!bit(f[3])) current.session_id[0] ^= 1u;
  if (!bit(f[4])) current.subject_reference[0] ^= 1u;
  if (!bit(f[7])) successor.session_id[0] ^= 1u;
  if (!bit(f[8])) successor.subject_reference[0] ^= 1u;
  assert(lineage_replace_classify_possession(bit(f[1])?&current:NULL,bit(f[5])?&successor:NULL,session,predecessor,successor_ref)==expected(f[9])); cases+=1u;
 }
 fclose(fp); assert(version==1&&cases==10u); puts("lineage-replace possession corpus: ok"); return EXIT_SUCCESS; }
