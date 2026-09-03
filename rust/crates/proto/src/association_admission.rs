//! Fail-closed postcondition for admitting or retaining a secure association.
//!
//! This module is deliberately wire-neutral and does not own AUTH, TRUST,
//! LINK, or BIND state. Callers supply decisions derived by those owning
//! layers. The classifier only answers whether their current combination is
//! sufficient to establish or retain an association.

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct AssociationAdmissionFacts {
    pub auth_complete: bool,
    pub preexisting_trust_record: bool,
    pub authorization_present: bool,
    pub authorization_fresh: bool,
    pub authorization_generation_bound: bool,
    pub authorization_generation_current: bool,
    pub revocation_current: bool,
    pub explicitly_revoked: bool,
    pub lineage_current: bool,
    pub replay_continuity_current: bool,
    pub restart_continuity_current: bool,
    pub binding_required: bool,
    pub binding_valid: bool,
    pub rollback_suspected: bool,
    pub trust_mutation_requested: bool,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum AssociationAdmissionAction {
    Establish,
    FailClosed,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum AssociationAdmissionReason {
    Current,
    InvalidFacts,
    RollbackSuspected,
    TrustMutationRequested,
    AuthIncomplete,
    TrustRecordMissing,
    AuthorizationMissing,
    AuthorizationStale,
    RevocationStale,
    Revoked,
    LineageStale,
    ReplayContinuityStale,
    BindingInvalid,
    AuthorizationGenerationStale,
    RestartContinuityStale,
    AuthorizationGenerationUnbound,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct AssociationAdmissionDecision {
    pub action: AssociationAdmissionAction,
    pub reason: AssociationAdmissionReason,
}

fn decision(
    action: AssociationAdmissionAction,
    reason: AssociationAdmissionReason,
) -> AssociationAdmissionDecision {
    AssociationAdmissionDecision { action, reason }
}

pub fn classify_association_admission(
    facts: &AssociationAdmissionFacts,
) -> AssociationAdmissionDecision {
    if facts.rollback_suspected {
        return decision(
            AssociationAdmissionAction::FailClosed,
            AssociationAdmissionReason::RollbackSuspected,
        );
    }
    if facts.trust_mutation_requested {
        return decision(
            AssociationAdmissionAction::FailClosed,
            AssociationAdmissionReason::TrustMutationRequested,
        );
    }
    if !facts.auth_complete {
        return decision(
            AssociationAdmissionAction::FailClosed,
            AssociationAdmissionReason::AuthIncomplete,
        );
    }
    if !facts.preexisting_trust_record {
        return decision(
            AssociationAdmissionAction::FailClosed,
            AssociationAdmissionReason::TrustRecordMissing,
        );
    }
    if !facts.authorization_present {
        return decision(
            AssociationAdmissionAction::FailClosed,
            AssociationAdmissionReason::AuthorizationMissing,
        );
    }
    if !facts.authorization_fresh {
        return decision(
            AssociationAdmissionAction::FailClosed,
            AssociationAdmissionReason::AuthorizationStale,
        );
    }
    if !facts.authorization_generation_bound {
        return decision(
            AssociationAdmissionAction::FailClosed,
            AssociationAdmissionReason::AuthorizationGenerationUnbound,
        );
    }
    if !facts.authorization_generation_current {
        return decision(
            AssociationAdmissionAction::FailClosed,
            AssociationAdmissionReason::AuthorizationGenerationStale,
        );
    }
    if !facts.revocation_current {
        return decision(
            AssociationAdmissionAction::FailClosed,
            AssociationAdmissionReason::RevocationStale,
        );
    }
    if facts.explicitly_revoked {
        return decision(
            AssociationAdmissionAction::FailClosed,
            AssociationAdmissionReason::Revoked,
        );
    }
    if !facts.lineage_current {
        return decision(
            AssociationAdmissionAction::FailClosed,
            AssociationAdmissionReason::LineageStale,
        );
    }
    if !facts.replay_continuity_current {
        return decision(
            AssociationAdmissionAction::FailClosed,
            AssociationAdmissionReason::ReplayContinuityStale,
        );
    }
    if !facts.restart_continuity_current {
        return decision(
            AssociationAdmissionAction::FailClosed,
            AssociationAdmissionReason::RestartContinuityStale,
        );
    }
    if facts.binding_required && !facts.binding_valid {
        return decision(
            AssociationAdmissionAction::FailClosed,
            AssociationAdmissionReason::BindingInvalid,
        );
    }
    decision(
        AssociationAdmissionAction::Establish,
        AssociationAdmissionReason::Current,
    )
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

    fn action(value: &str) -> AssociationAdmissionAction {
        match value {
            "ESTABLISH" => AssociationAdmissionAction::Establish,
            "FAIL_CLOSED" => AssociationAdmissionAction::FailClosed,
            _ => panic!("invalid action: {value}"),
        }
    }

    fn reason(value: &str) -> AssociationAdmissionReason {
        match value {
            "CURRENT" => AssociationAdmissionReason::Current,
            "ROLLBACK_SUSPECTED" => AssociationAdmissionReason::RollbackSuspected,
            "TRUST_MUTATION_REQUESTED" => AssociationAdmissionReason::TrustMutationRequested,
            "AUTH_INCOMPLETE" => AssociationAdmissionReason::AuthIncomplete,
            "TRUST_RECORD_MISSING" => AssociationAdmissionReason::TrustRecordMissing,
            "AUTHORIZATION_MISSING" => AssociationAdmissionReason::AuthorizationMissing,
            "AUTHORIZATION_STALE" => AssociationAdmissionReason::AuthorizationStale,
            "AUTHORIZATION_GENERATION_UNBOUND" => {
                AssociationAdmissionReason::AuthorizationGenerationUnbound
            }
            "AUTHORIZATION_GENERATION_STALE" => {
                AssociationAdmissionReason::AuthorizationGenerationStale
            }
            "REVOCATION_STALE" => AssociationAdmissionReason::RevocationStale,
            "REVOKED" => AssociationAdmissionReason::Revoked,
            "LINEAGE_STALE" => AssociationAdmissionReason::LineageStale,
            "REPLAY_CONTINUITY_STALE" => AssociationAdmissionReason::ReplayContinuityStale,
            "RESTART_CONTINUITY_STALE" => AssociationAdmissionReason::RestartContinuityStale,
            "BINDING_INVALID" => AssociationAdmissionReason::BindingInvalid,
            _ => panic!("invalid reason: {value}"),
        }
    }

    #[test]
    fn canonical_corpus_matches_classifier() {
        let corpus = include_str!("../../../test-vectors/state/association-admission-v3.txt");
        let mut count = 0usize;
        for line in corpus.lines() {
            let Some(case) = line.strip_prefix("case=") else {
                continue;
            };
            let fields: Vec<&str> = case.split('|').collect();
            assert_eq!(fields.len(), 18);
            let facts = AssociationAdmissionFacts {
                auth_complete: bit(fields[1]),
                preexisting_trust_record: bit(fields[2]),
                authorization_present: bit(fields[3]),
                authorization_fresh: bit(fields[4]),
                authorization_generation_bound: bit(fields[5]),
                authorization_generation_current: bit(fields[6]),
                revocation_current: bit(fields[7]),
                explicitly_revoked: bit(fields[8]),
                lineage_current: bit(fields[9]),
                replay_continuity_current: bit(fields[10]),
                restart_continuity_current: bit(fields[11]),
                binding_required: bit(fields[12]),
                binding_valid: bit(fields[13]),
                rollback_suspected: bit(fields[14]),
                trust_mutation_requested: bit(fields[15]),
            };
            assert_eq!(
                classify_association_admission(&facts),
                AssociationAdmissionDecision {
                    action: action(fields[16]),
                    reason: reason(fields[17]),
                },
                "case {}",
                fields[0]
            );
            count += 1;
        }
        assert_eq!(count, 17);
    }
}
