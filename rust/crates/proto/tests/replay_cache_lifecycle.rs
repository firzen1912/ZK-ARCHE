use proto::store::fs::MemoryReplayCache;
use proto::store::ReplayCache;
use proto::Profile;

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

    assert_eq!(replay_capacity(cases[0].1), 32, "minimal replay capacity drift");
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
        assert!(cache.contains(&key), "{name}: accepted key must be retained");
        assert!(
            !cache.insert(key),
            "{name}: duplicate insert must be rejected while retained"
        );
    }
}

#[test]
fn cache_pressure_evicts_exactly_one_retained_key_per_overflow() {
    for (name, profile) in profile_cases() {
        let cap = replay_capacity(profile);
        assert!(cap > 0, "{name}: replay capacity must be non-zero");

        let mut cache = MemoryReplayCache::new(cap);
        let originals: Vec<[u8; 32]> = (0..cap).map(replay_key_fixture).collect();

        for key in &originals {
            assert!(cache.insert(*key), "{name}: initial fill key must be new");
        }

        let overflow = replay_key_fixture(cap);
        assert!(cache.insert(overflow), "{name}: overflow key must be accepted");
        assert!(
            cache.contains(&overflow),
            "{name}: overflow key must remain in replay memory"
        );

        let missing: Vec<[u8; 32]> = originals
            .iter()
            .copied()
            .filter(|key| !cache.contains(key))
            .collect();

        assert_eq!(
            missing.len(),
            1,
            "{name}: current HashSet policy should evict exactly one retained key on one overflow"
        );

        // The implementation intentionally chooses an arbitrary HashSet entry,
        // so the identity of the evicted key is not asserted. What matters for
        // replay semantics is that an accepted key can leave the bounded cache
        // under pressure and is then treated as new by the cache primitive.
        assert!(
            cache.insert(missing[0]),
            "{name}: an evicted accepted key is eligible for insertion again"
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
