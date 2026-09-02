//! Wire-neutral authorization-aware resumption decision core.
//!
//! A resumption secret is not authorization. Restart continuity loss, explicit
//! invalidation, stale authority state, or rollback cannot be repaired by
//! possession of a resumption credential.

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
    pub restart_continuity_current: bool,
    pub credential_epoch_current: bool,
    pub session_invalidated: bool,
    pub peer_match: bool,
    pub deployment_match: bool,
    pub audience_match: bool,
    pub profile_match: bool,
    pub rollback_suspected: bool,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ResumptionAction { Resume, FullAuthRequired, Reject }

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ResumptionReason {
    Current, InvalidFacts, RollbackSuspected, CredentialMissing, CredentialInvalid,
    BindingMismatch, Expired, ReuseLimitReached, AuthorizationContextMissing,
    AuthorizationStale, RevocationStale, Revoked, LineageStale,
    RestartContinuityStale, CredentialEpochStale, SessionInvalidated,
    PeerMismatch, DeploymentMismatch, AudienceMismatch, ProfileMismatch,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct ResumptionAuthorizationDecision {
    pub action: ResumptionAction,
    pub reason: ResumptionReason,
}

fn d(action: ResumptionAction, reason: ResumptionReason) -> ResumptionAuthorizationDecision {
    ResumptionAuthorizationDecision { action, reason }
}

pub fn classify_resumption_authorization(f: &ResumptionAuthorizationFacts) -> ResumptionAuthorizationDecision {
    use ResumptionAction::*;
    use ResumptionReason::*;
    if f.rollback_suspected { return d(Reject, RollbackSuspected); }
    if !f.restart_continuity_current { return d(Reject, RestartContinuityStale); }
    if f.session_invalidated { return d(Reject, SessionInvalidated); }
    if !f.credential_epoch_current { return d(FullAuthRequired, CredentialEpochStale); }
    if !f.credential_present { return d(FullAuthRequired, CredentialMissing); }
    if !f.credential_integrity_valid { return d(FullAuthRequired, CredentialInvalid); }
    if !f.binding_valid { return d(FullAuthRequired, BindingMismatch); }
    if f.expired { return d(FullAuthRequired, Expired); }
    if f.usage_limit == 0 || f.usage_count >= f.usage_limit { return d(FullAuthRequired, ReuseLimitReached); }
    if !f.authorization_context_present { return d(FullAuthRequired, AuthorizationContextMissing); }
    if !f.authorization_context_fresh { return d(FullAuthRequired, AuthorizationStale); }
    if !f.revocation_current { return d(Reject, RevocationStale); }
    if f.explicitly_revoked { return d(Reject, Revoked); }
    if !f.lineage_current { return d(Reject, LineageStale); }
    if !f.peer_match { return d(FullAuthRequired, PeerMismatch); }
    if !f.deployment_match { return d(FullAuthRequired, DeploymentMismatch); }
    if !f.audience_match { return d(FullAuthRequired, AudienceMismatch); }
    if !f.profile_match { return d(FullAuthRequired, ProfileMismatch); }
    d(Resume, Current)
}

#[cfg(test)]
mod tests {
    use super::*;
    fn b(v: &str) -> bool { match v { "0" => false, "1" => true, _ => panic!("bad bit") } }
    fn a(v: &str) -> ResumptionAction { match v {
        "RESUME" => ResumptionAction::Resume,
        "FULL_AUTH_REQUIRED" => ResumptionAction::FullAuthRequired,
        "REJECT" => ResumptionAction::Reject,
        _ => panic!("bad action"),
    }}
    fn r(v: &str) -> ResumptionReason {
        use ResumptionReason::*;
        match v {
            "CURRENT"=>Current, "ROLLBACK_SUSPECTED"=>RollbackSuspected,
            "CREDENTIAL_MISSING"=>CredentialMissing, "CREDENTIAL_INVALID"=>CredentialInvalid,
            "BINDING_MISMATCH"=>BindingMismatch, "EXPIRED"=>Expired,
            "REUSE_LIMIT_REACHED"=>ReuseLimitReached,
            "AUTHORIZATION_CONTEXT_MISSING"=>AuthorizationContextMissing,
            "AUTHORIZATION_STALE"=>AuthorizationStale, "REVOCATION_STALE"=>RevocationStale,
            "REVOKED"=>Revoked, "LINEAGE_STALE"=>LineageStale,
            "RESTART_CONTINUITY_STALE"=>RestartContinuityStale,
            "CREDENTIAL_EPOCH_STALE"=>CredentialEpochStale,
            "SESSION_INVALIDATED"=>SessionInvalidated, "PEER_MISMATCH"=>PeerMismatch,
            "DEPLOYMENT_MISMATCH"=>DeploymentMismatch, "AUDIENCE_MISMATCH"=>AudienceMismatch,
            "PROFILE_MISMATCH"=>ProfileMismatch, _=>panic!("bad reason"),
        }
    }
    #[test]
    fn canonical_corpus() {
        let corpus=include_str!("../../../test-vectors/state/resumption-authorization-v2.txt");
        let mut n=0;
        for line in corpus.lines().filter(|l| l.starts_with("case=")) {
            let x:Vec<&str>=line[5..].split('|').collect(); assert_eq!(x.len(),22);
            let f=ResumptionAuthorizationFacts {
                credential_present:b(x[1]), credential_integrity_valid:b(x[2]), binding_valid:b(x[3]),
                expired:b(x[4]), usage_count:x[5].parse().unwrap(), usage_limit:x[6].parse().unwrap(),
                authorization_context_present:b(x[7]), authorization_context_fresh:b(x[8]),
                revocation_current:b(x[9]), explicitly_revoked:b(x[10]), lineage_current:b(x[11]),
                restart_continuity_current:b(x[12]), credential_epoch_current:b(x[13]),
                session_invalidated:b(x[14]), peer_match:b(x[15]), deployment_match:b(x[16]),
                audience_match:b(x[17]), profile_match:b(x[18]), rollback_suspected:b(x[19]),
            };
            assert_eq!(classify_resumption_authorization(&f), ResumptionAuthorizationDecision{action:a(x[20]),reason:r(x[21])}, "{}", x[0]);
            n+=1;
        }
        assert_eq!(n,19);
    }
}
