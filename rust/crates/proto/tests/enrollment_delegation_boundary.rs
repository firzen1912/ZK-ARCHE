use proto::enrollment_grant::{
    classify_enrollment_grant, EnrollmentGrantAction, EnrollmentGrantFacts,
};
use proto::p2p_delegation::{
    classify_p2p_delegation, P2pDelegationAction, P2pDelegationFacts, P2pDelegationReason,
};

fn current_enrollment() -> EnrollmentGrantFacts {
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

fn current_delegation() -> P2pDelegationFacts {
    P2pDelegationFacts {
        issuer_trusted: true,
        issuer_trust_local: true,
        holder_authenticated: true,
        grant_present: true,
        grant_integrity_valid: true,
        scope_match: true,
        audience_match: true,
        deployment_match: true,
        validity_current: true,
        authorization_generation_bound: true,
        authorization_generation_current: true,
        epoch_current: true,
        revocation_current: true,
        explicitly_revoked: false,
        lineage_current: true,
        depth_within_limit: true,
        redelegation_permitted: false,
        redelegation_requested: false,
        rollback_suspected: false,
    }
}

#[test]
fn successful_enrollment_grant_does_not_bootstrap_local_issuer_trust() {
    let enrollment = classify_enrollment_grant(&current_enrollment());
    assert_eq!(enrollment.action, EnrollmentGrantAction::Issue);

    // Grant issuance is an explicit trust/authorization mutation at the
    // enrollment authority. It is not evidence that an arbitrary verifier has
    // locally trusted the resulting delegation issuer. Preserve the local,
    // non-transitive trust boundary even after a valid enrollment decision.
    let mut delegation = current_delegation();
    delegation.issuer_trust_local = false;

    let decision = classify_p2p_delegation(&delegation);
    assert_eq!(decision.action, P2pDelegationAction::Deny);
    assert_eq!(decision.reason, P2pDelegationReason::IssuerTrustNotLocal);
}

#[test]
fn retained_delegation_fails_after_authorization_generation_changes() {
    let enrollment = classify_enrollment_grant(&current_enrollment());
    assert_eq!(enrollment.action, EnrollmentGrantAction::Issue);

    // A previously acceptable delegation cannot survive a later authorization
    // generation change merely because the original enrollment was valid.
    let mut delegation = current_delegation();
    delegation.authorization_generation_current = false;

    let decision = classify_p2p_delegation(&delegation);
    assert_eq!(decision.action, P2pDelegationAction::Deny);
    assert_eq!(
        decision.reason,
        P2pDelegationReason::AuthorizationGenerationStale
    );
}

#[test]
fn retained_delegation_fails_after_revocation_or_lineage_change() {
    let enrollment = classify_enrollment_grant(&current_enrollment());
    assert_eq!(enrollment.action, EnrollmentGrantAction::Issue);

    let mut revoked = current_delegation();
    revoked.explicitly_revoked = true;
    let revoked_decision = classify_p2p_delegation(&revoked);
    assert_eq!(revoked_decision.action, P2pDelegationAction::Deny);
    assert_eq!(revoked_decision.reason, P2pDelegationReason::Revoked);

    let mut stale_lineage = current_delegation();
    stale_lineage.lineage_current = false;
    let lineage_decision = classify_p2p_delegation(&stale_lineage);
    assert_eq!(lineage_decision.action, P2pDelegationAction::Deny);
    assert_eq!(lineage_decision.reason, P2pDelegationReason::LineageStale);
}
