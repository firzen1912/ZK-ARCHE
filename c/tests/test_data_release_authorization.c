#include "auth/data_release_authorization.h"
#include <assert.h>
#include <stdio.h>
int main(void) {
    data_release_facts_t f = {1,1,1,1,0,1,1,1,1,1,1,1,0,0,0};
    data_release_decision_t d = data_release_authorization_classify(&f);
    assert(d.action == DATA_RELEASE_ACTION_RELEASE);
    f.purpose_match = false;
    d = data_release_authorization_classify(&f);
    assert(d.action == DATA_RELEASE_ACTION_DENY && d.reason == DATA_RELEASE_REASON_PURPOSE_MISMATCH);
    f.purpose_match = true; f.authenticated = false;
    d = data_release_authorization_classify(&f);
    assert(d.action == DATA_RELEASE_ACTION_FRESH_AUTH_REQUIRED);
    puts("data release authorization: ok");
    return 0;
}
