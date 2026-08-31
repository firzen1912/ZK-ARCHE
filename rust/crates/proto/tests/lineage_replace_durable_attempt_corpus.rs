use proto::lineage_replace::LineageReplaceState;
use proto::lineage_replace_attempt::{classify_lineage_replace_attempt, LineageReplaceAttemptDecision, LineageReplaceAttemptFacts};
use proto::lineage_replace_freshness::{recover_lineage_replace_with_freshness, LineageReplaceFreshnessFacts};
use proto::lineage_replace_recovery::LineageReplaceRecoveryFacts;

fn attempt(decision: &str) -> LineageReplaceAttemptDecision {
    let mut f = LineageReplaceAttemptFacts { local_authorized: true, peer_authorized: true, same_attempt: true, same_predecessor_generation: true, same_successor: true, same_context: true, local_confirmation_bound: true, peer_confirmation_bound: true };
    match decision {
        "CONVERGED" => {}
        "AWAITING_CONFIRMATION" => f.peer_confirmation_bound = false,
        "ATTEMPT_ID_MISMATCH" => f.same_attempt = false,
        "PREDECESSOR_MISMATCH" => f.same_predecessor_generation = false,
        "CONTEXT_MISMATCH" => f.same_context = false,
        _ => panic!("unknown attempt decision"),
    }
    classify_lineage_replace_attempt(&f)
}

fn recovery(kind: &str) -> LineageReplaceRecoveryFacts {
    match kind {
        "PREDECESSOR" => LineageReplaceRecoveryFacts { record_integrity_valid: true, predecessor_active: true, replacement_pending: false, successor_active: false, predecessor_retired: false, invalidations_complete: false },
        "SUCCESSOR" => LineageReplaceRecoveryFacts { record_integrity_valid: true, predecessor_active: false, replacement_pending: false, successor_active: true, predecessor_retired: true, invalidations_complete: true },
        "PARTIAL" => LineageReplaceRecoveryFacts { record_integrity_valid: true, predecessor_active: false, replacement_pending: true, successor_active: true, predecessor_retired: false, invalidations_complete: false },
        _ => panic!("unknown recovery state"),
    }
}

#[test]
fn durable_attempt_corpus() {
    let corpus = include_str!("../../../test-vectors/replay/lineage-replace-durable-attempt-v1.txt");
    let mut count = 0usize;
    for line in corpus.lines().filter(|l| l.starts_with("case=")) {
        let p: Vec<_> = line[5..].split('|').collect();
        assert_eq!(p.len(), 6);
        let decision = attempt(p[1]);
        let record_generation: u64 = p[2].parse().unwrap();
        let high_water: u64 = p[3].parse().unwrap();
        let recovered = recover_lineage_replace_with_freshness(&recovery(p[4]), &LineageReplaceFreshnessFacts { anchor_available: true, anchor_integrity_valid: true, anchor_binding_valid: true, record_generation, trusted_high_water_generation: high_water });
        let operable = match p[5] {
            "RESUME_SUCCESSOR" => decision == LineageReplaceAttemptDecision::Converged && recovered == LineageReplaceState::ActiveSuccessorPredecessorRetired,
            "RESUME_PREDECESSOR" => recovered == LineageReplaceState::ActivePredecessor,
            "CONTINUITY_BROKEN" => decision != LineageReplaceAttemptDecision::Converged || recovered == LineageReplaceState::ContinuityBroken,
            _ => panic!("unknown expected state"),
        };
        assert!(operable, "durable-attempt case {}", p[0]);
        count += 1;
    }
    assert_eq!(count, 10);
}
