//! Draft `iot-core` AUTH-v3 authorization-context schema.
//!
//! The schema is defined in `spec/iot-core-authorization-context.md` and is
//! deliberately non-advertised. It builds on the generic canonical `ZKCTX` v1
//! encoder without changing production v2 or enabling AUTH-v3 negotiation.

use crate::auth_v3_context::{
    encode_canonical_context, hash_canonical_context, ContextEncodingError, ContextEntry,
    ContextKind,
};
use crate::auth_v3_context_parser::{parse_canonical_context_bounded, ContextParseError};
use sha2::{Digest, Sha256};

pub const IOT_CORE_AUTHZ_HOLDER_BINDING_ID: u16 = 0x0001;
pub const IOT_CORE_AUTHZ_AUDIENCE_ID: u16 = 0x0002;
pub const IOT_CORE_AUTHZ_ROLE_POLICY_ID: u16 = 0x0003;
pub const IOT_CORE_AUTHZ_SCOPE_BITS_ID: u16 = 0x0004;
pub const IOT_CORE_AUTHZ_GENERATION_ID: u16 = 0x0005;
pub const IOT_CORE_AUTHZ_POLICY_EPOCH_ID: u16 = 0x0006;
pub const IOT_CORE_AUTHZ_REVOCATION_EPOCH_ID: u16 = 0x0007;

pub const IOT_CORE_SCOPE_SECURE_ASSOCIATION: u64 = 1;
pub const IOT_CORE_AUTHZ_ENTRY_COUNT: usize = 7;
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

/// One locally authoritative attribution tuple for `iot-core` AUTH.
///
/// This is local state only: it is not a wire format, registry identifier, or
/// enrollment mechanism. A caller selects a credential reference and expected
/// peer identity, then this tuple binds that selection to the exact holder,
/// audience, role/policy, scope, and freshness context used by AUTH.
#[derive(Clone, Debug, Eq, PartialEq)]
pub struct IotCoreAttributionRecordV1 {
    pub credential_reference: [u8; 32],
    pub peer_identity: [u8; 32],
    pub holder_binding: [u8; 32],
    pub audience_id: [u8; 32],
    pub role_policy_id: u64,
    pub scope_bits: u64,
    pub authorization_generation: u64,
    pub policy_epoch: u64,
    pub revocation_epoch: u64,
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum IotCoreAttributionError {
    MissingReference,
    AmbiguousReference,
    IdentityMismatch,
    AuthorizationMismatch,
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
    InvalidEncodingLength,
    InvalidContextKind,
    InvalidEntrySchema,
    Encoding(ContextEncodingError),
    Parsing(ContextParseError),
}

impl From<ContextEncodingError> for IotCoreAuthorizationError {
    fn from(value: ContextEncodingError) -> Self {
        Self::Encoding(value)
    }
}

impl From<ContextParseError> for IotCoreAuthorizationError {
    fn from(value: ContextParseError) -> Self {
        Self::Parsing(value)
    }
}

fn is_all_zero(value: &[u8]) -> bool {
    value.iter().all(|byte| *byte == 0)
}

fn load_u64_le(value: &[u8]) -> u64 {
    u64::from_le_bytes(value.try_into().expect("validated 8-byte field"))
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

/// Resolve one credential reference to exactly one locally authorized peer.
///
/// The resolver is intentionally pure and read-only. It does not create
/// aliases, repair records, learn credentials, or mutate trust. Ambiguous
/// references fail before identity or authorization use. A selected record
/// must match both the caller's expected peer identity and every authorization
/// field that is carried by the active `iot-core` context.
pub fn resolve_iot_core_attribution<'a>(
    records: &'a [IotCoreAttributionRecordV1],
    credential_reference: &[u8; 32],
    expected_peer_identity: &[u8; 32],
    context: &IotCoreAuthorizationContextV1,
) -> Result<&'a IotCoreAttributionRecordV1, IotCoreAttributionError> {
    let mut candidates = records
        .iter()
        .filter(|record| &record.credential_reference == credential_reference);

    let record = candidates
        .next()
        .ok_or(IotCoreAttributionError::MissingReference)?;
    if candidates.next().is_some() {
        return Err(IotCoreAttributionError::AmbiguousReference);
    }

    if &record.peer_identity != expected_peer_identity {
        return Err(IotCoreAttributionError::IdentityMismatch);
    }

    if record.holder_binding != context.holder_binding
        || record.audience_id != context.audience_id
        || record.role_policy_id != context.role_policy_id
        || record.scope_bits != context.scope_bits
        || record.authorization_generation != context.authorization_generation
        || record.policy_epoch != context.policy_epoch
        || record.revocation_epoch != context.revocation_epoch
    {
        return Err(IotCoreAttributionError::AuthorizationMismatch);
    }

