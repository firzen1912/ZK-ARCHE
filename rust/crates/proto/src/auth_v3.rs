//! Draft AUTH v3 reference primitives.
//!
//! This module implements the non-advertised transcript, key-confirmation, and
//! authenticated-completion construction proposed by ADR 0001. It is intended
//! only for deterministic vectors, review, and future Rust/C parity work.
//!
//! IMPORTANT:
//! - v2 wire bytes and state-machine behavior remain unchanged;
//! - no negotiation or packet-dispatch path selects these functions;
//! - the construction is not normative until the ADR/spec/vector gates pass.

use curve25519_dalek::ristretto::RistrettoPoint;
use curve25519_dalek::scalar::Scalar;
use hkdf::Hkdf;
use hmac::{Hmac, Mac};
use sha2::{Digest, Sha256};

type HmacSha256 = Hmac<Sha256>;

/// Draft AUTH v3 transcript domain from ADR 0001.
pub const T_KC_V3: &[u8] = b"zk-arche/kc/v3";

/// Canonical security context bound into the draft AUTH v3 KC transcript.
#[derive(Clone, Copy)]
pub struct AuthV3Context<'a> {
    pub protocol_version: u8,
    pub suite_id: u16,
    pub profile_id: u16,
    pub selected_capabilities: u64,
    pub session_id: &'a [u8; 16],
    pub authz_context_hash: &'a [u8; 32],
    pub critical_extensions_hash: &'a [u8; 32],
    pub channel_binding_hash: &'a [u8; 32],
}

/// AUTH proof/ephemeral values appended after the v3 security context.
#[derive(Clone, Copy)]
pub struct KcTranscriptV3Parts<'a> {
    pub context: AuthV3Context<'a>,
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

fn append_field(buf: &mut Vec<u8>, label: &[u8], value: &[u8]) {
    assert!(label.len() <= u8::MAX as usize, "label too long");
    let value_len = u32::try_from(value.len()).expect("transcript field too long");
    buf.push(label.len() as u8);
    buf.extend_from_slice(label);
    buf.extend_from_slice(&value_len.to_le_bytes());
    buf.extend_from_slice(value);
}

/// Build the exact draft `KC-TRANSCRIPT-v3` byte sequence.
///
/// Encoding deliberately matches the repository's existing transcript builder:
/// `u8(domain_len) || domain || repeated(u8(label_len) || label ||
/// u32_le(value_len) || value)`.
pub fn build_kc_transcript_v3(parts: &KcTranscriptV3Parts<'_>) -> Vec<u8> {
    assert!(T_KC_V3.len() <= u8::MAX as usize, "domain too long");
    let mut out = Vec::with_capacity(768);
    out.push(T_KC_V3.len() as u8);
    out.extend_from_slice(T_KC_V3);

    let c = parts.context;
    append_field(&mut out, b"protocol_version", &[c.protocol_version]);
    append_field(&mut out, b"suite_id", &c.suite_id.to_le_bytes());
    append_field(&mut out, b"profile_id", &c.profile_id.to_le_bytes());
    append_field(
        &mut out,
        b"selected_capabilities",
        &c.selected_capabilities.to_le_bytes(),
    );
    append_field(&mut out, b"session_id", c.session_id);
    append_field(&mut out, b"authz_context_hash", c.authz_context_hash);
    append_field(
        &mut out,
        b"critical_extensions_hash",
        c.critical_extensions_hash,
    );
    append_field(&mut out, b"channel_binding_hash", c.channel_binding_hash);

    append_field(&mut out, b"pid", parts.pid);
    append_field(&mut out, b"a_c", parts.a_c.compress().as_bytes());
    append_field(&mut out, b"s_c", &parts.s_c.to_bytes());
    append_field(&mut out, b"nonce_c", parts.nonce_c);
    append_field(&mut out, b"eph_c", parts.eph_c.compress().as_bytes());
    append_field(
        &mut out,
        b"server_pub",
        parts.server_pub.compress().as_bytes(),
    );
    append_field(&mut out, b"a_s", parts.a_s.compress().as_bytes());
    append_field(&mut out, b"s_s", &parts.s_s.to_bytes());
    append_field(&mut out, b"nonce_s", parts.nonce_s);
    append_field(&mut out, b"eph_s", parts.eph_s.compress().as_bytes());

    out
}

