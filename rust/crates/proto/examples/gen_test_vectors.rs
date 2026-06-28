//! Fully-deterministic test vector generator.
//!
//! Every byte of every output is reproducible from the published seed. A
//! conforming implementation in any language that:
//!
//!   1. instantiates ChaCha20 with the same 32-byte seed, and
//!   2. consumes bytes from it in the same order described in
//!      `test-vectors/0x0001/DRBG.md` (written by this tool),
//!
//! will produce byte-identical JSON vectors — including every proof
//! `(a, s)` — so cross-language conformance can be checked exactly.
//!
//! Run with:
//!
//! ```bash
//! cargo run --example gen_test_vectors --features test-vectors
//! ```

use std::fs;
use std::path::Path;

use curve25519_dalek::constants::RISTRETTO_BASEPOINT_POINT;
use curve25519_dalek::scalar::Scalar;
use rand::{RngCore, SeedableRng};
use rand_chacha::ChaCha20Rng;
use serde::Serialize;

use proto::{
    crypto::{
        attr_h, compute_pid, derive_kc_keys, derive_session_key, encode_role, hmac_tag,
        kc_transcript_hash, make_role_commitment, prove_auth_client_with_rng,
        prove_auth_server_with_rng, prove_rerandomization_with_rng,
        prove_role_set_membership_with_rng, verify_auth_client, verify_rerandomization,
        verify_role_set_membership,
    },
    transcript::{Transcript, T_PID},
};

// ---- DRBG ----

/// Fixed 32-byte seed. ASCII; 32 bytes exactly.
const DRBG_SEED: [u8; 32] = *b"iot-auth/test-vectors/v1        ";

fn drbg() -> ChaCha20Rng {
    ChaCha20Rng::from_seed(DRBG_SEED)
}

// ---- JSON envelope ----

#[derive(Serialize)]
struct Vector<I: Serialize, E: Serialize> {
    suite: &'static str,
    name: &'static str,
    notes: &'static str,
    inputs: I,
    expected: E,
}

fn write_json<T: Serialize>(path: &Path, value: &T) {
    fs::create_dir_all(path.parent().unwrap()).unwrap();
    let s = serde_json::to_string_pretty(value).unwrap();
    fs::write(path, s).unwrap();
    println!("wrote {}", path.display());
}

fn hx(b: &[u8]) -> String {
    hex::encode(b)
}

// ---- Vector 1: Transcript byte layout ----

#[derive(Serialize)]
struct TranscriptInputs {
    domain: String,
    fields: Vec<(String, String)>,
}
#[derive(Serialize)]
struct TranscriptExpected {
    transcript_bytes: String,
    challenge_scalar: String,
}

fn vec_transcript(out: &Path) {
    let mut t = Transcript::new(b"test-domain");
    t.append_message(b"a", b"hello");
    t.append_message(b"b", &[0x11, 0x22, 0x33, 0x44]);
    let challenge = t.challenge_scalar();

    let v = Vector {
        suite: "0x0001",
        name: "transcript_basic",
        notes: "Byte-for-byte transcript layout with two appended messages.",
        inputs: TranscriptInputs {
            domain: "test-domain".into(),
            fields: vec![
                ("a".into(), hx(b"hello")),
                ("b".into(), hx(&[0x11, 0x22, 0x33, 0x44])),
            ],
        },
        expected: TranscriptExpected {
            transcript_bytes: hx(t.as_bytes()),
            challenge_scalar: hx(&challenge.to_bytes()),
        },
    };
    write_json(out, &v);
}

// ---- Vector 2: PID derivation ----

#[derive(Serialize)]
struct PidInputs {
    domain: String,
    device_pub: String,
    nonce_c: String,
    eph_c: String,
    server_pub: String,
}
#[derive(Serialize)]
struct PidExpected {
    pid: String,
}

