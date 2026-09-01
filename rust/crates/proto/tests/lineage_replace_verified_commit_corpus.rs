use proto::auth_v3::AuthV3Context;
use proto::auth_v3_iot_core_authz::{
    IotCoreAttributionRecordV1, IotCoreAuthorizationContextV1, IOT_CORE_SCOPE_SECURE_ASSOCIATION,
};
use proto::lineage_replace::{
    LineageReplaceAuthorizationDecision, LineageReplaceDecision, LineageReplaceFacts,
    LineageReplaceTrigger,
};
use proto::lineage_replace_bound_auth_context::LineageReplaceVerifiedBoundAuthContextEvidence;
use proto::lineage_replace_possession::VerifiedLifecyclePossessionProof;
use proto::lineage_replace_session_binding::LineageReplaceSessionBindingExpectation;
use proto::lineage_replace_storage_transaction::{
    LineageReplaceStorageStep, LineageReplaceStorageTransactionResult,
};
use proto::lineage_replace_verified_commit::{
    execute_verified_bound_iot_core_lineage_replace_commit, LineageReplaceVerifiedCommitPhase,
};

fn phase(v: &str) -> LineageReplaceVerifiedCommitPhase {
    match v {
        "STORAGE_COMMITTED" => LineageReplaceVerifiedCommitPhase::StorageCommitted,
        "STORAGE_FAILED" => LineageReplaceVerifiedCommitPhase::StorageFailed,
        "REJECT_DECISION" => LineageReplaceVerifiedCommitPhase::RejectDecision,
        "REJECT_AUTHORIZATION" => LineageReplaceVerifiedCommitPhase::RejectAuthorization,
        _ => panic!("invalid phase"),
    }
}
fn authorization(v: &str) -> LineageReplaceAuthorizationDecision {
    match v {
        "AUTHORIZED_REPLACEMENT" => LineageReplaceAuthorizationDecision::AuthorizedReplacement,
        "REJECT_CURRENT_CREDENTIAL_CONTROL" => {
            LineageReplaceAuthorizationDecision::RejectCurrentCredentialControl
        }
        "REJECT_SESSION_AUTHENTICATION" => {
            LineageReplaceAuthorizationDecision::RejectSessionAuthentication
        }
        "REJECT_PRIVILEGE_EXPANSION" => {
            LineageReplaceAuthorizationDecision::RejectPrivilegeExpansion
        }
        _ => panic!("invalid authorization"),
    }
}
fn decision(v: &str) -> LineageReplaceDecision {
    match v {
        "ACCEPT_SUCCESSOR" => LineageReplaceDecision::AcceptSuccessor,
        "REJECT_AUTHORITY" => LineageReplaceDecision::RejectAuthority,
        "REJECT_FRESHNESS" => LineageReplaceDecision::RejectFreshness,
        "REJECT_REPLAY" => LineageReplaceDecision::RejectReplay,
        _ => panic!("invalid decision"),
    }
}
fn storage(v: &str) -> LineageReplaceStorageTransactionResult {
    match v {
        "COMMITTED" => LineageReplaceStorageTransactionResult::Committed,
        "REJECT_PLAN" => LineageReplaceStorageTransactionResult::RejectPlan,
        "FAIL_PENDING" => LineageReplaceStorageTransactionResult::FailPending,
        "FAIL_SUCCESSOR" => LineageReplaceStorageTransactionResult::FailSuccessor,
        _ => panic!("invalid storage result"),
    }
}
fn step(v: &str) -> Option<LineageReplaceStorageStep> {
    match v {
        "NONE" => None,
        "PERSIST_PENDING" => Some(LineageReplaceStorageStep::PersistPending),
        "ACTIVATE_SUCCESSOR" => Some(LineageReplaceStorageStep::ActivateSuccessor),
        _ => panic!("invalid step"),
    }
}
fn step_name(v: LineageReplaceStorageStep) -> &'static str {
    match v {
        LineageReplaceStorageStep::PersistPending => "PERSIST_PENDING",
        LineageReplaceStorageStep::ActivateSuccessor => "ACTIVATE_SUCCESSOR",
        LineageReplaceStorageStep::RetirePredecessor => "RETIRE_PREDECESSOR",
        LineageReplaceStorageStep::InvalidateDependentState => "INVALIDATE_DEPENDENT_STATE",
        LineageReplaceStorageStep::ClearPending => "CLEAR_PENDING",
    }
}
fn base_facts() -> LineageReplaceFacts {
    LineageReplaceFacts {
        trigger: LineageReplaceTrigger::Lifecycle,
        authority_valid: false,
        predecessor_valid: true,
        successor_valid: true,
        context_valid: true,
        freshness_valid: true,
        replay_free: true,
        concurrent_free: true,
        rollback_clear: true,
        storage_safe: true,
        dependent_state_safe: true,
    }
}

