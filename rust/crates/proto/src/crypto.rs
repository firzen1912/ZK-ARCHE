//! Cryptographic core. Pure functions — no I/O, no transport, no storage.
//! Anything in this module can be cross-validated against the test vectors
//! published under `test-vectors/`.

use curve25519_dalek::constants::RISTRETTO_BASEPOINT_POINT;
use curve25519_dalek::ristretto::{CompressedRistretto, RistrettoPoint};
use curve25519_dalek::scalar::Scalar;
use hkdf::Hkdf;
use hmac::{Hmac, Mac};
use rand::{rngs::OsRng, RngCore};
use sha2::{Digest, Sha256, Sha512};

use blake2::digest::{Update as _, VariableOutput};
use blake2::{Blake2b512, Blake2bVar};

use crate::error::{ErrorCode, ProtoError, Result};
use crate::transcript::{self as tr, TVal, Transcript};

type HmacSha256 = Hmac<Sha256>;

pub const NONCE_LEN: usize = 32;
pub const SETUP_CHALLENGE_LEN: usize = 16;

// ---- RNG and encoding helpers ----

pub fn random_scalar() -> Scalar {
    let mut bytes = [0u8; 64];
    OsRng.fill_bytes(&mut bytes);
    Scalar::from_bytes_mod_order_wide(&bytes)
}

/// Deterministic scalar sampler: same 64 uniform bytes -> same scalar.
/// The reduction is `Scalar::from_bytes_mod_order_wide`, so any
/// implementation that feeds the same 64 bytes obtains the same scalar.
pub fn scalar_from_wide_bytes(bytes: &[u8; 64]) -> Scalar {
    Scalar::from_bytes_mod_order_wide(bytes)
}

/// Sample a scalar from a caller-provided RNG. Used by the test-vector
/// harness to produce byte-reproducible proofs; production code paths stay
/// on `OsRng` via `random_scalar`.
pub fn random_scalar_with_rng<R: RngCore>(rng: &mut R) -> Scalar {
    let mut bytes = [0u8; 64];
    rng.fill_bytes(&mut bytes);
    Scalar::from_bytes_mod_order_wide(&bytes)
}

pub fn random_bytes_32() -> [u8; 32] {
    let mut b = [0u8; 32];
    OsRng.fill_bytes(&mut b);
    b
}

pub fn hash_to_point(label: &[u8]) -> RistrettoPoint {
    let mut h = Sha512::new();
    Digest::update(&mut h, b"ristretto-hash-to-point-v1");
    Digest::update(&mut h, label);
    let digest = h.finalize();
    let mut wide = [0u8; 64];
    wide.copy_from_slice(&digest);
    RistrettoPoint::from_uniform_bytes(&wide)
}

/// Blinding generator for the role commitment.
pub fn attr_h() -> RistrettoPoint {
    hash_to_point(b"iot-auth/attr-h/v1")
}

/// Reject the identity point. Callers MUST run this on every point received
/// from the wire or loaded from storage.
pub fn reject_identity(p: &RistrettoPoint, what: &'static str) -> Result<()> {
    if *p == RistrettoPoint::default() {
        return Err(ProtoError::wire(
            ErrorCode::IdentityPoint,
            format!("{what} is the identity point"),
        ));
    }
    Ok(())
}

pub fn decompress_point(bytes: &[u8; 32], what: &'static str) -> Result<RistrettoPoint> {
    let p = CompressedRistretto(*bytes).decompress().ok_or_else(|| {
        ProtoError::wire(ErrorCode::InvalidPoint, format!("invalid point: {what}"))
    })?;
    reject_identity(&p, what)?;
    Ok(p)
}

pub fn decode_scalar(bytes: &[u8; 32], what: &'static str) -> Result<Scalar> {
    Option::from(Scalar::from_canonical_bytes(*bytes)).ok_or_else(|| {
        ProtoError::wire(
            ErrorCode::NonCanonicalScalar,
            format!("non-canonical scalar: {what}"),
        )
    })
}

// ---- Role commitment / role scalar ----

pub fn encode_role(role_code: u64) -> Scalar {
    Scalar::from(role_code)
}

