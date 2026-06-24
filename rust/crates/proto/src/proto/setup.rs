//! Setup / enrollment state machine.
//!
//! Transport-agnostic: operates on `ClientTransport` and `Transport` from
//! `transport::`. The same code drives UDP, TCP, CoAP, or any future binding.

use curve25519_dalek::constants::RISTRETTO_BASEPOINT_POINT;
use curve25519_dalek::ristretto::RistrettoPoint;
use subtle::ConstantTimeEq;
use zeroize::Zeroize;

use crate::crypto::{
    derive_device_id, derive_device_scalar, encode_role, make_role_commitment,
    prove_setup_client, prove_setup_server, random_bytes_32, random_scalar,
    reject_identity, verify_setup_client, verify_setup_server,
};
use crate::error::{ErrorCode, ProtoError, Result};
use crate::profile::Profile;
use crate::store::{CredentialStore, DeviceRecord, RegistryStore, RoleCredential, ServerKeyStore};
use crate::transport::ClientTransport;
use crate::wire::{
    build_packet, PKT_SETUP_1, PKT_SETUP_2, PKT_SETUP_3, PKT_SETUP_ACK, SESSION_ID_LEN,
};
use super::common::{rand_session_id, send_expect};
use super::payloads::{decode_ack, encode_ack, Setup1, Setup2, Setup3};

/// Client-side setup flow.
///
/// Runs `SETUP_1 -> SETUP_2 -> SETUP_3 -> SETUP_ACK`. On success, the pinned
/// server public key is written into the `CredentialStore`. If a TOFU-disabled
/// client has no pinned key and `allow_tofu` is false, returns
/// `ErrorCode::CredentialMissing` without sending anything.
pub fn run_setup_client<T: ClientTransport, C: CredentialStore>(
    transport:     &mut T,
    store:         &mut C,
    profile:       &Profile,
    pairing_token: Option<&str>,
    allow_tofu:    bool,
) -> Result<()> {
    // Load / create device root; derive id + scalar; zeroize root eagerly.
    let mut root = store.load_or_create_device_root(true)?;
    let device_id = derive_device_id(&root);
    let mut x = derive_device_scalar(&root);
    root.zeroize();

    let device_pub = RISTRETTO_BASEPOINT_POINT * x;
    reject_identity(&device_pub, "device_pub")?;

    // Role credential: create on first use.
    let role_cred = match store.load_role_credential()? {
        Some(c) => c,
        None => {
            let role_code = 1u64;
            let blind = random_scalar();
            let commitment = make_role_commitment(&encode_role(role_code), &blind);
            let c = RoleCredential { role_code, blind, commitment };
            store.save_role_credential(&c)?;
            c
        }
    };

    // Enforce pin policy BEFORE talking to the network.
    let pinned = store.load_server_pub()?;
    if pinned.is_none() && !allow_tofu {
        x.zeroize();
        return Err(ProtoError::wire(
            ErrorCode::CredentialMissing,
            "no pinned server key; pin OOB or pass --allow-tofu-setup (lab-only)",
        ));
    }

    let session_id = rand_session_id();
    let client_nonce = random_bytes_32();

    // --- SETUP_1 -> SETUP_2 ---
    let s1 = Setup1 {
        pairing_token: pairing_token.map(|t| t.as_bytes().to_vec()),
        device_id,
        device_pub,
        client_nonce,
        role_commitment: role_cred.commitment,
    };
    let s2_bytes = send_expect(transport, profile, PKT_SETUP_1, &session_id, 0, &s1.encode(), PKT_SETUP_2)?;
    let s2 = Setup2::decode(&s2_bytes)?;

    // TOFU vs pinned check.
    let server_pub_bytes = s2.server_pub.compress().to_bytes();
    if let Some(p) = pinned {
        if !bool::from(p.compress().to_bytes().ct_eq(&server_pub_bytes)) {
            x.zeroize();
            return Err(ProtoError::wire(
                ErrorCode::PeerKeyMismatch,
                "server key does not match pinned value",
            ));
        }
    }

    if !verify_setup_server(
        &s2.server_pub,
        &device_id,
        &device_pub,
        &client_nonce,
        &s2.server_nonce,
        &s2.setup_challenge,
        &s2.server_proof,
    ) {
        x.zeroize();
        return Err(ProtoError::wire(
            ErrorCode::ProofVerifyFailed,
            "server setup proof failed verification",
        ));
    }

    // --- SETUP_3 ---
    let client_proof = prove_setup_client(
        &x, &device_id, &device_pub, &s2.server_pub,
        &client_nonce, &s2.server_nonce, &s2.setup_challenge,
    );
    x.zeroize();
    let s3 = Setup3 { client_proof };
    let ack_bytes = send_expect(transport, profile, PKT_SETUP_3, &session_id, 1, &s3.encode(), PKT_SETUP_ACK)?;
    decode_ack(&ack_bytes)?;

    store.save_server_pub(&s2.server_pub)?;
    Ok(())
}

