//! Storage-neutral freshness classifier for `LINEAGE_REPLACE`.
//!
//! This layer keeps record integrity separate from rollback freshness. A future
//! storage adapter may resume an otherwise valid lineage state only when its
//! authenticated record generation exactly matches a valid, context-bound
//! trusted high-water generation. The model does not claim that any particular
//! filesystem, flash device, secure element, or monotonic counter provides the
//! required durability or anti-rollback property.

use crate::lineage_replace::LineageReplaceState;
use crate::lineage_replace_recovery::{recover_lineage_replace, LineageReplaceRecoveryFacts};

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct LineageReplaceFreshnessFacts {
    pub anchor_available: bool,
    pub anchor_integrity_valid: bool,
    pub anchor_binding_valid: bool,
    pub record_generation: u64,
    pub trusted_high_water_generation: u64,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum LineageReplaceFreshnessDecision {
    Current,
    AnchorUnavailable,
    AnchorInvalid,
    BindingMismatch,
    RollbackDetected,
    GenerationAhead,
}

pub fn classify_lineage_replace_freshness(
    facts: &LineageReplaceFreshnessFacts,
) -> LineageReplaceFreshnessDecision {
    if !facts.anchor_integrity_valid {
        return LineageReplaceFreshnessDecision::AnchorInvalid;
    }
    if !facts.anchor_available {
        return LineageReplaceFreshnessDecision::AnchorUnavailable;
    }
    if !facts.anchor_binding_valid {
        return LineageReplaceFreshnessDecision::BindingMismatch;
    }
    if facts.record_generation < facts.trusted_high_water_generation {
        return LineageReplaceFreshnessDecision::RollbackDetected;
    }
    if facts.record_generation > facts.trusted_high_water_generation {
        return LineageReplaceFreshnessDecision::GenerationAhead;
    }
    LineageReplaceFreshnessDecision::Current
}

pub fn recover_lineage_replace_with_freshness(
    recovery: &LineageReplaceRecoveryFacts,
    freshness: &LineageReplaceFreshnessFacts,
) -> LineageReplaceState {
    if classify_lineage_replace_freshness(freshness) != LineageReplaceFreshnessDecision::Current {
        return LineageReplaceState::ContinuityBroken;
    }
    recover_lineage_replace(recovery)
}
