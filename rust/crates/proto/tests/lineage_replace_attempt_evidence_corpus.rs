use proto::lineage_replace_attempt_evidence::{
    classify_lineage_replace_attempt_evidence, LineageReplaceAttemptEvidenceDecision,
    LineageReplaceAttemptEvidenceFacts,
};

const CORPUS: &str = include_str!(
    "../../../test-vectors/replay/lineage-replace-attempt-evidence-v1.txt"
);

fn id(value: &str) -> Option<u32> {
    let value = value.parse::<u32>().expect("valid attempt id");
    (value != 0).then_some(value)
}

fn expected(value: &str) -> LineageReplaceAttemptEvidenceDecision {
    use LineageReplaceAttemptEvidenceDecision::*;
    match value {
        "FRESH_CURRENT_ATTEMPT" => FreshCurrentAttempt,
        "MISSING_CURRENT_ATTEMPT" => MissingCurrentAttempt,
        "ATTEMPT_MISMATCH" => AttemptMismatch,
        "LOCAL_CONFIRMATION_MISSING" => LocalConfirmationMissing,
        "PEER_CONFIRMATION_MISSING" => PeerConfirmationMissing,
        other => panic!("invalid attempt-evidence decision {other}"),
    }
}

#[test]
fn shared_attempt_evidence_corpus() {
    let mut cases = 0usize;
    let mut saw_version = false;
    for line in CORPUS.lines() {
        if line == "version=1" {
            saw_version = true;
            continue;
        }
        let Some(body) = line.strip_prefix("case=") else {
            continue;
        };
        let fields: Vec<&str> = body.split('|').collect();
        assert_eq!(fields.len(), 6, "malformed attempt-evidence case {body}");
        let facts = LineageReplaceAttemptEvidenceFacts {
            local_attempt_id: id(fields[1]),
            peer_attempt_id: id(fields[2]),
            local_confirmation_attempt_id: id(fields[3]),
            peer_confirmation_attempt_id: id(fields[4]),
        };
        assert_eq!(
            classify_lineage_replace_attempt_evidence(&facts),
            expected(fields[5]),
            "attempt-evidence case {}",
            fields[0]
        );
        cases += 1;
    }
    assert!(saw_version);
    assert_eq!(cases, 10);
}
