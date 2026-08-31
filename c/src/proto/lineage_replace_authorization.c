#include "auth/lineage_replace_authorization.h"

#include <stddef.h>

lineage_replace_authorization_decision_t lineage_replace_classify_authorization(
    const lineage_replace_authorization_facts_t *facts) {
    if (facts == NULL || !facts->current_credential_control_valid)
        return LINEAGE_REPLACE_REJECT_CURRENT_CREDENTIAL_CONTROL;
    if (!facts->successor_key_control_valid)
        return LINEAGE_REPLACE_REJECT_SUCCESSOR_KEY_CONTROL;
    if (!facts->current_session_authenticated)
        return LINEAGE_REPLACE_REJECT_SESSION_AUTHENTICATION;
    if (!facts->current_session_authorized)
        return LINEAGE_REPLACE_REJECT_SESSION_AUTHORIZATION;
    if (!facts->current_context_bound)
        return LINEAGE_REPLACE_REJECT_CONTEXT_BINDING;
    if (!facts->predecessor_binding_current)
        return LINEAGE_REPLACE_REJECT_PREDECESSOR_BINDING;
    if (!facts->successor_scope_within_authorized_scope)
        return LINEAGE_REPLACE_REJECT_PRIVILEGE_EXPANSION;
    return LINEAGE_REPLACE_AUTHORIZED_REPLACEMENT;
}
