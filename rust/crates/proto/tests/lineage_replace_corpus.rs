use proto::lineage_replace::advance_lineage_replace;
use proto::lineage_replace::evaluate_lineage_replace;
use proto::lineage_replace::plan_lineage_replace;
use proto::lineage_replace::LineageReplaceDecision;
use proto::lineage_replace::LineageReplaceEvent;
use proto::lineage_replace::LineageReplaceFacts;
use proto::lineage_replace::LineageReplacePlan;
use proto::lineage_replace::LineageReplaceState;
use proto::lineage_replace::LineageReplaceTrigger;

const CORPUS: &str = include_str!("../../../test-vectors/replay/lineage-replace-decisions-v1.txt");
const PLAN_CORPUS: &str = include_str!("../../../test-vectors/replay/lineage-replace-plans-v1.txt");
const STATE_CORPUS: &str =
    include_str!("../../../test-vectors/replay/lineage-replace-states-v1.txt");

fn fixture() -> LineageReplaceFacts {
    LineageReplaceFacts {
        trigger: LineageReplaceTrigger::Lifecycle,
        authority_valid: true,
        predecessor_valid: true,
        successor_valid: true,
        context_valid: true,
        freshness_valid: true,
        replay_free: true,
        concurrent_free: true,
        rollback_clear: true,
        storage_safe: true,
        dependent_state_safe: true,
    }
}

fn expected(name: &str) -> LineageReplaceDecision {
    match name {
        "ACCEPT_SUCCESSOR" => LineageReplaceDecision::AcceptSuccessor,
        "REJECT_AUTHORITY" => LineageReplaceDecision::RejectAuthority,
        "REJECT_PREDECESSOR" => LineageReplaceDecision::RejectPredecessor,
        "REJECT_SUCCESSOR" => LineageReplaceDecision::RejectSuccessor,
        "REJECT_CONTEXT" => LineageReplaceDecision::RejectContext,
        "REJECT_FRESHNESS" => LineageReplaceDecision::RejectFreshness,
        "REJECT_REPLAY" => LineageReplaceDecision::RejectReplay,
        "REJECT_CONCURRENT" => LineageReplaceDecision::RejectConcurrent,
        "REJECT_ROLLBACK" => LineageReplaceDecision::RejectRollback,
        "REJECT_STORAGE" => LineageReplaceDecision::RejectStorage,
        other => panic!("unknown lineage-replace decision: {other}"),
    }
}

fn expected_state(name: &str) -> LineageReplaceState {
    match name {
        "ACTIVE_PREDECESSOR" => LineageReplaceState::ActivePredecessor,
        "REPLACEMENT_PENDING" => LineageReplaceState::ReplacementPending,
        "ACTIVE_SUCCESSOR_PREDECESSOR_RETIRED" => {
            LineageReplaceState::ActiveSuccessorPredecessorRetired
        }
        "CONTINUITY_BROKEN" => LineageReplaceState::ContinuityBroken,
        other => panic!("unknown lineage-replace state: {other}"),
    }
}

fn expected_event(name: &str) -> LineageReplaceEvent {
    match name {
        "BEGIN" => LineageReplaceEvent::Begin,
        "COMMIT" => LineageReplaceEvent::Commit,
        "INTERRUPT" => LineageReplaceEvent::Interrupt,
        other => panic!("unknown lineage-replace event: {other}"),
    }
}

fn plan_fixture(marker: &str) -> Option<LineageReplacePlan> {
    match marker {
        "full" => plan_lineage_replace(LineageReplaceDecision::AcceptSuccessor),
        "partial" => {
            let mut plan = plan_lineage_replace(LineageReplaceDecision::AcceptSuccessor).unwrap();
            plan.invalidate_replay_state = false;
            Some(plan)
        }
        "none" => None,
        other => panic!("unknown lineage-replace plan marker: {other}"),
    }
}