fn vec_pid(out: &Path) {
    // Fixed non-trivial points.
    let device_pub = RISTRETTO_BASEPOINT_POINT * Scalar::from(7u64);
    let server_pub = RISTRETTO_BASEPOINT_POINT * Scalar::from(13u64);
    let eph_c = RISTRETTO_BASEPOINT_POINT * Scalar::from(19u64);
    let nonce_c = [0x42u8; 32];

    let pid = compute_pid(&device_pub, &nonce_c, &eph_c, &server_pub);
    let v = Vector {
        suite: "0x0001",
        name:  "pid_basic",
        notes: "PID = SHA256( u32_LE(len(T_PID)) || T_PID || device_pub || nonce_c || eph_c || server_pub ).",
        inputs: PidInputs {
            domain:     String::from_utf8(T_PID.to_vec()).unwrap(),
            device_pub: hx(device_pub.compress().as_bytes()),
            nonce_c:    hx(&nonce_c),
            eph_c:      hx(eph_c.compress().as_bytes()),
            server_pub: hx(server_pub.compress().as_bytes()),
        },
        expected: PidExpected { pid: hx(&pid) },
    };
    write_json(out, &v);
}

// ---- Vector 3: Schnorr auth-client prove + verify (DETERMINISTIC) ----

#[derive(Serialize)]
struct SchnorrAuthInputs {
    x: String,
    device_pub: String,
    pid: String,
    nonce_c: String,
    eph_c: String,
    // Exact 64 bytes the DRBG must emit for `r`.
    drbg_bytes_for_r: String,
}
#[derive(Serialize)]
struct SchnorrAuthExpected {
    proof_a: String,
    proof_s: String,
    verify_ok_own: bool,
    verify_ok_wrong_pubkey: bool,
}

fn vec_schnorr_auth(out: &Path) {
    // Fresh local RNG: a reproducer fed the same 64 wide-bytes must obtain
    // the same `r` and therefore the same `(a, s)`.
    let mut rng = drbg();
    // Consume fixed inputs first so we expose exactly which 64 bytes the
    // DRBG emits for `r`.
    let x = Scalar::from(11u64);
    let device_pub = RISTRETTO_BASEPOINT_POINT * x;
    let pid = [0xABu8; 32];
    let nonce_c = [0x55u8; 32];
    let eph_c = RISTRETTO_BASEPOINT_POINT * Scalar::from(23u64);

    // Reveal the 64 wide-bytes the prover will consume from the DRBG.
    let mut peek = drbg(); // same seed; we peek the first 64 bytes
    let mut wide_for_r = [0u8; 64];
    peek.fill_bytes(&mut wide_for_r);

    let proof = prove_auth_client_with_rng(&mut rng, &x, &pid, &nonce_c, &eph_c);

    let ok = verify_auth_client(&device_pub, &pid, &nonce_c, &eph_c, &proof);
    let wrong = verify_auth_client(
        &(RISTRETTO_BASEPOINT_POINT * Scalar::from(3u64)),
        &pid,
        &nonce_c,
        &eph_c,
        &proof,
    );

    let v = Vector {
        suite: "0x0001",
        name: "schnorr_auth_client",
        notes: "Fully deterministic. Reproducing implementations: instantiate \
                ChaCha20 with the published DRBG_SEED, consume exactly 64 \
                bytes for `r` (reduced via scalar_from_wide_bytes), and \
                produce the published (a, s) byte-for-byte.",
        inputs: SchnorrAuthInputs {
            x: hx(&x.to_bytes()),
            device_pub: hx(device_pub.compress().as_bytes()),
            pid: hx(&pid),
            nonce_c: hx(&nonce_c),
            eph_c: hx(eph_c.compress().as_bytes()),
            drbg_bytes_for_r: hx(&wide_for_r),
        },
        expected: SchnorrAuthExpected {
            proof_a: hx(proof.a.compress().as_bytes()),
            proof_s: hx(&proof.s.to_bytes()),
            verify_ok_own: ok,
            verify_ok_wrong_pubkey: wrong,
        },
    };
    write_json(out, &v);
}

// ---- Vector 4: Role set-membership (DETERMINISTIC, branch-order documented) ----

