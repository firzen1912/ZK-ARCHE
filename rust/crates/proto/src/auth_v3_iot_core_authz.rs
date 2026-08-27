//! Draft `iot-core` AUTH-v3 authorization-context schema.
//!
//! The schema is defined in `spec/iot-core-authorization-context.md` and is
//! deliberately non-advertised. It builds on the generic canonical `ZKCTX` v1
//! encoder without changing production v2 or enabling AUTH-v3 negotiation.

use crate::auth_v3_context::{
    encode_canonical_context, hash_canonical_context, ContextEncodingError, ContextEntry,
    ContextKind,
};

pub const IOT_CORE_AUTHZ_HOLDER_BINDING_ID: u16 = 0x0001;
pub const IOT_CORE_AUTHZ_AUDIENCE_ID: u16 = 0x0002;
pub const IOT_CORE_AUTHZ_ROLE_POLICY_ID: u16 = 0x0003;
pub const IOT_CORE_AUTHZ_SCOPE_BITS_ID: u16 = 0x0004;
pub const IOT_CORE_AUTHZ_GENERATION_ID: u16 = 0x0005;
pub const IOT_CORE_AUTHZ_POLICY_EPOCH_ID: u16 = 0x0006;
pub const IOT_CORE_AUTHZ_REVOCATION_EPOCH_ID: u16 = 0x0007;

pub const IOT_CORE_SCOPE_SECURE_ASSOCIATION: u64 = 1;
pub const IOT_CORE_AUTHZ_CANONICAL_LEN: usize = 148;

