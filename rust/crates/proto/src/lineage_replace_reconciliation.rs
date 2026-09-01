//! Wire-neutral pair reconciliation for durable lineage replacement.
//!
//! This module composes already-normalized replacement-attempt and durable
//! recovery outcomes across two peers. It allocates no wire format, performs no
//! trust mutation, and does not infer missing confirmation from persisted state.

use crate::lineage_replace::LineageReplaceState;
use crate::lineage_replace_attempt::LineageReplaceAttemptDecision;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct LineageReplaceReconciliationFacts {
    pub local_attempt: LineageReplaceAttemptDecision,
    pub local_state: LineageReplaceState,
    pub peer_attempt: LineageReplaceAttemptDecision,
    pub peer_state: LineageReplaceState,
    pub same_successor: bool,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum LineageReplaceReconciliationDecision {
    PairSuccessorReady,
    PairPredecessorReady,
    ReconciliationRequired,
    PairContinuityBroken,
    SuccessorDivergence,
}

pub fn classify_lineage_replace_reconciliation(
    facts: &LineageReplaceReconciliationFacts,
) -> LineageReplaceReconciliationDecision {
    if facts.local_state == LineageReplaceState::ContinuityBroken
        || facts.peer_state == LineageReplaceState::ContinuityBroken
    {
        return LineageReplaceReconciliationDecision::PairContinuityBroken;
    }

    let local_successor =
        facts.local_state == LineageReplaceState::ActiveSuccessorPredecessorRetired;
    let peer_successor = facts.peer_state == LineageReplaceState::ActiveSuccessorPredecessorRetired;

    if local_successor && peer_successor {
        if !facts.same_successor {
            return LineageReplaceReconciliationDecision::SuccessorDivergence;
        }
        if facts.local_attempt == LineageReplaceAttemptDecision::Converged
            && facts.peer_attempt == LineageReplaceAttemptDecision::Converged
        {
            return LineageReplaceReconciliationDecision::PairSuccessorReady;
        }
        return LineageReplaceReconciliationDecision::ReconciliationRequired;
    }

    if facts.local_state == LineageReplaceState::ActivePredecessor
        && facts.peer_state == LineageReplaceState::ActivePredecessor
        && facts.local_attempt != LineageReplaceAttemptDecision::Converged
        && facts.peer_attempt != LineageReplaceAttemptDecision::Converged
    {
        return LineageReplaceReconciliationDecision::PairPredecessorReady;
    }

    LineageReplaceReconciliationDecision::ReconciliationRequired
}
