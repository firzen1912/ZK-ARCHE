use proto::lineage_replace_convergence::{
    classify_lineage_replace_convergence, LineageReplaceConvergenceDecision,
    LineageReplaceConvergenceFacts,
};

const EVENT_CORPUS: &str =
    include_str!("../../../test-vectors/replay/lineage-replace-convergence-events-v1.txt");

#[derive(Default)]
struct EventState {
    local_authorized: bool,
    peer_authorized: bool,
    local_successor: Option<u32>,
    peer_successor: Option<u32>,
    local_confirmed: bool,
    peer_confirmed: bool,
    successor_conflict: bool,
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

fn successor(event: &str) -> u32 {
    event[2..]
        .parse::<u32>()
        .unwrap_or_else(|_| panic!("invalid successor event {event}"))
}

fn observe(slot: &mut Option<u32>, conflict: &mut bool, value: u32) {
    match *slot {
        Some(existing) if existing != value => *conflict = true,
        None => *slot = Some(value),
        _ => {}
    }
}

fn apply(state: &mut EventState, event: &str) {
    match event {
        "LA" => state.local_authorized = true,
        "PA" => state.peer_authorized = true,
        "CL" => state.local_confirmed = false,
        "CP" => state.peer_confirmed = false,
        _ if event.starts_with("LS") => {
            observe(
                &mut state.local_successor,
                &mut state.successor_conflict,
                successor(event),
            );
        }
        _ if event.starts_with("PS") => {
            observe(
                &mut state.peer_successor,
                &mut state.successor_conflict,
                successor(event),
            );
        }
        _ if event.starts_with("LC") => {
            let value = successor(event);
            if state.local_successor == Some(value) {
                state.local_confirmed = true;
            }
        }
        _ if event.starts_with("PC") => {
            let value = successor(event);
            if state.peer_successor == Some(value) {
                state.peer_confirmed = true;
            }
        }
        _ => panic!("unknown convergence event {event}"),
    }
}

fn decision(state: &EventState) -> LineageReplaceConvergenceDecision {
    let facts = LineageReplaceConvergenceFacts {
        local_authorized: state.local_authorized,
        peer_authorized: state.peer_authorized,
        same_successor: state.local_successor.is_some()
            && state.local_successor == state.peer_successor
            && !state.successor_conflict,
        local_confirmed: state.local_confirmed,
        peer_confirmed: state.peer_confirmed,
    };
    classify_lineage_replace_convergence(&facts)
}

#[test]
fn shared_convergence_event_corpus() {
    let mut cases = 0usize;
    let mut saw_version = false;
    for line in EVENT_CORPUS.lines() {
        if line == "version=1" {
            saw_version = true;
            continue;
        }
        let Some(body) = line.strip_prefix("case=") else {
            continue;
        };
        let fields: Vec<&str> = body.split('|').collect();
        assert_eq!(fields.len(), 3, "malformed convergence-event case {body}");
        let mut state = EventState::default();
        for event in fields[1].split(',') {
            apply(&mut state, event);
        }
        assert_eq!(
            decision(&state),
            expected(fields[2]),
            "event case {}",
            fields[0]
        );
        cases += 1;
    }
    assert!(saw_version);
    assert_eq!(cases, 10);
}
