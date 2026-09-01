#include "auth/enrollment_grant.h"
#include <stdio.h>
#include <string.h>
static const char *reason_name(zk_enrollment_grant_reason r) {
    static const char *n[]={"CURRENT","ROLLBACK_SUSPECTED","NORMAL_AUTH_FORBIDDEN","EXPLICIT_ENROLL_REQUIRED","COMMISSIONER_UNAUTHENTICATED","COMMISSIONER_UNAUTHORIZED","COMMISSIONER_REVOKED","SUBJECT_POSSESSION_MISSING","AUTHORITY_ESCALATION","SCOPE_UNBOUNDED","AUDIENCE_UNBOUND","DEPLOYMENT_UNBOUND","VALIDITY_UNBOUNDED","EPOCH_STALE","REVOCATION_STALE","LINEAGE_STALE","DELEGATION_DEPTH_EXCEEDED"};
    return n[(int)r];
}
int main(void) {
    FILE *fp=fopen("../rust/test-vectors/state/enrollment-grant-v1.txt","r");
    char line[1024], name[64], action[16], reason[64]; int v[16], count=0;
    if (!fp) { perror("enrollment vector"); return 2; }
    while (fgets(line,sizeof line,fp)) {
        if (line[0]=='#' || line[0]=='\n') continue;
        int n=sscanf(line,"%63s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %15s %63s", name,&v[0],&v[1],&v[2],&v[3],&v[4],&v[5],&v[6],&v[7],&v[8],&v[9],&v[10],&v[11],&v[12],&v[13],&v[14],&v[15],action,reason);
        if (n!=19) { fprintf(stderr,"bad vector line: %s",line); return 3; }
        zk_enrollment_grant_facts f={v[0],v[1],v[2],v[3],v[4],v[5],v[6],v[7],v[8],v[9],v[10],v[11],v[12],v[13],v[14],v[15]};
        zk_enrollment_grant_decision d=zk_classify_enrollment_grant(&f);
        const char *got_action=d.action==ZK_ENROLLMENT_GRANT_ISSUE?"ISSUE":"DENY";
        if (strcmp(got_action,action)||strcmp(reason_name(d.reason),reason)) { fprintf(stderr,"%s: mismatch\n",name); return 4; }
        count++;
    }
    fclose(fp);
    printf("enrollment grant corpus: ok cases=%d\n",count);
    return count==17?0:5;
}
