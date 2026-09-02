use proto::auth_v3::AuthV3Context;
use proto::auth_v3_iot_core_authz::{
    IotCoreAttributionRecordV1, IotCoreAuthorizationContextV1, IOT_CORE_SCOPE_SECURE_ASSOCIATION,
};
use proto::lineage_replace::LineageReplaceAuthorizationDecision;
use proto::lineage_replace_bound_auth_context::{
    lineage_replace_authorization_from_bound_iot_core, LineageReplaceBoundAuthContextEvidence,
};
use proto::lineage_replace_session_binding::LineageReplaceSessionBindingExpectation;

fn bit(value: &str) -> bool {
    match value {
        "1" => true,
        "0" => false,
        _ => panic!("invalid bit {value}"),
    }
}
fn expected(value: &str) -> LineageReplaceAuthorizationDecision {
    use LineageReplaceAuthorizationDecision::*;
    match value {
        "AUTHORIZED_REPLACEMENT" => AuthorizedReplacement,
        "REJECT_CURRENT_CREDENTIAL_CONTROL" => RejectCurrentCredentialControl,
        "REJECT_SUCCESSOR_KEY_CONTROL" => RejectSuccessorKeyControl,
        "REJECT_SESSION_AUTHENTICATION" => RejectSessionAuthentication,
        "REJECT_SESSION_AUTHORIZATION" => RejectSessionAuthorization,
        "REJECT_PREDECESSOR_BINDING" => RejectPredecessorBinding,
        "REJECT_PRIVILEGE_EXPANSION" => RejectPrivilegeExpansion,
        _ => panic!("unknown decision {value}"),
    }
}
fn base_authorization() -> IotCoreAuthorizationContextV1 {
    IotCoreAuthorizationContextV1 {
        holder_binding: [0x11; 32],
        audience_id: [0x22; 32],
        role_policy_id: 7,
        scope_bits: IOT_CORE_SCOPE_SECURE_ASSOCIATION,
        authorization_generation: 9,
        policy_epoch: 3,
        revocation_epoch: 4,
    }
}
fn base_record(c: &IotCoreAuthorizationContextV1) -> IotCoreAttributionRecordV1 {
    IotCoreAttributionRecordV1 {
        credential_reference: [0x33; 32],
        peer_identity: [0x44; 32],
        holder_binding: c.holder_binding,
        audience_id: c.audience_id,
        role_policy_id: c.role_policy_id,
        scope_bits: c.scope_bits,
        authorization_generation: c.authorization_generation,
        policy_epoch: c.policy_epoch,
        revocation_epoch: c.revocation_epoch,
    }
}
fn base_evidence() -> LineageReplaceBoundAuthContextEvidence {
    LineageReplaceBoundAuthContextEvidence {
        current_credential_control_valid: true,
        successor_key_control_valid: true,
        expected_peer_identity: [0x44; 32],
        predecessor_credential_reference: [0x33; 32],
        requested_successor_scope_bits: IOT_CORE_SCOPE_SECURE_ASSOCIATION,
    }
}
#[test]
fn canonical_bound_auth_context_corpus() {
    let vectors =
        include_str!("../../../test-vectors/replay/lineage-replace-bound-auth-context-v1.txt");
    let session_id = [0x51; 16];
    let authz_hash = [0x52; 32];
    let critical_hash = [0x53; 32];
    let channel_hash = [0x54; 32];
    let mut cases = 0usize;
    for line in vectors.lines() {
        if !line.starts_with("case=") {
            continue;
        }
        let f: Vec<_> = line[5..].split('|').collect();
        assert_eq!(f.len(), 11);
        let authorization = base_authorization();
        let mut record = base_record(&authorization);
        let mut evidence = base_evidence();
        let context = AuthV3Context {
            protocol_version: 3,
            suite_id: 1,
            profile_id: 1,
            selected_capabilities: 0,
            session_id: &session_id,
            authz_context_hash: &authz_hash,
            critical_extensions_hash: &critical_hash,
            channel_binding_hash: &channel_hash,
        };
        let mut expectation = LineageReplaceSessionBindingExpectation {
            auth_completion_verified: bit(f[3]),
            expected_protocol_version: 3,
            expected_suite_id: 1,
            expected_profile_id: 1,
            expected_session_id: session_id,
            expected_authz_context_hash: authz_hash,
            expected_channel_binding_hash: channel_hash,
        };
        evidence.current_credential_control_valid = bit(f[1]);
        evidence.successor_key_control_valid = bit(f[2]);
        if !bit(f[4]) {
            expectation.expected_session_id[0] ^= 1;
        }
        if !bit(f[5]) {
            expectation.expected_authz_context_hash[0] ^= 1;
        }
        if !bit(f[6]) {
            expectation.expected_channel_binding_hash[0] ^= 1;
        }
        if !bit(f[7]) {
            record.policy_epoch += 1;
        }
        if !bit(f[8]) {
            evidence.predecessor_credential_reference[0] ^= 1;
        }
        if !bit(f[9]) {
            evidence.requested_successor_scope_bits = 2;
        }
        assert_eq!(
            lineage_replace_authorization_from_bound_iot_core(
                Some(&context),
                Some(&expectation),
                Some(&authorization),
                Some(&record),
                Some(&evidence)
            ),
            expected(f[10]),
            "{}",
            f[0]
        );
        cases += 1;
    }
    assert_eq!(cases, 12);
}
#[test]
fn missing_session_inputs_fail_as_session_authentication() {
    let authorization = base_authorization();
    let record = base_record(&authorization);
    let evidence = base_evidence();
    let session_id = [0x51; 16];
    let authz_hash = [0x52; 32];
    let critical_hash = [0x53; 32];
    let channel_hash = [0x54; 32];
    let context = AuthV3Context {
        protocol_version: 3,
        suite_id: 1,
        profile_id: 1,
        selected_capabilities: 0,
        session_id: &session_id,
        authz_context_hash: &authz_hash,
        critical_extensions_hash: &critical_hash,
        channel_binding_hash: &channel_hash,
    };
    let expectation = LineageReplaceSessionBindingExpectation {
        auth_completion_verified: true,
        expected_protocol_version: 3,
        expected_suite_id: 1,
        expected_profile_id: 1,
        expected_session_id: session_id,
        expected_authz_context_hash: authz_hash,
        expected_channel_binding_hash: channel_hash,
    };
    assert_eq!(
        lineage_replace_authorization_from_bound_iot_core(
            None,
            Some(&expectation),
            Some(&authorization),
            Some(&record),
            Some(&evidence)
        ),
        LineageReplaceAuthorizationDecision::RejectSessionAuthentication
    );
    assert_eq!(
        lineage_replace_authorization_from_bound_iot_core(
            Some(&context),
            None,
            Some(&authorization),
            Some(&record),
            Some(&evidence)
        ),
        LineageReplaceAuthorizationDecision::RejectSessionAuthentication
    );
}
