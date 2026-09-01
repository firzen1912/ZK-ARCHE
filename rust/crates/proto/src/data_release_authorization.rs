//! Wire-neutral policy-bound DATA release authorization classifier.
//!
//! Authentication alone never authorizes protected-data release. This module
//! consumes already-verified local facts and returns whether a device may
//! release key material, must require fresh AUTH, or must deny the request.

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct DataReleaseFacts {
    pub authenticated: bool,
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
    Current, RollbackSuspected, Unauthenticated, AuthorizationMissing,
    AuthorizationStale, RevocationStale, Revoked, LineageStale, HolderMismatch,
    AudienceMismatch, PurposeMismatch, DataTypeMismatch, PolicyMismatch,
    EpochMismatch, ChannelBindingMissingOrInvalid,
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