#[derive(Clone, Debug, Eq, PartialEq)]
pub struct IotCoreAuthorizationContextV1 {
    pub holder_binding: [u8; 32],
    pub audience_id: [u8; 32],
    pub role_policy_id: u64,
    pub scope_bits: u64,
    pub authorization_generation: u64,
    pub policy_epoch: u64,
    pub revocation_epoch: u64,
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum IotCoreAuthorizationError {
    InvalidHolderBinding,
    InvalidAudience,
    InvalidRolePolicy,
    InvalidScope,
    InvalidAuthorizationGeneration,
    InvalidPolicyEpoch,
    InvalidRevocationEpoch,
    Encoding(ContextEncodingError),
}

impl From<ContextEncodingError> for IotCoreAuthorizationError {
    fn from(value: ContextEncodingError) -> Self {
        Self::Encoding(value)
    }
}

fn is_all_zero(value: &[u8]) -> bool {
    value.iter().all(|byte| *byte == 0)
}

pub fn validate_iot_core_authorization_context(
    context: &IotCoreAuthorizationContextV1,
) -> Result<(), IotCoreAuthorizationError> {
    if is_all_zero(&context.holder_binding) {
        return Err(IotCoreAuthorizationError::InvalidHolderBinding);
    }
    if is_all_zero(&context.audience_id) {
        return Err(IotCoreAuthorizationError::InvalidAudience);
    }
    if context.role_policy_id == 0 {
        return Err(IotCoreAuthorizationError::InvalidRolePolicy);
    }
    if context.scope_bits != IOT_CORE_SCOPE_SECURE_ASSOCIATION {
        return Err(IotCoreAuthorizationError::InvalidScope);
    }
    if context.authorization_generation == 0 {
        return Err(IotCoreAuthorizationError::InvalidAuthorizationGeneration);
    }
    if context.policy_epoch == 0 {
        return Err(IotCoreAuthorizationError::InvalidPolicyEpoch);
    }
    if context.revocation_epoch == 0 {
        return Err(IotCoreAuthorizationError::InvalidRevocationEpoch);
    }
    Ok(())
}

fn with_entries<T>(
    context: &IotCoreAuthorizationContextV1,
    operation: impl FnOnce(&[ContextEntry<'_>]) -> T,
) -> T {
    let role_policy_id = context.role_policy_id.to_le_bytes();
    let scope_bits = context.scope_bits.to_le_bytes();
    let authorization_generation = context.authorization_generation.to_le_bytes();
    let policy_epoch = context.policy_epoch.to_le_bytes();
    let revocation_epoch = context.revocation_epoch.to_le_bytes();

    let entries = [
        ContextEntry {
            id: IOT_CORE_AUTHZ_HOLDER_BINDING_ID,
            value: &context.holder_binding,
        },
        ContextEntry {
            id: IOT_CORE_AUTHZ_AUDIENCE_ID,
            value: &context.audience_id,
        },
        ContextEntry {
            id: IOT_CORE_AUTHZ_ROLE_POLICY_ID,
            value: &role_policy_id,
        },
        ContextEntry {
            id: IOT_CORE_AUTHZ_SCOPE_BITS_ID,
            value: &scope_bits,
        },
        ContextEntry {
            id: IOT_CORE_AUTHZ_GENERATION_ID,
            value: &authorization_generation,
        },
        ContextEntry {
            id: IOT_CORE_AUTHZ_POLICY_EPOCH_ID,
            value: &policy_epoch,
        },
        ContextEntry {
            id: IOT_CORE_AUTHZ_REVOCATION_EPOCH_ID,
            value: &revocation_epoch,
        },
    ];

    operation(&entries)
}

pub fn encode_iot_core_authorization_context(
    context: &IotCoreAuthorizationContextV1,
) -> Result<Vec<u8>, IotCoreAuthorizationError> {
    validate_iot_core_authorization_context(context)?;
    let encoded = with_entries(context, |entries| {
        encode_canonical_context(ContextKind::Authorization, entries)
    })?;
    debug_assert_eq!(encoded.len(), IOT_CORE_AUTHZ_CANONICAL_LEN);
    Ok(encoded)
}

pub fn hash_iot_core_authorization_context(
    context: &IotCoreAuthorizationContextV1,
) -> Result<[u8; 32], IotCoreAuthorizationError> {
    validate_iot_core_authorization_context(context)?;
    with_entries(context, |entries| {
        hash_canonical_context(ContextKind::Authorization, entries)
    })
    .map_err(Into::into)
}

#[cfg(test)]
mod tests {
    use super::*;

    const VECTOR: &str =
        include_str!("../../../test-vectors/auth-v3/iot-core-authorization-v1.txt");

    fn vector_value(key: &str) -> &str {
        VECTOR
            .lines()
            .find_map(|line| line.strip_prefix(&format!("{key}=")))
            .expect("vector key")
    }

    fn decode_32(key: &str) -> [u8; 32] {
        let bytes = hex::decode(vector_value(key)).expect("hex vector");
        bytes.try_into().expect("32-byte vector")
    }

    fn fixture() -> IotCoreAuthorizationContextV1 {
        IotCoreAuthorizationContextV1 {
            holder_binding: decode_32("holder_binding"),
            audience_id: decode_32("audience_id"),
            role_policy_id: vector_value("role_policy_id").parse().unwrap(),
            scope_bits: vector_value("scope_bits").parse().unwrap(),
            authorization_generation: vector_value("authorization_generation")
                .parse()
                .unwrap(),
            policy_epoch: vector_value("policy_epoch").parse().unwrap(),
            revocation_epoch: vector_value("revocation_epoch").parse().unwrap(),
        }
    }

    #[test]
    fn shared_vector_is_stable() {
        let context = fixture();
        let encoded = encode_iot_core_authorization_context(&context).unwrap();
        let hash = hash_iot_core_authorization_context(&context).unwrap();

        assert_eq!(encoded.len(), IOT_CORE_AUTHZ_CANONICAL_LEN);
        assert_eq!(hex::encode(encoded), vector_value("encoded"));
        assert_eq!(hex::encode(hash), vector_value("sha256"));
    }

    #[test]
    fn semantic_invalid_values_fail_closed() {
        let base = fixture();

        let mut case = base.clone();
        case.holder_binding = [0u8; 32];
        assert_eq!(
            validate_iot_core_authorization_context(&case),
            Err(IotCoreAuthorizationError::InvalidHolderBinding)
        );

        let mut case = base.clone();
        case.audience_id = [0u8; 32];
        assert_eq!(
            validate_iot_core_authorization_context(&case),
            Err(IotCoreAuthorizationError::InvalidAudience)
        );

        let mut case = base.clone();
        case.role_policy_id = 0;
        assert_eq!(
            validate_iot_core_authorization_context(&case),
            Err(IotCoreAuthorizationError::InvalidRolePolicy)
        );

        for invalid_scope in [0u64, 2u64, u64::MAX] {
            let mut case = base.clone();
            case.scope_bits = invalid_scope;
            assert_eq!(
                validate_iot_core_authorization_context(&case),
                Err(IotCoreAuthorizationError::InvalidScope)
            );
        }

        let mut case = base.clone();
        case.authorization_generation = 0;
        assert_eq!(
            validate_iot_core_authorization_context(&case),
            Err(IotCoreAuthorizationError::InvalidAuthorizationGeneration)
        );

        let mut case = base.clone();
        case.policy_epoch = 0;
        assert_eq!(
            validate_iot_core_authorization_context(&case),
            Err(IotCoreAuthorizationError::InvalidPolicyEpoch)
        );

        let mut case = base;
        case.revocation_epoch = 0;
        assert_eq!(
            validate_iot_core_authorization_context(&case),
            Err(IotCoreAuthorizationError::InvalidRevocationEpoch)
        );
    }
}
