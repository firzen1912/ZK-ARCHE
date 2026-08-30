use proto::lineage_replace::{
    evaluate_lineage_replace, LineageReplaceDecision, LineageReplaceFacts, LineageReplaceTrigger,
};

const CORPUS: &str =
    include_str!("../../../test-vectors/replay/lineage-replace-decisions-v1.txt");

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