pub fn make_role_commitment(role_scalar: &Scalar, blind: &Scalar) -> RistrettoPoint {
    RISTRETTO_BASEPOINT_POINT * role_scalar + attr_h() * blind
}

// ---- PID ----

pub use crate::transcript::compute_pid;

// ---- Setup Schnorr (client enrollment) ----

#[derive(Clone, Copy, Debug)]
pub struct SchnorrProof {
    pub a: RistrettoPoint,
    pub s: Scalar,
}

/// Shared binding fields for the setup (enrollment) Schnorr proofs. Groups the
/// six values that the client and server proofs both commit to, so the prove/
/// verify signatures stay within clippy's argument limit without changing the
/// transcript bytes.
#[derive(Clone, Copy)]
pub struct SetupBinding<'a> {
    pub device_id: &'a [u8; 32],
    pub device_pub: &'a RistrettoPoint,
    pub server_pub: &'a RistrettoPoint,
    pub client_nonce: &'a [u8; 32],
    pub server_nonce: &'a [u8; 32],
    pub setup_challenge: &'a [u8; SETUP_CHALLENGE_LEN],
}

pub fn prove_setup_client(x: &Scalar, binding: &SetupBinding) -> SchnorrProof {
    let mut rng = OsRng;
    prove_setup_client_with_rng(&mut rng, x, binding)
}

pub fn prove_setup_client_with_rng<R: RngCore>(
    rng: &mut R,
    x: &Scalar,
    binding: &SetupBinding,
) -> SchnorrProof {
    let SetupBinding {
        device_id,
        device_pub,
        server_pub,
        client_nonce,
        server_nonce,
        setup_challenge,
    } = *binding;
    let r = random_scalar_with_rng(rng);
    let a = RISTRETTO_BASEPOINT_POINT * r;
    let c = tr::challenge(
        tr::T_SETUP,
        &[
            (b"role", TVal::Bytes(b"client")),
            (b"device_id", TVal::Bytes(device_id)),
            (b"device_pub", TVal::Point(device_pub)),
            (b"server_pub", TVal::Point(server_pub)),
            (b"a", TVal::Point(&a)),
            (b"client_nonce", TVal::Bytes(client_nonce)),
            (b"server_nonce", TVal::Bytes(server_nonce)),
            (b"setup_challenge", TVal::Bytes(setup_challenge)),
        ],
    );
    SchnorrProof { a, s: r + c * x }
}

pub fn verify_setup_client(binding: &SetupBinding, proof: &SchnorrProof) -> bool {
    let SetupBinding {
        device_id,
        device_pub,
        server_pub,
        client_nonce,
        server_nonce,
        setup_challenge,
    } = *binding;
    let c = tr::challenge(
        tr::T_SETUP,
        &[
            (b"role", TVal::Bytes(b"client")),
            (b"device_id", TVal::Bytes(device_id)),
            (b"device_pub", TVal::Point(device_pub)),
            (b"server_pub", TVal::Point(server_pub)),
            (b"a", TVal::Point(&proof.a)),
            (b"client_nonce", TVal::Bytes(client_nonce)),
            (b"server_nonce", TVal::Bytes(server_nonce)),
            (b"setup_challenge", TVal::Bytes(setup_challenge)),
        ],
    );
    RISTRETTO_BASEPOINT_POINT * proof.s == proof.a + device_pub * c
}

pub fn prove_setup_server(server_sk: &Scalar, binding: &SetupBinding) -> SchnorrProof {
    let mut rng = OsRng;
    prove_setup_server_with_rng(&mut rng, server_sk, binding)
}

pub fn prove_setup_server_with_rng<R: RngCore>(
    rng: &mut R,
    server_sk: &Scalar,
    binding: &SetupBinding,
) -> SchnorrProof {
    let SetupBinding {
        device_id,
        device_pub,
        server_pub,
        client_nonce,
        server_nonce,
        setup_challenge,
    } = *binding;
    let r = random_scalar_with_rng(rng);
    let a = RISTRETTO_BASEPOINT_POINT * r;
    let c = tr::challenge(
        tr::T_SETUP_SERVER,
        &[
            (b"role", TVal::Bytes(b"server")),
            (b"device_id", TVal::Bytes(device_id)),
            (b"device_pub", TVal::Point(device_pub)),
            (b"server_pub", TVal::Point(server_pub)),
            (b"a", TVal::Point(&a)),
            (b"client_nonce", TVal::Bytes(client_nonce)),
            (b"server_nonce", TVal::Bytes(server_nonce)),
            (b"setup_challenge", TVal::Bytes(setup_challenge)),
        ],
    );
    SchnorrProof {
        a,
        s: r + c * server_sk,
    }
}

