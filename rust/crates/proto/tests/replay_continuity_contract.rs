use proto::replay_continuity::{ReplayContinuity, ReplayContinuityEvent, ReplayContinuityState};

const CORPUS: &str = include_str!("../../../test-vectors/replay-continuity/state-v1.txt");

fn parse_state(value: &str) -> ReplayContinuityState {
    match value {
        "TRUSTED" => ReplayContinuityState::Trusted,
        "RESTORING" => ReplayContinuityState::Restoring,
        "CONTINUITY_BROKEN" => ReplayContinuityState::ContinuityBroken,
        other => panic!("unknown replay continuity state: {other}"),
    }
}

fn parse_event(value: &str) -> ReplayContinuityEvent {
    match value {
        "restart" => ReplayContinuityEvent::Restart,
        "restored_trusted_window" => ReplayContinuityEvent::RestoredTrustedWindow,
        "restore_missing" => ReplayContinuityEvent::RestoreMissing,
        "restore_corrupt" => ReplayContinuityEvent::RestoreCorrupt,
        "restore_stale" => ReplayContinuityEvent::RestoreStale,
        "rollback_suspected" => ReplayContinuityEvent::RollbackSuspected,
        "empty_cache_reset" => ReplayContinuityEvent::EmptyCacheReset,
        "fresh_outer_session" => ReplayContinuityEvent::FreshOuterSession,
        "failed_auth" => ReplayContinuityEvent::FailedAuth,
        other => panic!("unknown replay continuity event: {other}"),
    }
}

#[test]
fn shared_replay_continuity_decision_corpus() {
    let mut lines = CORPUS.lines();
    assert_eq!(lines.next(), Some("format=ZKREPLAYCONTINUITY/1"));
    assert_eq!(lines.next(), Some("id|initial|event|expected|apply_ok|auth_allowed"));

    let mut cases = 0usize;
    for line in lines {
        assert!(!line.is_empty(), "blank corpus line");
        let mut fields = line.split('|');
        let id = fields.next().expect("scenario id");
        let initial = parse_state(fields.next().expect("initial state"));
        let event = parse_event(fields.next().expect("event"));
        let expected = parse_state(fields.next().expect("expected state"));
        let apply_ok = match fields.next().expect("apply_ok") {
            "0" => false,
            "1" => true,
            other => panic!("invalid apply_ok: {other}"),
        };
        let auth_allowed = match fields.next().expect("auth_allowed") {
            "0" => false,
            "1" => true,
            other => panic!("invalid auth_allowed: {other}"),
        };
        assert!(fields.next().is_none(), "extra corpus field for {id}");

        let mut continuity = ReplayContinuity::new(initial);
        assert_eq!(continuity.apply(event).is_ok(), apply_ok, "apply result mismatch for {id}");
        assert_eq!(continuity.state(), expected, "state mismatch for {id}");
        assert_eq!(
            continuity.auth_admission_allowed(),
            auth_allowed,
            "AUTH admission mismatch for {id}"
        );
        cases += 1;
    }

    assert!(cases >= 12, "replay continuity corpus unexpectedly small");
}
