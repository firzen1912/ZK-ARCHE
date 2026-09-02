#include "auth/transport_continuation.h"

#include <stddef.h>

static transport_continuation_decision_t decision(
    transport_continuation_action_t action,
    transport_continuation_reason_t reason) {
    transport_continuation_decision_t out = {action, reason};
    return out;
}

transport_continuation_decision_t transport_continuation_classify(
    const transport_continuation_facts_t *facts) {
    if (facts == NULL)
        return decision(TRANSPORT_CONTINUATION_REJECT,
                        TRANSPORT_CONTINUATION_REASON_INVALID_FACTS);
    if (facts->transport_address_as_identity)
        return decision(TRANSPORT_CONTINUATION_REJECT,
                        TRANSPORT_CONTINUATION_REASON_TRANSPORT_ADDRESS_AS_IDENTITY);
    if (facts->transport_metadata_as_authority)
        return decision(TRANSPORT_CONTINUATION_REJECT,
                        TRANSPORT_CONTINUATION_REASON_TRANSPORT_METADATA_AS_AUTHORITY);
    if (facts->association_invalidated)
        return decision(TRANSPORT_CONTINUATION_REJECT,
                        TRANSPORT_CONTINUATION_REASON_ASSOCIATION_INVALIDATED);
    if (!facts->replay_continuity_current)
        return decision(TRANSPORT_CONTINUATION_REJECT,
                        TRANSPORT_CONTINUATION_REASON_REPLAY_CONTINUITY_STALE);
    if (!facts->authenticated_peer_context_match)
        return decision(TRANSPORT_CONTINUATION_REJECT,
                        TRANSPORT_CONTINUATION_REASON_PEER_CONTEXT_MISMATCH);
    if (!facts->authenticated_profile_context_match)
        return decision(TRANSPORT_CONTINUATION_FULL_AUTH_REQUIRED,
                        TRANSPORT_CONTINUATION_REASON_PROFILE_CONTEXT_MISMATCH);
    if (facts->binding_required && !facts->binding_valid)
        return decision(TRANSPORT_CONTINUATION_REJECT,
                        TRANSPORT_CONTINUATION_REASON_BINDING_INVALID);
    if (!facts->resumption_authorized)
        return decision(TRANSPORT_CONTINUATION_FULL_AUTH_REQUIRED,
                        TRANSPORT_CONTINUATION_REASON_RESUMPTION_NOT_AUTHORIZED);
    if (!facts->continuation_requested)
        return decision(TRANSPORT_CONTINUATION_FULL_AUTH_REQUIRED,
                        TRANSPORT_CONTINUATION_REASON_CONTINUATION_NOT_REQUESTED);

    (void)facts->transport_route_changed;
    (void)facts->transport_connection_changed;

    return decision(TRANSPORT_CONTINUATION_CONTINUE,
                    TRANSPORT_CONTINUATION_REASON_CURRENT);
}
