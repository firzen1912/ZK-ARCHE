#![no_main]

use libfuzzer_sys::fuzz_target;
use proto::p2p_delegation::{
    classify_p2p_delegation, P2pDelegationAction, P2pDelegationFacts, P2pDelegationReason,
};

fn bit(data: &[u8], index: usize) -> bool {
    data.get(index / 8)
        .map_or(false, |byte| byte & (1u8 << (index % 8)) != 0)
}

fn expected_reason(facts: &P2pDelegationFacts) -> P2pDelegationReason {
    if facts.issuer_trust_local && !facts.issuer_trusted {
        P2pDelegationReason::InvalidFacts
    } else if facts.rollback_suspected {
        P2pDelegationReason::RollbackSuspected
    } else if !facts.issuer_trusted {
        P2pDelegationReason::IssuerUntrusted
    } else if !facts.issuer_trust_local {
        P2pDelegationReason::IssuerTrustNotLocal
    } else if !facts.holder_authenticated {
        P2pDelegationReason::HolderUnauthenticated
    } else if !facts.grant_present {
        P2pDelegationReason::GrantMissing
    } else if !facts.grant_integrity_valid {
        P2pDelegationReason::GrantInvalid
    } else if !facts.scope_match {
        P2pDelegationReason::ScopeMismatch
    } else if !facts.audience_match {
        P2pDelegationReason::AudienceMismatch
    } else if !facts.deployment_match {
        P2pDelegationReason::DeploymentMismatch
    } else if !facts.validity_current {
        P2pDelegationReason::ExpiredOrNotYetValid
    } else if !facts.authorization_generation_bound {
        P2pDelegationReason::AuthorizationGenerationUnbound
    } else if !facts.authorization_generation_current {
        P2pDelegationReason::AuthorizationGenerationStale
    } else if !facts.epoch_current {
        P2pDelegationReason::EpochStale
    } else if !facts.revocation_current {
        P2pDelegationReason::RevocationStale
    } else if facts.explicitly_revoked {
        P2pDelegationReason::Revoked
    } else if !facts.lineage_current {
        P2pDelegationReason::LineageStale
    } else if !facts.depth_within_limit {
        P2pDelegationReason::DepthExceeded
    } else if facts.redelegation_requested && !facts.redelegation_permitted {
        P2pDelegationReason::RedelegationForbidden
    } else {
        P2pDelegationReason::Current
    }
}

fuzz_target!(|data: &[u8]| {
    let facts = P2pDelegationFacts {
        issuer_trusted: bit(data, 0),
        issuer_trust_local: bit(data, 1),
        holder_authenticated: bit(data, 2),
        grant_present: bit(data, 3),
        grant_integrity_valid: bit(data, 4),
        scope_match: bit(data, 5),
        audience_match: bit(data, 6),
        deployment_match: bit(data, 7),
        validity_current: bit(data, 8),
        authorization_generation_bound: bit(data, 9),
        authorization_generation_current: bit(data, 10),
        epoch_current: bit(data, 11),
        revocation_current: bit(data, 12),
        explicitly_revoked: bit(data, 13),
        lineage_current: bit(data, 14),
        depth_within_limit: bit(data, 15),
        redelegation_permitted: bit(data, 16),
        redelegation_requested: bit(data, 17),
        rollback_suspected: bit(data, 18),
    };

    let decision = classify_p2p_delegation(&facts);
    let expected_reason = expected_reason(&facts);
    let expected_action = if expected_reason == P2pDelegationReason::Current {
        P2pDelegationAction::Accept
    } else {
        P2pDelegationAction::Deny
    };

    // Every reachable fact combination must preserve the normative fail-closed
    // discriminator ordering, not merely return some denial. This also keeps
    // internally contradictory local-trust state ahead of rollback suspicion.
    assert_eq!(decision.reason, expected_reason);
    assert_eq!(decision.action, expected_action);

    if decision.action == P2pDelegationAction::Accept {
        assert!(facts.issuer_trusted);
        assert!(facts.issuer_trust_local);
        assert!(facts.holder_authenticated);
        assert!(facts.grant_present);
        assert!(facts.grant_integrity_valid);
        assert!(facts.scope_match);
        assert!(facts.audience_match);
        assert!(facts.deployment_match);
        assert!(facts.validity_current);
        assert!(facts.authorization_generation_bound);
        assert!(facts.authorization_generation_current);
        assert!(facts.epoch_current);
        assert!(facts.revocation_current);
        assert!(!facts.explicitly_revoked);
        assert!(facts.lineage_current);
        assert!(facts.depth_within_limit);
        assert!(!facts.redelegation_requested || facts.redelegation_permitted);
        assert!(!facts.rollback_suspected);
    }

    // Third-party/transitive trust must never become a local delegation root.
    // Once the earlier invalid-facts/rollback/issuer-presence guards are clear,
    // downstream grant or lifecycle state cannot reclassify this condition.
    if !facts.rollback_suspected
        && facts.issuer_trusted
        && !facts.issuer_trust_local
    {
        assert_eq!(decision.action, P2pDelegationAction::Deny);
        assert_eq!(decision.reason, P2pDelegationReason::IssuerTrustNotLocal);
    }
});
