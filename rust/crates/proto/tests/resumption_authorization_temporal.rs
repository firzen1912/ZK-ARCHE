use proto::resumption_authorization::{
    classify_resumption_authorization, ResumptionAction, ResumptionAuthorizationFacts,
    ResumptionReason,
};

fn current() -> ResumptionAuthorizationFacts {
    ResumptionAuthorizationFacts {
        credential_present: true,
        credential_integrity_valid: true,
        binding_valid: true,
        expired: false,
        usage_count: 0,
        usage_limit: 2,
        usage_counter_continuity_current: true,
        authorization_context_present: true,
        authorization_context_fresh: true,
        authorization_generation_bound: true,
        authorization_generation_current: true,
        revocation_current: true,
        explicitly_revoked: false,
        lineage_current: true,
        restart_continuity_current: true,
        credential_epoch_current: true,
        session_invalidated: false,
        privacy_identifier_state_current: true,
        repeated_identifier_linkable: false,
        peer_match: true,
        deployment_match: true,
        audience_match: true,
        profile_match: true,
        rollback_suspected: false,
    }
}

fn expect(facts: &ResumptionAuthorizationFacts, action: ResumptionAction, reason: ResumptionReason) {
    let got = classify_resumption_authorization(facts);
    assert_eq!(got.action, action);
    assert_eq!(got.reason, reason);
}

#[test]
fn retained_resumption_eligibility_is_revalidated_after_lifecycle_change() {
    let base = current();
    expect(&base, ResumptionAction::Resume, ResumptionReason::Current);

    let mut f = base;
    f.authorization_generation_current = false;
    expect(&f, ResumptionAction::FullAuthRequired, ResumptionReason::AuthorizationGenerationStale);

    let mut f = base;
    f.revocation_current = false;
    expect(&f, ResumptionAction::Reject, ResumptionReason::RevocationStale);

    let mut f = base;
    f.explicitly_revoked = true;
    expect(&f, ResumptionAction::Reject, ResumptionReason::Revoked);

    let mut f = base;
    f.lineage_current = false;
    expect(&f, ResumptionAction::Reject, ResumptionReason::LineageStale);

    let mut f = base;
    f.credential_epoch_current = false;
    expect(&f, ResumptionAction::FullAuthRequired, ResumptionReason::CredentialEpochStale);

    let mut f = base;
    f.session_invalidated = true;
    expect(&f, ResumptionAction::Reject, ResumptionReason::SessionInvalidated);
}

#[test]
fn reuse_and_restart_continuity_fail_closed_at_the_boundary() {
    let mut f = current();
    f.usage_count = 1;
    expect(&f, ResumptionAction::Resume, ResumptionReason::Current);

    f.usage_count = 2;
    expect(&f, ResumptionAction::FullAuthRequired, ResumptionReason::ReuseLimitReached);

    let mut f = current();
    f.usage_counter_continuity_current = false;
    expect(&f, ResumptionAction::Reject, ResumptionReason::UsageCounterContinuityStale);

    let mut f = current();
    f.restart_continuity_current = false;
    expect(&f, ResumptionAction::Reject, ResumptionReason::RestartContinuityStale);

    let mut f = current();
    f.rollback_suspected = true;
    expect(&f, ResumptionAction::Reject, ResumptionReason::RollbackSuspected);
}

#[test]
fn privacy_or_binding_drift_cannot_be_repaired_by_the_resumption_secret() {
    let base = current();

    let mut f = base;
    f.binding_valid = false;
    expect(&f, ResumptionAction::FullAuthRequired, ResumptionReason::BindingMismatch);

    let mut f = base;
    f.privacy_identifier_state_current = false;
    expect(&f, ResumptionAction::FullAuthRequired, ResumptionReason::PrivacyIdentifierStateStale);

    let mut f = base;
    f.repeated_identifier_linkable = true;
    expect(&f, ResumptionAction::FullAuthRequired, ResumptionReason::RepeatedIdentifierLinkable);
}