/// SHA-256 digest of the draft v3 KC transcript.
pub fn kc_transcript_hash_v3(parts: &KcTranscriptV3Parts<'_>) -> [u8; 32] {
    let transcript = build_kc_transcript_v3(parts);
    let mut out = [0u8; 32];
    out.copy_from_slice(&Sha256::digest(transcript));
    out
}

/// Draft v3 purpose-separated KC/completion keys.
pub fn derive_kc_keys_v3(
    session_key: &[u8; 32],
    th_v3: &[u8; 32],
) -> ([u8; 32], [u8; 32], [u8; 32]) {
    let hk = Hkdf::<Sha256>::new(Some(th_v3), session_key);
    let mut k_s2c = [0u8; 32];
    let mut k_c2s = [0u8; 32];
    let mut k_complete = [0u8; 32];
    hk.expand(b"kc s2c v3", &mut k_s2c)
        .expect("HKDF expand 32B always succeeds");
    hk.expand(b"kc c2s v3", &mut k_c2s)
        .expect("HKDF expand 32B always succeeds");
    hk.expand(b"kc complete s2c v3", &mut k_complete)
        .expect("HKDF expand 32B always succeeds");
    (k_s2c, k_c2s, k_complete)
}

/// Draft v3 directional finished tag.
pub fn finished_tag_v3(key: &[u8; 32], label: &[u8], th_v3: &[u8; 32]) -> [u8; 32] {
    let mut mac = <HmacSha256 as Mac>::new_from_slice(key).expect("HMAC key size ok");
    Mac::update(&mut mac, label);
    Mac::update(&mut mac, th_v3);
    let mut out = [0u8; 32];
    out.copy_from_slice(&mac.finalize().into_bytes());
    out
}

/// Draft post-AUTH_3 completion digest from ADR 0001.
pub fn completion_hash_v3(th_v3: &[u8; 32], tag_c: &[u8; 32]) -> [u8; 32] {
    let mut h = Sha256::new();
    h.update(b"zk-arche/auth-complete/v3");
    h.update(th_v3);
    h.update(tag_c);
    let mut out = [0u8; 32];
    out.copy_from_slice(&h.finalize());
    out
}

