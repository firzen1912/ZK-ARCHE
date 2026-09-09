#![no_main]

use libfuzzer_sys::fuzz_target;
use proto::p2p_delegation::{
    classify_p2p_delegation, P2pDelegationAction, P2pDelegationFacts, P2pDelegationReason,
};

fn bit(data: &[u8], index: usize) -> bool {
    data.get(index / 8)
        .map_or(false, |byte| byte & (1u8 << (index % 8)) != 0)
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

    if facts.rollback_suspected {
        assert_eq!(decision.action, P2pDelegationAction::Deny);
        assert_eq!(decision.reason, P2pDelegationReason::RollbackSuspected);
    }

    if !facts.issuer_trust_local {
        assert_eq!(decision.action, P2pDelegationAction::Deny);
    }

    if facts.redelegation_requested && !facts.redelegation_permitted {
        assert_eq!(decision.action, P2pDelegationAction::Deny);
    }
});
