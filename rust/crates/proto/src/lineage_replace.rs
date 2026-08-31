//! Wire-neutral `LINEAGE_REPLACE` decision predicate, authorization gate, commit planner, and logical state machine.
//!
//! The public authorization classifier makes current-credential control,
//! successor-key control, current-session authorization/context binding, and
//! privilege preservation explicit before the normalized replacement predicate
//! may accept a successor. No wire allocation or trust mutation occurs here.

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum LineageReplaceTrigger { Lifecycle, Restart, TransportChange, Auth }

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct LineageReplaceAuthorizationFacts {
    pub current_credential_control_valid: bool,
    pub successor_key_control_valid: bool,
    pub current_session_authenticated: bool,
    pub current_session_authorized: bool,
    pub current_context_bound: bool,
    pub predecessor_binding_current: bool,
    pub successor_scope_within_authorized_scope: bool,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum LineageReplaceAuthorizationDecision {
    AuthorizedReplacement,
    RejectCurrentCredentialControl,
    RejectSuccessorKeyControl,
    RejectSessionAuthentication,
    RejectSessionAuthorization,
    RejectContextBinding,
    RejectPredecessorBinding,
    RejectPrivilegeExpansion,
}

pub fn classify_lineage_replace_authorization(facts: &LineageReplaceAuthorizationFacts) -> LineageReplaceAuthorizationDecision {
    use LineageReplaceAuthorizationDecision::*;
    if !facts.current_credential_control_valid { return RejectCurrentCredentialControl; }
    if !facts.successor_key_control_valid { return RejectSuccessorKeyControl; }
    if !facts.current_session_authenticated { return RejectSessionAuthentication; }
    if !facts.current_session_authorized { return RejectSessionAuthorization; }
    if !facts.current_context_bound { return RejectContextBinding; }
    if !facts.predecessor_binding_current { return RejectPredecessorBinding; }
    if !facts.successor_scope_within_authorized_scope { return RejectPrivilegeExpansion; }
    AuthorizedReplacement
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum LineageReplaceDecision { AcceptSuccessor, RejectAuthority, RejectPredecessor, RejectSuccessor, RejectContext, RejectFreshness, RejectReplay, RejectConcurrent, RejectRollback, RejectStorage }

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

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct LineageReplacePlan {
    pub retire_predecessor: bool, pub activate_successor: bool, pub invalidate_session_keys: bool,
    pub invalidate_resumption: bool, pub invalidate_authorization_cache: bool,
    pub invalidate_attribution_cache: bool, pub invalidate_channel_binding: bool,
    pub invalidate_replay_state: bool,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum LineageReplaceState { ActivePredecessor, ReplacementPending, ActiveSuccessorPredecessorRetired, ContinuityBroken }
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum LineageReplaceEvent { Begin, Commit, Interrupt }

/// Normalized lower-layer predicate. Lifecycle request handlers must derive the
/// authority input with `classify_lineage_replace_authorization` and use
/// `evaluate_authorized_lineage_replace` rather than manufacturing this bit.
pub fn evaluate_lineage_replace(facts: &LineageReplaceFacts) -> LineageReplaceDecision {
    if facts.trigger != LineageReplaceTrigger::Lifecycle || !facts.authority_valid { return LineageReplaceDecision::RejectAuthority; }
    if !facts.predecessor_valid { return LineageReplaceDecision::RejectPredecessor; }
    if !facts.successor_valid { return LineageReplaceDecision::RejectSuccessor; }
    if !facts.context_valid || !facts.dependent_state_safe { return LineageReplaceDecision::RejectContext; }
    if !facts.freshness_valid { return LineageReplaceDecision::RejectFreshness; }
    if !facts.replay_free { return LineageReplaceDecision::RejectReplay; }
    if !facts.concurrent_free { return LineageReplaceDecision::RejectConcurrent; }
    if !facts.rollback_clear { return LineageReplaceDecision::RejectRollback; }
    if !facts.storage_safe { return LineageReplaceDecision::RejectStorage; }
    LineageReplaceDecision::AcceptSuccessor
}

pub fn evaluate_authorized_lineage_replace(authorization: LineageReplaceAuthorizationDecision, facts: &LineageReplaceFacts) -> LineageReplaceDecision {
    let mut normalized = *facts;
    normalized.authority_valid = authorization == LineageReplaceAuthorizationDecision::AuthorizedReplacement;
    evaluate_lineage_replace(&normalized)
}

pub fn plan_lineage_replace(decision: LineageReplaceDecision) -> Option<LineageReplacePlan> {
    if decision != LineageReplaceDecision::AcceptSuccessor { return None; }
    Some(LineageReplacePlan { retire_predecessor: true, activate_successor: true, invalidate_session_keys: true, invalidate_resumption: true, invalidate_authorization_cache: true, invalidate_attribution_cache: true, invalidate_channel_binding: true, invalidate_replay_state: true })
}
fn lineage_replace_plan_complete(plan: &LineageReplacePlan) -> bool { plan.retire_predecessor && plan.activate_successor && plan.invalidate_session_keys && plan.invalidate_resumption && plan.invalidate_authorization_cache && plan.invalidate_attribution_cache && plan.invalidate_channel_binding && plan.invalidate_replay_state }
pub fn advance_lineage_replace(state: LineageReplaceState, event: LineageReplaceEvent, plan: Option<&LineageReplacePlan>) -> (LineageReplaceState, bool) {
    let plan_complete = plan.is_some_and(lineage_replace_plan_complete);
    if state == LineageReplaceState::ActivePredecessor && event == LineageReplaceEvent::Begin && plan_complete { return (LineageReplaceState::ReplacementPending, true); }
    if state == LineageReplaceState::ReplacementPending && event == LineageReplaceEvent::Commit && plan_complete { return (LineageReplaceState::ActiveSuccessorPredecessorRetired, true); }
    if state == LineageReplaceState::ReplacementPending && event == LineageReplaceEvent::Interrupt { return (LineageReplaceState::ContinuityBroken, true); }
    (state, false)
}