pub fn verify_setup_server(binding: &SetupBinding, proof: &SchnorrProof) -> bool {
    let SetupBinding {
        device_id,
        device_pub,
        server_pub,
        client_nonce,
        server_nonce,
        setup_challenge,
    } = *binding;
    let c = tr::challenge(
        tr::T_SETUP_SERVER,
        &[
            (b"role", TVal::Bytes(b"server")),
            (b"device_id", TVal::Bytes(device_id)),
            (b"device_pub", TVal::Point(device_pub)),
            (b"server_pub", TVal::Point(server_pub)),
            (b"a", TVal::Point(&proof.a)),
            (b"client_nonce", TVal::Bytes(client_nonce)),
            (b"server_nonce", TVal::Bytes(server_nonce)),
            (b"setup_challenge", TVal::Bytes(setup_challenge)),
        ],
    );
    RISTRETTO_BASEPOINT_POINT * proof.s == proof.a + server_pub * c
}

// ---- Auth v2 Schnorr (session-bound, transcripts bind to pid) ----

pub fn prove_auth_client(
    x: &Scalar,
    pid: &[u8; 32],
    nonce_c: &[u8; 32],
    eph_c: &RistrettoPoint,
) -> SchnorrProof {
    let mut rng = OsRng;
    prove_auth_client_with_rng(&mut rng, x, pid, nonce_c, eph_c)
}

pub fn prove_auth_client_with_rng<R: RngCore>(
    rng: &mut R,
    x: &Scalar,
    pid: &[u8; 32],
    nonce_c: &[u8; 32],
    eph_c: &RistrettoPoint,
) -> SchnorrProof {
    let pubkey = RISTRETTO_BASEPOINT_POINT * x;
    let r = random_scalar_with_rng(rng);
    let a = RISTRETTO_BASEPOINT_POINT * r;
    let c = tr::challenge(
        tr::T_CLIENT_V2,
        &[
            (b"pid", TVal::Bytes(pid)),
            (b"pubkey", TVal::Point(&pubkey)),
            (b"a", TVal::Point(&a)),
            (b"nonce_c", TVal::Bytes(nonce_c)),
            (b"eph_c", TVal::Point(eph_c)),
        ],
    );
    SchnorrProof { a, s: r + c * x }
}

pub fn verify_auth_client(
    device_pub: &RistrettoPoint,
    pid: &[u8; 32],
    nonce_c: &[u8; 32],
    eph_c: &RistrettoPoint,
    proof: &SchnorrProof,
) -> bool {
    let c = tr::challenge(
        tr::T_CLIENT_V2,
        &[
            (b"pid", TVal::Bytes(pid)),
            (b"pubkey", TVal::Point(device_pub)),
            (b"a", TVal::Point(&proof.a)),
            (b"nonce_c", TVal::Bytes(nonce_c)),
            (b"eph_c", TVal::Point(eph_c)),
        ],
    );
    RISTRETTO_BASEPOINT_POINT * proof.s == proof.a + device_pub * c
}

pub fn prove_auth_server(
    server_sk: &Scalar,
    nonce_s: &[u8; 32],
    eph_s: &RistrettoPoint,
) -> SchnorrProof {
    let mut rng = OsRng;
    prove_auth_server_with_rng(&mut rng, server_sk, nonce_s, eph_s)
}

pub fn prove_auth_server_with_rng<R: RngCore>(
    rng: &mut R,
    server_sk: &Scalar,
    nonce_s: &[u8; 32],
    eph_s: &RistrettoPoint,
) -> SchnorrProof {
    let r = random_scalar_with_rng(rng);
    let a = RISTRETTO_BASEPOINT_POINT * r;
    let server_pub = RISTRETTO_BASEPOINT_POINT * server_sk;
    let c = tr::challenge(
        tr::T_SERVER,
        &[
            (b"pubkey", TVal::Point(&server_pub)),
            (b"a", TVal::Point(&a)),
            (b"nonce_s", TVal::Bytes(nonce_s)),
            (b"eph_s", TVal::Point(eph_s)),
        ],
    );
    SchnorrProof {
        a,
        s: r + c * server_sk,
    }
}

