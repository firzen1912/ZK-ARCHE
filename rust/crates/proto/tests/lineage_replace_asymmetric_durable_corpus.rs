use proto::lineage_replace::LineageReplaceState;
use proto::lineage_replace_attempt::{
    classify_lineage_replace_attempt, LineageReplaceAttemptDecision, LineageReplaceAttemptFacts,
};
use proto::lineage_replace_freshness::{
    recover_lineage_replace_with_freshness, LineageReplaceFreshnessFacts,
};
use proto::lineage_replace_reconciliation::{
    classify_lineage_replace_reconciliation, LineageReplaceReconciliationDecision,
    LineageReplaceReconciliationFacts,
};
use proto::lineage_replace_recovery::LineageReplaceRecoveryFacts;

fn attempt(name: &str) -> LineageReplaceAttemptDecision {
    let mut facts = LineageReplaceAttemptFacts {
        local_authorized: true,
        peer_authorized: true,
        same_attempt: true,
        same_predecessor_generation: true,
        same_successor: true,
        same_context: true,
        local_confirmation_bound: true,
        peer_confirmation_bound: true,
    };
    match name {
        "CONVERGED" => {}
        "AWAITING_CONFIRMATION" => facts.peer_confirmation_bound = false,
        "ATTEMPT_ID_MISMATCH" => facts.same_attempt = false,
        _ => panic!("unknown attempt decision"),
    }
    classify_lineage_replace_attempt(&facts)
}

fn recovery(kind: &str) -> LineageReplaceRecoveryFacts {
    match kind {
        "PREDECESSOR" => LineageReplaceRecoveryFacts {
            record_integrity_valid: true,
            predecessor_active: true,
            replacement_pending: false,
            successor_active: false,
            predecessor_retired: false,
            invalidations_complete: false,
        },
        "SUCCESSOR" => LineageReplaceRecoveryFacts {
            record_integrity_valid: true,
            predecessor_active: false,
            replacement_pending: false,
            successor_active: true,
            predecessor_retired: true,
            invalidations_complete: true,
        },
        "PARTIAL" => LineageReplaceRecoveryFacts {
            record_integrity_valid: true,
            predecessor_active: false,
            replacement_pending: true,
            successor_active: true,
            predecessor_retired: false,
            invalidations_complete: false,
        },
        _ => panic!("unknown recovery state"),
    }
}

fn durable_state(kind: &str, generation: u64, high_water: u64) -> LineageReplaceState {
    recover_lineage_replace_with_freshness(
        &recovery(kind),
        &LineageReplaceFreshnessFacts {
            anchor_available: true,
            anchor_integrity_valid: true,
            anchor_binding_valid: true,
            record_generation: generation,
            trusted_high_water_generation: high_water,
        },
    )
}

fn expected(name: &str) -> LineageReplaceReconciliationDecision {
    match name {
        "PAIR_SUCCESSOR_READY" => LineageReplaceReconciliationDecision::PairSuccessorReady,
        "PAIR_PREDECESSOR_READY" => LineageReplaceReconciliationDecision::PairPredecessorReady,
        "RECONCILIATION_REQUIRED" => {
            LineageReplaceReconciliationDecision::ReconciliationRequired
        }
        "CONTINUITY_BROKEN" => LineageReplaceReconciliationDecision::PairContinuityBroken,
        "SUCCESSOR_DIVERGENCE" => LineageReplaceReconciliationDecision::SuccessorDivergence,
        _ => panic!("unknown pair result"),
    }
}

#[test]
fn asymmetric_durable_corpus() {
    let corpus = include_str!(
        "../../../test-vectors/replay/lineage-replace-asymmetric-durable-v1.txt"
    );
    let mut count = 0usize;

    for line in corpus.lines().filter(|line| line.starts_with("case=")) {
        let fields: Vec<_> = line[5..].split('|').collect();
        assert_eq!(fields.len(), 11);

        let local_generation: u64 = fields[2].parse().unwrap();
        let local_high_water: u64 = fields[3].parse().unwrap();
        let peer_generation: u64 = fields[6].parse().unwrap();
        let peer_high_water: u64 = fields[7].parse().unwrap();
        let same_successor = match fields[9] {
            "0" => false,
            "1" => true,
            _ => panic!("invalid same-successor bit"),
        };

        let result = classify_lineage_replace_reconciliation(&LineageReplaceReconciliationFacts {
            local_attempt: attempt(fields[1]),
            local_state: durable_state(fields[4], local_generation, local_high_water),
            peer_attempt: attempt(fields[5]),
            peer_state: durable_state(fields[8], peer_generation, peer_high_water),
            same_successor,
        });
        assert_eq!(
            result,
            expected(fields[10]),
            "asymmetric durable case {}",
            fields[0]
        );
        count += 1;
    }

    assert_eq!(count, 14);
}
