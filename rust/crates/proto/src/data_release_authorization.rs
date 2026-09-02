//! Wire-neutral policy-bound DATA release authorization classifier.
//!
//! Authentication alone never authorizes protected-data release. This module
//! consumes already-verified local facts and returns whether a device may
//! release key material, must require fresh AUTH, or must deny the request.
//!
//! The device-local sovereignty authority remains the final release authority:
//! external policy, successful AUTH, or transport metadata cannot substitute
//! for an explicit current local release authority.

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct DataReleaseFacts {
    pub authenticated: bool,
    pub device_release_authority_present: bool,
    pub device_release_authority_current: bool,
    pub protected_data_encrypted: bool,
    pub release_key_scope_match: bool,
    pub authorization_present: bool,
    pub authorization_fresh: bool,
    pub revocation_current: bool,
    pub explicitly_revoked: bool,
    pub lineage_current: bool,
    pub holder_match: bool,
    pub audience_match: bool,
    pub purpose_match: bool,
    pub data_type_match: bool,
    pub policy_match: bool,
    pub epoch_match: bool,
    pub channel_binding_required: bool,
    pub channel_binding_valid: bool,
    pub rollback_suspected: bool,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum DataReleaseAction { Release, FreshAuthRequired, Deny }

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum DataReleaseReason {
    Current,
    RollbackSuspected,
    Unauthenticated,
    DeviceReleaseAuthorityMissing,
    DeviceReleaseAuthorityStale,
    ProtectedDataNotEncrypted,
    ReleaseKeyScopeMismatch,
    AuthorizationMissing,
    AuthorizationStale,
    RevocationStale,
    Revoked,
    LineageStale,
    HolderMismatch,
    AudienceMismatch,
    PurposeMismatch,
    DataTypeMismatch,
    PolicyMismatch,
    EpochMismatch,
    ChannelBindingMissingOrInvalid,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct DataReleaseDecision {
    pub action: DataReleaseAction,
    pub reason: DataReleaseReason,
}

fn decision(action: DataReleaseAction, reason: DataReleaseReason) -> DataReleaseDecision {
    DataReleaseDecision { action, reason }
}

pub fn classify_data_release(f: &DataReleaseFacts) -> DataReleaseDecision {
    if f.rollback_suspected { return decision(DataReleaseAction::Deny, DataReleaseReason::RollbackSuspected); }
    if !f.authenticated { return decision(DataReleaseAction::FreshAuthRequired, DataReleaseReason::Unauthenticated); }
    if !f.device_release_authority_present { return decision(DataReleaseAction::Deny, DataReleaseReason::DeviceReleaseAuthorityMissing); }
    if !f.device_release_authority_current { return decision(DataReleaseAction::Deny, DataReleaseReason::DeviceReleaseAuthorityStale); }
    if !f.protected_data_encrypted { return decision(DataReleaseAction::Deny, DataReleaseReason::ProtectedDataNotEncrypted); }
    if !f.release_key_scope_match { return decision(DataReleaseAction::Deny, DataReleaseReason::ReleaseKeyScopeMismatch); }
    if !f.authorization_present { return decision(DataReleaseAction::Deny, DataReleaseReason::AuthorizationMissing); }
    if !f.authorization_fresh { return decision(DataReleaseAction::Deny, DataReleaseReason::AuthorizationStale); }
    if !f.revocation_current { return decision(DataReleaseAction::Deny, DataReleaseReason::RevocationStale); }
    if f.explicitly_revoked { return decision(DataReleaseAction::Deny, DataReleaseReason::Revoked); }
    if !f.lineage_current { return decision(DataReleaseAction::Deny, DataReleaseReason::LineageStale); }
    if !f.holder_match { return decision(DataReleaseAction::Deny, DataReleaseReason::HolderMismatch); }
    if !f.audience_match { return decision(DataReleaseAction::Deny, DataReleaseReason::AudienceMismatch); }
    if !f.purpose_match { return decision(DataReleaseAction::Deny, DataReleaseReason::PurposeMismatch); }
    if !f.data_type_match { return decision(DataReleaseAction::Deny, DataReleaseReason::DataTypeMismatch); }
    if !f.policy_match { return decision(DataReleaseAction::Deny, DataReleaseReason::PolicyMismatch); }
    if !f.epoch_match { return decision(DataReleaseAction::Deny, DataReleaseReason::EpochMismatch); }
    if f.channel_binding_required && !f.channel_binding_valid {
        return decision(DataReleaseAction::Deny, DataReleaseReason::ChannelBindingMissingOrInvalid);
    }
    decision(DataReleaseAction::Release, DataReleaseReason::Current)
}

#[cfg(test)]
mod tests {
    use super::*;
    fn bit(value: &str) -> bool { match value { "0" => false, "1" => true, _ => panic!("invalid bit: {value}") } }
    fn action(value: &str) -> DataReleaseAction {
        match value { "RELEASE" => DataReleaseAction::Release, "FRESH_AUTH_REQUIRED" => DataReleaseAction::FreshAuthRequired, "DENY" => DataReleaseAction::Deny, _ => panic!("invalid action: {value}") }
    }
    fn reason(value: &str) -> DataReleaseReason {
        match value {
            "CURRENT" => DataReleaseReason::Current,
            "ROLLBACK_SUSPECTED" => DataReleaseReason::RollbackSuspected,
            "UNAUTHENTICATED" => DataReleaseReason::Unauthenticated,
            "DEVICE_RELEASE_AUTHORITY_MISSING" => DataReleaseReason::DeviceReleaseAuthorityMissing,
            "DEVICE_RELEASE_AUTHORITY_STALE" => DataReleaseReason::DeviceReleaseAuthorityStale,
            "PROTECTED_DATA_NOT_ENCRYPTED" => DataReleaseReason::ProtectedDataNotEncrypted,
            "RELEASE_KEY_SCOPE_MISMATCH" => DataReleaseReason::ReleaseKeyScopeMismatch,
            "AUTHORIZATION_MISSING" => DataReleaseReason::AuthorizationMissing,
            "AUTHORIZATION_STALE" => DataReleaseReason::AuthorizationStale,
            "REVOCATION_STALE" => DataReleaseReason::RevocationStale,
            "REVOKED" => DataReleaseReason::Revoked,
            "LINEAGE_STALE" => DataReleaseReason::LineageStale,
            "HOLDER_MISMATCH" => DataReleaseReason::HolderMismatch,
            "AUDIENCE_MISMATCH" => DataReleaseReason::AudienceMismatch,
            "PURPOSE_MISMATCH" => DataReleaseReason::PurposeMismatch,
            "DATA_TYPE_MISMATCH" => DataReleaseReason::DataTypeMismatch,
            "POLICY_MISMATCH" => DataReleaseReason::PolicyMismatch,
            "EPOCH_MISMATCH" => DataReleaseReason::EpochMismatch,
            "CHANNEL_BINDING_MISSING_OR_INVALID" => DataReleaseReason::ChannelBindingMissingOrInvalid,
            _ => panic!("invalid reason: {value}"),
        }
    }
    #[test]
    fn canonical_v2_corpus_matches_classifier() {
        let corpus = include_str!("../../../test-vectors/state/data-release-authorization-v2.txt");
        let mut count = 0usize;
        for line in corpus.lines() {
            let Some(case) = line.strip_prefix("case=") else { continue; };
            let fields: Vec<&str> = case.split('|').collect();
            assert_eq!(fields.len(), 22);
            let facts = DataReleaseFacts {
                authenticated: bit(fields[1]), device_release_authority_present: bit(fields[2]),
                device_release_authority_current: bit(fields[3]), protected_data_encrypted: bit(fields[4]),
                release_key_scope_match: bit(fields[5]), authorization_present: bit(fields[6]),
                authorization_fresh: bit(fields[7]), revocation_current: bit(fields[8]),
                explicitly_revoked: bit(fields[9]), lineage_current: bit(fields[10]),
                holder_match: bit(fields[11]), audience_match: bit(fields[12]), purpose_match: bit(fields[13]),
                data_type_match: bit(fields[14]), policy_match: bit(fields[15]), epoch_match: bit(fields[16]),
                channel_binding_required: bit(fields[17]), channel_binding_valid: bit(fields[18]),
                rollback_suspected: bit(fields[19]),
            };
            assert_eq!(classify_data_release(&facts),
                DataReleaseDecision { action: action(fields[20]), reason: reason(fields[21]) }, "case {}", fields[0]);
            count += 1;
        }
        assert_eq!(count, 19);
    }
}
