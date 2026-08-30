//! Storage-neutral restart classifier for `LINEAGE_REPLACE`.
//!
//! The classifier accepts only two unambiguous durable observations: the clean
//! predecessor state or the fully committed successor state. Pending, mixed,
//! incomplete, corrupt, or empty observations fail closed to
//! `ContinuityBroken`. It does not perform storage I/O or claim rollback-proof
//! persistence.

use crate::lineage_replace::LineageReplaceState;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct LineageReplaceRecoveryFacts {
    pub record_integrity_valid: bool,
    pub predecessor_active: bool,
    pub replacement_pending: bool,
    pub successor_active: bool,
    pub predecessor_retired: bool,
    pub invalidations_complete: bool,
}

pub fn recover_lineage_replace(facts: &LineageReplaceRecoveryFacts) -> LineageReplaceState {
    if !facts.record_integrity_valid {
        return LineageReplaceState::ContinuityBroken;
    }

    if facts.predecessor_active
        && !facts.replacement_pending
        && !facts.successor_active
        && !facts.predecessor_retired
        && !facts.invalidations_complete
    {
        return LineageReplaceState::ActivePredecessor;
    }

    if !facts.predecessor_active
        && !facts.replacement_pending
        && facts.successor_active
        && facts.predecessor_retired
        && facts.invalidations_complete
    {
        return LineageReplaceState::ActiveSuccessorPredecessorRetired;
    }

    LineageReplaceState::ContinuityBroken
}
