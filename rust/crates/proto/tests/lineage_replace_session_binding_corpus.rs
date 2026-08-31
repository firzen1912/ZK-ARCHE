use proto::auth_v3::AuthV3Context;
use proto::lineage_replace_session_binding::{classify_lineage_replace_session_binding, LineageReplaceSessionBindingDecision as D, LineageReplaceSessionBindingExpectation as E};

#[test]
fn session_binding_rejects_stale_or_unbound_auth_context() {
    let sid=[1u8;16]; let authz=[2u8;32]; let critical=[4u8;32]; let channel=[3u8;32];
    let context=AuthV3Context{protocol_version:3,suite_id:1,profile_id:1,selected_capabilities:5,session_id:&sid,authz_context_hash:&authz,critical_extensions_hash:&critical,channel_binding_hash:&channel};
    let mut e=E{auth_completion_verified:true,expected_protocol_version:3,expected_suite_id:1,expected_profile_id:1,expected_session_id:sid,expected_authz_context_hash:authz,expected_channel_binding_hash:channel};
    assert_eq!(classify_lineage_replace_session_binding(Some(&context),Some(&e)),D::Bound);
    e.auth_completion_verified=false; assert_eq!(classify_lineage_replace_session_binding(Some(&context),Some(&e)),D::RejectCompletion);
    e.auth_completion_verified=true; e.expected_session_id[0]=9; assert_eq!(classify_lineage_replace_session_binding(Some(&context),Some(&e)),D::RejectSessionId);
    e.expected_session_id=sid; e.expected_authz_context_hash[0]=9; assert_eq!(classify_lineage_replace_session_binding(Some(&context),Some(&e)),D::RejectAuthzContext);
    e.expected_authz_context_hash=authz; e.expected_channel_binding_hash[0]=9; assert_eq!(classify_lineage_replace_session_binding(Some(&context),Some(&e)),D::RejectChannelBinding);
}