#[derive(Serialize)]
struct RoleSetInputs {
    allowed_roles: Vec<u64>,
    role_code: u64,
    blind_prime: String,
    c_prime: String,
    pid: String,
    nonce_c: String,
    eph_c: String,
    drbg_consumption: &'static str,
}
#[derive(Serialize)]
struct RoleSetBranchSer {
    a: String,
    c: String,
    s: String,
}
#[derive(Serialize)]
struct RoleSetExpected {
    branches: Vec<RoleSetBranchSer>,
    verify_ok: bool,
    verify_wrong_c_prime: bool,
}

fn vec_role_set(out: &Path) {
    let allowed = vec![1u64, 2u64, 3u64];
    let role_code = 2u64; // true_index = 1

    let blind = Scalar::from(97u64);
    let c = make_role_commitment(&encode_role(role_code), &blind);
    let delta = Scalar::from(41u64);
    let c_prime = c + attr_h() * delta;
    let blind_prime = blind + delta;

    let pid = [0xCDu8; 32];
    let nonce_c = [0xEFu8; 32];
    let eph_c = RISTRETTO_BASEPOINT_POINT * Scalar::from(101u64);

    let mut rng = drbg();
    let branches = prove_role_set_membership_with_rng(
        &mut rng,
        &allowed,
        &c_prime,
        role_code,
        &blind_prime,
        &pid,
        &nonce_c,
        &eph_c,
    );
    let ok = verify_role_set_membership(&allowed, &c_prime, &pid, &nonce_c, &eph_c, &branches);

    // Tamper case.
    let bad_c = c_prime + attr_h() * Scalar::from(999u64);
    let bad = verify_role_set_membership(&allowed, &bad_c, &pid, &nonce_c, &eph_c, &branches);

    let branches_ser = branches
        .iter()
        .map(|(a, c, s)| RoleSetBranchSer {
            a: hx(a.compress().as_bytes()),
            c: hx(&c.to_bytes()),
            s: hx(&s.to_bytes()),
        })
        .collect();

    let v = Vector {
        suite: "0x0001",
        name: "role_set_membership",
        notes: "Fully deterministic CDS-OR proof. DRBG consumption order \
                matches the loop in prove_role_set_membership_with_rng(): \
                for i in 0..allowed_roles.len(), the true branch consumes \
                1 scalar (w), every other branch consumes 2 scalars (c_i, s_i). \
                Verification MUST accept the published branches; tampering with \
                c_prime MUST cause rejection.",
        inputs: RoleSetInputs {
            allowed_roles: allowed,
            role_code,
            blind_prime: hx(&blind_prime.to_bytes()),
            c_prime: hx(c_prime.compress().as_bytes()),
            pid: hx(&pid),
            nonce_c: hx(&nonce_c),
            eph_c: hx(eph_c.compress().as_bytes()),
            drbg_consumption: "for each i in 0..n: if i == true_index consume 1 scalar (64B); \
                 else consume 2 scalars (128B). n=3, true_index=1 here.",
        },
        expected: RoleSetExpected {
            branches: branches_ser,
            verify_ok: ok,
            verify_wrong_c_prime: bad,
        },
    };
    write_json(out, &v);
}

// ---- Vector 5: Rerandomization proof (DETERMINISTIC) ----

#[derive(Serialize)]
struct RerandInputs {
    stored_c: String,
    blind: String,
    delta: String,
    pid: String,
    nonce_c: String,
    eph_c: String,
    drbg_bytes_for_r: String,
}
#[derive(Serialize)]
struct RerandExpected {
    c_prime: String,
    proof_a: String,
    proof_s: String,
    verify_ok: bool,
}

