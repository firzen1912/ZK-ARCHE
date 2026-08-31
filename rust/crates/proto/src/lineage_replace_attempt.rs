//! Wire-neutral attempt and confirmation binding for lineage replacement.
//!
//! A confirmation is usable only when both peers authorize one replacement
//! attempt and agree on its predecessor generation, successor, and authenticated
//! context. This module allocates no wire format or cryptographic construction.

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct LineageReplaceAttemptFacts {
    pub local_authorized: bool,
    pub peer_authorized: bool,
    pub same_attempt: bool,
    pub same_predecessor_generation: bool,
    pub same_successor: bool,
    pub same_context: bool,
    pub local_confirmation_bound: bool,
    pub peer_confirmation_bound: bool,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum LineageReplaceAttemptDecision {
    Converged,
    AwaitingConfirmation,
    Unauthorized,
    AttemptIdMismatch,
    PredecessorMismatch,
    SuccessorMismatch,
    ContextMismatch,
}

pub fn classify_lineage_replace_attempt(
    facts: &LineageReplaceAttemptFacts,
) -> LineageReplaceAttemptDecision {
    if !facts.local_authorized || !facts.peer_authorized {
        return LineageReplaceAttemptDecision::Unauthorized;
    }
    if !facts.same_attempt {
        return LineageReplaceAttemptDecision::AttemptIdMismatch;
    }
    if !facts.same_predecessor_generation {
        return LineageReplaceAttemptDecision::PredecessorMismatch;
    }
    if !facts.same_successor {
        return LineageReplaceAttemptDecision::SuccessorMismatch;
    }
    if !facts.same_context {
        return LineageReplaceAttemptDecision::ContextMismatch;
    }
    if !facts.local_confirmation_bound || !facts.peer_confirmation_bound {
        return LineageReplaceAttemptDecision::AwaitingConfirmation;
    }
    LineageReplaceAttemptDecision::Converged
}
