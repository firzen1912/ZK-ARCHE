use proto::lineage_replace_storage_capability::{
    classify_lineage_replace_storage_capability, LineageReplaceStorageCapability,
    LineageReplaceStorageCapabilityDecision,
};

fn flag(value: &str) -> bool {
    match value {
        "0" => false,
        "1" => true,
        _ => panic!("invalid capability flag"),
    }
}

fn expected(value: &str) -> LineageReplaceStorageCapabilityDecision {
    match value {
        "QUALIFIED" => LineageReplaceStorageCapabilityDecision::Qualified,
        "REJECT_DURABILITY" => LineageReplaceStorageCapabilityDecision::RejectDurability,
        "REJECT_POWER_LOSS_RECOVERY" => {
            LineageReplaceStorageCapabilityDecision::RejectPowerLossRecovery
        }
        "REJECT_RECORD_INTEGRITY" => LineageReplaceStorageCapabilityDecision::RejectRecordIntegrity,
        "REJECT_REPLAY_PROTECTION" => {
            LineageReplaceStorageCapabilityDecision::RejectReplayProtection
        }
        "REJECT_FRESHNESS_ANCHOR" => LineageReplaceStorageCapabilityDecision::RejectFreshnessAnchor,
        "REJECT_FRESHNESS_INTEGRITY" => {
            LineageReplaceStorageCapabilityDecision::RejectFreshnessIntegrity
        }
        "REJECT_FRESHNESS_BINDING" => {
            LineageReplaceStorageCapabilityDecision::RejectFreshnessBinding
        }
        _ => panic!("invalid capability decision"),
    }
}

#[test]
fn canonical_lineage_replace_storage_capability_corpus() {
    let corpus =
        include_str!("../../../test-vectors/replay/lineage-replace-storage-capability-v1.txt");
    let mut cases = 0usize;

    for line in corpus.lines() {
        if !line.starts_with("case=") {
            continue;
        }
        let fields: Vec<&str> = line[5..].split('|').collect();
        assert_eq!(fields.len(), 9);
        let capability = LineageReplaceStorageCapability {
            durable_commit_confirmed: flag(fields[1]),
            power_loss_recovery_supported: flag(fields[2]),
            record_integrity_protected: flag(fields[3]),
            replay_protection_supported: flag(fields[4]),
            freshness_anchor_available: flag(fields[5]),
            freshness_anchor_integrity_valid: flag(fields[6]),
            freshness_anchor_lineage_bound: flag(fields[7]),
        };
        assert_eq!(
            classify_lineage_replace_storage_capability(Some(&capability)),
            expected(fields[8])
        );
        cases += 1;
    }

    assert_eq!(cases, 10);
    assert_eq!(
        classify_lineage_replace_storage_capability(None),
        LineageReplaceStorageCapabilityDecision::RejectDurability
    );
}
