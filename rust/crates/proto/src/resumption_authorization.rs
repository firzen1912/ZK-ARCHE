//! Wire-neutral authorization-aware resumption decision core.
//!
//! This module does not define a ticket/PSK wire format or issue credentials.
//! It classifies whether locally available resumption evidence is sufficient to
//! resume, requires a fresh full AUTH, or must fail closed. In particular,
//! stale revocation state, explicit revocation, stale lineage, and rollback
//! suspicion cannot be repaired by possession of a resumption secret alone.

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct ResumptionAuthorizationFacts {
    pub credential_present: bool,
    pub credential_integrity_valid: bool,
    pub binding_valid: bool,
    pub expired: bool,
    pub usage_count: u32,
    pub usage_limit: u32,
    pub authorization_context_present: bool,
    pub authorization_context_fresh: bool,
    pub revocation_current: bool,
    pub explicitly_revoked: bool,
    pub lineage_current: bool,
    pub peer_match: bool,
    pub deployment_match: bool,
    pub audience_match: bool,
    pub profile_match: bool,
    pub rollback_suspected: bool,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ResumptionAction {
    Resume,
    FullAuthRequired,
    Reject,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ResumptionReason {
    Current,
    InvalidFacts,
    RollbackSuspected,
    CredentialMissing,
    CredentialInvalid,
    BindingMismatch,
    Expired,
    ReuseLimitReached,
    AuthorizationContextMissing,
    AuthorizationStale,
    RevocationStale,
    Revoked,
    LineageStale,
    PeerMismatch,
    DeploymentMismatch,
    AudienceMismatch,
    ProfileMismatch,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct ResumptionAuthorizationDecision {
    pub action: ResumptionAction,
    pub reason: ResumptionReason,
}

fn decision(action: ResumptionAction, reason: ResumptionReason) -> ResumptionAuthorizationDecision {
    ResumptionAuthorizationDecision { action, reason }
}

pub fn classify_resumption_authorization(
    facts: &ResumptionAuthorizationFacts,
) -> ResumptionAuthorizationDecision {
    if facts.rollback_suspected {
        return decision(ResumptionAction::Reject, ResumptionReason::RollbackSuspected);
    }
    if !facts.credential_present {
        return decision(
            ResumptionAction::FullAuthRequired,
            ResumptionReason::CredentialMissing,
        );
    }
    if !facts.credential_integrity_valid {
        return decision(
            ResumptionAction::FullAuthRequired,
            ResumptionReason::CredentialInvalid,
        );
    }
    if !facts.binding_valid {
        return decision(
            ResumptionAction::FullAuthRequired,
            ResumptionReason::BindingMismatch,
        );
    }
    if facts.expired {
        return decision(ResumptionAction::FullAuthRequired, ResumptionReason::Expired);
    }
    if facts.usage_limit == 0 || facts.usage_count >= facts.usage_limit {
        return decision(
            ResumptionAction::FullAuthRequired,
            ResumptionReason::ReuseLimitReached,
        );
    }
    if !facts.authorization_context_present {
        return decision(
            ResumptionAction::FullAuthRequired,
            ResumptionReason::AuthorizationContextMissing,
        );
    }
    if !facts.authorization_context_fresh {
        return decision(
            ResumptionAction::FullAuthRequired,
            ResumptionReason::AuthorizationStale,
        );
    }
    if !facts.revocation_current {
        return decision(ResumptionAction::Reject, ResumptionReason::RevocationStale);
    }
    if facts.explicitly_revoked {
        return decision(ResumptionAction::Reject, ResumptionReason::Revoked);
    }
    if !facts.lineage_current {
        return decision(ResumptionAction::Reject, ResumptionReason::LineageStale);
    }
    if !facts.peer_match {
        return decision(
            ResumptionAction::FullAuthRequired,
            ResumptionReason::PeerMismatch,
        );
    }
    if !facts.deployment_match {
        return decision(
            ResumptionAction::FullAuthRequired,
            ResumptionReason::DeploymentMismatch,
        );
    }
    if !facts.audience_match {
        return decision(
            ResumptionAction::FullAuthRequired,
            ResumptionReason::AudienceMismatch,
        );
    }
    if !facts.profile_match {
        return decision(
            ResumptionAction::FullAuthRequired,
            ResumptionReason::ProfileMismatch,
        );
    }
    decision(ResumptionAction::Resume, ResumptionReason::Current)
}

#[cfg(test)]
mod tests {
    use super::*;

    fn bit(value: &str) -> bool {
        match value {
            "0" => false,
            "1" => true,
            _ => panic!("invalid bit: {value}"),
        }
    }

    fn action(value: &str) -> ResumptionAction {
        match value {
            "RESUME" => ResumptionAction::Resume,
            "FULL_AUTH_REQUIRED" => ResumptionAction::FullAuthRequired,
            "REJECT" => ResumptionAction::Reject,
            _ => panic!("invalid action: {value}"),
        }
    }

    fn reason(value: &str) -> ResumptionReason {
        match value {
            "CURRENT" => ResumptionReason::Current,
            "ROLLBACK_SUSPECTED" => ResumptionReason::RollbackSuspected,
            "CREDENTIAL_MISSING" => ResumptionReason::CredentialMissing,
            "CREDENTIAL_INVALID" => ResumptionReason::CredentialInvalid,
            "BINDING_MISMATCH" => ResumptionReason::BindingMismatch,
            "EXPIRED" => ResumptionReason::Expired,
            "REUSE_LIMIT_REACHED" => ResumptionReason::ReuseLimitReached,
            "AUTHORIZATION_CONTEXT_MISSING" => ResumptionReason::AuthorizationContextMissing,
            "AUTHORIZATION_STALE" => ResumptionReason::AuthorizationStale,
            "REVOCATION_STALE" => ResumptionReason::RevocationStale,
            "REVOKED" => ResumptionReason::Revoked,
            "LINEAGE_STALE" => ResumptionReason::LineageStale,
            "PEER_MISMATCH" => ResumptionReason::PeerMismatch,
            "DEPLOYMENT_MISMATCH" => ResumptionReason::DeploymentMismatch,
            "AUDIENCE_MISMATCH" => ResumptionReason::AudienceMismatch,
            "PROFILE_MISMATCH" => ResumptionReason::ProfileMismatch,
            _ => panic!("invalid reason: {value}"),
        }
    }

    #[test]
    fn canonical_corpus_matches_classifier() {
        let corpus = include_str!("../../../test-vectors/state/resumption-authorization-v1.txt");
        let mut count = 0usize;
        for line in corpus.lines() {
            let Some(case) = line.strip_prefix("case=") else {
                continue;
            };
            let fields: Vec<&str> = case.split('|').collect();
            assert_eq!(fields.len(), 19);
            let facts = ResumptionAuthorizationFacts {
                credential_present: bit(fields[1]),
                credential_integrity_valid: bit(fields[2]),
                binding_valid: bit(fields[3]),
                expired: bit(fields[4]),
                usage_count: fields[5].parse().unwrap(),
                usage_limit: fields[6].parse().unwrap(),
                authorization_context_present: bit(fields[7]),
                authorization_context_fresh: bit(fields[8]),
                revocation_current: bit(fields[9]),
                explicitly_revoked: bit(fields[10]),
                lineage_current: bit(fields[11]),
                peer_match: bit(fields[12]),
                deployment_match: bit(fields[13]),
                audience_match: bit(fields[14]),
                profile_match: bit(fields[15]),
                rollback_suspected: bit(fields[16]),
            };
            assert_eq!(
                classify_resumption_authorization(&facts),
                ResumptionAuthorizationDecision {
                    action: action(fields[17]),
                    reason: reason(fields[18]),
                },
                "case {}",
                fields[0]
            );
            count += 1;
        }
        assert_eq!(count, 16);
    }
}