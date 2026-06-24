//! Payload-level codecs for SETUP_1..SETUP_ACK and AUTH_1..AUTH_ACK.
//!
//! The header (`wire::Header`) is handled in `wire.rs`. This module concerns
//! only the *payload bytes* carried inside a framed packet.
//!
//! All layouts here are part of the wire-format specification.

use curve25519_dalek::ristretto::RistrettoPoint;
use curve25519_dalek::scalar::Scalar;

use crate::crypto::{
    decode_scalar, decompress_point, SchnorrProof, SetBranch, SETUP_CHALLENGE_LEN,
};
use crate::error::{ErrorCode, ProtoError, Result};

// ---- Helpers ----

pub(crate) fn put_point(out: &mut Vec<u8>, p: &RistrettoPoint) {
    out.extend_from_slice(p.compress().as_bytes());
}

pub(crate) fn put_scalar(out: &mut Vec<u8>, s: &Scalar) {
    out.extend_from_slice(&s.to_bytes());
}

pub(crate) fn take_fixed<'a, const N: usize>(buf: &'a [u8]) -> Result<(&'a [u8; N], &'a [u8])> {
    if buf.len() < N {
        return Err(ProtoError::wire(ErrorCode::PayloadTooShort, "short read"));
    }
    // SAFETY: slice length is N.
    let (head, tail) = buf.split_at(N);
    Ok((head.try_into().unwrap(), tail))
}

pub(crate) fn take_point<'a>(buf: &'a [u8], what: &'static str) -> Result<(RistrettoPoint, &'a [u8])> {
    let (b, rest) = take_fixed::<32>(buf)?;
    Ok((decompress_point(b, what)?, rest))
}

pub(crate) fn take_scalar<'a>(buf: &'a [u8], what: &'static str) -> Result<(Scalar, &'a [u8])> {
    let (b, rest) = take_fixed::<32>(buf)?;
    Ok((decode_scalar(b, what)?, rest))
}

// ---- SETUP_1 (client -> server) ----

/// `SETUP_1` payload layout:
/// ```text
/// u8  token_len
/// token_len bytes  pairing_token (may be empty)
/// 32  device_id
/// 32  device_pub
/// 32  client_nonce
/// 32  role_commitment
/// ```
pub struct Setup1 {
    pub pairing_token:    Option<Vec<u8>>,
    pub device_id:        [u8; 32],
    pub device_pub:       RistrettoPoint,
    pub client_nonce:     [u8; 32],
    pub role_commitment:  RistrettoPoint,
}

impl Setup1 {
    pub fn encode(&self) -> Vec<u8> {
        let mut out = Vec::with_capacity(1 + 128 + 32 + 32);
        let t = self.pairing_token.as_deref().unwrap_or(&[]);
        assert!(t.len() <= 128, "pairing token too long");
        out.push(t.len() as u8);
        out.extend_from_slice(t);
        out.extend_from_slice(&self.device_id);
        put_point(&mut out, &self.device_pub);
        out.extend_from_slice(&self.client_nonce);
        put_point(&mut out, &self.role_commitment);
        out
    }

    pub fn decode(mut buf: &[u8]) -> Result<Self> {
        if buf.is_empty() {
            return Err(ProtoError::wire(ErrorCode::PayloadTooShort, "SETUP_1 empty"));
        }
        let tlen = buf[0] as usize;
        buf = &buf[1..];
        if tlen > 128 {
            return Err(ProtoError::wire(ErrorCode::MalformedPacket, "pairing token len > 128"));
        }
        let pairing_token = if tlen == 0 { None } else {
            let (t, rest) = buf.split_at(tlen.min(buf.len()));
            if t.len() != tlen {
                return Err(ProtoError::wire(ErrorCode::PayloadTooShort, "pairing token truncated"));
            }
            buf = rest;
            Some(t.to_vec())
        };

        let (device_id, rest)  = take_fixed::<32>(buf)?;
        let (device_pub, rest) = take_point(rest, "device_pub")?;
        let (client_nonce, rest) = take_fixed::<32>(rest)?;
        let (role_commitment, _rest) = take_point(rest, "role_commitment")?;

        Ok(Self {
            pairing_token,
            device_id: *device_id,
            device_pub,
            client_nonce: *client_nonce,
            role_commitment,
        })
    }
}

// ---- SETUP_2 (server -> client) ----

/// `SETUP_2` payload:
/// ```text
/// 32  server_nonce
/// 16  setup_challenge
/// 32  server_pub
/// 32  a_s
/// 32  s_s
/// ```
pub struct Setup2 {
    pub server_nonce:    [u8; 32],
    pub setup_challenge: [u8; SETUP_CHALLENGE_LEN],
    pub server_pub:      RistrettoPoint,
    pub server_proof:    SchnorrProof,
}

