use proto::lineage_replace_attempt_evidence::{
    classify_lineage_replace_attempt_evidence, LineageReplaceAttemptEvidenceDecision,
    LineageReplaceAttemptEvidenceFacts,
};
use proto::lineage_replace_reconciliation::LineageReplaceReconciliationDecision;
use proto::lineage_replace_reconciliation_transition::{
    classify_lineage_replace_reconciliation_transition,
    LineageReplaceReconciliationTransition,
    LineageReplaceReconciliationTransitionFacts,
};

const CORPUS: &str = include_str!(
    "../../../test-vectors/replay/lineage-replace-reconciliation-provenance-v1.txt"
);

#[derive(Default)]
struct ProvenanceState {
    local_attempt: Option<u32>,
    peer_attempt: Option<u32>,
    local_confirmation: Option<u32>,
    peer_confirmation: Option<u32>,
}

fn event_id(event: &str, prefix_len: usize) -> u32 {
    event[prefix_len..]
        .parse::<u32>()
        .unwrap_or_else(|_| panic!("invalid provenance event {event}"))
}

fn observe(attempt: &mut Option<u32>, confirmation: &mut Option<u32>, id: u32) {
    if *attempt != Some(id) {
        *attempt = Some(id);
        *confirmation = None;
    }
}

fn apply(state: &mut ProvenanceState, event: &str) {
    match event {
        "RL" => {
            state.local_attempt = None;
            state.local_confirmation = None;
        }
        "RP" => {
            state.peer_attempt = None;
            state.peer_confirmation = None;
        }
        "X" => *state = ProvenanceState::default(),
        _ if event.starts_with("LC") => {
            let id = event_id(event, 2);
            if state.local_attempt == Some(id) {
                state.local_confirmation = Some(id);
            }
        }
        _ if event.starts_with("PC") => {
            let id = event_id(event, 2);
            if state.peer_attempt == Some(id) {
                state.peer_confirmation = Some(id);
            }
        }
        _ if event.starts_with('L') => {
            let id = event_id(event, 1);
            observe(&mut state.local_attempt, &mut state.local_confirmation, id);
        }
        _ if event.starts_with('P') => {
            let id = event_id(event, 1);
            observe(&mut state.peer_attempt, &mut state.peer_confirmation, id);
        }
        _ => panic!("unknown provenance event {event}"),
    }
}

fn evidence(state: &ProvenanceState) -> LineageReplaceAttemptEvidenceDecision {
    classify_lineage_replace_attempt_evidence(&LineageReplaceAttemptEvidenceFacts {
        local_attempt_id: state.local_attempt,
        peer_attempt_id: state.peer_attempt,
        local_confirmation_attempt_id: state.local_confirmation,
        peer_confirmation_attempt_id: state.peer_confirmation,
    })
}

fn decision(value: &str) -> LineageReplaceReconciliationDecision {
    use LineageReplaceReconciliationDecision::*;
    match value {
        "PAIR_SUCCESSOR_READY" => PairSuccessorReady,
        "PAIR_PREDECESSOR_READY" => PairPredecessorReady,
        "RECONCILIATION_REQUIRED" => ReconciliationRequired,
        "PAIR_CONTINUITY_BROKEN" => PairContinuityBroken,
        "SUCCESSOR_DIVERGENCE" => SuccessorDivergence,
        other => panic!("invalid reconciliation decision {other}"),
    }
}

fn transition(value: &str) -> LineageReplaceReconciliationTransition {
    use LineageReplaceReconciliationTransition::*;
    match value {
        "HOLD" => Hold,
        "ACTIVATE_SUCCESSOR" => ActivateSuccessor,
        "RESUME_PREDECESSOR" => ResumePredecessor,
        "REJECT_DIVERGENCE" => RejectDivergence,
        "CONTINUITY_BROKEN" => ContinuityBroken,
        other => panic!("invalid transition decision {other}"),
    }
}

#[test]
fn shared_reconciliation_provenance_corpus() {
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
        assert_eq!(fields.len(), 6, "malformed provenance case {body}");
        let mut state = ProvenanceState::default();
        for event in fields[3].split(',') {
            apply(&mut state, event);
        }
        let facts = LineageReplaceReconciliationTransitionFacts {
            prior: decision(fields[1]),
            current: decision(fields[2]),
            attempt_evidence: evidence(&state),
            explicit_clean_retry: fields[4] == "1",
        };
        assert_eq!(
            classify_lineage_replace_reconciliation_transition(&facts),
            transition(fields[5]),
            "provenance case {}",
            fields[0]
        );
        cases += 1;
    }
    assert!(saw_version);
    assert_eq!(cases, 14);
}
