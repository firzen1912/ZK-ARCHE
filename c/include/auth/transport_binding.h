#ifndef AUTH_TRANSPORT_BINDING_H
#define AUTH_TRANSPORT_BINDING_H

#include <stdbool.h>

typedef struct {
    bool profile_requires_binding;
    bool binding_present;
    bool binding_integrity_valid;
    bool binding_fresh;
    bool auth_instance_match;
    bool peer_context_match;
    bool profile_context_match;
    bool transport_address_as_identity;
    bool transport_metadata_as_authority;
} transport_binding_facts_t;

typedef enum {
    TRANSPORT_BINDING_BOUND = 0,
    TRANSPORT_BINDING_UNBOUND_ALLOWED = 1,
    TRANSPORT_BINDING_REJECT = 2
} transport_binding_disposition_t;

typedef enum {
    TRANSPORT_BINDING_REASON_CURRENT = 0,
    TRANSPORT_BINDING_REASON_INVALID_FACTS = 1,
    TRANSPORT_BINDING_REASON_TRANSPORT_ADDRESS_AS_IDENTITY = 2,
    TRANSPORT_BINDING_REASON_TRANSPORT_METADATA_AS_AUTHORITY = 3,
    TRANSPORT_BINDING_REASON_BINDING_MISSING = 4,
    TRANSPORT_BINDING_REASON_BINDING_NOT_REQUIRED = 5,
    TRANSPORT_BINDING_REASON_BINDING_INVALID = 6,
    TRANSPORT_BINDING_REASON_BINDING_STALE = 7,
    TRANSPORT_BINDING_REASON_AUTH_INSTANCE_MISMATCH = 8,
    TRANSPORT_BINDING_REASON_PEER_CONTEXT_MISMATCH = 9,
    TRANSPORT_BINDING_REASON_PROFILE_CONTEXT_MISMATCH = 10
} transport_binding_reason_t;

typedef struct {
    transport_binding_disposition_t disposition;
    transport_binding_reason_t reason;
} transport_binding_decision_t;

transport_binding_decision_t transport_binding_classify(const transport_binding_facts_t *facts);

#endif
