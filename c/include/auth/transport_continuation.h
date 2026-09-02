#ifndef AUTH_TRANSPORT_CONTINUATION_H
#define AUTH_TRANSPORT_CONTINUATION_H

#include <stdbool.h>

typedef struct {
    bool continuation_requested;
    bool authenticated_peer_context_match;
    bool authenticated_profile_context_match;
    bool binding_required;
    bool binding_valid;
    bool resumption_authorized;
    bool replay_continuity_current;
    bool usage_counter_continuity_current;
    bool authorization_generation_current;
    bool association_invalidated;
    bool transport_route_changed;
    bool transport_connection_changed;
    bool transport_address_as_identity;
    bool transport_metadata_as_authority;
} transport_continuation_facts_t;

typedef enum {
    TRANSPORT_CONTINUATION_CONTINUE = 0,
    TRANSPORT_CONTINUATION_FULL_AUTH_REQUIRED = 1,
    TRANSPORT_CONTINUATION_REJECT = 2
} transport_continuation_action_t;

typedef enum {
    TRANSPORT_CONTINUATION_REASON_CURRENT = 0,
    TRANSPORT_CONTINUATION_REASON_INVALID_FACTS = 1,
    TRANSPORT_CONTINUATION_REASON_TRANSPORT_ADDRESS_AS_IDENTITY = 2,
    TRANSPORT_CONTINUATION_REASON_TRANSPORT_METADATA_AS_AUTHORITY = 3,
    TRANSPORT_CONTINUATION_REASON_ASSOCIATION_INVALIDATED = 4,
    TRANSPORT_CONTINUATION_REASON_REPLAY_CONTINUITY_STALE = 5,
    TRANSPORT_CONTINUATION_REASON_USAGE_COUNTER_CONTINUITY_STALE = 6,
    TRANSPORT_CONTINUATION_REASON_AUTHORIZATION_GENERATION_STALE = 7,
    TRANSPORT_CONTINUATION_REASON_PEER_CONTEXT_MISMATCH = 8,
    TRANSPORT_CONTINUATION_REASON_PROFILE_CONTEXT_MISMATCH = 9,
    TRANSPORT_CONTINUATION_REASON_BINDING_INVALID = 10,
    TRANSPORT_CONTINUATION_REASON_RESUMPTION_NOT_AUTHORIZED = 11,
    TRANSPORT_CONTINUATION_REASON_CONTINUATION_NOT_REQUESTED = 12
} transport_continuation_reason_t;

typedef struct {
    transport_continuation_action_t action;
    transport_continuation_reason_t reason;
} transport_continuation_decision_t;

transport_continuation_decision_t transport_continuation_classify(
    const transport_continuation_facts_t *facts);

#endif
