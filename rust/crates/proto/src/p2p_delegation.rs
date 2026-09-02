//! Wire-neutral bounded P2P delegation decision.
//!
//! Trust remains local and non-transitive by default. A delegation is accepted
//! only when the local verifier already trusts the issuer and every explicit
//! scope, validity, epoch, revocation, lineage, depth, and redelegation guard is
//! satisfied. This module consumes already-verified local facts; it does not
//! parse or cryptographically verify a grant.

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct P2pDelegationFacts {
    pub issuer_trusted: bool,
    pub holder_authenticated: bool,
    pub grant_present: bool,
    pub grant_integrity_valid: bool,
    pub scope_match: bool,
    pub audience_match: bool,
    pub deployment_match: bool,
    pub validity_current: bool,
    pub epoch_current: bool,
    pub revocation_current: bool,
    pub explicitly_revoked: bool,
    pub lineage_current: bool,
    pub depth_within_limit: bool,
    pub redelegation_permitted: bool,
    pub redelegation_requested: bool,
    pub rollback_suspected: bool,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum P2pDelegationAction {
    Accept,
    Deny,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum P2pDelegationReason {
    Current,
    RollbackSuspected,
    IssuerUntrusted,
    HolderUnauthenticated,
    GrantMissing,
    GrantInvalid,
    ScopeMismatch,
    AudienceMismatch,
    DeploymentMismatch,
    ExpiredOrNotYetValid,
    EpochStale,
    RevocationStale,
    Revoked,
    LineageStale,
    DepthExceeded,
    RedelegationForbidden,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct P2pDelegationDecision {
    pub action: P2pDelegationAction,
    pub reason: P2pDelegationReason,
}

fn decision(action: P2pDelegationAction, reason: P2pDelegationReason) -> P2pDelegationDecision {
    P2pDelegationDecision { action, reason }
}

pub fn classify_p2p_delegation(f: &P2pDelegationFacts) -> P2pDelegationDecision {
    if f.rollback_suspected {
        return decision(
            P2pDelegationAction::Deny,
            P2pDelegationReason::RollbackSuspected,
        );
    }
    if !f.issuer_trusted {
        return decision(P2pDelegationAction::Deny, P2pDelegationReason::IssuerUntrusted);
    }
    if !f.holder_authenticated {
        return decision(
            P2pDelegationAction::Deny,
            P2pDelegationReason::HolderUnauthenticated,
        );
    }
    if !f.grant_present {
        return decision(P2pDelegationAction::Deny, P2pDelegationReason::GrantMissing);
    }
    if !f.grant_integrity_valid {
        return decision(P2pDelegationAction::Deny, P2pDelegationReason::GrantInvalid);
    }
    if !f.scope_match {
        return decision(P2pDelegationAction::Deny, P2pDelegationReason::ScopeMismatch);
    }
    if !f.audience_match {
        return decision(P2pDelegationAction::Deny, P2pDelegationReason::AudienceMismatch);
    }
    if !f.deployment_match {
        return decision(
            P2pDelegationAction::Deny,
            P2pDelegationReason::DeploymentMismatch,
        );
    }
    if !f.validity_current {
        return decision(
            P2pDelegationAction::Deny,
            P2pDelegationReason::ExpiredOrNotYetValid,
        );
    }
    if !f.epoch_current {
        return decision(P2pDelegationAction::Deny, P2pDelegationReason::EpochStale);
    }
    if !f.revocation_current {
        return decision(P2pDelegationAction::Deny, P2pDelegationReason::RevocationStale);
    }
    if f.explicitly_revoked {
        return decision(P2pDelegationAction::Deny, P2pDelegationReason::Revoked);
    }
    if !f.lineage_current {
        return decision(P2pDelegationAction::Deny, P2pDelegationReason::LineageStale);
    }
    if !f.depth_within_limit {
        return decision(P2pDelegationAction::Deny, P2pDelegationReason::DepthExceeded);
    }
    if f.redelegation_requested && !f.redelegation_permitted {
        return decision(
            P2pDelegationAction::Deny,
            P2pDelegationReason::RedelegationForbidden,
        );
    }
    decision(P2pDelegationAction::Accept, P2pDelegationReason::Current)
}
