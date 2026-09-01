use proto::auth_v3_iot_core_authz::{
    IotCoreAttributionRecordV1, IotCoreAuthorizationContextV1, IOT_CORE_SCOPE_SECURE_ASSOCIATION,
};
use proto::lineage_replace::LineageReplaceAuthorizationDecision;
use proto::lineage_replace_auth_context::{
    lineage_replace_authorization_from_iot_core, LineageReplaceAuthContextEvidence,
};

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
        "REJECT_CONTEXT_BINDING" => RejectContextBinding,
        "REJECT_PREDECESSOR_BINDING" => RejectPredecessorBinding,
        "REJECT_PRIVILEGE_EXPANSION" => RejectPrivilegeExpansion,
        _ => panic!("unknown decision {value}"),
    }
}

fn base_context() -> IotCoreAuthorizationContextV1 {
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

fn base_record(context: &IotCoreAuthorizationContextV1) -> IotCoreAttributionRecordV1 {
    IotCoreAttributionRecordV1 {
        credential_reference: [0x33; 32],
        peer_identity: [0x44; 32],
        holder_binding: context.holder_binding,
        audience_id: context.audience_id,
        role_policy_id: context.role_policy_id,
        scope_bits: context.scope_bits,
        authorization_generation: context.authorization_generation,
        policy_epoch: context.policy_epoch,
        revocation_epoch: context.revocation_epoch,
    }
}

fn base_evidence() -> LineageReplaceAuthContextEvidence {
    LineageReplaceAuthContextEvidence {
        current_credential_control_valid: true,
        successor_key_control_valid: true,
        current_session_authenticated: true,
        expected_peer_identity: [0x44; 32],
        predecessor_credential_reference: [0x33; 32],
        requested_successor_scope_bits: IOT_CORE_SCOPE_SECURE_ASSOCIATION,
    }
}

#[test]
fn canonical_auth_context_corpus() {
    let vectors = include_str!("../../../test-vectors/replay/lineage-replace-auth-context-v1.txt");
    let mut cases = 0usize;
    for line in vectors.lines() {
        if !line.starts_with("case=") {
            continue;
        }
        let fields: Vec<_> = line[5..].split('|').collect();
        assert_eq!(fields.len(), 10);
        let mut context = base_context();
        let mut record = base_record(&context);
        let mut evidence = base_evidence();
        evidence.current_credential_control_valid = bit(fields[1]);
        evidence.successor_key_control_valid = bit(fields[2]);
        evidence.current_session_authenticated = bit(fields[3]);
        if !bit(fields[4]) {
            context.authorization_generation = 0;
        }
        if !bit(fields[5]) {
            record.policy_epoch += 1;
        }
        if !bit(fields[6]) {
            record.peer_identity[0] ^= 1;
        }
        if !bit(fields[7]) {
            evidence.predecessor_credential_reference[0] ^= 1;
        }
        if !bit(fields[8]) {
            evidence.requested_successor_scope_bits = 2;
        }
        assert_eq!(
            lineage_replace_authorization_from_iot_core(
                Some(&context),
                Some(&record),
                Some(&evidence),
            ),
            expected(fields[9]),
            "{}",
            fields[0]
        );
        cases += 1;
    }
    assert_eq!(cases, 12);
}

#[test]
fn missing_inputs_fail_closed() {
    let context = base_context();
    let record = base_record(&context);
    let evidence = base_evidence();
    assert_eq!(
        lineage_replace_authorization_from_iot_core(None, Some(&record), Some(&evidence)),
        LineageReplaceAuthorizationDecision::RejectSessionAuthorization
    );
    assert_eq!(
        lineage_replace_authorization_from_iot_core(Some(&context), None, Some(&evidence)),
        LineageReplaceAuthorizationDecision::RejectSessionAuthorization
    );
    assert_eq!(
        lineage_replace_authorization_from_iot_core(Some(&context), Some(&record), None),
        LineageReplaceAuthorizationDecision::RejectCurrentCredentialControl
    );
}
