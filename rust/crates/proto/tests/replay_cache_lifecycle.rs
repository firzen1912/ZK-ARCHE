use proto::store::fs::MemoryReplayCache;
use proto::store::ReplayCache;
use proto::Profile;

const SHARED_FIFO_CORPUS: &str =
    include_str!("../../../test-vectors/replay-cache/fifo-capacity-64.txt");

fn replay_key_fixture(i: usize) -> [u8; 32] {
    let mut key = [0u8; 32];
    let lo = (i as u64).to_le_bytes();
    let hi = (!(i as u64)).to_le_bytes();
    key[0..8].copy_from_slice(&lo);
    key[8..16].copy_from_slice(&hi);
    key[16..24].copy_from_slice(&lo);
    key[24..32].copy_from_slice(&hi);
    key
}

fn shared_corpus_key_fixture(i: usize) -> [u8; 32] {
    let mut key = [0u8; 32];
    key[0..4].copy_from_slice(&(i as u32).to_le_bytes());
    key
}

fn profile_cases() -> [(&'static str, Profile); 3] {
    [
        ("minimal", Profile::minimal()),
        ("standard", Profile::standard()),
        ("gateway", Profile::gateway()),
    ]
}

fn replay_capacity(profile: Profile) -> usize {
    // Mirrors the reference server policy in crates/server/src/main.rs.
    profile.max_cached_responses.saturating_mul(2)
}

#[test]
fn replay_capacity_contract_matches_current_server_profiles() {
    let cases = profile_cases();

    assert_eq!(
        replay_capacity(cases[0].1),
        32,
        "minimal replay capacity drift"
    );
    assert_eq!(
        replay_capacity(cases[1].1),
        4096,
        "standard replay capacity drift"
    );
    assert_eq!(
        replay_capacity(cases[2].1),
        32768,
        "gateway replay capacity drift"
    );
}

#[test]
fn duplicate_is_rejected_before_cache_pressure() {
    for (name, profile) in profile_cases() {
        let cap = replay_capacity(profile);
        let mut cache = MemoryReplayCache::new(cap);
        let key = replay_key_fixture(7);

        assert!(cache.insert(key), "{name}: first insert must be accepted");
        assert!(
            cache.contains(&key),
            "{name}: accepted key must be retained"
        );
        assert!(
            !cache.insert(key),
            "{name}: duplicate insert must be rejected while retained"
        );
    }
}

#[test]
fn fifo_cache_pressure_evicts_oldest_retained_key_per_overflow() {
    for (name, profile) in profile_cases() {
        let cap = replay_capacity(profile);
        assert!(cap > 0, "{name}: replay capacity must be non-zero");

        let mut cache = MemoryReplayCache::new(cap);
        let originals: Vec<[u8; 32]> = (0..cap).map(replay_key_fixture).collect();

        for key in &originals {
            assert!(cache.insert(*key), "{name}: initial fill key must be new");
        }

        let overflow = replay_key_fixture(cap);
        assert!(
            cache.insert(overflow),
            "{name}: overflow key must be accepted"
        );
        assert!(
            cache.contains(&overflow),
            "{name}: overflow key must remain in replay memory"
        );
        assert!(
            !cache.contains(&originals[0]),
            "{name}: FIFO pressure must evict the oldest retained replay key"
        );
        assert!(
            originals[1..].iter().all(|key| cache.contains(key)),
            "{name}: one overflow must retain every non-oldest original key"
        );

        assert!(
            cache.insert(originals[0]),
            "{name}: an evicted accepted key is eligible for insertion again"
        );
        assert!(
            cache.contains(&originals[0]),
            "{name}: reinserted evicted key must be retained"
        );
        assert!(
            !cache.contains(&originals[1]),
            "{name}: reinserting the evicted key must evict the next-oldest key"
        );
    }
}

#[test]
fn process_restart_loses_volatile_replay_memory_for_each_profile() {
    for (name, profile) in profile_cases() {
        let cap = replay_capacity(profile);
        let accepted = replay_key_fixture(11);

        let mut before_restart = MemoryReplayCache::new(cap);
        assert!(
            before_restart.insert(accepted),
            "{name}: first insert before restart must be accepted"
        );
        assert!(
            before_restart.contains(&accepted),
            "{name}: accepted key must exist before restart"
        );

        // A newly constructed cache models the current process/server restart
        // behavior: replay state is volatile and no persisted epoch is loaded.
        let mut after_restart = MemoryReplayCache::new(cap);
        assert!(
            !after_restart.contains(&accepted),
            "{name}: restart currently loses replay memory"
        );
        assert!(
            after_restart.insert(accepted),
            "{name}: the same replay key is treated as new after restart"
        );
    }
}

#[test]
fn shared_fifo_capacity_64_corpus_matches_rust_replay_cache() {
    let mut cache: Option<MemoryReplayCache> = None;
    let mut declared_capacity = None;

    for (line_no, raw) in SHARED_FIFO_CORPUS.lines().enumerate() {
        let line = raw.trim();
        if line.is_empty() || line.starts_with('#') {
            continue;
        }

        if let Some(value) = line.strip_prefix("capacity=") {
            let cap = value
                .parse::<usize>()
                .unwrap_or_else(|e| panic!("line {}: invalid capacity: {e}", line_no + 1));
            assert_eq!(cap, 64, "shared corpus capacity changed unexpectedly");
            declared_capacity = Some(cap);
            cache = Some(MemoryReplayCache::new(cap));
            continue;
        }

        let parts: Vec<&str> = line.split_whitespace().collect();
        assert_eq!(
            parts.len(),
            3,
            "line {}: expected '<op> <index> <expectation>'",
            line_no + 1
        );
        let index = parts[1]
            .parse::<usize>()
            .unwrap_or_else(|e| panic!("line {}: invalid key index: {e}", line_no + 1));
        let key = shared_corpus_key_fixture(index);
        let cache = cache
            .as_mut()
            .unwrap_or_else(|| panic!("line {}: capacity must be declared first", line_no + 1));

        match (parts[0], parts[2]) {
            ("insert", "new") => assert!(
                cache.insert(key),
                "line {}: key {index} should be accepted as new",
                line_no + 1
            ),
            ("insert", "duplicate") => assert!(
                !cache.insert(key),
                "line {}: key {index} should be rejected as duplicate",
                line_no + 1
            ),
            ("contains", "present") => assert!(
                cache.contains(&key),
                "line {}: key {index} should be retained",
                line_no + 1
            ),
            ("contains", "absent") => assert!(
                !cache.contains(&key),
                "line {}: key {index} should be absent",
                line_no + 1
            ),
            (op, expected) => panic!(
                "line {}: unsupported corpus operation/expectation: {op} {expected}",
                line_no + 1
            ),
        }
    }

    assert_eq!(
        declared_capacity,
        Some(64),
        "shared replay corpus must declare capacity 64"
    );
}
