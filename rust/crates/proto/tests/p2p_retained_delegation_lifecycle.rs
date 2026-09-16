use proto::p2p_delegation::{
    classify_p2p_delegation, P2pDelegationAction, P2pDelegationFacts, P2pDelegationReason,
};

fn baseline() -> P2pDelegationFacts {
    P2pDelegationFacts {
        issuer_trusted: true, issuer_trust_local: true, holder_authenticated: true,
        grant_present: true, grant_integrity_valid: true, scope_match: true,
        audience_match: true, deployment_match: true, validity_current: true,
        authorization_generation_bound: true, authorization_generation_current: true,
        epoch_current: true, revocation_current: true, explicitly_revoked: false,
        lineage_current: true, depth_within_limit: true, redelegation_permitted: false,
        redelegation_requested: false, rollback_suspected: false,
    }
}

fn reason(s: &str) -> P2pDelegationReason {
    match s {
        "AUTHORIZATION_GENERATION_STALE" => P2pDelegationReason::AuthorizationGenerationStale,
        "REVOCATION_STALE" => P2pDelegationReason::RevocationStale,
        "REVOKED" => P2pDelegationReason::Revoked,
        "LINEAGE_STALE" => P2pDelegationReason::LineageStale,
        "ISSUER_TRUST_NOT_LOCAL" => P2pDelegationReason::IssuerTrustNotLocal,
        "EPOCH_STALE" => P2pDelegationReason::EpochStale,
        "ROLLBACK_SUSPECTED" => P2pDelegationReason::RollbackSuspected,
        _ => panic!("unknown reason {s}"),
    }
}

#[test]
fn retained_delegation_temporally_fails_closed_across_peer_classes() {
    let corpus = include_str!("../../../test-vectors/p2p/retained-delegation-lifecycle-v1.txt");
    let mut cases = 0usize;
    for line in corpus.lines() {
        let Some(case) = line.strip_prefix("case=") else { continue };
        let f: Vec<&str> = case.split('|').collect();
        assert_eq!(f.len(), 7, "{}", f[0]);
        assert!(matches!(f[1], "mcu-core"|"mcu-plus"|"linux-edge"|"accelerated-edge"));
        assert!(matches!(f[2], "mcu-core"|"mcu-plus"|"linux-edge"|"accelerated-edge"));
        assert!(matches!(f[3], "0"|"1"));
        let mut facts = baseline();
        let before = classify_p2p_delegation(&facts);
        assert_eq!(before.action, P2pDelegationAction::Accept, "{} precondition", f[0]);
        assert_eq!(before.reason, P2pDelegationReason::Current, "{} precondition", f[0]);
        match f[4] {
            "authorization_generation_stale" => facts.authorization_generation_current = false,
            "revocation_stale" => facts.revocation_current = false,
            "revoked" => facts.explicitly_revoked = true,
            "lineage_stale" => facts.lineage_current = false,
            "issuer_trust_not_local" => facts.issuer_trust_local = false,
            "epoch_stale" => facts.epoch_current = false,
            "rollback_suspected" => facts.rollback_suspected = true,
            other => panic!("unknown mutation {other}"),
        }
        let after = classify_p2p_delegation(&facts);
        assert_eq!(f[5], "DENY");
        assert_eq!(after.action, P2pDelegationAction::Deny, "{}", f[0]);
        assert_eq!(after.reason, reason(f[6]), "{}", f[0]);
        cases += 1;
    }
    assert_eq!(cases, 10);
}
