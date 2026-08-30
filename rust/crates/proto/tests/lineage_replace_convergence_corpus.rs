use proto::lineage_replace_convergence::{
    classify_lineage_replace_convergence, LineageReplaceConvergenceDecision,
    LineageReplaceConvergenceFacts,
};

const CONVERGENCE_CORPUS: &str =
    include_str!("../../../test-vectors/replay/lineage-replace-convergence-v1.txt");

fn bit(value: &str) -> bool {
    match value {
        "0" => false,
        "1" => true,
        other => panic!("invalid bit {other}"),
    }
}

fn expected(value: &str) -> LineageReplaceConvergenceDecision {
    match value {
        "CONVERGED" => LineageReplaceConvergenceDecision::Converged,
        "AWAITING_CONFIRMATION" => LineageReplaceConvergenceDecision::AwaitingConfirmation,
        "UNAUTHORIZED" => LineageReplaceConvergenceDecision::Unauthorized,
        "SUCCESSOR_CONFLICT" => LineageReplaceConvergenceDecision::SuccessorConflict,
        other => panic!("invalid decision {other}"),
    }
}

#[test]
fn shared_convergence_corpus() {
    let mut cases = 0usize;
    let mut saw_version = false;
    for line in CONVERGENCE_CORPUS.lines() {
        if line == "version=1" {
            saw_version = true;
            continue;
        }
        let Some(body) = line.strip_prefix("case=") else {
            continue;
        };
        let fields: Vec<&str> = body.split('|').collect();
        assert_eq!(fields.len(), 7, "malformed convergence case {body}");
        let facts = LineageReplaceConvergenceFacts {
            local_authorized: bit(fields[1]),
            peer_authorized: bit(fields[2]),
            same_successor: bit(fields[3]),
            local_confirmed: bit(fields[4]),
            peer_confirmed: bit(fields[5]),
        };
        assert_eq!(
            classify_lineage_replace_convergence(&facts),
            expected(fields[6]),
            "convergence case {}",
            fields[0]
        );
        cases += 1;
    }
    assert!(saw_version);
    assert_eq!(cases, 8);
}
