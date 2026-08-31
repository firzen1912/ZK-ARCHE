use crate::auth_v3::AuthV3Context;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum LineageReplaceSessionBindingDecision {
    Bound,
    RejectCompletion,
    RejectVersion,
    RejectSuite,
    RejectProfile,
    RejectSessionId,
    RejectAuthzContext,
    RejectChannelBinding,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct LineageReplaceSessionBindingExpectation {
    pub auth_completion_verified: bool,
    pub expected_protocol_version: u8,
    pub expected_suite_id: u16,
    pub expected_profile_id: u16,
    pub expected_session_id: [u8; 16],
    pub expected_authz_context_hash: [u8; 32],
    pub expected_channel_binding_hash: [u8; 32],
}

pub fn classify_lineage_replace_session_binding(
    context: Option<&AuthV3Context<'_>>,
    expectation: Option<&LineageReplaceSessionBindingExpectation>,
) -> LineageReplaceSessionBindingDecision {
    let (Some(context), Some(expectation)) = (context, expectation) else {
        return LineageReplaceSessionBindingDecision::RejectCompletion;
    };
    if !expectation.auth_completion_verified {
        return LineageReplaceSessionBindingDecision::RejectCompletion;
    }
    if context.protocol_version != expectation.expected_protocol_version {
        return LineageReplaceSessionBindingDecision::RejectVersion;
    }
    if context.suite_id != expectation.expected_suite_id {
        return LineageReplaceSessionBindingDecision::RejectSuite;
    }
    if context.profile_id != expectation.expected_profile_id {
        return LineageReplaceSessionBindingDecision::RejectProfile;
    }
    if *context.session_id != expectation.expected_session_id {
        return LineageReplaceSessionBindingDecision::RejectSessionId;
    }
    if *context.authz_context_hash != expectation.expected_authz_context_hash {
        return LineageReplaceSessionBindingDecision::RejectAuthzContext;
    }
    if *context.channel_binding_hash != expectation.expected_channel_binding_hash {
        return LineageReplaceSessionBindingDecision::RejectChannelBinding;
    }
    LineageReplaceSessionBindingDecision::Bound
}
