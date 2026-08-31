use proto::lineage_replace_attempt::{
    classify_lineage_replace_attempt, LineageReplaceAttemptDecision, LineageReplaceAttemptFacts,
};

const EVENT_CORPUS: &str =
    include_str!("../../../test-vectors/replay/lineage-replace-multi-attempt-events-v1.txt");

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
struct Attempt {
    id: u32,
    predecessor_generation: u32,
    successor: u32,
    context: u32,
}

#[derive(Default)]
struct MultiAttemptState {
    local_authorized: bool,
    peer_authorized: bool,
    local_attempt: Option<Attempt>,
    peer_attempt: Option<Attempt>,
    local_confirmation: Option<u32>,
    peer_confirmation: Option<u32>,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
enum EventDecision {
    IncompleteAttempt,
    Classified(LineageReplaceAttemptDecision),
}

fn attempt(id: u32) -> Attempt {
    match id {
        1 => Attempt {
            id: 1,
            predecessor_generation: 10,
            successor: 20,
            context: 30,
        },
        2 => Attempt {
            id: 2,
            predecessor_generation: 10,
            successor: 21,
            context: 30,
        },
        3 => Attempt {
            id: 3,
            predecessor_generation: 11,
            successor: 22,
            context: 31,
        },
        other => panic!("invalid attempt id {other}"),
    }
}

fn event_id(event: &str, prefix_len: usize) -> u32 {
    event[prefix_len..]
        .parse::<u32>()
        .unwrap_or_else(|_| panic!("invalid multi-attempt event {event}"))
}

fn observe(slot: &mut Option<Attempt>, confirmation: &mut Option<u32>, id: u32) {
    let candidate = attempt(id);
    if slot.map(|current| current.id) != Some(id) {
        *slot = Some(candidate);
        *confirmation = None;
    }
}

fn apply(state: &mut MultiAttemptState, event: &str) {
    match event {
        "LA" => state.local_authorized = true,
        "PA" => state.peer_authorized = true,
        "RL" => {
            state.local_attempt = None;
            state.local_confirmation = None;
        }
        "RP" => {
            state.peer_attempt = None;
            state.peer_confirmation = None;
        }
        "X" => {
            state.local_attempt = None;
            state.peer_attempt = None;
            state.local_confirmation = None;
            state.peer_confirmation = None;
        }
        _ if event.starts_with("LC") => {
            let id = event_id(event, 2);
            if state.local_attempt.map(|current| current.id) == Some(id) {
                state.local_confirmation = Some(id);
            }
        }
        _ if event.starts_with("PC") => {
            let id = event_id(event, 2);
            if state.peer_attempt.map(|current| current.id) == Some(id) {
                state.peer_confirmation = Some(id);
            }
        }
        _ if event.starts_with('L') => {
            observe(
                &mut state.local_attempt,
                &mut state.local_confirmation,
                event_id(event, 1),
            );
        }
        _ if event.starts_with('P') => {
            observe(
                &mut state.peer_attempt,
                &mut state.peer_confirmation,
                event_id(event, 1),
            );
        }
        _ => panic!("unknown multi-attempt event {event}"),
    }
}

fn decision(state: &MultiAttemptState) -> EventDecision {
    let (Some(local), Some(peer)) = (state.local_attempt, state.peer_attempt) else {
        return EventDecision::IncompleteAttempt;
    };
    let facts = LineageReplaceAttemptFacts {
        local_authorized: state.local_authorized,
        peer_authorized: state.peer_authorized,
        same_attempt: local.id == peer.id,
        same_predecessor_generation: local.predecessor_generation == peer.predecessor_generation,
        same_successor: local.successor == peer.successor,
        same_context: local.context == peer.context,
        local_confirmation_bound: state.local_confirmation == Some(local.id),
        peer_confirmation_bound: state.peer_confirmation == Some(peer.id),
    };
    EventDecision::Classified(classify_lineage_replace_attempt(&facts))
}

fn expected(value: &str) -> EventDecision {
    match value {
        "INCOMPLETE_ATTEMPT" => EventDecision::IncompleteAttempt,
        "CONVERGED" => EventDecision::Classified(LineageReplaceAttemptDecision::Converged),
        "AWAITING_CONFIRMATION" => {
            EventDecision::Classified(LineageReplaceAttemptDecision::AwaitingConfirmation)
        }
        "ATTEMPT_ID_MISMATCH" => {
            EventDecision::Classified(LineageReplaceAttemptDecision::AttemptIdMismatch)
        }
        other => panic!("invalid multi-attempt decision {other}"),
    }
}

#[test]
fn shared_multi_attempt_event_corpus() {
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
        assert_eq!(fields.len(), 3, "malformed multi-attempt case {body}");
        let mut state = MultiAttemptState::default();
        for event in fields[1].split(',') {
            apply(&mut state, event);
        }
        assert_eq!(
            decision(&state),
            expected(fields[2]),
            "multi-attempt case {}",
            fields[0]
        );
        cases += 1;
    }
    assert!(saw_version);
    assert_eq!(cases, 14);
}
