#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum LineageReplacePossessionDecision {
    RejectCurrentCredentialControl,
    RejectSuccessorKeyControl,
    Verified,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct VerifiedLifecyclePossessionProof {
    pub verification_valid: bool,
    pub session_id: [u8; 16],
    pub subject_reference: [u8; 32],
}

pub fn classify_lineage_replace_possession(
    current_credential_proof: Option<&VerifiedLifecyclePossessionProof>,
    successor_key_proof: Option<&VerifiedLifecyclePossessionProof>,
    expected_session_id: &[u8; 16],
    expected_predecessor_credential_reference: &[u8; 32],
    expected_successor_key_reference: &[u8; 32],
) -> LineageReplacePossessionDecision {
    let Some(current) = current_credential_proof else {
        return LineageReplacePossessionDecision::RejectCurrentCredentialControl;
    };
    if !current.verification_valid
        || current.session_id != *expected_session_id
        || current.subject_reference != *expected_predecessor_credential_reference
    {
        return LineageReplacePossessionDecision::RejectCurrentCredentialControl;
    }

    let Some(successor) = successor_key_proof else {
        return LineageReplacePossessionDecision::RejectSuccessorKeyControl;
    };
    if !successor.verification_valid
        || successor.session_id != *expected_session_id
        || successor.subject_reference != *expected_successor_key_reference
    {
        return LineageReplacePossessionDecision::RejectSuccessorKeyControl;
    }

    LineageReplacePossessionDecision::Verified
}
