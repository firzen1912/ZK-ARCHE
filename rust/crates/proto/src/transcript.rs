//! Canonical transcript construction for Schnorr challenges, KC hashes, and
//! PID derivation. These byte layouts are part of the wire-format spec;
//! changing them breaks cross-implementation interop.
//!
//! ## Canonical encoding rules
//!
//! * domain length: `u8`
//! * label length:  `u8`
//! * message length: little-endian `u32`
//! * points:        32-byte compressed Ristretto255
//! * scalars:       32-byte canonical form (`Scalar::to_bytes`)
//! * `u8`:          1 byte
//! * `u64`:         little-endian 8 bytes
//!
//! A peer implementing this in C/Python/Go MUST produce byte-identical
//! transcripts. See `test-vectors/transcripts.json`.

use curve25519_dalek::ristretto::RistrettoPoint;
use curve25519_dalek::scalar::Scalar;
use sha2::{Digest, Sha256, Sha512};

/// Deterministic transcript buffer used for Fiat-Shamir challenges and KC
/// hashing. Append-only; hashed with SHA-512 for scalar challenges and SHA-256
/// for 32-byte transcript hashes.
pub struct Transcript {
    buf: Vec<u8>,
}

impl Transcript {
    pub fn new(domain: &[u8]) -> Self {
        assert!(domain.len() <= 255, "domain too long");
        let mut buf = Vec::with_capacity(512);
        buf.push(domain.len() as u8);
        buf.extend_from_slice(domain);
        Self { buf }
    }

    pub fn append_message(&mut self, label: &[u8], msg: &[u8]) {
        assert!(label.len() <= 255, "label too long");
        self.buf.push(label.len() as u8);
        self.buf.extend_from_slice(label);
        self.buf.extend_from_slice(&(msg.len() as u32).to_le_bytes());
        self.buf.extend_from_slice(msg);
    }

    pub fn append_u8(&mut self, label: &[u8], v: u8) {
        self.append_message(label, &[v]);
    }

    pub fn append_u64(&mut self, label: &[u8], v: u64) {
        self.append_message(label, &v.to_le_bytes());
    }

    pub fn append_point(&mut self, label: &[u8], p: &RistrettoPoint) {
        self.append_message(label, p.compress().as_bytes());
    }

    pub fn append_scalar(&mut self, label: &[u8], s: &Scalar) {
        self.append_message(label, &s.to_bytes());
    }

    /// Hash to a Schnorr challenge scalar via SHA-512 wide reduction.
    pub fn challenge_scalar(&self) -> Scalar {
        let digest = Sha512::digest(&self.buf);
        let mut wide = [0u8; 64];
        wide.copy_from_slice(&digest);
        Scalar::from_bytes_mod_order_wide(&wide)
    }

    /// Hash to a 32-byte transcript digest via SHA-256. Used for KC binding.
    pub fn hash_sha256(&self) -> [u8; 32] {
        let mut out = [0u8; 32];
        out.copy_from_slice(&Sha256::digest(&self.buf));
        out
    }

    /// Expose the raw buffer for test-vector publication. Do not use for
    /// anything else.
    pub fn as_bytes(&self) -> &[u8] {
        &self.buf
    }
}

/// Typed transcript values. Keeps call-sites declarative and prevents the
/// ordering mistakes that would otherwise silently break interop.
#[derive(Clone, Copy)]
pub enum TVal<'a> {
    Bytes(&'a [u8]),
    U8(u8),
    U64(u64),
    Point(&'a RistrettoPoint),
    Scalar(&'a Scalar),
}

pub fn build(domain: &[u8], fields: &[(&[u8], TVal<'_>)]) -> Transcript {
    let mut t = Transcript::new(domain);
    for (label, v) in fields {
        match v {
            TVal::Bytes(b)  => t.append_message(label, b),
            TVal::U8(n)     => t.append_u8(label, *n),
            TVal::U64(n)    => t.append_u64(label, *n),
            TVal::Point(p)  => t.append_point(label, p),
            TVal::Scalar(s) => t.append_scalar(label, s),
        }
    }
    t
}

pub fn challenge(domain: &[u8], fields: &[(&[u8], TVal<'_>)]) -> Scalar {
    build(domain, fields).challenge_scalar()
}

// ---- Domain separators (wire-stable constants) ----

// Enrollment (setup) — stable since v1.
pub const T_SETUP:        &[u8] = b"setup_client_schnorr_v1";
pub const T_SETUP_SERVER: &[u8] = b"setup_server_schnorr_v1";
pub const T_SERVER:       &[u8] = b"server_schnorr_v1";

// ZK-ARCHE v2 online auth.
pub const T_PID:          &[u8] = b"iot-auth/pid/v1";
pub const T_CLIENT_V2:    &[u8] = b"client_schnorr_v2";
pub const T_KC_V2:        &[u8] = b"kc_v2";
pub const T_ROLE_SET:     &[u8] = b"client_role_set_v1";
pub const T_ROLE_RERAND:  &[u8] = b"client_role_rerand_v1";

/// PID = SHA-256( len(T_PID) || T_PID || device_pub || nonce_c || eph_c || server_pub ).
/// Encoded exactly as in the reference implementation; used as the session
/// pseudonym for all subsequent transcript bindings.
pub fn compute_pid(
    device_pub: &RistrettoPoint,
    nonce_c: &[u8; 32],
    eph_c: &RistrettoPoint,
    server_pub: &RistrettoPoint,
) -> [u8; 32] {
    let mut h = Sha256::new();
    h.update((T_PID.len() as u32).to_le_bytes());
    h.update(T_PID);
    h.update(device_pub.compress().as_bytes());
    h.update(nonce_c);
    h.update(eph_c.compress().as_bytes());
    h.update(server_pub.compress().as_bytes());
    let mut out = [0u8; 32];
    out.copy_from_slice(&h.finalize());
    out
}

#[cfg(test)]
mod tests {
    use super::*;
    use curve25519_dalek::constants::RISTRETTO_BASEPOINT_POINT;

    #[test]
    fn transcript_layout_is_byte_stable() {
        let mut t = Transcript::new(b"test-domain");
        t.append_message(b"x", b"hello");
        // [0b=11, "test-domain"=11b, label_len=1, label="x", len=5 LE, "hello"]
        let expected: &[u8] = &[
            11, b't', b'e', b's', b't', b'-', b'd', b'o', b'm', b'a', b'i', b'n',
            1, b'x',
            5, 0, 0, 0,
            b'h', b'e', b'l', b'l', b'o',
        ];
        assert_eq!(t.as_bytes(), expected);
    }

    #[test]
    fn pid_is_deterministic() {
        let g = RISTRETTO_BASEPOINT_POINT;
        let pid1 = compute_pid(&g, &[7u8; 32], &g, &g);
        let pid2 = compute_pid(&g, &[7u8; 32], &g, &g);
        assert_eq!(pid1, pid2);
        let pid3 = compute_pid(&g, &[8u8; 32], &g, &g);
        assert_ne!(pid1, pid3);
    }
}
