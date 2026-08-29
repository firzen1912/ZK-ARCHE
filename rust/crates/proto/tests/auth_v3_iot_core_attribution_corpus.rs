use proto::auth_v3_iot_core_authz::{
    resolve_iot_core_attribution, IotCoreAttributionError, IotCoreAttributionRecordV1,
    IotCoreAuthorizationContextV1,
};

const CORPUS: &str =
    include_str!("../../../test-vectors/auth-v3/iot-core-attribution-decisions-v1.txt");

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

fn expected(name: &str) -> Result<(), IotCoreAttributionError> {
    match name {
        "OK" => Ok(()),
        "MISSING_REFERENCE" => Err(IotCoreAttributionError::MissingReference),
        "AMBIGUOUS_REFERENCE" => Err(IotCoreAttributionError::AmbiguousReference),
        "IDENTITY_MISMATCH" => Err(IotCoreAttributionError::IdentityMismatch),
        "AUTHORIZATION_MISMATCH" => Err(IotCoreAttributionError::AuthorizationMismatch),
        other => panic!("unknown expected decision: {other}"),
    }
}

#[test]
fn shared_attribution_decision_corpus_is_enforced() {
    assert!(CORPUS.lines().any(|line| line == "version=1"));

    for line in CORPUS.lines().filter(|line| line.starts_with("case=")) {
        let body = line.strip_prefix("case=").unwrap();
        let mut fields = body.split('|');
        let name = fields.next().unwrap();
        let mutation = fields.next().unwrap();
        let expected_name = fields.next().unwrap();
        assert!(fields.next().is_none(), "malformed corpus line: {line}");

        let context = context();
        let base = record(&context);
        let mut records = vec![base.clone()];
        let mut credential_reference = base.credential_reference;
        let mut expected_peer_identity = base.peer_identity;

        match mutation {
            "none" => {}
            "empty_records" => records.clear(),
            "duplicate_reference_peer_b2" => {
                let mut second = base.clone();
                second.peer_identity = [0xb2; 32];
                records.push(second);
            }
            "expected_peer_b2" => expected_peer_identity = [0xb2; 32],
            "record_generation_minus_1" => records[0].authorization_generation -= 1,
            "record_role_plus_1" => records[0].role_policy_id += 1,
            "record_audience_flip_0" => records[0].audience_id[0] ^= 1,
            "second_ref_peer_b2_query_first_ref_peer_b2" => {
                let mut second = base.clone();
                second.credential_reference = [0xa2; 32];
                second.peer_identity = [0xb2; 32];
                records.push(second);
                expected_peer_identity = [0xb2; 32];
            }
            "second_ref_peer_b2_query_second" => {
                let mut second = base.clone();
                second.credential_reference = [0xa2; 32];
                second.peer_identity = [0xb2; 32];
                records.push(second);
                credential_reference = [0xa2; 32];
                expected_peer_identity = [0xb2; 32];
            }
            other => panic!("unknown corpus mutation: {other}"),
        }

        let actual = resolve_iot_core_attribution(
            &records,
            &credential_reference,
            &expected_peer_identity,
            &context,
        )
        .map(|_| ());

        assert_eq!(actual, expected(expected_name), "corpus case {name}");
    }
}
