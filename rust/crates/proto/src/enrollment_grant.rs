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
    pub commissioner_not_revoked: bool,
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
    CommissionerRevoked,
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
    if !f.commissioner_not_revoked {
        return decision(
            EnrollmentGrantAction::Deny,
            EnrollmentGrantReason::CommissionerRevoked,
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

    fn b(s: &str) -> bool {
        s == "1"
    }

    #[test]
    fn canonical_enrollment_grant_corpus() {
        let corpus = include_str!("../../../test-vectors/state/enrollment-grant-v1.txt");
        let mut count = 0usize;
        for line in corpus.lines() {
            if line.is_empty() || line.starts_with('#') {
                continue;
            }
            let f: Vec<&str> = line.split_whitespace().collect();
            assert_eq!(f.len(), 19, "bad vector: {line}");
            let facts = EnrollmentGrantFacts {
                explicit_enroll_operation: b(f[1]),
                normal_auth_path: b(f[2]),
                commissioner_authenticated: b(f[3]),
                commissioner_authorized: b(f[4]),
                commissioner_not_revoked: b(f[5]),
                subject_possession_verified: b(f[6]),
                requested_authority_within_commissioner_scope: b(f[7]),
                scope_bounded: b(f[8]),
                audience_bound: b(f[9]),
                deployment_bound: b(f[10]),
                validity_bounded: b(f[11]),
                epoch_current: b(f[12]),
                revocation_current: b(f[13]),
                lineage_current: b(f[14]),
                delegation_depth_within_limit: b(f[15]),
                rollback_suspected: b(f[16]),
            };
            let d = classify_enrollment_grant(&facts);
            let action = match d.action {
                EnrollmentGrantAction::Issue => "ISSUE",
                EnrollmentGrantAction::Deny => "DENY",
            };
            let normalized = match d.reason {
                EnrollmentGrantReason::Current => "CURRENT",
                EnrollmentGrantReason::RollbackSuspected => "ROLLBACK_SUSPECTED",
                EnrollmentGrantReason::NormalAuthForbidden => "NORMAL_AUTH_FORBIDDEN",
                EnrollmentGrantReason::ExplicitEnrollRequired => "EXPLICIT_ENROLL_REQUIRED",
                EnrollmentGrantReason::CommissionerUnauthenticated => {
                    "COMMISSIONER_UNAUTHENTICATED"
                }
                EnrollmentGrantReason::CommissionerUnauthorized => "COMMISSIONER_UNAUTHORIZED",
                EnrollmentGrantReason::CommissionerRevoked => "COMMISSIONER_REVOKED",
                EnrollmentGrantReason::SubjectPossessionMissing => "SUBJECT_POSSESSION_MISSING",
                EnrollmentGrantReason::AuthorityEscalation => "AUTHORITY_ESCALATION",
                EnrollmentGrantReason::ScopeUnbounded => "SCOPE_UNBOUNDED",
                EnrollmentGrantReason::AudienceUnbound => "AUDIENCE_UNBOUND",
                EnrollmentGrantReason::DeploymentUnbound => "DEPLOYMENT_UNBOUND",
                EnrollmentGrantReason::ValidityUnbounded => "VALIDITY_UNBOUNDED",
                EnrollmentGrantReason::EpochStale => "EPOCH_STALE",
                EnrollmentGrantReason::RevocationStale => "REVOCATION_STALE",
                EnrollmentGrantReason::LineageStale => "LINEAGE_STALE",
                EnrollmentGrantReason::DelegationDepthExceeded => "DELEGATION_DEPTH_EXCEEDED",
            };
            assert_eq!(action, f[17], "{}", f[0]);
            assert_eq!(normalized, f[18], "{}", f[0]);
            count += 1;
        }
        assert_eq!(count, 17);
    }
}
