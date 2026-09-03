//! Wire-neutral bounded P2P delegation decision.
//!
//! Trust remains local and non-transitive by default. A delegation is accepted
//! only when the local verifier already trusts the issuer through its own local
//! trust state and every explicit scope, validity, authorization-generation,
//! epoch, revocation, lineage, depth, and redelegation guard is satisfied.
//! Trust inferred only through a third party is not an acceptable issuer trust
//! root. This module consumes already-verified local facts; it does not parse
//! or cryptographically verify a grant.

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct P2pDelegationFacts {
    pub issuer_trusted: bool,
    pub issuer_trust_local: bool,
    pub holder_authenticated: bool,
    pub grant_present: bool,
    pub grant_integrity_valid: bool,
    pub scope_match: bool,
    pub audience_match: bool,
    pub deployment_match: bool,
    pub validity_current: bool,
    pub authorization_generation_bound: bool,
    pub authorization_generation_current: bool,
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
pub enum P2pDelegationAction { Accept, Deny }

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum P2pDelegationReason {
    Current,
    RollbackSuspected,
    IssuerUntrusted,
    IssuerTrustNotLocal,
    HolderUnauthenticated,
    GrantMissing,
    GrantInvalid,
    ScopeMismatch,
    AudienceMismatch,
    DeploymentMismatch,
    ExpiredOrNotYetValid,
    AuthorizationGenerationUnbound,
    AuthorizationGenerationStale,
    EpochStale,
    RevocationStale,
    Revoked,
    LineageStale,
    DepthExceeded,
    RedelegationForbidden,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct P2pDelegationDecision { pub action: P2pDelegationAction, pub reason: P2pDelegationReason }

fn decision(action: P2pDelegationAction, reason: P2pDelegationReason) -> P2pDelegationDecision { P2pDelegationDecision { action, reason } }

pub fn classify_p2p_delegation(f: &P2pDelegationFacts) -> P2pDelegationDecision {
    if f.rollback_suspected { return decision(P2pDelegationAction::Deny, P2pDelegationReason::RollbackSuspected); }
    if !f.issuer_trusted { return decision(P2pDelegationAction::Deny, P2pDelegationReason::IssuerUntrusted); }
    if !f.issuer_trust_local { return decision(P2pDelegationAction::Deny, P2pDelegationReason::IssuerTrustNotLocal); }
    if !f.holder_authenticated { return decision(P2pDelegationAction::Deny, P2pDelegationReason::HolderUnauthenticated); }
    if !f.grant_present { return decision(P2pDelegationAction::Deny, P2pDelegationReason::GrantMissing); }
    if !f.grant_integrity_valid { return decision(P2pDelegationAction::Deny, P2pDelegationReason::GrantInvalid); }
    if !f.scope_match { return decision(P2pDelegationAction::Deny, P2pDelegationReason::ScopeMismatch); }
    if !f.audience_match { return decision(P2pDelegationAction::Deny, P2pDelegationReason::AudienceMismatch); }
    if !f.deployment_match { return decision(P2pDelegationAction::Deny, P2pDelegationReason::DeploymentMismatch); }
    if !f.validity_current { return decision(P2pDelegationAction::Deny, P2pDelegationReason::ExpiredOrNotYetValid); }
    if !f.authorization_generation_bound { return decision(P2pDelegationAction::Deny, P2pDelegationReason::AuthorizationGenerationUnbound); }
    if !f.authorization_generation_current { return decision(P2pDelegationAction::Deny, P2pDelegationReason::AuthorizationGenerationStale); }
    if !f.epoch_current { return decision(P2pDelegationAction::Deny, P2pDelegationReason::EpochStale); }
    if !f.revocation_current { return decision(P2pDelegationAction::Deny, P2pDelegationReason::RevocationStale); }
    if f.explicitly_revoked { return decision(P2pDelegationAction::Deny, P2pDelegationReason::Revoked); }
    if !f.lineage_current { return decision(P2pDelegationAction::Deny, P2pDelegationReason::LineageStale); }
    if !f.depth_within_limit { return decision(P2pDelegationAction::Deny, P2pDelegationReason::DepthExceeded); }
    if f.redelegation_requested && !f.redelegation_permitted { return decision(P2pDelegationAction::Deny, P2pDelegationReason::RedelegationForbidden); }
    decision(P2pDelegationAction::Accept, P2pDelegationReason::Current)
}

#[cfg(test)]
mod tests {
    use super::*;
    fn bit(value: &str) -> bool { match value { "0" => false, "1" => true, _ => panic!("invalid bit: {value}") } }
    fn action(value: &str) -> P2pDelegationAction { match value { "ACCEPT" => P2pDelegationAction::Accept, "DENY" => P2pDelegationAction::Deny, _ => panic!("invalid action: {value}") } }
    fn reason(value: &str) -> P2pDelegationReason {
        match value {
            "CURRENT" => P2pDelegationReason::Current,
            "ROLLBACK_SUSPECTED" => P2pDelegationReason::RollbackSuspected,
            "ISSUER_UNTRUSTED" => P2pDelegationReason::IssuerUntrusted,
            "ISSUER_TRUST_NOT_LOCAL" => P2pDelegationReason::IssuerTrustNotLocal,
            "HOLDER_UNAUTHENTICATED" => P2pDelegationReason::HolderUnauthenticated,
            "GRANT_MISSING" => P2pDelegationReason::GrantMissing,
            "GRANT_INVALID" => P2pDelegationReason::GrantInvalid,
            "SCOPE_MISMATCH" => P2pDelegationReason::ScopeMismatch,
            "AUDIENCE_MISMATCH" => P2pDelegationReason::AudienceMismatch,
            "DEPLOYMENT_MISMATCH" => P2pDelegationReason::DeploymentMismatch,
            "EXPIRED_OR_NOT_YET_VALID" => P2pDelegationReason::ExpiredOrNotYetValid,
            "AUTHORIZATION_GENERATION_UNBOUND" => P2pDelegationReason::AuthorizationGenerationUnbound,
            "AUTHORIZATION_GENERATION_STALE" => P2pDelegationReason::AuthorizationGenerationStale,
            "EPOCH_STALE" => P2pDelegationReason::EpochStale,
            "REVOCATION_STALE" => P2pDelegationReason::RevocationStale,
            "REVOKED" => P2pDelegationReason::Revoked,
            "LINEAGE_STALE" => P2pDelegationReason::LineageStale,
            "DEPTH_EXCEEDED" => P2pDelegationReason::DepthExceeded,
            "REDELEGATION_FORBIDDEN" => P2pDelegationReason::RedelegationForbidden,
            _ => panic!("invalid reason: {value}"),
        }
    }
    #[test]
    fn canonical_v3_corpus_matches_classifier() {
        let corpus = include_str!("../../../test-vectors/p2p/bounded-delegation-v3.txt");
        let mut count = 0usize;
        for line in corpus.lines() {
            let Some(case) = line.strip_prefix("case=") else { continue; };
            let fields: Vec<&str> = case.split('|').collect();
            assert_eq!(fields.len(), 22);
            let facts = P2pDelegationFacts {
                issuer_trusted: bit(fields[1]), issuer_trust_local: bit(fields[2]), holder_authenticated: bit(fields[3]), grant_present: bit(fields[4]),
                grant_integrity_valid: bit(fields[5]), scope_match: bit(fields[6]), audience_match: bit(fields[7]), deployment_match: bit(fields[8]),
                validity_current: bit(fields[9]), authorization_generation_bound: bit(fields[10]), authorization_generation_current: bit(fields[11]),
                epoch_current: bit(fields[12]), revocation_current: bit(fields[13]), explicitly_revoked: bit(fields[14]), lineage_current: bit(fields[15]),
                depth_within_limit: bit(fields[16]), redelegation_permitted: bit(fields[17]), redelegation_requested: bit(fields[18]), rollback_suspected: bit(fields[19]),
            };
            assert_eq!(classify_p2p_delegation(&facts), P2pDelegationDecision { action: action(fields[20]), reason: reason(fields[21]) }, "case {}", fields[0]);
            count += 1;
        }
        assert_eq!(count, 20);
    }
}
