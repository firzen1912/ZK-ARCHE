use proto::lineage_replace_attempt::{
    classify_lineage_replace_attempt, LineageReplaceAttemptDecision, LineageReplaceAttemptFacts,
};

const ATTEMPT_CORPUS: &str =
    include_str!("../../../test-vectors/replay/lineage-replace-attempt-binding-v1.txt");

fn bit(value: &str) -> bool {
    match value {
        "0" => false,
        "1" => true,
        other => panic!("invalid bit {other}"),
    }
}

fn expected(value: &str) -> LineageReplaceAttemptDecision {
    match value {
        "CONVERGED" => LineageReplaceAttemptDecision::Converged,
        "AWAITING_CONFIRMATION" => LineageReplaceAttemptDecision::AwaitingConfirmation,
        "UNAUTHORIZED" => LineageReplaceAttemptDecision::Unauthorized,
        "ATTEMPT_ID_MISMATCH" => LineageReplaceAttemptDecision::AttemptIdMismatch,
        "PREDECESSOR_MISMATCH" => LineageReplaceAttemptDecision::PredecessorMismatch,
        "SUCCESSOR_MISMATCH" => LineageReplaceAttemptDecision::SuccessorMismatch,
        "CONTEXT_MISMATCH" => LineageReplaceAttemptDecision::ContextMismatch,
        other => panic!("invalid decision {other}"),
    }
}

#[test]
fn shared_attempt_binding_corpus() {
    let mut cases = 0usize;
    let mut saw_version = false;
    for line in ATTEMPT_CORPUS.lines() {
        if line == "version=1" {
            saw_version = true;
            continue;
        }
        let Some(body) = line.strip_prefix("case=") else {
            continue;
        };
        let fields: Vec<&str> = body.split('|').collect();
        assert_eq!(fields.len(), 10, "malformed attempt binding case {body}");
        let facts = LineageReplaceAttemptFacts {
            local_authorized: bit(fields[1]),
            peer_authorized: bit(fields[2]),
            same_attempt: bit(fields[3]),
            same_predecessor_generation: bit(fields[4]),
            same_successor: bit(fields[5]),
            same_context: bit(fields[6]),
            local_confirmation_bound: bit(fields[7]),
            peer_confirmation_bound: bit(fields[8]),
        };
        assert_eq!(
            classify_lineage_replace_attempt(&facts),
            expected(fields[9]),
            "attempt binding case {}",
            fields[0]
        );
        cases += 1;
    }
    assert!(saw_version);
    assert_eq!(cases, 12);
}