fn vec_rerand(out: &Path) {
    let blind = Scalar::from(31u64);
    let delta = Scalar::from(77u64);
    let role_code = 1u64;
    let stored_c = make_role_commitment(&encode_role(role_code), &blind);
    let c_prime = stored_c + attr_h() * delta;

    let pid = [0x77u8; 32];
    let nonce_c = [0x88u8; 32];
    let eph_c = RISTRETTO_BASEPOINT_POINT * Scalar::from(42u64);

    let mut rng = drbg();
    let mut peek = drbg();
    let mut wide = [0u8; 64];
    peek.fill_bytes(&mut wide);
    let proof = prove_rerandomization_with_rng(
        &mut rng, &stored_c, &c_prime, &delta, &pid, &nonce_c, &eph_c,
    );
    let ok = verify_rerandomization(&stored_c, &c_prime, &pid, &nonce_c, &eph_c, &proof);

    let v = Vector {
        suite: "0x0001",
        name: "rerandomization",
        notes: "Fully deterministic. Prove c_prime = stored_c + h*delta \
                without revealing delta or blind.",
        inputs: RerandInputs {
            stored_c: hx(stored_c.compress().as_bytes()),
            blind: hx(&blind.to_bytes()),
            delta: hx(&delta.to_bytes()),
            pid: hx(&pid),
            nonce_c: hx(&nonce_c),
            eph_c: hx(eph_c.compress().as_bytes()),
            drbg_bytes_for_r: hx(&wide),
        },
        expected: RerandExpected {
            c_prime: hx(c_prime.compress().as_bytes()),
            proof_a: hx(proof.a.compress().as_bytes()),
            proof_s: hx(&proof.s.to_bytes()),
            verify_ok: ok,
        },
    };
    write_json(out, &v);
}

// ---- Vector 6: Session key + KC tags (deterministic; no DRBG needed) ----

#[derive(Serialize)]
struct KdfKcInputs {
    client_eph_sk: String,
    server_eph_sk: String,
    nonce_c: String,
    nonce_s: String,
    pid: String,
    eph_c: String,
    eph_s: String,
    server_pub: String,
    a_c: String,
    s_c: String,
    a_s: String,
    s_s: String,
}
#[derive(Serialize)]
struct KdfKcExpected {
    session_key_from_client: String,
    session_key_from_server: String,
    session_keys_match: bool,
    transcript_hash: String,
    k_s2c: String,
    k_c2s: String,
    tag_s: String,
    tag_c: String,
}

fn vec_kdf_kc(out: &Path) {
    // Fixed, tiny scalars so the vector is compact.
    let client_sk = Scalar::from(11u64);
    let server_sk = Scalar::from(13u64);
    let client_eph = Scalar::from(17u64);
    let server_eph = Scalar::from(19u64);

    let eph_c = RISTRETTO_BASEPOINT_POINT * client_eph;
    let eph_s = RISTRETTO_BASEPOINT_POINT * server_eph;
    let server_pub = RISTRETTO_BASEPOINT_POINT * server_sk;

    let nonce_c = [0xA1u8; 32];
    let nonce_s = [0xB2u8; 32];

    let device_pub = RISTRETTO_BASEPOINT_POINT * client_sk;
    let pid = compute_pid(&device_pub, &nonce_c, &eph_c, &server_pub);

    // Produce valid Schnorr proofs with the DRBG so the transcript hash is
    // reproducible too. Client prover consumes 64 bytes, then server prover
    // consumes 64 bytes.
    let mut rng = drbg();
    let cp = prove_auth_client_with_rng(&mut rng, &client_sk, &pid, &nonce_c, &eph_c);
    let sp = prove_auth_server_with_rng(&mut rng, &server_sk, &nonce_s, &eph_s);

    let kc = derive_session_key(
        &client_eph,
        &eph_s,
        &nonce_c,
        &nonce_s,
        &pid,
        &eph_c,
        &eph_s,
    );
    let ks = derive_session_key(
        &server_eph,
        &eph_c,
        &nonce_c,
        &nonce_s,
        &pid,
        &eph_c,
        &eph_s,
    );

    let th = kc_transcript_hash(
        &pid,
        &cp.a,
        &cp.s,
        &nonce_c,
        &eph_c,
        &server_pub,
        &sp.a,
        &sp.s,
        &nonce_s,
        &eph_s,
    );
    let (k_s2c, k_c2s) = derive_kc_keys(&kc, &th);
    let tag_s = hmac_tag(&k_s2c, b"server finished", &th);
    let tag_c = hmac_tag(&k_c2s, b"client finished", &th);

    let v = Vector {
        suite: "0x0001",
        name: "kdf_kc",
        notes: "Fully deterministic. Both peers MUST derive identical \
                session keys. Schnorr proofs use DRBG bytes 0..64 for \
                the client prover and 64..128 for the server prover.",
        inputs: KdfKcInputs {
            client_eph_sk: hx(&client_eph.to_bytes()),
            server_eph_sk: hx(&server_eph.to_bytes()),
            nonce_c: hx(&nonce_c),
            nonce_s: hx(&nonce_s),
            pid: hx(&pid),
            eph_c: hx(eph_c.compress().as_bytes()),
            eph_s: hx(eph_s.compress().as_bytes()),
            server_pub: hx(server_pub.compress().as_bytes()),
            a_c: hx(cp.a.compress().as_bytes()),
            s_c: hx(&cp.s.to_bytes()),
            a_s: hx(sp.a.compress().as_bytes()),
            s_s: hx(&sp.s.to_bytes()),
        },
        expected: KdfKcExpected {
            session_key_from_client: hx(&kc),
            session_key_from_server: hx(&ks),
            session_keys_match: kc == ks,
            transcript_hash: hx(&th),
            k_s2c: hx(&k_s2c),
            k_c2s: hx(&k_c2s),
            tag_s: hx(&tag_s),
            tag_c: hx(&tag_c),
        },
    };
    write_json(out, &v);
}