impl Setup2 {
    pub fn encode(&self) -> Vec<u8> {
        let mut out = Vec::with_capacity(32 + SETUP_CHALLENGE_LEN + 32 * 3);
        out.extend_from_slice(&self.server_nonce);
        out.extend_from_slice(&self.setup_challenge);
        put_point (&mut out, &self.server_pub);
        put_point (&mut out, &self.server_proof.a);
        put_scalar(&mut out, &self.server_proof.s);
        out
    }

    pub fn decode(buf: &[u8]) -> Result<Self> {
        let (sn, rest) = take_fixed::<32>(buf)?;
        let (sc, rest) = take_fixed::<SETUP_CHALLENGE_LEN>(rest)?;
        let (server_pub, rest) = take_point(rest, "server_pub")?;
        let (a, rest) = take_point(rest, "setup a_s")?;
        let (s, _rest) = take_scalar(rest, "setup s_s")?;
        Ok(Self {
            server_nonce: *sn,
            setup_challenge: *sc,
            server_pub,
            server_proof: SchnorrProof { a, s },
        })
    }
}

// ---- SETUP_3 (client -> server) ----

/// `SETUP_3` payload: `a_c (32) || s_c (32)`.
pub struct Setup3 {
    pub client_proof: SchnorrProof,
}

impl Setup3 {
    pub fn encode(&self) -> Vec<u8> {
        let mut out = Vec::with_capacity(64);
        put_point (&mut out, &self.client_proof.a);
        put_scalar(&mut out, &self.client_proof.s);
        out
    }
    pub fn decode(buf: &[u8]) -> Result<Self> {
        let (a, rest) = take_point(buf, "setup a_c")?;
        let (s, _)    = take_scalar(rest, "setup s_c")?;
        Ok(Self { client_proof: SchnorrProof { a, s } })
    }
}

// ---- AUTH_1 (client -> server) ----

/// `AUTH_1` payload:
/// ```text
/// 32  pid
/// 32  a_c
/// 32  s_c
/// 32  nonce_c
/// 32  eph_c
/// 32  c_prime
/// 32  rerand_a
/// 32  rerand_s
/// u16 branches_len (LE)
/// branches_len * (32 A_i | 32 c_i | 32 s_i)
/// ```
pub struct Auth1 {
    pub pid:          [u8; 32],
    pub client_proof: SchnorrProof,
    pub nonce_c:      [u8; 32],
    pub eph_c:        RistrettoPoint,
    pub c_prime:      RistrettoPoint,
    pub rerand_proof: SchnorrProof,
    pub branches:     Vec<SetBranch>,
}

impl Auth1 {
    pub fn encode(&self) -> Vec<u8> {
        let n = self.branches.len();
        let mut out = Vec::with_capacity(32 * 8 + 2 + 96 * n);
        out.extend_from_slice(&self.pid);
        put_point (&mut out, &self.client_proof.a);
        put_scalar(&mut out, &self.client_proof.s);
        out.extend_from_slice(&self.nonce_c);
        put_point (&mut out, &self.eph_c);
        put_point (&mut out, &self.c_prime);
        put_point (&mut out, &self.rerand_proof.a);
        put_scalar(&mut out, &self.rerand_proof.s);
        out.extend_from_slice(&(n as u16).to_le_bytes());
        for (a, c, s) in &self.branches {
            put_point (&mut out, a);
            put_scalar(&mut out, c);
            put_scalar(&mut out, s);
        }
        out
    }

    pub fn decode(buf: &[u8]) -> Result<Self> {
        let (pid, rest) = take_fixed::<32>(buf)?;
        let (ac, rest)  = take_point (rest, "a_c")?;
        let (sc, rest)  = take_scalar(rest, "s_c")?;
        let (nc, rest)  = take_fixed::<32>(rest)?;
        let (eph_c, rest) = take_point(rest, "eph_c")?;
        let (c_prime, rest) = take_point(rest, "c_prime")?;
        let (rand_a, rest) = take_point (rest, "rerand_a")?;
        let (rand_s, rest) = take_scalar(rest, "rerand_s")?;

        if rest.len() < 2 {
            return Err(ProtoError::wire(ErrorCode::PayloadTooShort, "branch count missing"));
        }
        let n = u16::from_le_bytes([rest[0], rest[1]]) as usize;
        let mut rest = &rest[2..];
        if rest.len() < 96 * n {
            return Err(ProtoError::wire(ErrorCode::PayloadTooShort, "branch payload truncated"));
        }
        let mut branches = Vec::with_capacity(n);
        for _ in 0..n {
            let (a, r2) = take_point (rest, "branch A")?;
            let (c, r2) = take_scalar(r2,   "branch c")?;
            let (s, r2) = take_scalar(r2,   "branch s")?;
            branches.push((a, c, s));
            rest = r2;
        }

        Ok(Self {
            pid: *pid,
            client_proof: SchnorrProof { a: ac, s: sc },
            nonce_c: *nc,
            eph_c,
            c_prime,
            rerand_proof: SchnorrProof { a: rand_a, s: rand_s },
            branches,
        })
    }
}

