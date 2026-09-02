#include "auth/association_admission.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VECTOR_PATH "../rust/test-vectors/p2p/common-contract-c-lifecycle-v1.txt"

static bool bit(const char *v){assert(v);if(strcmp(v,"true")==0)return true;assert(strcmp(v,"false")==0);return false;}
static bool known_peer(const char *v){return strcmp(v,"mcu-core")==0||strcmp(v,"linux-edge")==0;}

int main(void){
 FILE *fp=fopen(VECTOR_PATH,"r"); char line[1024]; unsigned cases=0u, established=0u, failed=0u; assert(fp);
 while(fgets(line,sizeof line,fp)){
  char *f[17]; size_t i; association_admission_facts_t facts; association_admission_decision_t got; bool floor_ok; bool infrastructure;
  line[strcspn(line,"\r\n")]='\0'; if(line[0]=='#'||strncmp(line,"XC-",3)!=0)continue;
  f[0]=strtok(line,"|"); for(i=1u;i<17u;++i)f[i]=strtok(NULL,"|"); assert(f[16]&&strtok(NULL,"|")==NULL);
  assert(known_peer(f[1])&&known_peer(f[2]));
  infrastructure=bit(f[3]); (void)infrastructure;
  facts=(association_admission_facts_t){
   bit(f[4]),bit(f[5]),bit(f[6]),bit(f[7]),bit(f[8]),bit(f[9]),bit(f[10]),bit(f[11]),
   bit(f[13]),bit(f[14]),false,bit(f[15])
  };
  floor_ok=bit(f[12]);
  got=association_admission_classify(&facts);
  if(!floor_ok) got.action=ASSOCIATION_ADMISSION_FAIL_CLOSED;
  if(strcmp(f[16],"ESTABLISH")==0){assert(got.action==ASSOCIATION_ADMISSION_ESTABLISH);established++;}
  else{assert(strcmp(f[16],"FAIL_CLOSED")==0);assert(got.action==ASSOCIATION_ADMISSION_FAIL_CLOSED);failed++;}
  cases++;
 }
 fclose(fp);
 assert(cases==16u); assert(established==5u); assert(failed==11u);
 puts("p2p common-contract C lifecycle qualification: ok cases=16 establish=5 fail_closed=11");
 return EXIT_SUCCESS;
}
