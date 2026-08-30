//! Wire-neutral `LINEAGE_REPLACE` decision predicate, commit planner, and logical state machine.
//!
//! This module classifies normalized lifecycle replacement facts and, only
//! after an accepted decision, derives the minimum atomic commit plan. The
//! logical state machine can stage that plan, commit it logically, or fail
//! closed on interruption. It does not parse packets, mutate trust, allocate
//! registry values, perform durable writes, or activate a production successor
//! lineage. Normal AUTH must never use it as an implicit trust mutation path.

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum LineageReplaceTrigger {
    Lifecycle,
    Restart,
    TransportChange,
    Auth,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum LineageReplaceDecision {
    AcceptSuccessor,
    RejectAuthority,
    RejectPredecessor,
    RejectSuccessor,
    RejectContext,
    RejectFreshness,
    RejectReplay,
    RejectConcurrent,
    RejectRollback,
    RejectStorage,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct LineageReplaceFacts {
    pub trigger: LineageReplaceTrigger,
    pub authority_valid: bool,
    pub predecessor_valid: bool,
    pub successor_valid: bool,
    pub context_valid: bool,
    pub freshness_valid: bool,
    pub replay_free: bool,
    pub concurrent_free: bool,
    pub rollback_clear: bool,
    pub storage_safe: bool,
    pub dependent_state_safe: bool,
}

/// Pure logical plan for the eventual atomic predecessor -> successor commit.
///
/// Every dependent-state bit is intentionally invalidating rather than
/// revalidating. Revalidation may be introduced only by a separately specified
/// and tested lifecycle rule; the constrained floor fails closed by dropping
/// predecessor-bound state.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct LineageReplacePlan {
    pub retire_predecessor: bool,
    pub activate_successor: bool,
    pub invalidate_session_keys: bool,
    pub invalidate_resumption: bool,
    pub invalidate_authorization_cache: bool,
    pub invalidate_attribution_cache: bool,
    pub invalidate_channel_binding: bool,
    pub invalidate_replay_state: bool,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum LineageReplaceState {
    ActivePredecessor,
    ReplacementPending,
    ActiveSuccessorPredecessorRetired,
    ContinuityBroken,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum LineageReplaceEvent {
    Begin,
    Commit,
    Interrupt,
}

pub fn evaluate_lineage_replace(facts: &LineageReplaceFacts) -> LineageReplaceDecision {
    if facts.trigger != LineageReplaceTrigger::Lifecycle || !facts.authority_valid {
        return LineageReplaceDecision::RejectAuthority;
    }
    if !facts.predecessor_valid {
        return LineageReplaceDecision::RejectPredecessor;
    }
    if !facts.successor_valid {
        return LineageReplaceDecision::RejectSuccessor;
    }
    if !facts.context_valid || !facts.dependent_state_safe {
        return LineageReplaceDecision::RejectContext;
    }
    if !facts.freshness_valid {
        return LineageReplaceDecision::RejectFreshness;
    }
    if !facts.replay_free {
        return LineageReplaceDecision::RejectReplay;
    }
    if !facts.concurrent_free {
        return LineageReplaceDecision::RejectConcurrent;
    }
    if !facts.rollback_clear {
        return LineageReplaceDecision::RejectRollback;
    }
    if !facts.storage_safe {
        return LineageReplaceDecision::RejectStorage;
    }
    LineageReplaceDecision::AcceptSuccessor
}

/// Derive a mutation-free commit plan only for an already accepted decision.
///
/// Rejected decisions cannot produce a plan. This makes the planner unsuitable
/// as an alternate authorization path and keeps trust mutation downstream of
/// the explicit lifecycle decision gate.
pub fn plan_lineage_replace(decision: LineageReplaceDecision) -> Option<LineageReplacePlan> {
    if decision != LineageReplaceDecision::AcceptSuccessor {
        return None;
    }

    Some(LineageReplacePlan {
        retire_predecessor: true,
        activate_successor: true,
        invalidate_session_keys: true,
        invalidate_resumption: true,
        invalidate_authorization_cache: true,
        invalidate_attribution_cache: true,
        invalidate_channel_binding: true,
        invalidate_replay_state: true,
    })
}

fn lineage_replace_plan_complete(plan: &LineageReplacePlan) -> bool {
    plan.retire_predecessor
        && plan.activate_successor
        && plan.invalidate_session_keys
        && plan.invalidate_resumption
        && plan.invalidate_authorization_cache
        && plan.invalidate_attribution_cache
        && plan.invalidate_channel_binding
        && plan.invalidate_replay_state
}

/// Advance the storage-neutral replacement state machine.
///
/// A complete accepted plan is required both to stage and to logically commit
/// replacement. `Interrupt` is meaningful only while replacement is pending and
/// moves the machine to `ContinuityBroken`. This layer intentionally has no
/// recovery transition out of `ContinuityBroken`; recovery requires separately
/// specified durable evidence and authority.
pub fn advance_lineage_replace(
    state: LineageReplaceState,
    event: LineageReplaceEvent,
    plan: Option<&LineageReplacePlan>,
) -> (LineageReplaceState, bool) {
    let plan_complete = plan.is_some_and(lineage_replace_plan_complete);

    if state == LineageReplaceState::ActivePredecessor
        && event == LineageReplaceEvent::Begin
        && plan_complete
    {
        return (LineageReplaceState::ReplacementPending, true);
    }

    if state == LineageReplaceState::ReplacementPending
        && event == LineageReplaceEvent::Commit
        && plan_complete
    {
        return (LineageReplaceState::ActiveSuccessorPredecessorRetired, true);
    }

    if state == LineageReplaceState::ReplacementPending && event == LineageReplaceEvent::Interrupt {
        return (LineageReplaceState::ContinuityBroken, true);
    }

    (state, false)
}