// ---- AUTH_2 (server -> client) ----

/// `AUTH_2` payload: `server_pub(32) | a_s(32) | s_s(32) | nonce_s(32) | eph_s(32) | tag_s(32)`
pub struct Auth2 {
    pub server_pub:   RistrettoPoint,
    pub server_proof: SchnorrProof,
    pub nonce_s:      [u8; 32],
    pub eph_s:        RistrettoPoint,
    pub tag_s:        [u8; 32],
}

impl Auth2 {
    pub fn encode(&self) -> Vec<u8> {
        let mut out = Vec::with_capacity(192);
        put_point (&mut out, &self.server_pub);
        put_point (&mut out, &self.server_proof.a);
        put_scalar(&mut out, &self.server_proof.s);
        out.extend_from_slice(&self.nonce_s);
        put_point (&mut out, &self.eph_s);
        out.extend_from_slice(&self.tag_s);
        out
    }

    pub fn decode(buf: &[u8]) -> Result<Self> {
        let (sp, rest) = take_point (buf,  "server_pub")?;
        let (a,  rest) = take_point (rest, "a_s")?;
        let (s,  rest) = take_scalar(rest, "s_s")?;
        let (ns, rest) = take_fixed::<32>(rest)?;
        let (es, rest) = take_point (rest, "eph_s")?;
        let (ts, _)    = take_fixed::<32>(rest)?;
        Ok(Self { server_pub: sp, server_proof: SchnorrProof { a, s }, nonce_s: *ns, eph_s: es, tag_s: *ts })
    }
}

// ---- AUTH_3 (client -> server) ----

/// `AUTH_3` payload: `tag_c(32)`.
pub struct Auth3 {
    pub tag_c: [u8; 32],
}

impl Auth3 {
    pub fn encode(&self) -> Vec<u8> { self.tag_c.to_vec() }
    pub fn decode(buf: &[u8]) -> Result<Self> {
        let (tag, _) = take_fixed::<32>(buf)?;
        Ok(Self { tag_c: *tag })
    }
}

// ---- ACKs ----

/// `SETUP_ACK` and `AUTH_ACK` both carry a single `0x01` byte.
pub const ACK_BYTE: u8 = 0x01;

pub fn encode_ack() -> Vec<u8> { vec![ACK_BYTE] }

pub fn decode_ack(buf: &[u8]) -> Result<()> {
    if buf.len() != 1 || buf[0] != ACK_BYTE {
        return Err(ProtoError::wire(ErrorCode::MalformedPacket, "bad ACK"));
    }
    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;
    use curve25519_dalek::constants::RISTRETTO_BASEPOINT_POINT;

    fn dummy_scalar() -> Scalar { Scalar::from(7u64) }

    #[test]
    fn setup1_roundtrip() {
        let msg = Setup1 {
            pairing_token: Some(b"abc".to_vec()),
            device_id: [1u8; 32],
            device_pub: RISTRETTO_BASEPOINT_POINT,
            client_nonce: [2u8; 32],
            role_commitment: RISTRETTO_BASEPOINT_POINT,
        };
        let bytes = msg.encode();
        let round = Setup1::decode(&bytes).unwrap();
        assert_eq!(round.device_id, [1u8; 32]);
        assert_eq!(round.pairing_token.as_deref(), Some(&b"abc"[..]));
    }

    #[test]
    fn auth1_roundtrip() {
        let proof = SchnorrProof { a: RISTRETTO_BASEPOINT_POINT, s: dummy_scalar() };
        let msg = Auth1 {
            pid: [9u8; 32],
            client_proof: proof,
            nonce_c: [3u8; 32],
            eph_c: RISTRETTO_BASEPOINT_POINT,
            c_prime: RISTRETTO_BASEPOINT_POINT,
            rerand_proof: proof,
            branches: vec![
                (RISTRETTO_BASEPOINT_POINT, dummy_scalar(), dummy_scalar()),
                (RISTRETTO_BASEPOINT_POINT, dummy_scalar(), dummy_scalar()),
            ],
        };
        let bytes = msg.encode();
        let round = Auth1::decode(&bytes).unwrap();
        assert_eq!(round.branches.len(), 2);
        assert_eq!(round.nonce_c, [3u8; 32]);
    }
}
