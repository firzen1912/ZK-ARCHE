use proto::association_admission::{
    classify_association_admission, AssociationAdmissionAction, AssociationAdmissionFacts,
};

fn bit(value: &str) -> bool {
    match value {
        "true" => true,
        "false" => false,
        _ => panic!("invalid bool: {value}"),
    }
}

fn known_peer(value: &str) -> bool {
    matches!(value, "mcu-core" | "linux-edge")
}

#[test]
fn canonical_p2p_common_contract_lifecycle_corpus() {
    let corpus = include_str!("../../../test-vectors/p2p/common-contract-lifecycle-v2.txt");
    let mut cases = 0usize;
    let mut established = 0usize;
    let mut failed = 0usize;
    let mut offline_established = 0usize;
    let mut cross_class = 0usize;

    for line in corpus.lines() {
        if line.starts_with('#') || !line.starts_with("XC2-") {
            continue;
        }

        let fields: Vec<&str> = line.split('|').collect();
        assert_eq!(fields.len(), 19, "bad corpus row: {line}");
        assert!(known_peer(fields[1]));
        assert!(known_peer(fields[2]));

        let infrastructure_available = bit(fields[3]);
        let authorization_generation_current = bit(fields[8]);
        let restart_continuity_current = bit(fields[13]);
        let mandatory_floor_compatible = bit(fields[14]);

        let facts = AssociationAdmissionFacts {
            auth_complete: bit(fields[4]),
            preexisting_trust_record: bit(fields[5]),
            authorization_present: bit(fields[6]),
            authorization_fresh: bit(fields[7]),
            revocation_current: bit(fields[9]),
            explicitly_revoked: bit(fields[10]),
            lineage_current: bit(fields[11]),
            replay_continuity_current: bit(fields[12]),
            binding_required: bit(fields[15]),
            binding_valid: bit(fields[16]),
            rollback_suspected: false,
            trust_mutation_requested: bit(fields[17]),
        };

        let mut action = classify_association_admission(&facts).action;
        if !authorization_generation_current
            || !restart_continuity_current
            || !mandatory_floor_compatible
        {
            action = AssociationAdmissionAction::FailClosed;
        }

        match fields[18] {
            "ESTABLISH" => {
                assert_eq!(
                    action,
                    AssociationAdmissionAction::Establish,
                    "{}",
                    fields[0]
                );
                established += 1;
                if !infrastructure_available {
                    offline_established += 1;
                }
            }
            "FAIL_CLOSED" => {
                assert_eq!(
                    action,
                    AssociationAdmissionAction::FailClosed,
                    "{}",
                    fields[0]
                );
                failed += 1;
            }
            other => panic!("invalid expected action: {other}"),
        }

        if fields[1] != fields[2] {
            cross_class += 1;
        }
        cases += 1;
    }

    assert_eq!(cases, 20);
    assert_eq!(established, 6);
    assert_eq!(failed, 14);
    assert_eq!(offline_established, 5);
    assert!(cross_class >= 16);
}
