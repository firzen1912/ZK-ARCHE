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
    let corpus = include_str!("../../../test-vectors/p2p/common-contract-lifecycle-v1.txt");
    let mut cases = 0usize;
    let mut established = 0usize;
    let mut failed = 0usize;

    for line in corpus.lines() {
        if line.starts_with('#') || !line.starts_with("XC-") {
            continue;
        }

        let fields: Vec<&str> = line.split('|').collect();
        assert_eq!(fields.len(), 17, "bad corpus row: {line}");
        assert!(known_peer(fields[1]));
        assert!(known_peer(fields[2]));

        let _infrastructure_available = bit(fields[3]);
        let mandatory_floor_compatible = bit(fields[12]);

        let facts = AssociationAdmissionFacts {
            auth_complete: bit(fields[4]),
            preexisting_trust_record: bit(fields[5]),
            authorization_present: bit(fields[6]),
            authorization_fresh: bit(fields[7]),
            revocation_current: bit(fields[8]),
            explicitly_revoked: bit(fields[9]),
            lineage_current: bit(fields[10]),
            replay_continuity_current: bit(fields[11]),
            binding_required: bit(fields[13]),
            binding_valid: bit(fields[14]),
            rollback_suspected: false,
            trust_mutation_requested: bit(fields[15]),
        };

        let mut action = classify_association_admission(&facts).action;
        if !mandatory_floor_compatible {
            action = AssociationAdmissionAction::FailClosed;
        }

        match fields[16] {
            "ESTABLISH" => {
                assert_eq!(action, AssociationAdmissionAction::Establish, "{}", fields[0]);
                established += 1;
            }
            "FAIL_CLOSED" => {
                assert_eq!(action, AssociationAdmissionAction::FailClosed, "{}", fields[0]);
                failed += 1;
            }
            other => panic!("invalid expected action: {other}"),
        }
        cases += 1;
    }

    assert_eq!(cases, 16);
    assert_eq!(established, 5);
    assert_eq!(failed, 11);
}
