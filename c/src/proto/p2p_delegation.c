#include "auth/p2p_delegation.h"
#include <stddef.h>
static p2p_delegation_decision_t decision(p2p_delegation_action_t a, p2p_delegation_reason_t r) { p2p_delegation_decision_t out={a,r}; return out; }
p2p_delegation_decision_t p2p_delegation_classify(const p2p_delegation_facts_t *f) {
    if (f==NULL) return decision(P2P_DELEGATION_DENY,P2P_DELEGATION_REASON_INVALID_FACTS);
    if (f->rollback_suspected) return decision(P2P_DELEGATION_DENY,P2P_DELEGATION_REASON_ROLLBACK_SUSPECTED);
    if (!f->issuer_trusted) return decision(P2P_DELEGATION_DENY,P2P_DELEGATION_REASON_ISSUER_UNTRUSTED);
    if (!f->issuer_trust_local) return decision(P2P_DELEGATION_DENY,P2P_DELEGATION_REASON_ISSUER_TRUST_NOT_LOCAL);
    if (!f->holder_authenticated) return decision(P2P_DELEGATION_DENY,P2P_DELEGATION_REASON_HOLDER_UNAUTHENTICATED);
    if (!f->grant_present) return decision(P2P_DELEGATION_DENY,P2P_DELEGATION_REASON_GRANT_MISSING);
    if (!f->grant_integrity_valid) return decision(P2P_DELEGATION_DENY,P2P_DELEGATION_REASON_GRANT_INVALID);
    if (!f->scope_match) return decision(P2P_DELEGATION_DENY,P2P_DELEGATION_REASON_SCOPE_MISMATCH);
    if (!f->audience_match) return decision(P2P_DELEGATION_DENY,P2P_DELEGATION_REASON_AUDIENCE_MISMATCH);
    if (!f->deployment_match) return decision(P2P_DELEGATION_DENY,P2P_DELEGATION_REASON_DEPLOYMENT_MISMATCH);
    if (!f->validity_current) return decision(P2P_DELEGATION_DENY,P2P_DELEGATION_REASON_EXPIRED_OR_NOT_YET_VALID);
    if (!f->authorization_generation_bound) return decision(P2P_DELEGATION_DENY,P2P_DELEGATION_REASON_AUTHORIZATION_GENERATION_UNBOUND);
    if (!f->authorization_generation_current) return decision(P2P_DELEGATION_DENY,P2P_DELEGATION_REASON_AUTHORIZATION_GENERATION_STALE);
    if (!f->epoch_current) return decision(P2P_DELEGATION_DENY,P2P_DELEGATION_REASON_EPOCH_STALE);
    if (!f->revocation_current) return decision(P2P_DELEGATION_DENY,P2P_DELEGATION_REASON_REVOCATION_STALE);
    if (f->explicitly_revoked) return decision(P2P_DELEGATION_DENY,P2P_DELEGATION_REASON_REVOKED);
    if (!f->lineage_current) return decision(P2P_DELEGATION_DENY,P2P_DELEGATION_REASON_LINEAGE_STALE);
    if (!f->depth_within_limit) return decision(P2P_DELEGATION_DENY,P2P_DELEGATION_REASON_DEPTH_EXCEEDED);
    if (f->redelegation_requested && !f->redelegation_permitted) return decision(P2P_DELEGATION_DENY,P2P_DELEGATION_REASON_REDELEGATION_FORBIDDEN);
    return decision(P2P_DELEGATION_ACCEPT,P2P_DELEGATION_REASON_CURRENT);
}
