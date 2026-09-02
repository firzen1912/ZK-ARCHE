use proto::lineage_replace::LineageReplacePlan;
use proto::lineage_replace_storage_transaction::{
    execute_lineage_replace_storage_transaction, LineageReplaceStorageStep,
    LineageReplaceStorageTransactionResult,
};

fn bit(v: &str) -> bool {
    match v {
        "1" => true,
        "0" => false,
        _ => panic!("invalid bit"),
    }
}
fn step(v: &str) -> Option<LineageReplaceStorageStep> {
    match v {
        "NONE" => None,
        "PERSIST_PENDING" => Some(LineageReplaceStorageStep::PersistPending),
        "ACTIVATE_SUCCESSOR" => Some(LineageReplaceStorageStep::ActivateSuccessor),
        "RETIRE_PREDECESSOR" => Some(LineageReplaceStorageStep::RetirePredecessor),
        "INVALIDATE_DEPENDENT_STATE" => Some(LineageReplaceStorageStep::InvalidateDependentState),
        "CLEAR_PENDING" => Some(LineageReplaceStorageStep::ClearPending),
        _ => panic!("invalid step"),
    }
}
fn expected(v: &str) -> LineageReplaceStorageTransactionResult {
    match v {
        "COMMITTED" => LineageReplaceStorageTransactionResult::Committed,
        "REJECT_PLAN" => LineageReplaceStorageTransactionResult::RejectPlan,
        "FAIL_PENDING" => LineageReplaceStorageTransactionResult::FailPending,
        "FAIL_SUCCESSOR" => LineageReplaceStorageTransactionResult::FailSuccessor,
        "FAIL_PREDECESSOR" => LineageReplaceStorageTransactionResult::FailPredecessor,
        "FAIL_INVALIDATIONS" => LineageReplaceStorageTransactionResult::FailInvalidations,
        "FAIL_CLEAR" => LineageReplaceStorageTransactionResult::FailClear,
        _ => panic!("invalid result"),
    }
}
fn name(v: LineageReplaceStorageStep) -> &'static str {
    match v {
        LineageReplaceStorageStep::PersistPending => "PERSIST_PENDING",
        LineageReplaceStorageStep::ActivateSuccessor => "ACTIVATE_SUCCESSOR",
        LineageReplaceStorageStep::RetirePredecessor => "RETIRE_PREDECESSOR",
        LineageReplaceStorageStep::InvalidateDependentState => "INVALIDATE_DEPENDENT_STATE",
        LineageReplaceStorageStep::ClearPending => "CLEAR_PENDING",
    }
}
fn canonical_plan() -> LineageReplacePlan {
    LineageReplacePlan {
        retire_predecessor: true,
        activate_successor: true,
        invalidate_session_keys: true,
        invalidate_resumption: true,
        invalidate_authorization_cache: true,
        invalidate_attribution_cache: true,
        invalidate_channel_binding: true,
        invalidate_replay_state: true,
    }
}

#[test]
fn canonical_lineage_replace_storage_transaction_corpus() {
    let corpus =
        include_str!("../../../test-vectors/replay/lineage-replace-storage-transaction-v1.txt");
    let mut cases = 0usize;
    for line in corpus.lines() {
        if !line.starts_with("case=") {
            continue;
        }
        let f: Vec<&str> = line[5..].split('|').collect();
        assert_eq!(f.len(), 5);
        let mut plan = canonical_plan();
        if !bit(f[1]) {
            plan.invalidate_replay_state = false;
        }
        let fail = step(f[2]);
        let mut trace: Vec<&'static str> = Vec::new();
        let result = execute_lineage_replace_storage_transaction(Some(&plan), |s| {
            trace.push(name(s));
            Some(s) != fail
        });
        assert_eq!(result, expected(f[3]));
        let actual = if trace.is_empty() {
            "NONE".to_string()
        } else {
            trace.join(",")
        };
        assert_eq!(actual, f[4]);
        cases += 1;
    }
    assert_eq!(cases, 7);
}
