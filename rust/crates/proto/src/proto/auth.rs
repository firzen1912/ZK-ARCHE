//! Authentication state machine.
//!
//! Drives AUTH_1 -> AUTH_2 -> AUTH_3 -> AUTH_ACK. Transport-agnostic.

use curve25519_dalek::constants::RISTRETTO_BASEPOINT_POINT;
use subtle::ConstantTimeEq;
use zeroize::Zeroize;

use super::common::{rand_session_id, send_expect};
use super::payloads::{decode_ack, encode_ack, Auth1, Auth2, Auth3};
use crate::crypto::{
    compute_pid, derive_device_id, derive_device_scalar, derive_kc_keys, derive_session_key,
    hmac_tag, kc_transcript_hash, prove_auth_client, prove_auth_server, prove_rerandomization,
    prove_role_set_membership, random_bytes_32, random_scalar, reject_identity,
    rerandomize_commitment, verify_auth_client, verify_rerandomization, verify_role_set_membership,
    KcTranscriptParts, RoleSetBinding,
};
use crate::error::{ErrorCode, ProtoError, Result};
use crate::profile::Profile;
use crate::store::{fs::replay_key, CredentialStore, RegistryStore, ReplayCache, ServerKeyStore};
use crate::transport::ClientTransport;
use crate::wire::{build_packet, PKT_AUTH_1, PKT_AUTH_2, PKT_AUTH_3, PKT_AUTH_ACK, SESSION_ID_LEN};

// ---- Client ----

/// Run the full client-side authentication flow. On success, returns the
/// 32-byte session key (caller zeroizes when done).
pub fn run_auth_client<T: ClientTransport, C: CredentialStore>(
    transport: &mut T,
    store: &mut C,
    profile: &Profile,
    allowed_roles: &[u64],
) -> Result<[u8; 32]> {
    // Load device credentials.
    let mut root = store.load_or_create_device_root(false)?;
    let _device_id = derive_device_id(&root);
    let mut x = derive_device_scalar(&root);
    root.zeroize();

    let device_pub = RISTRETTO_BASEPOINT_POINT * x;
    reject_identity(&device_pub, "device_pub")?;

    let server_pub = store.load_server_pub()?.ok_or_else(|| {
        ProtoError::wire(
            ErrorCode::CredentialMissing,
            "no pinned server key; run --setup first",
        )
    })?;

    let role = store
        .load_role_credential()?
        .ok_or_else(|| ProtoError::wire(ErrorCode::CredentialMissing, "role credential missing"))?;

    // --- AUTH_1 ---
    let nonce_c = random_bytes_32();
    let mut eph_secret = random_scalar();
    let eph_c = RISTRETTO_BASEPOINT_POINT * eph_secret;
    let pid = compute_pid(&device_pub, &nonce_c, &eph_c, &server_pub);

    let client_proof = prove_auth_client(&x, &pid, &nonce_c, &eph_c);

    let (c_prime, blind_prime, delta) = rerandomize_commitment(&role.commitment, &role.blind);
    let rerand_proof =
        prove_rerandomization(&role.commitment, &c_prime, &delta, &pid, &nonce_c, &eph_c);
    let branches = prove_role_set_membership(
        &RoleSetBinding {
            allowed_roles,
            c_prime: &c_prime,
            pid: &pid,
            nonce_c: &nonce_c,
            eph_c: &eph_c,
        },
        role.role_code,
        &blind_prime,
    );

    let a1 = Auth1 {
        pid,
        client_proof,
        nonce_c,
        eph_c,
        c_prime,
        rerand_proof,
        branches,
    };
    let session_id = rand_session_id();

    let a2_bytes = send_expect(
        transport,
        profile,
        PKT_AUTH_1,
        &session_id,
        0,
        &a1.encode(),
        PKT_AUTH_2,
    )?;
    let a2 = Auth2::decode(&a2_bytes)?;

    // Pin check (redundant vs transcript but catches obvious MITM early).
    if !bool::from(
        a2.server_pub
            .compress()
            .to_bytes()
            .ct_eq(server_pub.compress().as_bytes()),
    ) {
        x.zeroize();
        eph_secret.zeroize();
        return Err(ProtoError::wire(
            ErrorCode::PeerKeyMismatch,
            "server pubkey mismatch",
        ));
    }

    if !crate::crypto::verify_auth_server(&a2.server_pub, &a2.nonce_s, &a2.eph_s, &a2.server_proof)
    {
        x.zeroize();
        eph_secret.zeroize();
        return Err(ProtoError::wire(
            ErrorCode::ProofVerifyFailed,
            "server auth proof invalid",
        ));
    }

    let session_key = derive_session_key(
        &eph_secret,
        &a2.eph_s,
        &nonce_c,
        &a2.nonce_s,
        &pid,
        &eph_c,
        &a2.eph_s,
    );
    let th = kc_transcript_hash(&KcTranscriptParts {
        pid: &pid,
        a_c: &client_proof.a,
        s_c: &client_proof.s,
        nonce_c: &nonce_c,
        eph_c: &eph_c,
        server_pub: &a2.server_pub,
        a_s: &a2.server_proof.a,
        s_s: &a2.server_proof.s,
        nonce_s: &a2.nonce_s,
        eph_s: &a2.eph_s,
    });
    let (k_s2c, k_c2s) = derive_kc_keys(&session_key, &th);

    let expected_tag_s = hmac_tag(&k_s2c, b"server finished", &th);
    if !bool::from(expected_tag_s.ct_eq(&a2.tag_s)) {
        x.zeroize();
        eph_secret.zeroize();
        return Err(ProtoError::wire(
            ErrorCode::KeyConfirmFailed,
            "server finished tag mismatch",
        ));
    }

    // --- AUTH_3 ---
    let tag_c = hmac_tag(&k_c2s, b"client finished", &th);
    let a3 = Auth3 { tag_c };
    let ack_bytes = send_expect(
        transport,
        profile,
        PKT_AUTH_3,
        &session_id,
        1,
        &a3.encode(),
        PKT_AUTH_ACK,
    )?;
    decode_ack(&ack_bytes)?;

    x.zeroize();
    eph_secret.zeroize();
    Ok(session_key)
}

