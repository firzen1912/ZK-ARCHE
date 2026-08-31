#ifndef AUTH_LINEAGE_REPLACE_VERIFIED_COMMIT_H
#define AUTH_LINEAGE_REPLACE_VERIFIED_COMMIT_H

#include "auth/lineage_replace_bound_auth_context.h"
#include "auth/lineage_replace_storage_transaction.h"

typedef enum {
    LINEAGE_REPLACE_VERIFIED_COMMIT_REJECT_AUTHORIZATION = 0,
    LINEAGE_REPLACE_VERIFIED_COMMIT_REJECT_DECISION,
    LINEAGE_REPLACE_VERIFIED_COMMIT_REJECT_PLAN,
    LINEAGE_REPLACE_VERIFIED_COMMIT_REJECT_STORAGE_ADAPTER,
    LINEAGE_REPLACE_VERIFIED_COMMIT_STORAGE_COMMITTED,
    LINEAGE_REPLACE_VERIFIED_COMMIT_STORAGE_FAILED
} lineage_replace_verified_commit_phase_t;

typedef struct {
    lineage_replace_verified_commit_phase_t phase;
    lineage_replace_authorization_decision_t authorization;
    lineage_replace_decision_t decision;
    lineage_replace_storage_transaction_result_t storage_result;
} lineage_replace_verified_commit_result_t;

/* Compose verified possession + exact AUTH-session binding + iot-core
 * authorization with the normalized lineage predicate and storage transaction.
 * No storage callback is invoked unless every pre-storage gate accepts. */
lineage_replace_verified_commit_result_t lineage_replace_execute_verified_bound_iot_core_commit(
    const auth_v3_context_t *auth_context,
    const lineage_replace_session_binding_expectation_t *session_expectation,
    const auth_v3_iot_core_authorization_context_v1_t *authorization_context,
    const auth_v3_iot_core_attribution_record_v1_t *attribution,
    const lineage_replace_verified_bound_auth_context_evidence_t *evidence,
    const lineage_replace_facts_t *facts,
    lineage_replace_storage_apply_fn apply,
    void *storage_context);

#endif
