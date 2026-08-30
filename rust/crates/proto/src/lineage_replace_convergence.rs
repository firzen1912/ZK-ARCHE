//! Wire-neutral convergence classifier for authenticated lineage replacement.
//!
//! This model intentionally allocates no messages or packet fields. It captures
//! only the decision boundary a future transport-specific exchange must satisfy:
//! a replacement is usable only after both peers authenticate the same successor
//! lineage and both confirm the replacement. Competing authenticated successors
//! fail closed rather than selecting a winner implicitly.

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct LineageReplaceConvergenceFacts {
    pub local_authorized: bool,
    pub peer_authorized: bool,
    pub same_successor: bool,
    pub local_confirmed: bool,
    pub peer_confirmed: bool,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum LineageReplaceConvergenceDecision {
    Converged,
    AwaitingConfirmation,
    Unauthorized,
    SuccessorConflict,
}

pub fn classify_lineage_replace_convergence(
    facts: &LineageReplaceConvergenceFacts,
) -> LineageReplaceConvergenceDecision {
    if !facts.local_authorized || !facts.peer_authorized {
        return LineageReplaceConvergenceDecision::Unauthorized;
    }
    if !facts.same_successor {
        return LineageReplaceConvergenceDecision::SuccessorConflict;
    }
    if facts.local_confirmed && facts.peer_confirmed {
        return LineageReplaceConvergenceDecision::Converged;
    }
    LineageReplaceConvergenceDecision::AwaitingConfirmation
}
