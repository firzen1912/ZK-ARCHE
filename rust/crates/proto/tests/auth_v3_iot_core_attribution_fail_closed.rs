use proto::auth_v3_iot_core_authz::{
    resolve_iot_core_attribution, IotCoreAttributionError, IotCoreAttributionRecordV1,
    IotCoreAuthorizationContextV1,
};

fn valid_context() -> IotCoreAuthorizationContextV1 {
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

fn matching_record(context: &IotCoreAuthorizationContextV1) -> IotCoreAttributionRecordV1 {
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

fn assert_matching_invalid_context_rejected(context: IotCoreAuthorizationContextV1) {
    let record = matching_record(&context);
    let result = resolve_iot_core_attribution(
        std::slice::from_ref(&record),
        &record.credential_reference,
        &record.peer_identity,
        &context,
    )
    .map(|_| ());

    assert_eq!(
        result,
        Err(IotCoreAttributionError::AuthorizationMismatch),
        "semantically invalid AUTH context must fail before matching local attribution can authorize it"
    );
}

#[test]
fn semantically_invalid_context_cannot_self_validate_against_matching_local_state() {
    let mut context = valid_context();
    context.holder_binding = [0; 32];
    assert_matching_invalid_context_rejected(context);

    let mut context = valid_context();
    context.audience_id = [0; 32];
    assert_matching_invalid_context_rejected(context);

    let mut context = valid_context();
    context.role_policy_id = 0;
    assert_matching_invalid_context_rejected(context);

    let mut context = valid_context();
    context.scope_bits = 0;
    assert_matching_invalid_context_rejected(context);

    let mut context = valid_context();
    context.authorization_generation = 0;
    assert_matching_invalid_context_rejected(context);

    let mut context = valid_context();
    context.policy_epoch = 0;
    assert_matching_invalid_context_rejected(context);

    let mut context = valid_context();
    context.revocation_epoch = 0;
    assert_matching_invalid_context_rejected(context);
}
