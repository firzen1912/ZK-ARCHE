use proto::auth_v3_iot_core_authz::{
    resolve_iot_core_attribution, IotCoreAttributionError, IotCoreAttributionRecordV1,
    IotCoreAuthorizationContextV1,
};

const CORPUS: &str =
    include_str!("../../../test-vectors/auth-v3/iot-core-authorization-freshness-v1.txt");

fn context() -> IotCoreAuthorizationContextV1 {
    IotCoreAuthorizationContextV1 {
        holder_binding: [0x11; 32],
        audience_id: [0x22; 32],
        role_policy_id: 7,
        scope_bits: 1,
        authorization_generation: 9,
        policy_epoch: 11,
        revocation_epoch: 13,
    }
}

fn record(context: &IotCoreAuthorizationContextV1) -> IotCoreAttributionRecordV1 {
    IotCoreAttributionRecordV1 {
        credential_reference: [0xa1; 32],
        peer_identity: [0xb1; 32],
        holder_binding: context.holder_binding,
        audience_id: context.audience_id,
        role_policy_id: context.role_policy_id,
        scope_bits: context.scope_bits,
        authorization_generation: context.authorization_generation,
        policy_epoch: context.policy_epoch,
        revocation_epoch: context.revocation_epoch,
    }
}

#[test]
fn stale_policy_and_revocation_authority_fail_closed() {
    assert!(CORPUS.lines().any(|line| line == "version=1"));
    let context = context();
    let mut case_count = 0usize;

    for line in CORPUS.lines().filter(|line| line.starts_with("case=")) {
        let body = line.strip_prefix("case=").unwrap();
        let mut fields = body.split('|');
        let name = fields.next().unwrap();
        let mutation = fields.next().unwrap();
        let expected = fields.next().unwrap();
        assert!(fields.next().is_none(), "malformed corpus line: {line}");
        assert_eq!(expected, "AUTHORIZATION_MISMATCH", "corpus case {name}");

        let mut authority = record(&context);
        match mutation {
            "record_policy_epoch_minus_1" => authority.policy_epoch -= 1,
            "record_revocation_epoch_minus_1" => authority.revocation_epoch -= 1,
            other => panic!("unknown freshness mutation: {other}"),
        }

        assert_eq!(
            resolve_iot_core_attribution(
                &[authority.clone()],
                &authority.credential_reference,
                &authority.peer_identity,
                &context,
            )
            .map(|_| ()),
            Err(IotCoreAttributionError::AuthorizationMismatch),
            "corpus case {name}"
        );
        case_count += 1;
    }

    assert_eq!(case_count, 2);
}
