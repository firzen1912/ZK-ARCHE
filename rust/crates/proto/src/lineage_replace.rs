//! Wire-neutral `LINEAGE_REPLACE` decision predicate.
//!
//! This module classifies normalized lifecycle replacement facts. It does not
//! parse packets, mutate trust, allocate registry values, or activate a
//! successor lineage. Normal AUTH must never use it as an implicit trust
//! mutation path.

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
