use proto::lineage_replace::LineageReplaceState;
use proto::lineage_replace_recovery::recover_lineage_replace;
use proto::lineage_replace_recovery::LineageReplaceRecoveryFacts;

const CORPUS: &str =
    include_str!("../../../test-vectors/replay/lineage-replace-recovery-v1.txt");

fn bit(value: &str) -> bool {
    match value {
        "0" => false,
        "1" => true,
        other => panic!("invalid recovery bit: {other}"),
    }
}

fn expected_state(name: &str) -> LineageReplaceState {
    match name {
        "ACTIVE_PREDECESSOR" => LineageReplaceState::ActivePredecessor,
        "ACTIVE_SUCCESSOR_PREDECESSOR_RETIRED" => {
            LineageReplaceState::ActiveSuccessorPredecessorRetired
        }
        "CONTINUITY_BROKEN" => LineageReplaceState::ContinuityBroken,
        other => panic!("unknown recovery state: {other}"),
    }
}

#[test]
fn shared_lineage_replace_recovery_corpus_is_enforced() {
    assert!(CORPUS.lines().any(|line| line == "version=1"));
    let mut case_count = 0usize;

    for line in CORPUS.lines().filter(|line| line.starts_with("case=")) {
        let body = line.strip_prefix("case=").unwrap();
        let mut fields = body.split('|');
        let name = fields.next().unwrap();
        let facts = LineageReplaceRecoveryFacts {
            record_integrity_valid: bit(fields.next().unwrap()),
            predecessor_active: bit(fields.next().unwrap()),
            replacement_pending: bit(fields.next().unwrap()),
            successor_active: bit(fields.next().unwrap()),
            predecessor_retired: bit(fields.next().unwrap()),
            invalidations_complete: bit(fields.next().unwrap()),
        };
        let expected = expected_state(fields.next().unwrap());
        assert!(fields.next().is_none(), "malformed recovery corpus: {line}");
        assert_eq!(recover_lineage_replace(&facts), expected, "recovery case {name}");
        case_count += 1;
    }

    assert_eq!(case_count, 12);
}
