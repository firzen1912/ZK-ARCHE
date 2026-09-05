//! Wire-neutral enrollment grant issuance decision.
//!
//! Enrollment is an explicit trust/authorization mutation. Normal AUTH remains
//! NO-LEARNING and cannot invoke this decision as a side effect.

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct EnrollmentGrantFacts {
    pub explicit_enroll_operation: bool,
    pub normal_auth_path: bool,
    pub commissioner_authenticated: bool,
    pub commissioner_authorized: bool,
    pub commissioner_authorization_fresh: bool,
    pub commissioner_authorization_generation_bound: bool,
    pub commissioner_authorization_generation_current: bool,
    pub commissioner_not_revoked: bool,
    pub enrollment_nonce_unused: bool,
    pub subject_possession_verified: bool,
    pub requested_authority_within_commissioner_scope: bool,
    pub scope_bounded: bool,
    pub audience_bound: bool,
    pub deployment_bound: bool,
    pub validity_bounded: bool,
    pub epoch_current: bool,
    pub revocation_current: bool,
    pub lineage_current: bool,
    pub delegation_depth_within_limit: bool,
    pub rollback_suspected: bool,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum EnrollmentGrantAction {
    Issue,
    Deny,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum EnrollmentGrantReason {
    Current,
    RollbackSuspected,
    NormalAuthForbidden,
    ExplicitEnrollRequired,
    CommissionerUnauthenticated,
    CommissionerUnauthorized,
    CommissionerAuthorizationStale,
    CommissionerAuthorizationGenerationUnbound,
    CommissionerAuthorizationGenerationStale,
    CommissionerRevoked,
    EnrollmentReplayDetected,
    SubjectPossessionMissing,
    AuthorityEscalation,
    ScopeUnbounded,
    AudienceUnbound,
    DeploymentUnbound,
    ValidityUnbounded,
    EpochStale,
    RevocationStale,
    LineageStale,
    DelegationDepthExceeded,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct EnrollmentGrantDecision {
    pub action: EnrollmentGrantAction,
    pub reason: EnrollmentGrantReason,
}

fn decision(
    action: EnrollmentGrantAction,
    reason: EnrollmentGrantReason,
) -> EnrollmentGrantDecision {
    EnrollmentGrantDecision { action, reason }
}

pub fn classify_enrollment_grant(f: &EnrollmentGrantFacts) -> EnrollmentGrantDecision {
    if f.rollback_suspected {
        return decision(
            EnrollmentGrantAction::Deny,
            EnrollmentGrantReason::RollbackSuspected,
        );
    }
    if f.normal_auth_path {
        return decision(
            EnrollmentGrantAction::Deny,
            EnrollmentGrantReason::NormalAuthForbidden,
        );
    }
    if !f.explicit_enroll_operation {
        return decision(
            EnrollmentGrantAction::Deny,
            EnrollmentGrantReason::ExplicitEnrollRequired,
        );
    }
    if !f.commissioner_authenticated {
        return decision(
            EnrollmentGrantAction::Deny,
            EnrollmentGrantReason::CommissionerUnauthenticated,
        );
    }
    if !f.commissioner_authorized {
        return decision(
            EnrollmentGrantAction::Deny,
            EnrollmentGrantReason::CommissionerUnauthorized,
        );
    }
    if !f.commissioner_authorization_fresh {
        return decision(
            EnrollmentGrantAction::Deny,
            EnrollmentGrantReason::CommissionerAuthorizationStale,
        );
    }
    if !f.commissioner_authorization_generation_bound {
        return decision(
            EnrollmentGrantAction::Deny,
            EnrollmentGrantReason::CommissionerAuthorizationGenerationUnbound,
        );
    }
    if !f.commissioner_authorization_generation_current {
        return decision(
            EnrollmentGrantAction::Deny,
            EnrollmentGrantReason::CommissionerAuthorizationGenerationStale,
        );
    }
    if !f.commissioner_not_revoked {
        return decision(
            EnrollmentGrantAction::Deny,
            EnrollmentGrantReason::CommissionerRevoked,
        );
    }
    if !f.enrollment_nonce_unused {
        return decision(
            EnrollmentGrantAction::Deny,
            EnrollmentGrantReason::EnrollmentReplayDetected,
        );
    }
    if !f.subject_possession_verified {
        return decision(
            EnrollmentGrantAction::Deny,
            EnrollmentGrantReason::SubjectPossessionMissing,
        );
    }
    if !f.requested_authority_within_commissioner_scope {
        return decision(
            EnrollmentGrantAction::Deny,
            EnrollmentGrantReason::AuthorityEscalation,
        );
    }
    if !f.scope_bounded {
        return decision(
            EnrollmentGrantAction::Deny,
            EnrollmentGrantReason::ScopeUnbounded,
        );
    }
    if !f.audience_bound {
        return decision(
            EnrollmentGrantAction::Deny,
            EnrollmentGrantReason::AudienceUnbound,
        );
    }
    if !f.deployment_bound {
        return decision(
            EnrollmentGrantAction::Deny,
            EnrollmentGrantReason::DeploymentUnbound,
        );
    }
    if !f.validity_bounded {
        return decision(
            EnrollmentGrantAction::Deny,
            EnrollmentGrantReason::ValidityUnbounded,
        );
    }
    if !f.epoch_current {
        return decision(
            EnrollmentGrantAction::Deny,
            EnrollmentGrantReason::EpochStale,
        );
    }
    if !f.revocation_current {
        return decision(
            EnrollmentGrantAction::Deny,
            EnrollmentGrantReason::RevocationStale,
        );
    }
    if !f.lineage_current {
        return decision(
            EnrollmentGrantAction::Deny,
            EnrollmentGrantReason::LineageStale,
        );
    }
    if !f.delegation_depth_within_limit {
        return decision(
            EnrollmentGrantAction::Deny,
            EnrollmentGrantReason::DelegationDepthExceeded,
        );
    }
    decision(EnrollmentGrantAction::Issue, EnrollmentGrantReason::Current)
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

    fn action(value: &str) -> EnrollmentGrantAction {
        match value {
            "ISSUE" => EnrollmentGrantAction::Issue,
            "DENY" => EnrollmentGrantAction::Deny,
            _ => panic!("invalid action: {value}"),
        }
    }

    fn reason(value: &str) -> EnrollmentGrantReason {
        match value {
            "CURRENT" => EnrollmentGrantReason::Current,
            "ROLLBACK_SUSPECTED" => EnrollmentGrantReason::RollbackSuspected,
            "NORMAL_AUTH_FORBIDDEN" => EnrollmentGrantReason::NormalAuthForbidden,
            "EXPLICIT_ENROLL_REQUIRED" => EnrollmentGrantReason::ExplicitEnrollRequired,
            "COMMISSIONER_UNAUTHENTICATED" => EnrollmentGrantReason::CommissionerUnauthenticated,
            "COMMISSIONER_UNAUTHORIZED" => EnrollmentGrantReason::CommissionerUnauthorized,
            "COMMISSIONER_AUTHORIZATION_STALE" => {
                EnrollmentGrantReason::CommissionerAuthorizationStale
            }
            "COMMISSIONER_AUTHORIZATION_GENERATION_UNBOUND" => {
                EnrollmentGrantReason::CommissionerAuthorizationGenerationUnbound
            }
            "COMMISSIONER_AUTHORIZATION_GENERATION_STALE" => {
                EnrollmentGrantReason::CommissionerAuthorizationGenerationStale
            }
            "COMMISSIONER_REVOKED" => EnrollmentGrantReason::CommissionerRevoked,
            "ENROLLMENT_REPLAY_DETECTED" => EnrollmentGrantReason::EnrollmentReplayDetected,
            "SUBJECT_POSSESSION_MISSING" => EnrollmentGrantReason::SubjectPossessionMissing,
            "AUTHORITY_ESCALATION" => EnrollmentGrantReason::AuthorityEscalation,
            "SCOPE_UNBOUNDED" => EnrollmentGrantReason::ScopeUnbounded,
            "AUDIENCE_UNBOUND" => EnrollmentGrantReason::AudienceUnbound,
            "DEPLOYMENT_UNBOUND" => EnrollmentGrantReason::DeploymentUnbound,
            "VALIDITY_UNBOUNDED" => EnrollmentGrantReason::ValidityUnbounded,
            "EPOCH_STALE" => EnrollmentGrantReason::EpochStale,
            "REVOCATION_STALE" => EnrollmentGrantReason::RevocationStale,
            "LINEAGE_STALE" => EnrollmentGrantReason::LineageStale,
            "DELEGATION_DEPTH_EXCEEDED" => EnrollmentGrantReason::DelegationDepthExceeded,
            _ => panic!("invalid reason: {value}"),
        }
    }

    #[test]
    fn canonical_v4_corpus_matches_classifier() {
        let corpus = include_str!("../../../test-vectors/state/enrollment-grant-v4.txt");
        let mut count = 0usize;
        for line in corpus.lines() {
            let Some(case) = line.strip_prefix("case=") else {
                continue;
            };
            let fields: Vec<&str> = case.split('|').collect();
            assert_eq!(fields.len(), 23);
            let facts = EnrollmentGrantFacts {
                explicit_enroll_operation: bit(fields[1]),
                normal_auth_path: bit(fields[2]),
                commissioner_authenticated: bit(fields[3]),
                commissioner_authorized: bit(fields[4]),
                commissioner_authorization_fresh: bit(fields[5]),
                commissioner_authorization_generation_bound: bit(fields[6]),
                commissioner_authorization_generation_current: bit(fields[7]),
                commissioner_not_revoked: bit(fields[8]),
                enrollment_nonce_unused: bit(fields[9]),
                subject_possession_verified: bit(fields[10]),
                requested_authority_within_commissioner_scope: bit(fields[11]),
                scope_bounded: bit(fields[12]),
                audience_bound: bit(fields[13]),
                deployment_bound: bit(fields[14]),
                validity_bounded: bit(fields[15]),
                epoch_current: bit(fields[16]),
                revocation_current: bit(fields[17]),
                lineage_current: bit(fields[18]),
                delegation_depth_within_limit: bit(fields[19]),
                rollback_suspected: bit(fields[20]),
            };
            assert_eq!(
                classify_enrollment_grant(&facts),
                EnrollmentGrantDecision {
                    action: action(fields[21]),
                    reason: reason(fields[22]),
                },
                "case {}",
                fields[0]
            );
            count += 1;
        }
        assert_eq!(count, 21);
    }
}
