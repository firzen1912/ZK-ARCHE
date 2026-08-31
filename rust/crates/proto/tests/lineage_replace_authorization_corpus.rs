use proto::lineage_replace::{classify_lineage_replace_authorization, evaluate_authorized_lineage_replace, LineageReplaceAuthorizationDecision as A, LineageReplaceAuthorizationFacts, LineageReplaceDecision as D, LineageReplaceFacts, LineageReplaceTrigger};

const CORPUS: &str = include_str!("../../../test-vectors/replay/lineage-replace-authorization-v1.txt");
fn bit(v: &str) -> bool { match v { "1" => true, "0" => false, _ => panic!("bad bit {v}") } }
fn auth(v: &str) -> A { match v { "AUTHORIZED_REPLACEMENT" => A::AuthorizedReplacement, "REJECT_CURRENT_CREDENTIAL_CONTROL" => A::RejectCurrentCredentialControl, "REJECT_SUCCESSOR_KEY_CONTROL" => A::RejectSuccessorKeyControl, "REJECT_SESSION_AUTHENTICATION" => A::RejectSessionAuthentication, "REJECT_SESSION_AUTHORIZATION" => A::RejectSessionAuthorization, "REJECT_CONTEXT_BINDING" => A::RejectContextBinding, "REJECT_PREDECESSOR_BINDING" => A::RejectPredecessorBinding, "REJECT_PRIVILEGE_EXPANSION" => A::RejectPrivilegeExpansion, _ => panic!("bad auth decision {v}") } }
fn lifecycle(v: &str) -> D { match v { "ACCEPT_SUCCESSOR" => D::AcceptSuccessor, "REJECT_AUTHORITY" => D::RejectAuthority, _ => panic!("bad lifecycle decision {v}") } }
#[test]
fn shared_authorization_corpus_is_enforced() {
    assert!(CORPUS.lines().any(|l| l == "version=1")); let mut count=0usize;
    for line in CORPUS.lines().filter(|l| l.starts_with("case=")) {
        let f: Vec<&str> = line.trim_start_matches("case=").split('|').collect(); assert_eq!(f.len(),10);
        let facts=LineageReplaceAuthorizationFacts { current_credential_control_valid:bit(f[1]), successor_key_control_valid:bit(f[2]), current_session_authenticated:bit(f[3]), current_session_authorized:bit(f[4]), current_context_bound:bit(f[5]), predecessor_binding_current:bit(f[6]), successor_scope_within_authorized_scope:bit(f[7]) };
        let a=classify_lineage_replace_authorization(&facts); assert_eq!(a,auth(f[8]),"{}",f[0]);
        let normalized=LineageReplaceFacts { trigger:LineageReplaceTrigger::Lifecycle, authority_valid:false, predecessor_valid:true, successor_valid:true, context_valid:true, freshness_valid:true, replay_free:true, concurrent_free:true, rollback_clear:true, storage_safe:true, dependent_state_safe:true };
        assert_eq!(evaluate_authorized_lineage_replace(a,&normalized),lifecycle(f[9]),"{}",f[0]); count+=1;
    }
    assert_eq!(count,10);
}
