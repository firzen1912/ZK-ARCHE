use curve25519_dalek::constants::RISTRETTO_BASEPOINT_POINT;
use curve25519_dalek::ristretto::RistrettoPoint;
use curve25519_dalek::scalar::Scalar;
use rand::{Error as RandError, RngCore};

use proto::crypto::{
    attr_h, compute_pid, decode_scalar, decompress_point, derive_session_key, make_role_commitment,
    prove_auth_client_with_rng, prove_rerandomization_with_rng, prove_role_set_membership_with_rng,
    reject_identity, scalar_from_wide_bytes, verify_auth_client, verify_rerandomization,
    verify_role_set_membership, RoleSetBinding, SchnorrProof,
};
use proto::proto::payloads::{Auth1, Auth2, Auth3, Setup1, Setup2, Setup3};
use proto::store::fs::{replay_key, MemoryReplayCache};
use proto::store::ReplayCache;

struct FixedRng {
    bytes: Vec<u8>,
    pos: usize,
}

impl FixedRng {
    fn new() -> Self {
        let bytes = (0..4096).map(|i| (i * 73 + 19) as u8).collect();
        Self { bytes, pos: 0 }
    }
}

impl RngCore for FixedRng {
    fn next_u32(&mut self) -> u32 {
        let mut b = [0u8; 4];
        self.fill_bytes(&mut b);
        u32::from_le_bytes(b)
    }

    fn next_u64(&mut self) -> u64 {
        let mut b = [0u8; 8];
        self.fill_bytes(&mut b);
        u64::from_le_bytes(b)
    }

    fn fill_bytes(&mut self, dest: &mut [u8]) {
        for b in dest {
            *b = self.bytes[self.pos % self.bytes.len()];
            self.pos += 1;
        }
    }

    fn try_fill_bytes(&mut self, dest: &mut [u8]) -> Result<(), RandError> {
        self.fill_bytes(dest);
        Ok(())
    }
}

fn scalar(seed: u8) -> Scalar {
    scalar_from_wide_bytes(&[seed; 64])
}

fn point(seed: u8) -> RistrettoPoint {
    RISTRETTO_BASEPOINT_POINT * scalar(seed)
}

fn proof(seed: u8) -> SchnorrProof {
    SchnorrProof {
        a: point(seed),
        s: scalar(seed.wrapping_add(1)),
    }
}

#[test]
fn rejects_invalid_points_scalars_and_trailing_payload_bytes() {
    assert!(decompress_point(&[0u8; 32], "identity").is_err());
    assert!(reject_identity(&RistrettoPoint::default(), "identity").is_err());
    assert!(decompress_point(&[0xffu8; 32], "invalid point").is_err());
    assert!(decode_scalar(&[0xffu8; 32], "non-canonical scalar").is_err());

    let setup1 = Setup1 {
        pairing_token: Some(b"token".to_vec()),
        device_id: [1u8; 32],
        device_pub: point(2),
        client_nonce: [3u8; 32],
        role_commitment: point(4),
    };
    let mut bytes = setup1.encode();
    bytes.push(0xa5);
    assert!(Setup1::decode(&bytes).is_err());

    let setup2 = Setup2 {
        server_nonce: [5u8; 32],
        setup_challenge: [6u8; 16],
        server_pub: point(7),
        server_proof: proof(8),
    };
    let mut bytes = setup2.encode();
    bytes.push(0xa5);
    assert!(Setup2::decode(&bytes).is_err());

    let setup3 = Setup3 {
        client_proof: proof(10),
    };
    let mut bytes = setup3.encode();
    bytes.push(0xa5);
    assert!(Setup3::decode(&bytes).is_err());

    let auth1 = Auth1 {
        pid: [11u8; 32],
        client_proof: proof(12),
        nonce_c: [13u8; 32],
        eph_c: point(14),
        c_prime: point(15),
        rerand_proof: proof(16),
        branches: vec![(point(17), scalar(18), scalar(19))],
    };
    let mut bytes = auth1.encode();
    bytes.push(0xa5);
    assert!(Auth1::decode(&bytes).is_err());

    let auth2 = Auth2 {
        server_pub: point(20),
        server_proof: proof(21),
        nonce_s: [22u8; 32],
        eph_s: point(23),
        tag_s: [24u8; 32],
    };
    let mut bytes = auth2.encode();
    bytes.push(0xa5);
    assert!(Auth2::decode(&bytes).is_err());

    let auth3 = Auth3 { tag_c: [25u8; 32] };
    let mut bytes = auth3.encode();
    bytes.push(0xa5);
    assert!(Auth3::decode(&bytes).is_err());
}

