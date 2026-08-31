//! Wire-neutral current-attempt confirmation provenance for lineage replacement.
//!
//! This classifier is the semantic owner for whether both peers hold
//! confirmation bound to the same currently observed replacement attempt.
//! It does not define attempt identifiers on the wire or authenticate them.

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct LineageReplaceAttemptEvidenceFacts {
    pub local_attempt_id: Option<u32>,
    pub peer_attempt_id: Option<u32>,
    pub local_confirmation_attempt_id: Option<u32>,
    pub peer_confirmation_attempt_id: Option<u32>,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum LineageReplaceAttemptEvidenceDecision {
    FreshCurrentAttempt,
    MissingCurrentAttempt,
    AttemptMismatch,
    LocalConfirmationMissing,
    PeerConfirmationMissing,
}

pub fn classify_lineage_replace_attempt_evidence(
    facts: &LineageReplaceAttemptEvidenceFacts,
) -> LineageReplaceAttemptEvidenceDecision {
    use LineageReplaceAttemptEvidenceDecision::*;

    let (Some(local_attempt), Some(peer_attempt)) =
        (facts.local_attempt_id, facts.peer_attempt_id)
    else {
        return MissingCurrentAttempt;
    };
    if local_attempt != peer_attempt {
        return AttemptMismatch;
    }
    if facts.local_confirmation_attempt_id != Some(local_attempt) {
        return LocalConfirmationMissing;
    }
    if facts.peer_confirmation_attempt_id != Some(peer_attempt) {
        return PeerConfirmationMissing;
    }
    FreshCurrentAttempt
}