pub fn verify_auth_server(
    server_pub: &RistrettoPoint,
    nonce_s: &[u8; 32],
    eph_s: &RistrettoPoint,
    proof: &SchnorrProof,
) -> bool {
    let c = tr::challenge(
        tr::T_SERVER,
        &[
            (b"pubkey", TVal::Point(server_pub)),
            (b"a", TVal::Point(&proof.a)),
            (b"nonce_s", TVal::Bytes(nonce_s)),
            (b"eph_s", TVal::Point(eph_s)),
        ],
    );
    RISTRETTO_BASEPOINT_POINT * proof.s == proof.a + server_pub * c
}

// ---- Role re-randomization proof ----

pub type RerandProof = SchnorrProof;

/// Given stored `C = g^role + h^blind`, picks fresh `delta` and returns
/// `C' = C + h*delta`, the updated blinding scalar, and `delta`.
pub fn rerandomize_commitment(
    stored_c: &RistrettoPoint,
    stored_blind: &Scalar,
) -> (RistrettoPoint, Scalar, Scalar) {
    let delta = random_scalar();
    let c_prime = stored_c + attr_h() * delta;
    let blind_prime = stored_blind + delta;
    (c_prime, blind_prime, delta)
}

pub fn prove_rerandomization(
    stored_c: &RistrettoPoint,
    c_prime: &RistrettoPoint,
    delta: &Scalar,
    pid: &[u8; 32],
    nonce_c: &[u8; 32],
    eph_c: &RistrettoPoint,
) -> RerandProof {
    let mut rng = OsRng;
    prove_rerandomization_with_rng(&mut rng, stored_c, c_prime, delta, pid, nonce_c, eph_c)
}

pub fn prove_rerandomization_with_rng<R: RngCore>(
    rng: &mut R,
    stored_c: &RistrettoPoint,
    c_prime: &RistrettoPoint,
    delta: &Scalar,
    pid: &[u8; 32],
    nonce_c: &[u8; 32],
    eph_c: &RistrettoPoint,
) -> RerandProof {
    let h = attr_h();
    let r = random_scalar_with_rng(rng);
    let a = h * r;
    let c = tr::challenge(
        tr::T_ROLE_RERAND,
        &[
            (b"pid", TVal::Bytes(pid)),
            (b"nonce_c", TVal::Bytes(nonce_c)),
            (b"eph_c", TVal::Point(eph_c)),
            (b"stored_c", TVal::Point(stored_c)),
            (b"c_prime", TVal::Point(c_prime)),
            (b"a", TVal::Point(&a)),
        ],
    );
    SchnorrProof {
        a,
        s: r + c * delta,
    }
}

pub fn verify_rerandomization(
    stored_c: &RistrettoPoint,
    c_prime: &RistrettoPoint,
    pid: &[u8; 32],
    nonce_c: &[u8; 32],
    eph_c: &RistrettoPoint,
    proof: &RerandProof,
) -> bool {
    let h = attr_h();
    let c = tr::challenge(
        tr::T_ROLE_RERAND,
        &[
            (b"pid", TVal::Bytes(pid)),
            (b"nonce_c", TVal::Bytes(nonce_c)),
            (b"eph_c", TVal::Point(eph_c)),
            (b"stored_c", TVal::Point(stored_c)),
            (b"c_prime", TVal::Point(c_prime)),
            (b"a", TVal::Point(&proof.a)),
        ],
    );
    h * proof.s == proof.a + (c_prime - stored_c) * c
}

// ---- Role set-membership proof (CDS OR-composition) ----

pub type SetBranch = (RistrettoPoint, Scalar, Scalar);

