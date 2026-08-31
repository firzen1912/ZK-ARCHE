#include "auth/lineage_replace_reconciliation_transition.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define VECTOR_PATH "../rust/test-vectors/replay/lineage-replace-reconciliation-transition-v1.txt"
static lineage_replace_reconciliation_decision_t pair(const char *v){
 if(!strcmp(v,"PAIR_SUCCESSOR_READY")) return LINEAGE_REPLACE_PAIR_SUCCESSOR_READY;
 if(!strcmp(v,"PAIR_PREDECESSOR_READY")) return LINEAGE_REPLACE_PAIR_PREDECESSOR_READY;
 if(!strcmp(v,"RECONCILIATION_REQUIRED")) return LINEAGE_REPLACE_RECONCILIATION_REQUIRED;
 if(!strcmp(v,"PAIR_CONTINUITY_BROKEN")) return LINEAGE_REPLACE_PAIR_CONTINUITY_BROKEN;
 assert(!strcmp(v,"SUCCESSOR_DIVERGENCE")); return LINEAGE_REPLACE_SUCCESSOR_DIVERGENCE;
}
static lineage_replace_reconciliation_transition_t expected(const char *v){
 if(!strcmp(v,"HOLD")) return LINEAGE_REPLACE_RECONCILIATION_HOLD;
 if(!strcmp(v,"ACTIVATE_SUCCESSOR")) return LINEAGE_REPLACE_RECONCILIATION_ACTIVATE_SUCCESSOR;
 if(!strcmp(v,"RESUME_PREDECESSOR")) return LINEAGE_REPLACE_RECONCILIATION_RESUME_PREDECESSOR;
 if(!strcmp(v,"REJECT_DIVERGENCE")) return LINEAGE_REPLACE_RECONCILIATION_REJECT_DIVERGENCE;
 assert(!strcmp(v,"CONTINUITY_BROKEN")); return LINEAGE_REPLACE_RECONCILIATION_CONTINUITY_BROKEN;
}
static bool bit(const char *v){ if(!strcmp(v,"0")) return false; assert(!strcmp(v,"1")); return true; }
static lineage_replace_attempt_evidence_decision_t evidence(const char *v){ return bit(v) ? LINEAGE_REPLACE_ATTEMPT_EVIDENCE_FRESH_CURRENT_ATTEMPT : LINEAGE_REPLACE_ATTEMPT_EVIDENCE_MISSING_CURRENT_ATTEMPT; }
int main(void){
 FILE *fp=fopen(VECTOR_PATH,"r"); char line[512]; unsigned n=0; int ver=0; assert(fp);
 while(fgets(line,sizeof(line),fp)){ char *f[6]={0},*p; unsigned i=0; lineage_replace_reconciliation_transition_facts_t facts;
   line[strcspn(line,"\r\n")]='\0'; if(!strcmp(line,"version=1")){ver=1;continue;} if(strncmp(line,"case=",5u)) continue;
   p=strtok(line+5u,"|"); while(p && i<6u){f[i++]=p;p=strtok(NULL,"|");} assert(i==6u && p==NULL);
   facts=(lineage_replace_reconciliation_transition_facts_t){pair(f[1]),pair(f[2]),evidence(f[3]),bit(f[4])};
   assert(lineage_replace_classify_reconciliation_transition(&facts)==expected(f[5])); n++;
 }
 fclose(fp); assert(ver==1 && n==12u);
 assert(lineage_replace_classify_reconciliation_transition(NULL)==LINEAGE_REPLACE_RECONCILIATION_CONTINUITY_BROKEN);
 puts("lineage-replace reconciliation transition corpus: ok"); return EXIT_SUCCESS;
}
