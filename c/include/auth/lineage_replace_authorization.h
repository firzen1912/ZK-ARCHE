#ifndef AUTH_LINEAGE_REPLACE_AUTHORIZATION_H
#define AUTH_LINEAGE_REPLACE_AUTHORIZATION_H

#include <stdbool.h>

typedef enum {
    LINEAGE_REPLACE_AUTHORIZED_REPLACEMENT = 0,
    LINEAGE_REPLACE_REJECT_CURRENT_CREDENTIAL_CONTROL,
    LINEAGE_REPLACE_REJECT_SUCCESSOR_KEY_CONTROL,
    LINEAGE_REPLACE_REJECT_SESSION_AUTHENTICATION,
    LINEAGE_REPLACE_REJECT_SESSION_AUTHORIZATION,
    LINEAGE_REPLACE_REJECT_CONTEXT_BINDING,
    LINEAGE_REPLACE_REJECT_PREDECESSOR_BINDING,
    LINEAGE_REPLACE_REJECT_PRIVILEGE_EXPANSION
} lineage_replace_authorization_decision_t;

typedef struct {
    bool current_credential_control_valid;
    bool successor_key_control_valid;
    bool current_session_authenticated;
    bool current_session_authorized;
    bool current_context_bound;
    bool predecessor_binding_current;
    bool successor_scope_within_authorized_scope;
} lineage_replace_authorization_facts_t;

lineage_replace_authorization_decision_t lineage_replace_classify_authorization(
    const lineage_replace_authorization_facts_t *facts);

#endif