#[test]
fn canonical_lineage_replace_verified_commit_corpus() {
    let corpus =
        include_str!("../../../test-vectors/replay/lineage-replace-verified-commit-v1.txt");
    let mut cases = 0usize;
    for line in corpus.lines() {
        if !line.starts_with("case=") {
            continue;
        }
        let f: Vec<&str> = line[5..].split('|').collect();
        assert_eq!(f.len(), 9);
        let sid = [0x51u8; 16];
        let ah = [0x52u8; 32];
        let eh = [0x53u8; 32];
        let cb = [0x54u8; 32];
        let auth_context = AuthV3Context {
            protocol_version: 3,
            suite_id: 1,
            profile_id: 1,
            selected_capabilities: 0,
            session_id: &sid,
            authz_context_hash: &ah,
            critical_extensions_hash: &eh,
            channel_binding_hash: &cb,
        };
        let mut expectation = LineageReplaceSessionBindingExpectation {
            auth_completion_verified: true,
            expected_protocol_version: 3,
            expected_suite_id: 1,
            expected_profile_id: 1,
            expected_session_id: sid,
            expected_authz_context_hash: ah,
            expected_channel_binding_hash: cb,
        };
        let authz = IotCoreAuthorizationContextV1 {
            holder_binding: [0x11; 32],
            audience_id: [0x22; 32],
            role_policy_id: 7,
            scope_bits: IOT_CORE_SCOPE_SECURE_ASSOCIATION,
            authorization_generation: 9,
            policy_epoch: 3,
            revocation_epoch: 4,
        };
        let attribution = IotCoreAttributionRecordV1 {
            credential_reference: [0x33; 32],
            peer_identity: [0x44; 32],
            holder_binding: authz.holder_binding,
            audience_id: authz.audience_id,
            role_policy_id: authz.role_policy_id,
            scope_bits: authz.scope_bits,
            authorization_generation: authz.authorization_generation,
            policy_epoch: authz.policy_epoch,
            revocation_epoch: authz.revocation_epoch,
        };
        let mut evidence = LineageReplaceVerifiedBoundAuthContextEvidence {
            current_credential_proof: VerifiedLifecyclePossessionProof {
                verification_valid: true,
                session_id: sid,
                subject_reference: [0x33; 32],
            },
            successor_key_proof: VerifiedLifecyclePossessionProof {
                verification_valid: true,
                session_id: sid,
                subject_reference: [0x66; 32],
            },
            successor_key_reference: [0x66; 32],
            expected_peer_identity: [0x44; 32],
            predecessor_credential_reference: [0x33; 32],
            requested_successor_scope_bits: IOT_CORE_SCOPE_SECURE_ASSOCIATION,
        };
        let mut facts = base_facts();
        match f[1] {
            "NONE" => {}
            "CURRENT_PROOF" => evidence.current_credential_proof.verification_valid = false,
            "SESSION" => expectation.auth_completion_verified = false,
            "PRIVILEGE" => evidence.requested_successor_scope_bits = 2,
            _ => panic!("invalid auth fault"),
        }
        match f[2] {
            "NONE" => {}
            "FRESHNESS" => facts.freshness_valid = false,
            "REPLAY" => facts.replay_free = false,
            _ => panic!("invalid decision fault"),
        }
        let fail = step(f[3]);
        let mut trace: Vec<&'static str> = Vec::new();
        let result = execute_verified_bound_iot_core_lineage_replace_commit(
            Some(&auth_context),
            Some(&expectation),
            Some(&authz),
            Some(&attribution),
            Some(&evidence),
            Some(&facts),
            Some(|s| {
                trace.push(step_name(s));
                Some(s) != fail
            }),
        );
        assert_eq!(result.phase, phase(f[4]));
        assert_eq!(result.authorization, authorization(f[5]));
        assert_eq!(result.decision, decision(f[6]));
        assert_eq!(result.storage_result, storage(f[7]));
        let actual = if trace.is_empty() {
            "NONE".to_string()
        } else {
            trace.join(",")
        };
        assert_eq!(actual, f[8]);
        cases += 1;
    }
    assert_eq!(cases, 8);
}
