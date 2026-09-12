use proto::association_admission::{
    classify_association_admission, AssociationAdmissionAction, AssociationAdmissionFacts,
};
use proto::data_release_authorization::{
    classify_data_release, DataReleaseAction, DataReleaseFacts, DataReleaseReason,
};

#[derive(Clone, Copy)]
enum Mutation {
    AuthorizationGeneration,
    Revoked,
    Lineage,
    RestartContinuity,
    UsageContinuity,
    Rollback,
    Binding,
}

fn current_association() -> AssociationAdmissionFacts {
    AssociationAdmissionFacts {
        auth_complete: true,
        preexisting_trust_record: true,
        authorization_present: true,
        authorization_fresh: true,
        authorization_generation_bound: true,
        authorization_generation_current: true,
        revocation_current: true,
        explicitly_revoked: false,
        lineage_current: true,
        replay_continuity_current: true,
        restart_continuity_current: true,
        usage_counter_continuity_current: true,
        binding_required: true,
        binding_valid: true,
        rollback_suspected: false,
        trust_mutation_requested: false,
    }
}

fn current_release() -> DataReleaseFacts {
    DataReleaseFacts {
        authenticated: true,
        device_release_authority_present: true,
        device_release_authority_current: true,
        protected_data_encrypted: true,
        release_key_scope_match: true,
        authorization_present: true,
        authorization_fresh: true,
        authorization_generation_bound: true,
        authorization_generation_current: true,
        revocation_current: true,
        explicitly_revoked: false,
        lineage_current: true,
        holder_match: true,
        audience_match: true,
        purpose_match: true,
        data_type_match: true,
        policy_match: true,
        epoch_match: true,
        channel_binding_required: true,
        channel_binding_valid: true,
        release_operation_unused: true,
        rollback_suspected: false,
    }
}

fn apply_mutation(
    mutation: Mutation,
    association: &mut AssociationAdmissionFacts,
    release: &mut DataReleaseFacts,
) {
    match mutation {
        Mutation::AuthorizationGeneration => {
            association.authorization_generation_current = false;
            release.authorization_generation_current = false;
        }
        Mutation::Revoked => {
            association.explicitly_revoked = true;
            release.explicitly_revoked = true;
        }
        Mutation::Lineage => {
            association.lineage_current = false;
            release.lineage_current = false;
        }
        Mutation::RestartContinuity => association.restart_continuity_current = false,
        Mutation::UsageContinuity => association.usage_counter_continuity_current = false,
        Mutation::Rollback => {
            association.rollback_suspected = true;
            release.rollback_suspected = true;
        }
        Mutation::Binding => {
            association.binding_valid = false;
            release.channel_binding_valid = false;
        }
    }
}

#[test]
fn retained_association_loss_cannot_carry_new_data_release() {
    let mutations = [
        Mutation::AuthorizationGeneration,
        Mutation::Revoked,
        Mutation::Lineage,
        Mutation::RestartContinuity,
        Mutation::UsageContinuity,
        Mutation::Rollback,
        Mutation::Binding,
    ];

    for mutation in mutations {
        let mut association = current_association();
        let mut release = current_release();

        assert_eq!(
            classify_association_admission(&association).action,
            AssociationAdmissionAction::Establish
        );
        assert_eq!(classify_data_release(&release).action, DataReleaseAction::Release);

        apply_mutation(mutation, &mut association, &mut release);
        assert_eq!(
            classify_association_admission(&association).action,
            AssociationAdmissionAction::FailClosed
        );

        // Retained traffic keys or an open transport cannot synthesize current
        // authentication authority after association admission fails closed.
        release.authenticated = false;
        assert_ne!(classify_data_release(&release).action, DataReleaseAction::Release);
    }
}

#[test]
fn successor_association_does_not_revive_predecessor_release_authority() {
    let successor_association = current_association();
    assert_eq!(
        classify_association_admission(&successor_association).action,
        AssociationAdmissionAction::Establish
    );

    let mut predecessor_release = current_release();
    assert_eq!(
        classify_data_release(&predecessor_release).action,
        DataReleaseAction::Release
    );

    // A valid successor AUTH/association after LINEAGE_REPLACE does not make
    // predecessor-bound DATA release authority current again.
    predecessor_release.lineage_current = false;
    predecessor_release.authenticated = true;
    let decision = classify_data_release(&predecessor_release);
    assert_eq!(decision.action, DataReleaseAction::Deny);
    assert_eq!(decision.reason, DataReleaseReason::LineageStale);
}

#[test]
fn fresh_auth_and_rebinding_do_not_revive_stale_release_authority() {
    let mut stale_generation = current_release();
    stale_generation.authorization_generation_current = false;
    stale_generation.channel_binding_valid = false;
    stale_generation.authenticated = true;
    stale_generation.channel_binding_valid = true;
    let generation_decision = classify_data_release(&stale_generation);
    assert_eq!(generation_decision.action, DataReleaseAction::Deny);
    assert_eq!(
        generation_decision.reason,
        DataReleaseReason::AuthorizationGenerationStale
    );

    let mut revoked = current_release();
    revoked.explicitly_revoked = true;
    revoked.channel_binding_valid = false;
    revoked.authenticated = true;
    revoked.channel_binding_valid = true;
    let revoked_decision = classify_data_release(&revoked);
    assert_eq!(revoked_decision.action, DataReleaseAction::Deny);
    assert_eq!(revoked_decision.reason, DataReleaseReason::Revoked);

    let mut stale_lineage = current_release();
    stale_lineage.lineage_current = false;
    stale_lineage.channel_binding_valid = false;
    stale_lineage.authenticated = true;
    stale_lineage.channel_binding_valid = true;
    let lineage_decision = classify_data_release(&stale_lineage);
    assert_eq!(lineage_decision.action, DataReleaseAction::Deny);
    assert_eq!(lineage_decision.reason, DataReleaseReason::LineageStale);
}

#[test]
fn consumed_operation_remains_rejected_after_successful_release() {
    let mut release = current_release();
    assert_eq!(classify_data_release(&release).action, DataReleaseAction::Release);

    release.release_operation_unused = false;
    let replay = classify_data_release(&release);
    assert_eq!(replay.action, DataReleaseAction::Deny);
    assert_eq!(replay.reason, DataReleaseReason::ReleaseReplayDetected);
}
