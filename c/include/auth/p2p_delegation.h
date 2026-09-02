#ifndef AUTH_P2P_DELEGATION_H
#define AUTH_P2P_DELEGATION_H
#include <stdbool.h>

typedef struct {
    bool issuer_trusted, issuer_trust_local, holder_authenticated, grant_present;
    bool grant_integrity_valid, scope_match, audience_match, deployment_match;
    bool validity_current, epoch_current, revocation_current, explicitly_revoked;
    bool lineage_current, depth_within_limit, redelegation_permitted;
    bool redelegation_requested, rollback_suspected;
} p2p_delegation_facts_t;

typedef enum { P2P_DELEGATION_ACCEPT=0, P2P_DELEGATION_DENY=1 } p2p_delegation_action_t;
typedef enum {
    P2P_DELEGATION_REASON_CURRENT=0, P2P_DELEGATION_REASON_INVALID_FACTS,
    P2P_DELEGATION_REASON_ROLLBACK_SUSPECTED, P2P_DELEGATION_REASON_ISSUER_UNTRUSTED,
    P2P_DELEGATION_REASON_ISSUER_TRUST_NOT_LOCAL,
    P2P_DELEGATION_REASON_HOLDER_UNAUTHENTICATED, P2P_DELEGATION_REASON_GRANT_MISSING,
    P2P_DELEGATION_REASON_GRANT_INVALID, P2P_DELEGATION_REASON_SCOPE_MISMATCH,
    P2P_DELEGATION_REASON_AUDIENCE_MISMATCH, P2P_DELEGATION_REASON_DEPLOYMENT_MISMATCH,
    P2P_DELEGATION_REASON_EXPIRED_OR_NOT_YET_VALID, P2P_DELEGATION_REASON_EPOCH_STALE,
    P2P_DELEGATION_REASON_REVOCATION_STALE, P2P_DELEGATION_REASON_REVOKED,
    P2P_DELEGATION_REASON_LINEAGE_STALE, P2P_DELEGATION_REASON_DEPTH_EXCEEDED,
    P2P_DELEGATION_REASON_REDELEGATION_FORBIDDEN
} p2p_delegation_reason_t;

typedef struct { p2p_delegation_action_t action; p2p_delegation_reason_t reason; } p2p_delegation_decision_t;
p2p_delegation_decision_t p2p_delegation_classify(const p2p_delegation_facts_t *facts);
#endif
