use proto::enrollment_grant::{
    classify_enrollment_grant, EnrollmentGrantAction, EnrollmentGrantFacts, EnrollmentGrantReason,
};

fn current() -> EnrollmentGrantFacts {
    EnrollmentGrantFacts {
        explicit_enroll_operation: true,
        normal_auth_path: false,
        commissioner_authenticated: true,
        commissioner_authorized: true,
        commissioner_authorization_fresh: true,
        commissioner_authorization_generation_bound: true,
        commissioner_authorization_generation_current: true,
        commissioner_not_revoked: true,
        enrollment_nonce_unused: true,
        subject_possession_verified: true,
        requested_authority_within_commissioner_scope: true,
        scope_bounded: true,
        audience_bound: true,
        deployment_bound: true,
        validity_bounded: true,
        epoch_current: true,
        revocation_current: true,
        lineage_current: true,
        delegation_depth_within_limit: true,
        rollback_suspected: false,
    }
}

fn assert_denied(facts: &EnrollmentGrantFacts, reason: EnrollmentGrantReason) {
    let got = classify_enrollment_grant(facts);
    assert_eq!(got.action, EnrollmentGrantAction::Deny);
    assert_eq!(got.reason, reason);
}

#[test]
fn previously_issuable_grant_is_invalidated_by_authority_lifecycle_change() {
    let baseline = current();
    let issued = classify_enrollment_grant(&baseline);
    assert_eq!(issued.action, EnrollmentGrantAction::Issue);
    assert_eq!(issued.reason, EnrollmentGrantReason::Current);

    let mut generation = baseline;
    generation.commissioner_authorization_generation_current = false;
    assert_denied(
        &generation,
        EnrollmentGrantReason::CommissionerAuthorizationGenerationStale,
    );

    let mut commissioner_revoked = baseline;
    commissioner_revoked.commissioner_not_revoked = false;
    assert_denied(
        &commissioner_revoked,
        EnrollmentGrantReason::CommissionerRevoked,
    );

    let mut revocation_view = baseline;
    revocation_view.revocation_current = false;
    assert_denied(&revocation_view, EnrollmentGrantReason::RevocationStale);

    let mut lineage = baseline;
    lineage.lineage_current = false;
    assert_denied(&lineage, EnrollmentGrantReason::LineageStale);

    let mut epoch = baseline;
    epoch.epoch_current = false;
    assert_denied(&epoch, EnrollmentGrantReason::EpochStale);
}

#[test]
fn issued_grant_does_not_make_replay_or_rollback_acceptable() {
    let baseline = current();
    assert_eq!(
        classify_enrollment_grant(&baseline).action,
        EnrollmentGrantAction::Issue
    );

    let mut replay = baseline;
    replay.enrollment_nonce_unused = false;
    assert_denied(&replay, EnrollmentGrantReason::EnrollmentReplayDetected);

    let mut rollback = baseline;
    rollback.rollback_suspected = true;
    assert_denied(&rollback, EnrollmentGrantReason::RollbackSuspected);
}

#[test]
fn normal_auth_never_inherits_enrollment_mutation_authority() {
    let mut facts = current();
    facts.normal_auth_path = true;
    assert_denied(&facts, EnrollmentGrantReason::NormalAuthForbidden);
}