/// Generate a random 16-byte session id.
// ---- Server-side handlers ----

/// Server-side handler for a received SETUP_1 packet. Produces the SETUP_2
/// response packet bytes, together with the partial setup-session state that
/// the caller MUST cache in order to process the subsequent SETUP_3.
pub struct PendingSetup {
    pub session_id:      [u8; SESSION_ID_LEN],
    pub device_id:       [u8; 32],
    pub device_pub:      RistrettoPoint,
    pub client_nonce:    [u8; 32],
    pub server_nonce:    [u8; 32],
    pub setup_challenge: [u8; crate::crypto::SETUP_CHALLENGE_LEN],
    pub role_commitment: RistrettoPoint,
    pub response_packet: Vec<u8>,
}

pub fn handle_setup_1<K: ServerKeyStore>(
    key_store:  &mut K,
    session_id: [u8; SESSION_ID_LEN],
    seq:        u32,
    payload:    &[u8],
    require_pairing_token: Option<&str>,
) -> Result<PendingSetup> {
    let s1 = Setup1::decode(payload)?;

    if let Some(expected) = require_pairing_token {
        let provided = s1.pairing_token.as_deref().unwrap_or(&[]);
        if !bool::from(provided.ct_eq(expected.as_bytes())) {
            return Err(ProtoError::wire(
                ErrorCode::PairingTokenInvalid,
                "pairing token mismatch",
            ));
        }
    }

    let server_sk = key_store.load_or_create_server_sk()?;
    let server_pub = RISTRETTO_BASEPOINT_POINT * server_sk;

    let server_nonce = random_bytes_32();
    let mut sc = [0u8; crate::crypto::SETUP_CHALLENGE_LEN];
    {
        let bytes = random_bytes_32();
        sc.copy_from_slice(&bytes[..crate::crypto::SETUP_CHALLENGE_LEN]);
    }

    let server_proof = prove_setup_server(
        &server_sk,
        &s1.device_id, &s1.device_pub, &server_pub,
        &s1.client_nonce, &server_nonce, &sc,
    );

    let s2 = Setup2 {
        server_nonce,
        setup_challenge: sc,
        server_pub,
        server_proof,
    };
    let response_packet = build_packet(PKT_SETUP_2, &session_id, seq, &s2.encode());

    Ok(PendingSetup {
        session_id,
        device_id: s1.device_id,
        device_pub: s1.device_pub,
        client_nonce: s1.client_nonce,
        server_nonce,
        setup_challenge: sc,
        role_commitment: s1.role_commitment,
        response_packet,
    })
}

/// Server-side handler for SETUP_3. Produces the SETUP_ACK response and
/// registers the device in the registry on success.
pub fn handle_setup_3<R: RegistryStore>(
    registry:   &mut R,
    pending:    &PendingSetup,
    server_pub: &RistrettoPoint,
    session_id: [u8; SESSION_ID_LEN],
    seq:        u32,
    payload:    &[u8],
) -> Result<Vec<u8>> {
    let s3 = Setup3::decode(payload)?;

    let ok = verify_setup_client(
        &pending.device_id, &pending.device_pub, server_pub,
        &pending.client_nonce, &pending.server_nonce, &pending.setup_challenge,
        &s3.client_proof,
    );
    if !ok {
        return Err(ProtoError::wire(
            ErrorCode::ProofVerifyFailed,
            "client setup proof failed verification",
        ));
    }

    registry.save(
        pending.device_id,
        DeviceRecord { pubkey: pending.device_pub, role_commitment: pending.role_commitment },
    )?;

    Ok(build_packet(PKT_SETUP_ACK, &session_id, seq, &encode_ack()))
}
