use proto::lineage_replace_reconciliation::LineageReplaceReconciliationDecision;
use proto::lineage_replace_reconciliation_transition::{
    classify_lineage_replace_reconciliation_transition,
    LineageReplaceReconciliationTransition,
    LineageReplaceReconciliationTransitionFacts,
};

fn pair(name: &str) -> LineageReplaceReconciliationDecision {
    match name {
        "PAIR_SUCCESSOR_READY" => LineageReplaceReconciliationDecision::PairSuccessorReady,
        "PAIR_PREDECESSOR_READY" => LineageReplaceReconciliationDecision::PairPredecessorReady,
        "RECONCILIATION_REQUIRED" => {
            LineageReplaceReconciliationDecision::ReconciliationRequired
        }
        "PAIR_CONTINUITY_BROKEN" => LineageReplaceReconciliationDecision::PairContinuityBroken,
        "SUCCESSOR_DIVERGENCE" => LineageReplaceReconciliationDecision::SuccessorDivergence,
        _ => panic!("unknown pair result"),
    }
}

fn bit(value: &str) -> bool {
    match value {
        "0" => false,
        "1" => true,
        _ => panic!("invalid bit"),
    }
}

fn expected(name: &str) -> LineageReplaceReconciliationTransition {
    match name {
        "HOLD" => LineageReplaceReconciliationTransition::Hold,
        "ACTIVATE_SUCCESSOR" => LineageReplaceReconciliationTransition::ActivateSuccessor,
        "RESUME_PREDECESSOR" => LineageReplaceReconciliationTransition::ResumePredecessor,
        "REJECT_DIVERGENCE" => LineageReplaceReconciliationTransition::RejectDivergence,
        "CONTINUITY_BROKEN" => LineageReplaceReconciliationTransition::ContinuityBroken,
        _ => panic!("unknown transition result"),
    }
}

#[test]
fn reconciliation_transition_corpus() {
    let corpus = include_str!(
        "../../../test-vectors/replay/lineage-replace-reconciliation-transition-v1.txt"
    );
    let mut count = 0usize;

    for line in corpus.lines().filter(|line| line.starts_with("case=")) {
        let fields: Vec<_> = line[5..].split('|').collect();
        assert_eq!(fields.len(), 6);

        let result = classify_lineage_replace_reconciliation_transition(
            &LineageReplaceReconciliationTransitionFacts {
                prior: pair(fields[1]),
                current: pair(fields[2]),
                fresh_authenticated_attempt_evidence: bit(fields[3]),
                explicit_clean_retry: bit(fields[4]),
            },
        );
        assert_eq!(result, expected(fields[5]), "transition case {}", fields[0]);
        count += 1;
    }

    assert_eq!(count, 12);
}