/// Draft authenticated server-completion value carried by AUTH_ACK-v3.
pub fn completion_tag_v3(k_complete: &[u8; 32], completion_hash: &[u8; 32]) -> [u8; 32] {
    let mut mac = <HmacSha256 as Mac>::new_from_slice(k_complete).expect("HMAC key size ok");
    Mac::update(&mut mac, b"server complete v3");
    Mac::update(&mut mac, completion_hash);
    let mut out = [0u8; 32];
    out.copy_from_slice(&mac.finalize().into_bytes());
    out
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::crypto::{decode_scalar, decompress_point};

    fn bytes32(hex_value: &str) -> [u8; 32] {
        hex::decode(hex_value).unwrap().try_into().unwrap()
    }

    fn bytes16(hex_value: &str) -> [u8; 16] {
        hex::decode(hex_value).unwrap().try_into().unwrap()
    }

    #[test]
    fn draft_v3_reference_vector_is_stable_and_context_bound() {
        let pid = bytes32("1fe089931f5237f1306f8215eb0886fe261f77c7738cb85e6ce11a98e27f36ea");
        let a_c_bytes = bytes32("6438b7d6b72e5bf7d473250fe8cb3d24ad472abea8ed5686951816a62735042a");
        let s_c_bytes = bytes32("deb9331e101e11702f530e62c6b499b6ec6ba6d0a345ccb2ca8c6147766fa00c");
        let eph_c_bytes =
            bytes32("682802b3c90112e0f4e7d985e423cd2b16c5bfa63d9c967c52bb6cb7fea7ea7e");
        let server_pub_bytes =
            bytes32("aa52e000df2e16f55fb1032fc33bc42742dad6bd5a8fc0be0167436c5948501f");
        let a_s_bytes = bytes32("b69b1027f551d8f6801a0a96daec74b33c69e27f57cc73ca908a730f11859b63");
        let s_s_bytes = bytes32("c9ec78fcfb966d979d0268ac487230bd6683c3bdddfab82be95edee0a7a8a10f");
        let eph_s_bytes =
            bytes32("4cf1b9deda93eb9fd515fcc99262aed1368b48f24a27afd2984da8fe7bb2341f");

        let a_c = decompress_point(&a_c_bytes, "v3_vector_a_c").unwrap();
        let s_c = decode_scalar(&s_c_bytes, "v3_vector_s_c").unwrap();
        let eph_c = decompress_point(&eph_c_bytes, "v3_vector_eph_c").unwrap();
        let server_pub = decompress_point(&server_pub_bytes, "v3_vector_server_pub").unwrap();
        let a_s = decompress_point(&a_s_bytes, "v3_vector_a_s").unwrap();
        let s_s = decode_scalar(&s_s_bytes, "v3_vector_s_s").unwrap();
        let eph_s = decompress_point(&eph_s_bytes, "v3_vector_eph_s").unwrap();

        let session_id = bytes16("00112233445566778899aabbccddeeff");
        let authz_context_hash = [0x11u8; 32];
        let critical_extensions_hash = [0x22u8; 32];
        let channel_binding_hash = [0x33u8; 32];
        let nonce_c = [0xa1u8; 32];
        let nonce_s = [0xb2u8; 32];

        let context = AuthV3Context {
            protocol_version: 3,
            suite_id: 1,
            profile_id: 1,
            selected_capabilities: 5,
            session_id: &session_id,
            authz_context_hash: &authz_context_hash,
            critical_extensions_hash: &critical_extensions_hash,
            channel_binding_hash: &channel_binding_hash,
        };
        let parts = KcTranscriptV3Parts {
            context,
            pid: &pid,
            a_c: &a_c,
            s_c: &s_c,
            nonce_c: &nonce_c,
            eph_c: &eph_c,
            server_pub: &server_pub,
            a_s: &a_s,
            s_s: &s_s,
            nonce_s: &nonce_s,
            eph_s: &eph_s,
        };

        let transcript = build_kc_transcript_v3(&parts);
        assert_eq!(transcript.len(), 726);

        let th = kc_transcript_hash_v3(&parts);
        assert_eq!(
            hex::encode(th),
            "e2b85befd4f3f58b5e880673ce1b27e81de875bf4443d7af6971d811e10439d2"
        );

        let session_key =
            bytes32("5cea979c840f9cb1302db41f7dcfe91c4f8b22f7019b0586db183219e27ef348");
        let (k_s2c, k_c2s, k_complete) = derive_kc_keys_v3(&session_key, &th);
        assert_eq!(
            hex::encode(k_s2c),
            "622ae55fab76559100ab38020fe72070b2a29bc95b06ef49686d26be280f3cc9"
        );
        assert_eq!(
            hex::encode(k_c2s),
            "b8f77e03dd68595d4e2ed49f83112373f597955efe32a0ad855d1a1a2ff1cab8"
        );
        assert_eq!(
            hex::encode(k_complete),
            "ce41b5d3ceb6c54d88292b0fd8299a82bc64aff679a79b7886495d44c0e990ad"
        );

        let tag_s = finished_tag_v3(&k_s2c, b"server finished v3", &th);
        let tag_c = finished_tag_v3(&k_c2s, b"client finished v3", &th);
        assert_eq!(
            hex::encode(tag_s),
            "453edbad5976c5c08e720e6bffb0e111eb117501f26afb0f55b54cc719e38096"
        );
        assert_eq!(
            hex::encode(tag_c),
            "6c748a2e93e297095be36ba757c66bf886ec11fb9833ffdab7cbdcf5aa75d819"
        );

        let completion_hash = completion_hash_v3(&th, &tag_c);
        assert_eq!(
            hex::encode(completion_hash),
            "0291e9e9e586c2d49e35c849ee8147dc263f6f51c0553c571806dfd38ced91ea"
        );
        let tag_ack = completion_tag_v3(&k_complete, &completion_hash);
        assert_eq!(
            hex::encode(tag_ack),
            "5766a1ebf0af40b4da5f8226ec104b78fa47e16c60ad2f46b9a0a95822d39954"
        );

        let changed_session_id = bytes16("10112233445566778899aabbccddeeff");
        let changed_parts = KcTranscriptV3Parts {
            context: AuthV3Context {
                session_id: &changed_session_id,
                ..context
            },
            ..parts
        };
        assert_ne!(kc_transcript_hash_v3(&changed_parts), th);
    }
}