/// Shared public binding for the role set-membership proof. The witness inputs
/// (`role_code`, `blind_prime`) stay separate prove-only arguments.
#[derive(Clone, Copy)]
pub struct RoleSetBinding<'a> {
    pub allowed_roles: &'a [u64],
    pub c_prime: &'a RistrettoPoint,
    pub pid: &'a [u8; 32],
    pub nonce_c: &'a [u8; 32],
    pub eph_c: &'a RistrettoPoint,
}

pub fn prove_role_set_membership(
    binding: &RoleSetBinding,
    role_code: u64,
    blind_prime: &Scalar,
) -> Vec<SetBranch> {
    let mut rng = OsRng;
    prove_role_set_membership_with_rng(&mut rng, binding, role_code, blind_prime)
}

pub fn prove_role_set_membership_with_rng<R: RngCore>(
    rng: &mut R,
    binding: &RoleSetBinding,
    role_code: u64,
    blind_prime: &Scalar,
) -> Vec<SetBranch> {
    let RoleSetBinding {
        allowed_roles,
        c_prime,
        pid,
        nonce_c,
        eph_c,
    } = *binding;
    let h = attr_h();
    let n = allowed_roles.len();
    let true_index = allowed_roles
        .iter()
        .position(|r| *r == role_code)
        .expect("role not in allowed_roles");

    let y_points: Vec<RistrettoPoint> = allowed_roles
        .iter()
        .map(|r| c_prime - RISTRETTO_BASEPOINT_POINT * Scalar::from(*r))
        .collect();

    let mut a_points = vec![RistrettoPoint::default(); n];
    let mut c_vals = vec![Scalar::from(0u64); n];
    let mut s_vals = vec![Scalar::from(0u64); n];
    let mut w_true = Scalar::from(0u64);

    for i in 0..n {
        if i == true_index {
            let w = random_scalar_with_rng(rng);
            a_points[i] = h * w;
            w_true = w;
        } else {
            let c_i = random_scalar_with_rng(rng);
            let s_i = random_scalar_with_rng(rng);
            a_points[i] = h * s_i - y_points[i] * c_i;
            c_vals[i] = c_i;
            s_vals[i] = s_i;
        }
    }

    let mut t = Transcript::new(tr::T_ROLE_SET);
    t.append_message(b"pid", pid);
    t.append_message(b"nonce_c", nonce_c);
    t.append_point(b"eph_c", eph_c);
    t.append_point(b"c_prime", c_prime);
    for (i, r) in allowed_roles.iter().enumerate() {
        let label = format!("r_{i}");
        t.append_message(label.as_bytes(), &r.to_le_bytes());
    }
    for (i, a) in a_points.iter().enumerate() {
        let label = format!("A_{i}");
        t.append_point(label.as_bytes(), a);
    }
    let master_c = t.challenge_scalar();

    let sum_sim: Scalar = c_vals
        .iter()
        .enumerate()
        .filter(|(i, _)| *i != true_index)
        .map(|(_, c)| *c)
        .sum();
    let c_true = master_c - sum_sim;
    let s_true = w_true + c_true * blind_prime;

    c_vals[true_index] = c_true;
    s_vals[true_index] = s_true;

    (0..n)
        .map(|i| (a_points[i], c_vals[i], s_vals[i]))
        .collect()
}

pub fn verify_role_set_membership(binding: &RoleSetBinding, branches: &[SetBranch]) -> bool {
    let RoleSetBinding {
        allowed_roles,
        c_prime,
        pid,
        nonce_c,
        eph_c,
    } = *binding;
    let h = attr_h();
    let n = allowed_roles.len();
    if branches.len() != n {
        return false;
    }

    let y_points: Vec<RistrettoPoint> = allowed_roles
        .iter()
        .map(|r| c_prime - RISTRETTO_BASEPOINT_POINT * Scalar::from(*r))
        .collect();

    let mut t = Transcript::new(tr::T_ROLE_SET);
    t.append_message(b"pid", pid);
    t.append_message(b"nonce_c", nonce_c);
    t.append_point(b"eph_c", eph_c);
    t.append_point(b"c_prime", c_prime);
    for (i, r) in allowed_roles.iter().enumerate() {
        let label = format!("r_{i}");
        t.append_message(label.as_bytes(), &r.to_le_bytes());
    }
    for (i, (a, _, _)) in branches.iter().enumerate() {
        let label = format!("A_{i}");
        t.append_point(label.as_bytes(), a);
    }
    let master_c = t.challenge_scalar();

    let sum_c: Scalar = branches.iter().map(|(_, c, _)| *c).sum();
    if sum_c != master_c {
        return false;
    }

    for (i, (a, c, s)) in branches.iter().enumerate() {
        if h * s != a + y_points[i] * c {
            return false;
        }
    }
    true
}

