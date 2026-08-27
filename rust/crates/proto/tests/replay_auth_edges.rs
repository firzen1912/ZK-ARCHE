use std::sync::{Arc, Barrier, Mutex};
use std::thread;

use curve25519_dalek::constants::RISTRETTO_BASEPOINT_POINT;
use curve25519_dalek::ristretto::RistrettoPoint;
use curve25519_dalek::scalar::Scalar;

use proto::crypto::{
    compute_pid, encode_role, make_role_commitment, prove_auth_client, prove_rerandomization,
    prove_role_set_membership, rerandomize_commitment, RoleSetBinding,
};
use proto::error::ErrorCode;
use proto::proto::auth::{handle_auth_1, PendingAuth};
use proto::proto::payloads::Auth1;
use proto::store::fs::MemoryReplayCache;
use proto::store::{DeviceRecord, RegistryStore, ServerKeyStore};
use proto::{Result, DEFAULT_ALLOWED_ROLES};

#[derive(Clone)]
struct FixedServerKeyStore {
    sk: Scalar,
}

impl ServerKeyStore for FixedServerKeyStore {
    fn load_or_create_server_sk(&mut self) -> Result<Scalar> {
        Ok(self.sk)
    }
}

#[derive(Clone)]
struct SingleRegistry {
    device_id: [u8; 32],
    record: DeviceRecord,
}

impl RegistryStore for SingleRegistry {
    fn lookup_by_device_id(&self, device_id: &[u8; 32]) -> Result<Option<DeviceRecord>> {
        Ok(if device_id == &self.device_id {
            Some(self.record)
        } else {
            None
        })
    }

    fn iter(&self) -> Box<dyn Iterator<Item = ([u8; 32], DeviceRecord)> + '_> {
        Box::new(std::iter::once((self.device_id, self.record)))
    }

    fn save(&mut self, device_id: [u8; 32], record: DeviceRecord) -> Result<()> {
        self.device_id = device_id;
        self.record = record;
        Ok(())
    }
}

struct SharedAuthState {
    key_store: FixedServerKeyStore,
    registry: SingleRegistry,
    replay: MemoryReplayCache,
}

impl SharedAuthState {
    fn accept(&mut self, session_id: [u8; 16], seq: u32, payload: &[u8]) -> Result<PendingAuth> {
        let Self {
            key_store,
            registry,
            replay,
        } = self;
        handle_auth_1(
            key_store,
            registry,
            replay,
            DEFAULT_ALLOWED_ROLES,
            session_id,
            seq,
            payload,
        )
    }
}

fn valid_auth_fixture() -> (SharedAuthState, Vec<u8>) {
    let server_sk = Scalar::from(19u64);
    let server_pub = RISTRETTO_BASEPOINT_POINT * server_sk;

    let client_sk = Scalar::from(11u64);
    let device_pub = RISTRETTO_BASEPOINT_POINT * client_sk;
    let device_id = [41u8; 32];

    let role_code = 1u64;
    let role_blind = Scalar::from(31u64);
    let stored_commitment = make_role_commitment(&encode_role(role_code), &role_blind);

    let nonce_c = [23u8; 32];
    let eph_secret = Scalar::from(29u64);
    let eph_c: RistrettoPoint = RISTRETTO_BASEPOINT_POINT * eph_secret;
    let pid = compute_pid(&device_pub, &nonce_c, &eph_c, &server_pub);

    let client_proof = prove_auth_client(&client_sk, &pid, &nonce_c, &eph_c);
    let (c_prime, blind_prime, delta) = rerandomize_commitment(&stored_commitment, &role_blind);
    let rerand_proof =
        prove_rerandomization(&stored_commitment, &c_prime, &delta, &pid, &nonce_c, &eph_c);
    let branches = prove_role_set_membership(
        &RoleSetBinding {
            allowed_roles: DEFAULT_ALLOWED_ROLES,
            c_prime: &c_prime,
            pid: &pid,
            nonce_c: &nonce_c,
            eph_c: &eph_c,
        },
        role_code,
        &blind_prime,
    );

    let payload = Auth1 {
        pid,
        client_proof,
        nonce_c,
        eph_c,
        c_prime,
        rerand_proof,
        branches,
    }
    .encode();

    let state = SharedAuthState {
        key_store: FixedServerKeyStore { sk: server_sk },
        registry: SingleRegistry {
            device_id,
            record: DeviceRecord {
                pubkey: device_pub,
                role_commitment: stored_commitment,
            },
        },
        replay: MemoryReplayCache::new(64),
    };

    (state, payload)
}

fn assert_replay_detected(result: Result<PendingAuth>) {
    match result {
        Err(err) => assert_eq!(err.wire_code(), Some(ErrorCode::ReplayDetected)),
        Ok(_) => panic!("replayed AUTH_1 was accepted"),
    }
}

#[test]
fn ft022_rejects_same_auth_payload_under_fresh_outer_session() {
    let (mut state, payload) = valid_auth_fixture();

    let first = state
        .accept([0x11u8; 16], 0, &payload)
        .expect("first valid AUTH_1 must be accepted");
    assert_eq!(first.session_id, [0x11u8; 16]);

    // The replay key intentionally binds the authenticated AUTH_1 payload
    // fields, not the unauthenticated outer session_id/sequence. A fresh outer
    // header therefore must not turn an already accepted AUTH_1 into a new run.
    assert_replay_detected(state.accept([0x22u8; 16], 7, &payload));
}

#[test]
fn ft023_concurrent_duplicate_allows_exactly_one_new_acceptance() {
    let (state, payload) = valid_auth_fixture();
    let state = Arc::new(Mutex::new(state));
    let payload = Arc::new(payload);
    let start = Arc::new(Barrier::new(3));

    let mut workers = Vec::new();
    for marker in [0x31u8, 0x32u8] {
        let state = Arc::clone(&state);
        let payload = Arc::clone(&payload);
        let start = Arc::clone(&start);
        workers.push(thread::spawn(move || {
            start.wait();
            let mut guard = state.lock().expect("shared AUTH state mutex poisoned");
            match guard.accept([marker; 16], 0, payload.as_slice()) {
                Ok(_) => Ok(()),
                Err(err) => Err(err.wire_code()),
            }
        }));
    }

    start.wait();

    let results: Vec<_> = workers
        .into_iter()
        .map(|worker| worker.join().expect("AUTH worker panicked"))
        .collect();

    let accepted = results.iter().filter(|result| result.is_ok()).count();
    let replayed = results
        .iter()
        .filter(|result| matches!(result, Err(Some(ErrorCode::ReplayDetected))))
        .count();

    assert_eq!(
        accepted, 1,
        "exactly one duplicate AUTH_1 may create new accepted state"
    );
    assert_eq!(
        replayed, 1,
        "the other concurrent duplicate must be rejected as replay"
    );
}
