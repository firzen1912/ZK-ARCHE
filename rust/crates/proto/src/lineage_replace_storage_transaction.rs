use crate::lineage_replace::LineageReplacePlan;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum LineageReplaceStorageStep {
    PersistPending,
    ActivateSuccessor,
    RetirePredecessor,
    InvalidateDependentState,
    ClearPending,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum LineageReplaceStorageTransactionResult {
    Committed,
    RejectPlan,
    FailPending,
    FailSuccessor,
    FailPredecessor,
    FailInvalidations,
    FailClear,
}

fn plan_complete(plan: &LineageReplacePlan) -> bool {
    plan.retire_predecessor
        && plan.activate_successor
        && plan.invalidate_session_keys
        && plan.invalidate_resumption
        && plan.invalidate_authorization_cache
        && plan.invalidate_attribution_cache
        && plan.invalidate_channel_binding
        && plan.invalidate_replay_state
}

/// Execute the storage-neutral logical lineage-replacement transaction.
///
/// Returning `true` from `apply` asserts only that the adapter made the
/// requested logical step durable. Once the pending marker has succeeded, a
/// later failure remains fail-closed: this function never compensates, clears
/// pending, or infers completion.
pub fn execute_lineage_replace_storage_transaction<F>(
    plan: Option<&LineageReplacePlan>,
    mut apply: F,
) -> LineageReplaceStorageTransactionResult
where
    F: FnMut(LineageReplaceStorageStep) -> bool,
{
    let Some(plan) = plan else {
        return LineageReplaceStorageTransactionResult::RejectPlan;
    };
    if !plan_complete(plan) {
        return LineageReplaceStorageTransactionResult::RejectPlan;
    }
    if !apply(LineageReplaceStorageStep::PersistPending) {
        return LineageReplaceStorageTransactionResult::FailPending;
    }
    if !apply(LineageReplaceStorageStep::ActivateSuccessor) {
        return LineageReplaceStorageTransactionResult::FailSuccessor;
    }
    if !apply(LineageReplaceStorageStep::RetirePredecessor) {
        return LineageReplaceStorageTransactionResult::FailPredecessor;
    }
    if !apply(LineageReplaceStorageStep::InvalidateDependentState) {
        return LineageReplaceStorageTransactionResult::FailInvalidations;
    }
    if !apply(LineageReplaceStorageStep::ClearPending) {
        return LineageReplaceStorageTransactionResult::FailClear;
    }
    LineageReplaceStorageTransactionResult::Committed
}