// ---- Session key derivation, KC hash, KC MAC keys ----

/// HKDF-SHA256 extract(salt = nonce_c || nonce_s) then expand with
/// ("session key v2" || pid || eph_c || eph_s).
pub fn derive_session_key(
    eph_secret: &Scalar,
    peer_eph_pub: &RistrettoPoint,
    nonce_c: &[u8; 32],
    nonce_s: &[u8; 32],
    pid: &[u8; 32],
    eph_c: &RistrettoPoint,
    eph_s: &RistrettoPoint,
) -> [u8; 32] {
    let shared = peer_eph_pub * eph_secret;
    let shared_bytes = shared.compress().to_bytes();

    let mut salt = [0u8; 64];
    salt[..32].copy_from_slice(nonce_c);
    salt[32..].copy_from_slice(nonce_s);

    let mut info = Vec::with_capacity(14 + 32 + 32 + 32);
    info.extend_from_slice(b"session key v2");
    info.extend_from_slice(pid);
    info.extend_from_slice(eph_c.compress().as_bytes());
    info.extend_from_slice(eph_s.compress().as_bytes());

    let hk = Hkdf::<Sha256>::new(Some(&salt), &shared_bytes);
    let mut okm = [0u8; 32];
    hk.expand(&info, &mut okm)
        .expect("HKDF expand 32B always succeeds");
    okm
}

/// All ten transcript inputs for the key-confirmation hash, grouped so the
/// signature stays within clippy's argument limit. Field order matches the
/// transcript append order in the body and must not be reordered.
#[derive(Clone, Copy)]
pub struct KcTranscriptParts<'a> {
    pub pid: &'a [u8; 32],
    pub a_c: &'a RistrettoPoint,
    pub s_c: &'a Scalar,
    pub nonce_c: &'a [u8; 32],
    pub eph_c: &'a RistrettoPoint,
    pub server_pub: &'a RistrettoPoint,
    pub a_s: &'a RistrettoPoint,
    pub s_s: &'a Scalar,
    pub nonce_s: &'a [u8; 32],
    pub eph_s: &'a RistrettoPoint,
}

pub fn kc_transcript_hash(parts: &KcTranscriptParts) -> [u8; 32] {
    let KcTranscriptParts {
        pid,
        a_c,
        s_c,
        nonce_c,
        eph_c,
        server_pub,
        a_s,
        s_s,
        nonce_s,
        eph_s,
    } = *parts;
    let mut t = Transcript::new(tr::T_KC_V2);
    t.append_message(b"pid", pid);
    t.append_point(b"a_c", a_c);
    t.append_scalar(b"s_c", s_c);
    t.append_message(b"nonce_c", nonce_c);
    t.append_point(b"eph_c", eph_c);
    t.append_point(b"server_pub", server_pub);
    t.append_point(b"a_s", a_s);
    t.append_scalar(b"s_s", s_s);
    t.append_message(b"nonce_s", nonce_s);
    t.append_point(b"eph_s", eph_s);
    t.hash_sha256()
}

pub fn derive_kc_keys(session_key: &[u8; 32], th: &[u8; 32]) -> ([u8; 32], [u8; 32]) {
    let hk = Hkdf::<Sha256>::new(Some(th), session_key);
    let mut k_s2c = [0u8; 32];
    let mut k_c2s = [0u8; 32];
    hk.expand(b"kc s2c", &mut k_s2c).unwrap();
    hk.expand(b"kc c2s", &mut k_c2s).unwrap();
    (k_s2c, k_c2s)
}

