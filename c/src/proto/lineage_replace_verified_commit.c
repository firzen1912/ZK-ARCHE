#include "auth/lineage_replace_verified_commit.h"

lineage_replace_verified_commit_result_t lineage_replace_execute_verified_bound_iot_core_commit(
    const auth_v3_context_t *auth_context,
    const lineage_replace_session_binding_expectation_t *session_expectation,
    const auth_v3_iot_core_authorization_context_v1_t *authorization_context,
    const auth_v3_iot_core_attribution_record_v1_t *attribution,
    const lineage_replace_verified_bound_auth_context_evidence_t *evidence,
    const lineage_replace_facts_t *facts,
    lineage_replace_storage_apply_fn apply,
    void *storage_context) {
    lineage_replace_verified_commit_result_t out;
    lineage_replace_plan_t plan;

    out.phase = LINEAGE_REPLACE_VERIFIED_COMMIT_REJECT_AUTHORIZATION;
    out.authorization = LINEAGE_REPLACE_REJECT_CURRENT_CREDENTIAL_CONTROL;
    out.decision = LINEAGE_REPLACE_REJECT_AUTHORITY;
    out.storage_result = LINEAGE_REPLACE_STORAGE_TRANSACTION_REJECT_PLAN;

    out.authorization = lineage_replace_authorization_from_verified_bound_iot_core(
        auth_context, session_expectation, authorization_context, attribution, evidence);
    if (out.authorization != LINEAGE_REPLACE_AUTHORIZED_REPLACEMENT) {
        return out;
    }

    out.decision = lineage_replace_evaluate_authorized(out.authorization, facts);
    if (out.decision != LINEAGE_REPLACE_ACCEPT_SUCCESSOR) {
        out.phase = LINEAGE_REPLACE_VERIFIED_COMMIT_REJECT_DECISION;
        return out;
    }

    if (!lineage_replace_plan(out.decision, &plan)) {
        out.phase = LINEAGE_REPLACE_VERIFIED_COMMIT_REJECT_PLAN;
        return out;
    }
    if (apply == NULL) {
        out.phase = LINEAGE_REPLACE_VERIFIED_COMMIT_REJECT_STORAGE_ADAPTER;
        return out;
    }

    out.storage_result = lineage_replace_execute_storage_transaction(&plan, apply, storage_context);
    if (out.storage_result == LINEAGE_REPLACE_STORAGE_TRANSACTION_COMMITTED) {
        out.phase = LINEAGE_REPLACE_VERIFIED_COMMIT_STORAGE_COMMITTED;
    } else if (out.storage_result == LINEAGE_REPLACE_STORAGE_TRANSACTION_REJECT_PLAN) {
        out.phase = LINEAGE_REPLACE_VERIFIED_COMMIT_REJECT_PLAN;
    } else {
        out.phase = LINEAGE_REPLACE_VERIFIED_COMMIT_STORAGE_FAILED;
    }
    return out;
}
