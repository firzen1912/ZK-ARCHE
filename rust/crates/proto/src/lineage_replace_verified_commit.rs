use crate::auth_v3::AuthV3Context;
use crate::auth_v3_iot_core_authz::{IotCoreAttributionRecordV1, IotCoreAuthorizationContextV1};
use crate::lineage_replace::{
    evaluate_authorized_lineage_replace, plan_lineage_replace, LineageReplaceAuthorizationDecision,
    LineageReplaceDecision, LineageReplaceFacts,
};
use crate::lineage_replace_bound_auth_context::{
    lineage_replace_authorization_from_verified_bound_iot_core,
    LineageReplaceVerifiedBoundAuthContextEvidence,
};
use crate::lineage_replace_session_binding::LineageReplaceSessionBindingExpectation;
use crate::lineage_replace_storage_transaction::{
    execute_lineage_replace_storage_transaction, LineageReplaceStorageStep,
    LineageReplaceStorageTransactionResult,
};

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum LineageReplaceVerifiedCommitPhase {
    RejectAuthorization,
    RejectDecision,
    RejectPlan,
    RejectStorageAdapter,
    StorageCommitted,
    StorageFailed,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct LineageReplaceVerifiedCommitResult {
    pub phase: LineageReplaceVerifiedCommitPhase,
    pub authorization: LineageReplaceAuthorizationDecision,
    pub decision: LineageReplaceDecision,
    pub storage_result: LineageReplaceStorageTransactionResult,
}

/// Compose the verified lifecycle authorization chain with the normalized
/// replacement predicate and ordered storage transaction. `apply` is never
/// invoked unless authorization, lifecycle facts, and plan construction all
/// succeed.
pub fn execute_verified_bound_iot_core_lineage_replace_commit<F>(
    auth_context: Option<&AuthV3Context<'_>>,
    session_expectation: Option<&LineageReplaceSessionBindingExpectation>,
    authorization_context: Option<&IotCoreAuthorizationContextV1>,
    attribution: Option<&IotCoreAttributionRecordV1>,
    evidence: Option<&LineageReplaceVerifiedBoundAuthContextEvidence>,
    facts: Option<&LineageReplaceFacts>,
    mut apply: Option<F>,
) -> LineageReplaceVerifiedCommitResult
where
    F: FnMut(LineageReplaceStorageStep) -> bool,
{
    let authorization = lineage_replace_authorization_from_verified_bound_iot_core(
        auth_context,
        session_expectation,
        authorization_context,
        attribution,
        evidence,
    );
    if authorization != LineageReplaceAuthorizationDecision::AuthorizedReplacement {
        return LineageReplaceVerifiedCommitResult {
            phase: LineageReplaceVerifiedCommitPhase::RejectAuthorization,
            authorization,
            decision: LineageReplaceDecision::RejectAuthority,
            storage_result: LineageReplaceStorageTransactionResult::RejectPlan,
        };
    }

    let decision = facts.map_or(LineageReplaceDecision::RejectStorage, |facts| {
        evaluate_authorized_lineage_replace(authorization, facts)
    });
    if decision != LineageReplaceDecision::AcceptSuccessor {
        return LineageReplaceVerifiedCommitResult {
            phase: LineageReplaceVerifiedCommitPhase::RejectDecision,
            authorization,
            decision,
            storage_result: LineageReplaceStorageTransactionResult::RejectPlan,
        };
    }

    let Some(plan) = plan_lineage_replace(decision) else {
        return LineageReplaceVerifiedCommitResult {
            phase: LineageReplaceVerifiedCommitPhase::RejectPlan,
            authorization,
            decision,
            storage_result: LineageReplaceStorageTransactionResult::RejectPlan,
        };
    };
    let Some(ref mut apply) = apply else {
        return LineageReplaceVerifiedCommitResult {
            phase: LineageReplaceVerifiedCommitPhase::RejectStorageAdapter,
            authorization,
            decision,
            storage_result: LineageReplaceStorageTransactionResult::RejectPlan,
        };
    };

    let storage_result = execute_lineage_replace_storage_transaction(Some(&plan), apply);
    let phase = match storage_result {
        LineageReplaceStorageTransactionResult::Committed => {
            LineageReplaceVerifiedCommitPhase::StorageCommitted
        }
        LineageReplaceStorageTransactionResult::RejectPlan => {
            LineageReplaceVerifiedCommitPhase::RejectPlan
        }
        _ => LineageReplaceVerifiedCommitPhase::StorageFailed,
    };
    LineageReplaceVerifiedCommitResult {
        phase,
        authorization,
        decision,
        storage_result,
    }
}