#[test]
fn auth_proofs_are_bound_to_pid_nonce_ephemeral_key_and_public_key() {
    let x = scalar(31);
    let device_pub = RISTRETTO_BASEPOINT_POINT * x;
    let server_pub = point(32);
    let eph_secret = scalar(33);
    let eph_c = RISTRETTO_BASEPOINT_POINT * eph_secret;
    let nonce_c = [34u8; 32];
    let pid = compute_pid(&device_pub, &nonce_c, &eph_c, &server_pub);
    let proof = prove_auth_client_with_rng(&mut FixedRng::new(), &x, &pid, &nonce_c, &eph_c);

    assert!(verify_auth_client(
        &device_pub,
        &pid,
        &nonce_c,
        &eph_c,
        &proof
    ));

    let mut bad_pid = pid;
    bad_pid[0] ^= 1;
    assert!(!verify_auth_client(
        &device_pub,
        &bad_pid,
        &nonce_c,
        &eph_c,
        &proof
    ));

    let mut bad_nonce = nonce_c;
    bad_nonce[7] ^= 0x80;
    assert!(!verify_auth_client(
        &device_pub,
        &pid,
        &bad_nonce,
        &eph_c,
        &proof
    ));

    assert!(!verify_auth_client(
        &point(35),
        &pid,
        &nonce_c,
        &eph_c,
        &proof
    ));
    assert!(!verify_auth_client(
        &device_pub,
        &pid,
        &nonce_c,
        &point(36),
        &proof
    ));

    let bad_a = SchnorrProof {
        a: proof.a + RISTRETTO_BASEPOINT_POINT,
        s: proof.s,
    };
    assert!(!verify_auth_client(
        &device_pub,
        &pid,
        &nonce_c,
        &eph_c,
        &bad_a
    ));

    let bad_s = SchnorrProof {
        a: proof.a,
        s: proof.s + Scalar::from(1u64),
    };
    assert!(!verify_auth_client(
        &device_pub,
        &pid,
        &nonce_c,
        &eph_c,
        &bad_s
    ));
}

#[test]
fn role_proofs_and_session_keys_are_bound_to_the_auth_transcript() {
    let allowed = [1u64, 2, 3];
    let blind = scalar(41);
    let delta = scalar(42);
    let stored_c = make_role_commitment(&Scalar::from(2u64), &blind);
    let c_prime = stored_c + attr_h() * delta;
    let blind_prime = blind + delta;
    let pid = [43u8; 32];
    let nonce_c = [44u8; 32];
    let eph_c = point(45);

    let rerand = prove_rerandomization_with_rng(
        &mut FixedRng::new(),
        &stored_c,
        &c_prime,
        &delta,
        &pid,
        &nonce_c,
        &eph_c,
    );
    assert!(verify_rerandomization(
        &stored_c, &c_prime, &pid, &nonce_c, &eph_c, &rerand
    ));

    let mut bad_pid = pid;
    bad_pid[0] ^= 1;
    assert!(!verify_rerandomization(
        &stored_c, &c_prime, &bad_pid, &nonce_c, &eph_c, &rerand
    ));

    let binding = RoleSetBinding {
        allowed_roles: &allowed,
        c_prime: &c_prime,
        pid: &pid,
        nonce_c: &nonce_c,
        eph_c: &eph_c,
    };
    let branches =
        prove_role_set_membership_with_rng(&mut FixedRng::new(), &binding, 2, &blind_prime);
    assert!(verify_role_set_membership(&binding, &branches));

    let wrong_roles = [1u64, 3, 4];
    let wrong_binding = RoleSetBinding {
        allowed_roles: &wrong_roles,
        c_prime: &c_prime,
        pid: &pid,
        nonce_c: &nonce_c,
        eph_c: &eph_c,
    };
    assert!(!verify_role_set_membership(&wrong_binding, &branches));
    assert!(!verify_role_set_membership(&binding, &branches[..2]));

    let mut tampered = branches.clone();
    tampered[0].1 += Scalar::from(1u64);
    assert!(!verify_role_set_membership(&binding, &tampered));

    let client_eph = scalar(51);
    let server_eph = scalar(52);
    let ec = RISTRETTO_BASEPOINT_POINT * client_eph;
    let es = RISTRETTO_BASEPOINT_POINT * server_eph;
    let nonce_s = [53u8; 32];
    let key_c = derive_session_key(&client_eph, &es, &nonce_c, &nonce_s, &pid, &ec, &es);
    let key_s = derive_session_key(&server_eph, &ec, &nonce_c, &nonce_s, &pid, &ec, &es);
    assert_eq!(key_c, key_s);

    let mut altered_nonce_s = nonce_s;
    altered_nonce_s[0] ^= 1;
    assert_ne!(
        key_c,
        derive_session_key(&client_eph, &es, &nonce_c, &altered_nonce_s, &pid, &ec, &es)
    );
}

#[test]
fn replay_cache_rejects_duplicate_transcript_keys() {
    let pid = [61u8; 32];
    let nonce_c = [62u8; 32];
    let eph_c = point(63);
    let key = replay_key(&pid, &nonce_c, &eph_c);

    let mut cache = MemoryReplayCache::new(16);
    assert!(!cache.contains(&key));
    assert!(cache.insert(key));
    assert!(cache.contains(&key));
    assert!(!cache.insert(key));
}
