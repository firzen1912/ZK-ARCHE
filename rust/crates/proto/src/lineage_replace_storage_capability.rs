#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct LineageReplaceStorageCapability {
    pub durable_commit_confirmed: bool,
    pub power_loss_recovery_supported: bool,
    pub record_integrity_protected: bool,
    pub replay_protection_supported: bool,
    pub freshness_anchor_available: bool,
    pub freshness_anchor_integrity_valid: bool,
    pub freshness_anchor_lineage_bound: bool,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum LineageReplaceStorageCapabilityDecision {
    Qualified,
    RejectDurability,
    RejectPowerLossRecovery,
    RejectRecordIntegrity,
    RejectReplayProtection,
    RejectFreshnessAnchor,
    RejectFreshnessIntegrity,
    RejectFreshnessBinding,
}

/// Classify whether a concrete storage adapter declares the properties needed
/// by the rollback-resistant lineage-replacement qualification path.
///
/// This is a semantic gate over adapter metadata, not evidence that the target
/// actually provides the claimed property. A `true` capability therefore MUST
/// remain backed by target-specific retained evidence before a measured or
/// deployment-qualified claim is made.
pub fn classify_lineage_replace_storage_capability(
    capability: Option<&LineageReplaceStorageCapability>,
) -> LineageReplaceStorageCapabilityDecision {
    let Some(capability) = capability else {
        return LineageReplaceStorageCapabilityDecision::RejectDurability;
    };
    if !capability.durable_commit_confirmed {
        return LineageReplaceStorageCapabilityDecision::RejectDurability;
    }
    if !capability.power_loss_recovery_supported {
        return LineageReplaceStorageCapabilityDecision::RejectPowerLossRecovery;
    }
    if !capability.record_integrity_protected {
        return LineageReplaceStorageCapabilityDecision::RejectRecordIntegrity;
    }
    if !capability.replay_protection_supported {
        return LineageReplaceStorageCapabilityDecision::RejectReplayProtection;
    }
    if !capability.freshness_anchor_available {
        return LineageReplaceStorageCapabilityDecision::RejectFreshnessAnchor;
    }
    if !capability.freshness_anchor_integrity_valid {
        return LineageReplaceStorageCapabilityDecision::RejectFreshnessIntegrity;
    }
    if !capability.freshness_anchor_lineage_bound {
        return LineageReplaceStorageCapabilityDecision::RejectFreshnessBinding;
    }
    LineageReplaceStorageCapabilityDecision::Qualified
}