// ---- Server handlers ----

/// Partial auth-session state a server must keep between AUTH_1 and AUTH_3.
pub struct PendingAuth {
    pub session_id: [u8; SESSION_ID_LEN],
    pub pid: [u8; 32],
    pub device_id: [u8; 32],
    pub expected_tag_c: [u8; 32],
    pub response_packet: Vec<u8>,
}

pub fn handle_auth_1<K: ServerKeyStore, R: RegistryStore, P: ReplayCache>(
    key_store: &mut K,
    registry: &R,
    replay: &mut P,
    allowed_roles: &[u64],
    session_id: [u8; SESSION_ID_LEN],
    seq: u32,
    payload: &[u8],
) -> Result<PendingAuth> {
    let a1 = Auth1::decode(payload)?;

    // 1. Replay check: key = H(pid || nonce_c || eph_c).
    let rkey = replay_key(&a1.pid, &a1.nonce_c, &a1.eph_c);
    if replay.contains(&rkey) {
        return Err(ProtoError::wire(ErrorCode::ReplayDetected, "AUTH_1 replay"));
    }

    // 2. Find the device whose (device_pub, role_commitment) match this pid /
    //    re-randomization proof. Same lookup strategy as the reference impl:
    //    iterate all records and check.
    let server_sk = key_store.load_or_create_server_sk()?;
    let server_pub = RISTRETTO_BASEPOINT_POINT * server_sk;

    let mut found: Option<([u8; 32], crate::store::DeviceRecord)> = None;
    for (device_id, rec) in registry.iter() {
        let pid_candidate = compute_pid(&rec.pubkey, &a1.nonce_c, &a1.eph_c, &server_pub);
        if pid_candidate == a1.pid {
            found = Some((device_id, rec));
            break;
        }
    }
    let (device_id, rec) = found.ok_or_else(|| {
        ProtoError::wire(ErrorCode::UnknownDevice, "no enrolled device matches pid")
    })?;

    // 3. Verify the client identity proof.
    if !verify_auth_client(
        &rec.pubkey,
        &a1.pid,
        &a1.nonce_c,
        &a1.eph_c,
        &a1.client_proof,
    ) {
        return Err(ProtoError::wire(
            ErrorCode::ProofVerifyFailed,
            "client auth proof invalid",
        ));
    }

    // 4. Verify re-randomization proof.
    if !verify_rerandomization(
        &rec.role_commitment,
        &a1.c_prime,
        &a1.pid,
        &a1.nonce_c,
        &a1.eph_c,
        &a1.rerand_proof,
    ) {
        return Err(ProtoError::wire(
            ErrorCode::ProofVerifyFailed,
            "rerand proof invalid",
        ));
    }

    // 5. Verify role set-membership proof.
    if !verify_role_set_membership(
        &RoleSetBinding {
            allowed_roles,
            c_prime: &a1.c_prime,
            pid: &a1.pid,
            nonce_c: &a1.nonce_c,
            eph_c: &a1.eph_c,
        },
        &a1.branches,
    ) {
        return Err(ProtoError::wire(
            ErrorCode::RoleNotPermitted,
            "role set proof invalid",
        ));
    }

    // 6. Derive our ephemeral / nonce_s / server proof / session key.
    let nonce_s = random_bytes_32();
    let mut eph_s_sk = random_scalar();
    let eph_s = RISTRETTO_BASEPOINT_POINT * eph_s_sk;

    let server_proof = prove_auth_server(&server_sk, &nonce_s, &eph_s);

    let session_key = derive_session_key(
        &eph_s_sk,
        &a1.eph_c,
        &a1.nonce_c,
        &nonce_s,
        &a1.pid,
        &a1.eph_c,
        &eph_s,
    );

    let th = kc_transcript_hash(&KcTranscriptParts {
        pid: &a1.pid,
        a_c: &a1.client_proof.a,
        s_c: &a1.client_proof.s,
        nonce_c: &a1.nonce_c,
        eph_c: &a1.eph_c,
        server_pub: &server_pub,
        a_s: &server_proof.a,
        s_s: &server_proof.s,
        nonce_s: &nonce_s,
        eph_s: &eph_s,
    });
    let (k_s2c, k_c2s) = derive_kc_keys(&session_key, &th);
    let tag_s = hmac_tag(&k_s2c, b"server finished", &th);
    let expected_tag_c = hmac_tag(&k_c2s, b"client finished", &th);

    let a2 = Auth2 {
        server_pub,
        server_proof,
        nonce_s,
        eph_s,
        tag_s,
    };
    let response_packet = build_packet(PKT_AUTH_2, &session_id, seq, &a2.encode());

    // Insert the replay key only after full verification, to avoid poisoning
    // the cache on malformed AUTH_1s.
    replay.insert(rkey);
    eph_s_sk.zeroize();

    Ok(PendingAuth {
        session_id,
        pid: a1.pid,
        device_id,
        expected_tag_c,
        response_packet,
    })
}

pub fn handle_auth_3(
    pending: &PendingAuth,
    session_id: [u8; SESSION_ID_LEN],
    seq: u32,
    payload: &[u8],
) -> Result<Vec<u8>> {
    let a3 = Auth3::decode(payload)?;
    if !bool::from(a3.tag_c.ct_eq(&pending.expected_tag_c)) {
        return Err(ProtoError::wire(
            ErrorCode::KeyConfirmFailed,
            "client finished tag mismatch",
        ));
    }
    Ok(build_packet(PKT_AUTH_ACK, &session_id, seq, &encode_ack()))
}