fn apply_mutation(facts: &mut LineageReplaceFacts, mutation: &str) {
    match mutation {
        "none" => {}
        "unauthorized_authority" => facts.authority_valid = false,
        "restart_trigger" => facts.trigger = LineageReplaceTrigger::Restart,
        "transport_change_trigger" => facts.trigger = LineageReplaceTrigger::TransportChange,
        "auth_trigger" => facts.trigger = LineageReplaceTrigger::Auth,
        "replayed_transition" | "predecessor_domain_not_retired" => facts.replay_free = false,
        "storage_partial" | "storage_ambiguous" => facts.storage_safe = false,
        "rollback_suspected" => facts.rollback_clear = false,
        "stale_freshness" | "stale_revocation" => facts.freshness_valid = false,
        "dependent_state_not_revalidated" => facts.dependent_state_safe = false,
        "competing_successor" => facts.concurrent_free = false,
        "retired_predecessor" | "wrong_predecessor_generation" => {
            facts.predecessor_valid = false;
        }
        "successor_binding_mismatch" => facts.successor_valid = false,
        "downgrade_context" | "wrong_domain_context" => facts.context_valid = false,
        other => panic!("unknown lineage-replace corpus mutation: {other}"),
    }
}

#[test]
fn shared_lineage_replace_decision_corpus_is_enforced() {
    assert!(CORPUS.lines().any(|line| line == "version=1"));
    let mut case_count = 0usize;

    for line in CORPUS.lines().filter(|line| line.starts_with("case=")) {
        let body = line.strip_prefix("case=").unwrap();
        let mut fields = body.split('|');
        let name = fields.next().unwrap();
        let mutation = fields.next().unwrap();
        let expected_name = fields.next().unwrap();
        assert!(fields.next().is_none(), "malformed corpus line: {line}");

        let mut facts = fixture();
        apply_mutation(&mut facts, mutation);
        assert_eq!(
            evaluate_lineage_replace(&facts),
            expected(expected_name),
            "corpus case {name}"
        );
        case_count += 1;
    }

    assert_eq!(case_count, 20);
}

#[test]
fn shared_lineage_replace_plan_corpus_is_enforced() {
    assert!(PLAN_CORPUS.lines().any(|line| line == "version=1"));
    let mut case_count = 0usize;

    for line in PLAN_CORPUS.lines().filter(|line| line.starts_with("case=")) {
        let body = line.strip_prefix("case=").unwrap();
        let mut fields = body.split('|');
        let decision_name = fields.next().unwrap();
        let should_plan = fields.next().unwrap();
        assert!(
            fields.next().is_none(),
            "malformed plan corpus line: {line}"
        );

        let plan = plan_lineage_replace(expected(decision_name));
        match should_plan {
            "1" => {
                let plan = plan.expect("accepted decision must produce a commit plan");
                assert!(plan.retire_predecessor);
                assert!(plan.activate_successor);
                assert!(plan.invalidate_session_keys);
                assert!(plan.invalidate_resumption);
                assert!(plan.invalidate_authorization_cache);
                assert!(plan.invalidate_attribution_cache);
                assert!(plan.invalidate_channel_binding);
                assert!(plan.invalidate_replay_state);
            }
            "0" => assert!(plan.is_none(), "rejected decision produced a commit plan"),
            other => panic!("invalid should-plan marker: {other}"),
        }
        case_count += 1;
    }

    assert_eq!(case_count, 10);
}

#[test]
fn shared_lineage_replace_state_corpus_is_enforced() {
    assert!(STATE_CORPUS.lines().any(|line| line == "version=1"));
    let mut case_count = 0usize;

    for line in STATE_CORPUS
        .lines()
        .filter(|line| line.starts_with("case="))
    {
        let body = line.strip_prefix("case=").unwrap();
        let mut fields = body.split('|');
        let name = fields.next().unwrap();
        let initial = expected_state(fields.next().unwrap());
        let event = expected_event(fields.next().unwrap());
        let plan = plan_fixture(fields.next().unwrap());
        let should_advance = fields.next().unwrap();
        let expected_next = expected_state(fields.next().unwrap());
        assert!(
            fields.next().is_none(),
            "malformed state corpus line: {line}"
        );

        let (next, advanced) = advance_lineage_replace(initial, event, plan.as_ref());
        assert_eq!(next, expected_next, "state corpus case {name}");
        match should_advance {
            "1" => assert!(advanced, "state corpus case {name} did not advance"),
            "0" => assert!(!advanced, "state corpus case {name} unexpectedly advanced"),
            other => panic!("invalid should-advance marker: {other}"),
        }
        case_count += 1;
    }

    assert_eq!(case_count, 14);
}
