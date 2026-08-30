use proto::lineage_replace::{LineageReplaceState, LineageReplaceState::*};
use proto::lineage_replace_faults::{facts_at_write_cut, LineageReplaceWriteCut};
use proto::lineage_replace_recovery::recover_lineage_replace;

const CORPUS: &str = include_str!("../../../test-vectors/replay/lineage-replace-write-cuts-v1.txt");

fn cut(name: &str) -> LineageReplaceWriteCut {
    match name {
        "BEFORE_BEGIN" => LineageReplaceWriteCut::BeforeBegin,
        "AFTER_PENDING_MARKER" => LineageReplaceWriteCut::AfterPendingMarker,
        "AFTER_SUCCESSOR_ACTIVATION" => LineageReplaceWriteCut::AfterSuccessorActivation,
        "AFTER_PREDECESSOR_RETIREMENT" => LineageReplaceWriteCut::AfterPredecessorRetirement,
        "AFTER_INVALIDATIONS" => LineageReplaceWriteCut::AfterInvalidations,
        "AFTER_COMMIT_MARKER_CLEAR" => LineageReplaceWriteCut::AfterCommitMarkerClear,
        _ => panic!("unknown write cut: {name}"),
    }
}

fn state(name: &str) -> LineageReplaceState {
    match name {
        "ACTIVE_PREDECESSOR" => ActivePredecessor,
        "ACTIVE_SUCCESSOR_PREDECESSOR_RETIRED" => ActiveSuccessorPredecessorRetired,
        "CONTINUITY_BROKEN" => ContinuityBroken,
        _ => panic!("unknown state: {name}"),
    }
}

#[test]
fn canonical_write_cut_corpus_is_fail_closed() {
    let mut count = 0usize;
    let mut saw_version = false;

    for line in CORPUS.lines() {
        if line == "version=1" {
            saw_version = true;
            continue;
        }
        let Some(body) = line.strip_prefix("case=") else {
            continue;
        };
        let mut fields = body.split('|');
        let name = fields.next().expect("case name");
        let expected = fields.next().expect("expected state");
        assert!(fields.next().is_none(), "malformed write-cut corpus line");
        assert_eq!(recover_lineage_replace(&facts_at_write_cut(cut(name))), state(expected));
        count += 1;
    }

    assert!(saw_version);
    assert_eq!(count, 6);
}
