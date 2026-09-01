#include "auth/transport_binding.h"

#include <stddef.h>

static transport_binding_decision_t decision(transport_binding_disposition_t disposition,
                                              transport_binding_reason_t reason) {
    transport_binding_decision_t out = {disposition, reason};
    return out;
}

transport_binding_decision_t transport_binding_classify(const transport_binding_facts_t *facts) {
    if (facts == NULL)
        return decision(TRANSPORT_BINDING_REJECT, TRANSPORT_BINDING_REASON_INVALID_FACTS);
    if (facts->transport_address_as_identity)
        return decision(TRANSPORT_BINDING_REJECT,
                        TRANSPORT_BINDING_REASON_TRANSPORT_ADDRESS_AS_IDENTITY);
    if (facts->transport_metadata_as_authority)
        return decision(TRANSPORT_BINDING_REJECT,
                        TRANSPORT_BINDING_REASON_TRANSPORT_METADATA_AS_AUTHORITY);
    if (!facts->binding_present) {
        if (facts->profile_requires_binding)
            return decision(TRANSPORT_BINDING_REJECT, TRANSPORT_BINDING_REASON_BINDING_MISSING);
        return decision(TRANSPORT_BINDING_UNBOUND_ALLOWED,
                        TRANSPORT_BINDING_REASON_BINDING_NOT_REQUIRED);
    }
    if (!facts->binding_integrity_valid)
        return decision(TRANSPORT_BINDING_REJECT, TRANSPORT_BINDING_REASON_BINDING_INVALID);
    if (!facts->binding_fresh)
        return decision(TRANSPORT_BINDING_REJECT, TRANSPORT_BINDING_REASON_BINDING_STALE);
    if (!facts->auth_instance_match)
        return decision(TRANSPORT_BINDING_REJECT,
                        TRANSPORT_BINDING_REASON_AUTH_INSTANCE_MISMATCH);
    if (!facts->peer_context_match)
        return decision(TRANSPORT_BINDING_REJECT,
                        TRANSPORT_BINDING_REASON_PEER_CONTEXT_MISMATCH);
    if (!facts->profile_context_match)
        return decision(TRANSPORT_BINDING_REJECT,
                        TRANSPORT_BINDING_REASON_PROFILE_CONTEXT_MISMATCH);
    return decision(TRANSPORT_BINDING_BOUND, TRANSPORT_BINDING_REASON_CURRENT);
}