    Ok(record)
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

/// Decode and validate one byte-exact `iot-core` authorization context.
///
/// The fixed 148-byte profile bound is enforced before generic parsing, and
/// the parser receives the profile's seven-entry ceiling before allocating its
/// entry vector. No normalization or re-encoding is performed on receive.
pub fn decode_iot_core_authorization_context_bytes(
    input: &[u8],
) -> Result<IotCoreAuthorizationContextV1, IotCoreAuthorizationError> {
    if input.len() != IOT_CORE_AUTHZ_CANONICAL_LEN {
        return Err(IotCoreAuthorizationError::InvalidEncodingLength);
    }

    let parsed = parse_canonical_context_bounded(input, IOT_CORE_AUTHZ_ENTRY_COUNT)?;
    if parsed.kind != ContextKind::Authorization {
        return Err(IotCoreAuthorizationError::InvalidContextKind);
    }
    if parsed.entries.len() != IOT_CORE_AUTHZ_ENTRY_COUNT {
        return Err(IotCoreAuthorizationError::InvalidEntrySchema);
    }

    let expected = [
        (IOT_CORE_AUTHZ_HOLDER_BINDING_ID, 32usize),
        (IOT_CORE_AUTHZ_AUDIENCE_ID, 32usize),
        (IOT_CORE_AUTHZ_ROLE_POLICY_ID, 8usize),
        (IOT_CORE_AUTHZ_SCOPE_BITS_ID, 8usize),
        (IOT_CORE_AUTHZ_GENERATION_ID, 8usize),
        (IOT_CORE_AUTHZ_POLICY_EPOCH_ID, 8usize),
        (IOT_CORE_AUTHZ_REVOCATION_EPOCH_ID, 8usize),
    ];
    for (entry, (expected_id, expected_len)) in parsed.entries.iter().zip(expected) {
        if entry.id != expected_id || entry.value.len() != expected_len {
            return Err(IotCoreAuthorizationError::InvalidEntrySchema);
        }
    }

    let context = IotCoreAuthorizationContextV1 {
        holder_binding: parsed.entries[0]
            .value
            .try_into()
            .map_err(|_| IotCoreAuthorizationError::InvalidEntrySchema)?,
        audience_id: parsed.entries[1]
            .value
            .try_into()
            .map_err(|_| IotCoreAuthorizationError::InvalidEntrySchema)?,
        role_policy_id: load_u64_le(parsed.entries[2].value),
        scope_bits: load_u64_le(parsed.entries[3].value),
        authorization_generation: load_u64_le(parsed.entries[4].value),
        policy_epoch: load_u64_le(parsed.entries[5].value),
        revocation_epoch: load_u64_le(parsed.entries[6].value),
    };
    validate_iot_core_authorization_context(&context)?;
    Ok(context)
}

/// Validate the profile-specific receive contract, then hash the exact bytes.
pub fn hash_iot_core_authorization_context_bytes(
    input: &[u8],
) -> Result<[u8; 32], IotCoreAuthorizationError> {
    decode_iot_core_authorization_context_bytes(input)?;
    let mut out = [0u8; 32];
    out.copy_from_slice(&Sha256::digest(input));
    Ok(out)
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
            authorization_generation: vector_value("authorization_generation").parse().unwrap(),
            policy_epoch: vector_value("policy_epoch").parse().unwrap(),
            revocation_epoch: vector_value("revocation_epoch").parse().unwrap(),
        }
    }

    fn attribution_fixture() -> IotCoreAttributionRecordV1 {
        let context = fixture();
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
    fn shared_vector_is_stable() {
        let context = fixture();
        let encoded = encode_iot_core_authorization_context(&context).unwrap();
        let hash = hash_iot_core_authorization_context(&context).unwrap();

        assert_eq!(encoded.len(), IOT_CORE_AUTHZ_CANONICAL_LEN);
        assert_eq!(hex::encode(encoded), vector_value("encoded"));
        assert_eq!(hex::encode(hash), vector_value("sha256"));
    }

    #[test]
    fn shared_vector_decodes_under_profile_bounds() {
        let context = fixture();
        let encoded = hex::decode(vector_value("encoded")).unwrap();
        let decoded = decode_iot_core_authorization_context_bytes(&encoded).unwrap();
        let hash = hash_iot_core_authorization_context_bytes(&encoded).unwrap();

        assert_eq!(decoded, context);
        assert_eq!(hex::encode(hash), vector_value("sha256"));
    }

    #[test]
    fn receive_profile_bounds_fail_closed_before_semantic_use() {
        let encoded = hex::decode(vector_value("encoded")).unwrap();

        assert_eq!(
            decode_iot_core_authorization_context_bytes(&encoded[..encoded.len() - 1]),
            Err(IotCoreAuthorizationError::InvalidEncodingLength)
        );

        let mut oversized = encoded.clone();
        oversized.push(0);
        assert_eq!(
            decode_iot_core_authorization_context_bytes(&oversized),
            Err(IotCoreAuthorizationError::InvalidEncodingLength)
        );

        let mut too_many_entries = encoded.clone();
        too_many_entries[7] = 8;
        too_many_entries[8] = 0;
        assert_eq!(
            decode_iot_core_authorization_context_bytes(&too_many_entries),
            Err(IotCoreAuthorizationError::Parsing(
                ContextParseError::EntryLimitExceeded
            ))
        );

        let mut wrong_kind = encoded.clone();
        wrong_kind[6] = ContextKind::ChannelBinding as u8;
        assert_eq!(
            decode_iot_core_authorization_context_bytes(&wrong_kind),
            Err(IotCoreAuthorizationError::InvalidContextKind)
        );

        let mut wrong_schema = encoded;
        wrong_schema[135] = 8;
        wrong_schema[136] = 0;
        assert_eq!(
            decode_iot_core_authorization_context_bytes(&wrong_schema),
            Err(IotCoreAuthorizationError::InvalidEntrySchema)
        );
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

    #[test]
    fn attribution_resolver_accepts_exact_local_binding() {
        let context = fixture();
        let record = attribution_fixture();
        let records = [record.clone()];

        assert_eq!(
            resolve_iot_core_attribution(
                &records,
                &record.credential_reference,
                &record.peer_identity,
                &context,
            ),
            Ok(&record)
        );
    }

    #[test]
    fn attribution_resolver_rejects_missing_and_ambiguous_references() {
        let context = fixture();
        let record = attribution_fixture();

        assert_eq!(
            resolve_iot_core_attribution(&[], &[0xa1; 32], &[0xb1; 32], &context),
            Err(IotCoreAttributionError::MissingReference)
        );

        let mut conflicting = record.clone();
        conflicting.peer_identity = [0xb2; 32];
        let records = [record.clone(), conflicting];
        assert_eq!(
            resolve_iot_core_attribution(
                &records,
                &record.credential_reference,
                &record.peer_identity,
                &context,
            ),
            Err(IotCoreAttributionError::AmbiguousReference)
        );
    }

    #[test]
    fn attribution_resolver_rejects_identity_and_policy_substitution() {
        let context = fixture();
        let record = attribution_fixture();
        let records = [record.clone()];

        assert_eq!(
            resolve_iot_core_attribution(
                &records,
                &record.credential_reference,
                &[0xb2; 32],
                &context,
            ),
            Err(IotCoreAttributionError::IdentityMismatch)
        );

        let mut stale = record.clone();
        stale.authorization_generation -= 1;
        assert_eq!(
            resolve_iot_core_attribution(
                &[stale],
                &record.credential_reference,
                &record.peer_identity,
                &context,
            ),
            Err(IotCoreAttributionError::AuthorizationMismatch)
        );

        let mut wrong_role = record.clone();
        wrong_role.role_policy_id += 1;
        assert_eq!(
            resolve_iot_core_attribution(
                &[wrong_role],
                &record.credential_reference,
                &record.peer_identity,
                &context,
            ),
            Err(IotCoreAttributionError::AuthorizationMismatch)
        );

        let mut wrong_audience = record.clone();
        wrong_audience.audience_id[0] ^= 1;
        assert_eq!(
            resolve_iot_core_attribution(
                &[wrong_audience],
                &record.credential_reference,
                &record.peer_identity,
                &context,
            ),
            Err(IotCoreAttributionError::AuthorizationMismatch)
        );
    }

    #[test]
    fn same_holder_does_not_create_implicit_identity_equivalence() {
        let context = fixture();
        let first = attribution_fixture();
        let mut second = first.clone();
        second.credential_reference = [0xa2; 32];
        second.peer_identity = [0xb2; 32];
        let records = [first.clone(), second.clone()];

        assert_eq!(
            resolve_iot_core_attribution(
                &records,
                &first.credential_reference,
                &second.peer_identity,
                &context,
            ),
            Err(IotCoreAttributionError::IdentityMismatch)
        );

        assert_eq!(
            resolve_iot_core_attribution(
                &records,
                &second.credential_reference,
                &second.peer_identity,
                &context,
            ),
            Ok(&second)
        );
    }
}
