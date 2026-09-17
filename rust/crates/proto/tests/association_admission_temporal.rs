use zk_arche_proto::association_admission::{
    classify_association_admission, AssociationAdmissionAction, AssociationAdmissionFacts,
    AssociationAdmissionReason,
};

fn current() -> AssociationAdmissionFacts {
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

fn assert_fail_closed(facts: AssociationAdmissionFacts, reason: AssociationAdmissionReason) {
    let decision = classify_association_admission(&facts);
    assert_eq!(decision.action, AssociationAdmissionAction::FailClosed);
    assert_eq!(decision.reason, reason);
}

#[test]
fn retained_association_revalidates_lifecycle_state() {
    let baseline = current();
    let admitted = classify_association_admission(&baseline);
    assert_eq!(admitted.action, AssociationAdmissionAction::Establish);
    assert_eq!(admitted.reason, AssociationAdmissionReason::Current);

    let mut facts = baseline;
    facts.authorization_generation_current = false;
    assert_fail_closed(facts, AssociationAdmissionReason::AuthorizationGenerationStale);

    let mut facts = baseline;
    facts.revocation_current = false;
    assert_fail_closed(facts, AssociationAdmissionReason::RevocationStale);

    let mut facts = baseline;
    facts.explicitly_revoked = true;
    assert_fail_closed(facts, AssociationAdmissionReason::Revoked);

    let mut facts = baseline;
    facts.lineage_current = false;
    assert_fail_closed(facts, AssociationAdmissionReason::LineageStale);

    let mut facts = baseline;
    facts.replay_continuity_current = false;
    assert_fail_closed(facts, AssociationAdmissionReason::ReplayContinuityStale);

    let mut facts = baseline;
    facts.restart_continuity_current = false;
    assert_fail_closed(facts, AssociationAdmissionReason::RestartContinuityStale);

    let mut facts = baseline;
    facts.usage_counter_continuity_current = false;
    assert_fail_closed(facts, AssociationAdmissionReason::UsageCounterContinuityStale);

    let mut facts = baseline;
    facts.binding_valid = false;
    assert_fail_closed(facts, AssociationAdmissionReason::BindingInvalid);
}

#[test]
fn normal_auth_cannot_mutate_trust_after_admission() {
    let mut facts = current();
    facts.trust_mutation_requested = true;
    assert_fail_closed(facts, AssociationAdmissionReason::TrustMutationRequested);
}

#[test]
fn rollback_dominates_other_retention_failures() {
    let mut facts = current();
    facts.rollback_suspected = true;
    facts.authorization_generation_current = false;
    facts.revocation_current = false;
    facts.binding_valid = false;
    assert_fail_closed(facts, AssociationAdmissionReason::RollbackSuspected);
}
