use crate::auth_v3_iot_core_authz::{
    IotCoreAttributionRecordV1, IotCoreAuthorizationContextV1, IOT_CORE_SCOPE_SECURE_ASSOCIATION,
};
use crate::lineage_replace::{
    classify_lineage_replace_authorization, LineageReplaceAuthorizationDecision,
    LineageReplaceAuthorizationFacts,
};

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct LineageReplaceAuthContextEvidence {
    pub current_credential_control_valid: bool,
    pub successor_key_control_valid: bool,
    pub current_session_authenticated: bool,
    pub expected_peer_identity: [u8; 32],
    pub predecessor_credential_reference: [u8; 32],
    pub requested_successor_scope_bits: u64,
}

fn context_shape_valid(context: &IotCoreAuthorizationContextV1) -> bool {
    context.holder_binding.iter().any(|byte| *byte != 0)
        && context.audience_id.iter().any(|byte| *byte != 0)
        && context.role_policy_id != 0
        && context.scope_bits == IOT_CORE_SCOPE_SECURE_ASSOCIATION
        && context.authorization_generation != 0
        && context.policy_epoch != 0
        && context.revocation_epoch != 0
}

pub fn lineage_replace_authorization_from_iot_core(
    context: Option<&IotCoreAuthorizationContextV1>,
    attribution: Option<&IotCoreAttributionRecordV1>,
    evidence: Option<&LineageReplaceAuthContextEvidence>,
) -> LineageReplaceAuthorizationDecision {
    let Some(evidence) = evidence else {
        return LineageReplaceAuthorizationDecision::RejectCurrentCredentialControl;
    };
    let mut facts = LineageReplaceAuthorizationFacts {
        current_credential_control_valid: evidence.current_credential_control_valid,
        successor_key_control_valid: evidence.successor_key_control_valid,
        current_session_authenticated: evidence.current_session_authenticated,
        current_session_authorized: false,
        current_context_bound: false,
        predecessor_binding_current: false,
        successor_scope_within_authorized_scope: false,
    };
    let (Some(context), Some(attribution)) = (context, attribution) else {
        return classify_lineage_replace_authorization(&facts);
    };
    if !context_shape_valid(context) {
        return classify_lineage_replace_authorization(&facts);
    }
    let attribution_context_match = attribution.holder_binding == context.holder_binding
        && attribution.audience_id == context.audience_id
        && attribution.role_policy_id == context.role_policy_id
        && attribution.scope_bits == context.scope_bits
        && attribution.authorization_generation == context.authorization_generation
        && attribution.policy_epoch == context.policy_epoch
        && attribution.revocation_epoch == context.revocation_epoch;
    let peer_match = attribution.peer_identity == evidence.expected_peer_identity;
    let predecessor_match =
        attribution.credential_reference == evidence.predecessor_credential_reference;
    facts.current_session_authorized = attribution_context_match;
    facts.current_context_bound = attribution_context_match && peer_match;
    facts.predecessor_binding_current = predecessor_match
        && attribution.authorization_generation == context.authorization_generation;
    facts.successor_scope_within_authorized_scope = evidence.requested_successor_scope_bits != 0
        && (evidence.requested_successor_scope_bits & !context.scope_bits) == 0;
    classify_lineage_replace_authorization(&facts)
}
