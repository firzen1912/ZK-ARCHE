use crate::auth_v3::AuthV3Context;
use crate::auth_v3_iot_core_authz::{IotCoreAttributionRecordV1, IotCoreAuthorizationContextV1};
use crate::lineage_replace::LineageReplaceAuthorizationDecision;
use crate::lineage_replace_auth_context::{
    lineage_replace_authorization_from_iot_core, LineageReplaceAuthContextEvidence,
};
use crate::lineage_replace_possession::{
    classify_lineage_replace_possession, LineageReplacePossessionDecision,
    VerifiedLifecyclePossessionProof,
};
use crate::lineage_replace_session_binding::{
    classify_lineage_replace_session_binding, LineageReplaceSessionBindingDecision,
    LineageReplaceSessionBindingExpectation,
};

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct LineageReplaceBoundAuthContextEvidence {
    pub current_credential_control_valid: bool,
    pub successor_key_control_valid: bool,
    pub expected_peer_identity: [u8; 32],
    pub predecessor_credential_reference: [u8; 32],
    pub requested_successor_scope_bits: u64,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct LineageReplaceVerifiedBoundAuthContextEvidence {
    pub current_credential_proof: VerifiedLifecyclePossessionProof,
    pub successor_key_proof: VerifiedLifecyclePossessionProof,
    pub successor_key_reference: [u8; 32],
    pub expected_peer_identity: [u8; 32],
    pub predecessor_credential_reference: [u8; 32],
    pub requested_successor_scope_bits: u64,
}

pub fn lineage_replace_authorization_from_bound_iot_core(
    auth_context: Option<&AuthV3Context<'_>>,
    session_expectation: Option<&LineageReplaceSessionBindingExpectation>,
    authorization_context: Option<&IotCoreAuthorizationContextV1>,
    attribution: Option<&IotCoreAttributionRecordV1>,
    evidence: Option<&LineageReplaceBoundAuthContextEvidence>,
) -> LineageReplaceAuthorizationDecision {
    let Some(evidence) = evidence else {
        return LineageReplaceAuthorizationDecision::RejectCurrentCredentialControl;
    };
    let session_bound = matches!(
        classify_lineage_replace_session_binding(auth_context, session_expectation),
        LineageReplaceSessionBindingDecision::Bound
    );
    let normalized = LineageReplaceAuthContextEvidence {
        current_credential_control_valid: evidence.current_credential_control_valid,
        successor_key_control_valid: evidence.successor_key_control_valid,
        current_session_authenticated: session_bound,
        expected_peer_identity: evidence.expected_peer_identity,
        predecessor_credential_reference: evidence.predecessor_credential_reference,
        requested_successor_scope_bits: evidence.requested_successor_scope_bits,
    };
    lineage_replace_authorization_from_iot_core(
        authorization_context,
        attribution,
        Some(&normalized),
    )
}

pub fn lineage_replace_authorization_from_verified_bound_iot_core(
    auth_context: Option<&AuthV3Context<'_>>,
    session_expectation: Option<&LineageReplaceSessionBindingExpectation>,
    authorization_context: Option<&IotCoreAuthorizationContextV1>,
    attribution: Option<&IotCoreAttributionRecordV1>,
    evidence: Option<&LineageReplaceVerifiedBoundAuthContextEvidence>,
) -> LineageReplaceAuthorizationDecision {
    let Some(evidence) = evidence else {
        return LineageReplaceAuthorizationDecision::RejectCurrentCredentialControl;
    };
    let Some(session_expectation) = session_expectation else {
        return LineageReplaceAuthorizationDecision::RejectSessionAuthentication;
    };
    match classify_lineage_replace_possession(
        Some(&evidence.current_credential_proof),
        Some(&evidence.successor_key_proof),
        &session_expectation.expected_session_id,
        &evidence.predecessor_credential_reference,
        &evidence.successor_key_reference,
    ) {
        LineageReplacePossessionDecision::RejectCurrentCredentialControl => {
            return LineageReplaceAuthorizationDecision::RejectCurrentCredentialControl;
        }
        LineageReplacePossessionDecision::RejectSuccessorKeyControl => {
            return LineageReplaceAuthorizationDecision::RejectSuccessorKeyControl;
        }
        LineageReplacePossessionDecision::Verified => {}
    }
    let normalized = LineageReplaceBoundAuthContextEvidence {
        current_credential_control_valid: true,
        successor_key_control_valid: true,
        expected_peer_identity: evidence.expected_peer_identity,
        predecessor_credential_reference: evidence.predecessor_credential_reference,
        requested_successor_scope_bits: evidence.requested_successor_scope_bits,
    };
    lineage_replace_authorization_from_bound_iot_core(
        auth_context,
        Some(session_expectation),
        authorization_context,
        attribution,
        Some(&normalized),
    )
}