// ---- DRBG.md documentation file ----

fn write_drbg_doc(out: &Path) {
    let text = r#"# DRBG specification for test vectors (suite 0x0001)

All probabilistic fields in these vectors are derived from a single
deterministic bit generator so that any conforming implementation can
reproduce them byte-for-byte.

## Seed

```
DRBG_SEED = b"iot-auth/test-vectors/v1        "  (exactly 32 bytes)
```

## Generator

ChaCha20 with the above seed as the 256-bit key, 96-bit nonce = all zeros,
block counter starts at 0. Output is the keystream. This matches the
`rand_chacha::ChaCha20Rng::from_seed` RFC-compatible mode.

## Scalar sampling

```
rand_scalar(rng):
    bytes ← rng.next_bytes(64)          # 64 bytes = 512 bits
    return Scalar::from_bytes_mod_order_wide(bytes)
```

i.e. 64 uniform bytes, reduced mod the ristretto255 scalar group order via
the standard "wide" reduction used by libsodium, ed25519-donna, and
curve25519-dalek (all of these produce identical output given identical
64-byte inputs).

## Consumption order

Each vector file uses a **fresh** generator seeded with `DRBG_SEED`. The
bytes consumed inside a single vector are listed in the `inputs` block.
For provers that call `rand_scalar` multiple times, the consumption order
is:

- `schnorr_auth_client.json`:    64 bytes for `r`.
- `rerandomization.json`:        64 bytes for `r`.
- `role_set_membership.json`:    for each branch in `0..n`, if it is the
                                 true index consume 64 bytes (`w`), else
                                 consume 128 bytes (`c_i`, `s_i`).
- `kdf_kc.json`:                 bytes 0..64 → client Schnorr `r`;
                                 bytes 64..128 → server Schnorr `r`.

A reproducing implementation that uses the same seed, the same generator,
and consumes bytes in the same order will obtain the same `(a, s)` for
every proof published in this corpus.
"#;
    fs::create_dir_all(out.parent().unwrap()).unwrap();
    fs::write(out, text).unwrap();
    println!("wrote {}", out.display());
}

// ---- Entry point ----

fn main() {
    let base = Path::new("test-vectors/0x0001");
    vec_transcript(&base.join("transcript.json"));
    vec_pid(&base.join("pid.json"));
    vec_schnorr_auth(&base.join("schnorr_auth_client.json"));
    vec_role_set(&base.join("role_set_membership.json"));
    vec_rerand(&base.join("rerandomization.json"));
    vec_kdf_kc(&base.join("kdf_kc.json"));
    write_drbg_doc(&base.join("DRBG.md"));
    println!("\nall test vectors written under test-vectors/0x0001/");
}
