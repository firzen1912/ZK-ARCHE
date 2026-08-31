use proto::lineage_replace_possession::{classify_lineage_replace_possession, LineageReplacePossessionDecision, VerifiedLifecyclePossessionProof};

fn bit(v: &str) -> bool { match v { "1" => true, "0" => false, _ => panic!("invalid bit") } }
fn expected(v: &str) -> LineageReplacePossessionDecision { match v { "VERIFIED" => LineageReplacePossessionDecision::Verified, "REJECT_CURRENT_CREDENTIAL_CONTROL" => LineageReplacePossessionDecision::RejectCurrentCredentialControl, "REJECT_SUCCESSOR_KEY_CONTROL" => LineageReplacePossessionDecision::RejectSuccessorKeyControl, _ => panic!("invalid decision") } }

#[test]
fn canonical_lineage_replace_possession_corpus() {
    let corpus = include_str!("../../../test-vectors/replay/lineage-replace-possession-v1.txt");
    let session = [0x51u8; 16]; let predecessor = [0x33u8; 32]; let successor_ref = [0x66u8; 32]; let mut cases = 0usize;
    for line in corpus.lines() {
        if !line.starts_with("case=") { continue; }
        let f: Vec<&str> = line[5..].split('|').collect(); assert_eq!(f.len(), 10);
        let mut current = VerifiedLifecyclePossessionProof { verification_valid: bit(f[2]), session_id: session, subject_reference: predecessor };
        let mut successor = VerifiedLifecyclePossessionProof { verification_valid: bit(f[6]), session_id: session, subject_reference: successor_ref };
        if !bit(f[3]) { current.session_id[0] ^= 1; } if !bit(f[4]) { current.subject_reference[0] ^= 1; }
        if !bit(f[7]) { successor.session_id[0] ^= 1; } if !bit(f[8]) { successor.subject_reference[0] ^= 1; }
        assert_eq!(classify_lineage_replace_possession(if bit(f[1]) { Some(&current) } else { None }, if bit(f[5]) { Some(&successor) } else { None }, &session, &predecessor, &successor_ref), expected(f[9]));
        cases += 1;
    }
    assert_eq!(cases, 10);
}
