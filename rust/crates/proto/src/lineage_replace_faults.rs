//! Deterministic, storage-neutral write-cut model for `LINEAGE_REPLACE`.
//!
//! Each cut point produces the normalized restart facts that a future storage
//! adapter would expose if execution stopped at that logical boundary. This is
//! a model of ordering requirements, not evidence about any filesystem, flash
//! device, journal, secure element, or physical power-loss behavior.

use crate::lineage_replace_recovery::LineageReplaceRecoveryFacts;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum LineageReplaceWriteCut {
    BeforeBegin,
    AfterPendingMarker,
    AfterSuccessorActivation,
    AfterPredecessorRetirement,
    AfterInvalidations,
    AfterCommitMarkerClear,
}

pub fn facts_at_write_cut(cut: LineageReplaceWriteCut) -> LineageReplaceRecoveryFacts {
    match cut {
        LineageReplaceWriteCut::BeforeBegin => LineageReplaceRecoveryFacts {
            record_integrity_valid: true,
            predecessor_active: true,
            replacement_pending: false,
            successor_active: false,
            predecessor_retired: false,
            invalidations_complete: false,
        },
        LineageReplaceWriteCut::AfterPendingMarker => LineageReplaceRecoveryFacts {
            record_integrity_valid: true,
            predecessor_active: true,
            replacement_pending: true,
            successor_active: false,
            predecessor_retired: false,
            invalidations_complete: false,
        },
        LineageReplaceWriteCut::AfterSuccessorActivation => LineageReplaceRecoveryFacts {
            record_integrity_valid: true,
            predecessor_active: true,
            replacement_pending: true,
            successor_active: true,
            predecessor_retired: false,
            invalidations_complete: false,
        },
        LineageReplaceWriteCut::AfterPredecessorRetirement => LineageReplaceRecoveryFacts {
            record_integrity_valid: true,
            predecessor_active: false,
            replacement_pending: true,
            successor_active: true,
            predecessor_retired: true,
            invalidations_complete: false,
        },
        LineageReplaceWriteCut::AfterInvalidations => LineageReplaceRecoveryFacts {
            record_integrity_valid: true,
            predecessor_active: false,
            replacement_pending: true,
            successor_active: true,
            predecessor_retired: true,
            invalidations_complete: true,
        },
        LineageReplaceWriteCut::AfterCommitMarkerClear => LineageReplaceRecoveryFacts {
            record_integrity_valid: true,
            predecessor_active: false,
            replacement_pending: false,
            successor_active: true,
            predecessor_retired: true,
            invalidations_complete: true,
        },
    }
}
