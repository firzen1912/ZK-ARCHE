//! Wire-neutral transition guard for lineage replacement reconciliation.
//!
//! Durable state alone MUST NOT manufacture current-attempt confirmation
//! provenance required to activate a successor after reconciliation.

use crate::lineage_replace_attempt_evidence::LineageReplaceAttemptEvidenceDecision;
use crate::lineage_replace_reconciliation::LineageReplaceReconciliationDecision;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct LineageReplaceReconciliationTransitionFacts {
    pub prior: LineageReplaceReconciliationDecision,
    pub current: LineageReplaceReconciliationDecision,
    pub attempt_evidence: LineageReplaceAttemptEvidenceDecision,
    pub explicit_clean_retry: bool,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum LineageReplaceReconciliationTransition {
    Hold,
    ActivateSuccessor,
    ResumePredecessor,
    RejectDivergence,
    ContinuityBroken,
}

pub fn classify_lineage_replace_reconciliation_transition(
    facts: &LineageReplaceReconciliationTransitionFacts,
) -> LineageReplaceReconciliationTransition {
    use LineageReplaceReconciliationDecision::{
        PairContinuityBroken, PairPredecessorReady, PairSuccessorReady, ReconciliationRequired,
        SuccessorDivergence,
    };
    use LineageReplaceReconciliationTransition::{
        ActivateSuccessor, ContinuityBroken, Hold, RejectDivergence, ResumePredecessor,
    };

    if facts.prior == PairContinuityBroken || facts.current == PairContinuityBroken {
        return ContinuityBroken;
    }

    if facts.current == SuccessorDivergence {
        return RejectDivergence;
    }

    if facts.prior == SuccessorDivergence && !facts.explicit_clean_retry {
        return RejectDivergence;
    }

    if facts.current == PairSuccessorReady {
        if facts.prior == PairSuccessorReady {
            return ActivateSuccessor;
        }
        if facts.attempt_evidence == LineageReplaceAttemptEvidenceDecision::FreshCurrentAttempt
            && (facts.prior == ReconciliationRequired
                || (facts.prior == SuccessorDivergence && facts.explicit_clean_retry))
        {
            return ActivateSuccessor;
        }
        return Hold;
    }

    if facts.current == PairPredecessorReady
        && (facts.prior == PairPredecessorReady
            || facts.prior == ReconciliationRequired
            || (facts.prior == SuccessorDivergence && facts.explicit_clean_retry))
    {
        return ResumePredecessor;
    }

    Hold
}