pub fn hmac_tag(key: &[u8; 32], label: &[u8], th: &[u8; 32]) -> [u8; 32] {
    let mut mac = <HmacSha256 as Mac>::new_from_slice(key).expect("HMAC key size ok");
    Mac::update(&mut mac, label);
    Mac::update(&mut mac, th);
    let mut tag = [0u8; 32];
    tag.copy_from_slice(&mac.finalize().into_bytes());
    tag
}

// ---- Device-root derivations (Blake2b) ----

pub fn derive_device_id(root: &[u8; 32]) -> [u8; 32] {
    let mut h = Blake2bVar::new(32).expect("Blake2b-256 OK");
    h.update(b"device-id");
    h.update(root);
    let mut out = [0u8; 32];
    h.finalize_variable(&mut out).expect("finalize OK");
    out
}

pub fn derive_device_scalar(root: &[u8; 32]) -> Scalar {
    let mut h = Blake2b512::new();
    <Blake2b512 as Digest>::update(&mut h, b"device-auth-v1");
    <Blake2b512 as Digest>::update(&mut h, root);
    let d = h.finalize();
    let mut wide = [0u8; 64];
    wide.copy_from_slice(&d[..64]);
    Scalar::from_bytes_mod_order_wide(&wide)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn setup_roundtrip() {
        let x = random_scalar();
        let device_pub = RISTRETTO_BASEPOINT_POINT * x;
        let server_sk = random_scalar();
        let server_pub = RISTRETTO_BASEPOINT_POINT * server_sk;
        let device_id = [1u8; 32];
        let nc = [2u8; 32];
        let ns = [3u8; 32];
        let sc = [4u8; SETUP_CHALLENGE_LEN];

        let binding = SetupBinding {
            device_id: &device_id,
            device_pub: &device_pub,
            server_pub: &server_pub,
            client_nonce: &nc,
            server_nonce: &ns,
            setup_challenge: &sc,
        };
        let cp = prove_setup_client(&x, &binding);
        assert!(verify_setup_client(&binding, &cp));

        let sp = prove_setup_server(&server_sk, &binding);
        assert!(verify_setup_server(&binding, &sp));
    }

    #[test]
    fn auth_roundtrip_with_rerand_and_set() {
        let allowed = &[1u64, 2u64];
        let role_code = 1u64;
        let x = random_scalar();
        let device_pub = RISTRETTO_BASEPOINT_POINT * x;
        let server_sk = random_scalar();
        let server_pub = RISTRETTO_BASEPOINT_POINT * server_sk;

        let blind = random_scalar();
        let stored_c = make_role_commitment(&encode_role(role_code), &blind);

        let nc = [9u8; 32];
        let eph_secret = random_scalar();
        let eph_c = RISTRETTO_BASEPOINT_POINT * eph_secret;
        let pid = compute_pid(&device_pub, &nc, &eph_c, &server_pub);

        let (c_prime, blind_prime, delta) = rerandomize_commitment(&stored_c, &blind);
        let rp = prove_rerandomization(&stored_c, &c_prime, &delta, &pid, &nc, &eph_c);
        assert!(verify_rerandomization(
            &stored_c, &c_prime, &pid, &nc, &eph_c, &rp
        ));

        let role_binding = RoleSetBinding {
            allowed_roles: allowed,
            c_prime: &c_prime,
            pid: &pid,
            nonce_c: &nc,
            eph_c: &eph_c,
        };
        let branches = prove_role_set_membership(&role_binding, role_code, &blind_prime);
        assert!(verify_role_set_membership(&role_binding, &branches));

        let cp = prove_auth_client(&x, &pid, &nc, &eph_c);
        assert!(verify_auth_client(&device_pub, &pid, &nc, &eph_c, &cp));

        let ns = [8u8; 32];
        let eph_s_sk = random_scalar();
        let eph_s = RISTRETTO_BASEPOINT_POINT * eph_s_sk;
        let sp = prove_auth_server(&server_sk, &ns, &eph_s);
        assert!(verify_auth_server(&server_pub, &ns, &eph_s, &sp));

        let k_c = derive_session_key(&eph_secret, &eph_s, &nc, &ns, &pid, &eph_c, &eph_s);
        let k_s = derive_session_key(&eph_s_sk, &eph_c, &nc, &ns, &pid, &eph_c, &eph_s);
        assert_eq!(k_c, k_s);
    }
}
