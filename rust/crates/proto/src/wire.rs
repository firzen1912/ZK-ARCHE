//! Wire format — framing, packet types, and TLV codec.
//!
//! This is the **transport-neutral** packet layout. The same bytes travel
//! unchanged whether the transport binding is UDP, TCP, CoAP, BLE-L2CAP, or a
//! serial gateway. All binary fields are little-endian, points are 32-byte
//! compressed Ristretto255, scalars are 32-byte canonical form.
//!
//! ## Header (24 bytes)
//!
//! ```text
//! offset  size  field
//!   0      1    version          (u8, = PROTOCOL_VERSION)
//!   1      1    pkt_type         (u8, see PKT_* constants)
//!   2      2    flags            (u16 LE, see FLAG_* constants; reserved=0)
//!   4     16    session_id       (16 random bytes, peer-scoped)
//!  20      4    seq              (u32 LE, monotonically increasing)
//!  24     ..    payload
//! ```
//!
//! ## Payload layout
//!
//! Payloads use a deterministic fixed-field layout (compact binary). Where an
//! extensibility point is needed, the trailing bytes may carry a TLV list:
//!
//! ```text
//! tlv := u16_LE tag | u16_LE len | len bytes value
//! ```
//!
//! Reserved tags are listed in `Tlv`. Unknown tags MUST be ignored (skip).
//!
//! ## Packet types
//!
//! | Code  | Name         | Direction | Description                         |
//! |-------|--------------|-----------|-------------------------------------|
//! | 0x01  | HELLO        | C -> S    | Capability / version probe          |
//! | 0x02  | HELLO_REPLY  | S -> C    | Negotiated version/suite/caps       |
//! | 0x11  | SETUP_1      | C -> S    | Setup init (device_id, device_pub..) |
//! | 0x12  | SETUP_2      | S -> C    | Server challenge + server proof      |
//! | 0x13  | SETUP_3      | C -> S    | Client setup proof                   |
//! | 0x14  | SETUP_ACK    | S -> C    | Enrollment OK                        |
//! | 0x21  | AUTH_1       | C -> S    | Auth: pid + client proof + role proofs |
//! | 0x22  | AUTH_2       | S -> C    | Server proof + nonce_s + tag_s       |
//! | 0x23  | AUTH_3       | C -> S    | Client finished tag                  |
//! | 0x24  | AUTH_ACK     | S -> C    | Auth OK                              |
//! | 0x7f  | ERROR        | either    | `u16` code + utf8 message            |
//!
//! This layout is part of the spec. Do not reassign values.

use crate::error::{ErrorCode, ProtoError, Result};

// ---- Constants ----

pub const HEADER_LEN: usize = 24;
pub const SESSION_ID_LEN: usize = 16;
pub const MAX_DATAGRAM: usize = 2048;
pub const MAX_PAYLOAD: usize = MAX_DATAGRAM - HEADER_LEN;

// Packet types
pub const PKT_HELLO: u8 = 0x01;
pub const PKT_HELLO_REPLY: u8 = 0x02;
pub const PKT_SETUP_1: u8 = 0x11;
pub const PKT_SETUP_2: u8 = 0x12;
pub const PKT_SETUP_3: u8 = 0x13;
pub const PKT_SETUP_ACK: u8 = 0x14;
pub const PKT_AUTH_1: u8 = 0x21;
pub const PKT_AUTH_2: u8 = 0x22;
pub const PKT_AUTH_3: u8 = 0x23;
pub const PKT_AUTH_ACK: u8 = 0x24;
pub const PKT_ERROR: u8 = 0x7f;

// Flags (u16 LE)
pub const FLAG_NONE: u16 = 0x0000;
/// Indicates the packet is a retransmission. Peers MUST accept retransmitted
/// packets with the same `(session_id, seq)` and idempotently re-send the
/// cached response.
pub const FLAG_RETRANSMIT: u16 = 0x0001;

// TLV tags (for extensibility in HELLO / HELLO_REPLY / future messages)
pub mod tlv_tag {
    pub const MIN_VERSION: u16 = 0x0001;
    pub const SUITE_LIST: u16 = 0x0002;
    pub const CAPS: u16 = 0x0003;
    pub const MTU_HINT: u16 = 0x0004;
    pub const VENDOR_ID: u16 = 0x0100;
    pub const DEVICE_MODEL: u16 = 0x0101;
}

// ---- Header ----

#[derive(Clone, Copy, Debug)]
pub struct Header {
    pub version: u8,
    pub pkt_type: u8,
    pub flags: u16,
    pub session_id: [u8; SESSION_ID_LEN],
    pub seq: u32,
}

impl Header {
    pub fn new(pkt_type: u8, session_id: [u8; SESSION_ID_LEN], seq: u32) -> Self {
        Self {
            version: crate::caps::PROTOCOL_VERSION,
            pkt_type,
            flags: FLAG_NONE,
            session_id,
            seq,
        }
    }

    pub fn encode_into(&self, out: &mut Vec<u8>) {
        out.push(self.version);
        out.push(self.pkt_type);
        out.extend_from_slice(&self.flags.to_le_bytes());
        out.extend_from_slice(&self.session_id);
        out.extend_from_slice(&self.seq.to_le_bytes());
    }

    pub fn decode(bytes: &[u8]) -> Result<(Header, &[u8])> {
        if bytes.len() < HEADER_LEN {
            return Err(ProtoError::wire(
                ErrorCode::PayloadTooShort,
                "datagram < 24B header",
            ));
        }
        let version = bytes[0];
        let pkt_type = bytes[1];
        let flags = u16::from_le_bytes([bytes[2], bytes[3]]);
        let mut session_id = [0u8; SESSION_ID_LEN];
        session_id.copy_from_slice(&bytes[4..4 + SESSION_ID_LEN]);
        let seq = u32::from_le_bytes([bytes[20], bytes[21], bytes[22], bytes[23]]);

        // Version gating is done at negotiation time, but reject obviously
        // wrong headers here.
        if version < crate::caps::MIN_SUPPORTED_VERSION {
            return Err(ProtoError::wire(
                ErrorCode::UnsupportedVersion,
                format!(
                    "packet version {version} < min supported {}",
                    crate::caps::MIN_SUPPORTED_VERSION
                ),
            ));
        }
        Ok((
            Header {
                version,
                pkt_type,
                flags,
                session_id,
                seq,
            },
            &bytes[HEADER_LEN..],
        ))
    }
}

/// Build a complete framed packet: `header || payload`.
pub fn build_packet(
    pkt_type: u8,
    session_id: &[u8; SESSION_ID_LEN],
    seq: u32,
    payload: &[u8],
) -> Vec<u8> {
    if payload.len() > MAX_PAYLOAD {
        // Caller is expected to have validated this; panic would be a bug.
        panic!(
            "payload {} exceeds MAX_PAYLOAD {MAX_PAYLOAD}",
            payload.len()
        );
    }
    let mut out = Vec::with_capacity(HEADER_LEN + payload.len());
    Header::new(pkt_type, *session_id, seq).encode_into(&mut out);
    out.extend_from_slice(payload);
    out
}

/// Build an ERROR packet with a structured code and message.
pub fn build_error(
    session_id: &[u8; SESSION_ID_LEN],
    seq: u32,
    code: ErrorCode,
    msg: &str,
) -> Vec<u8> {
    let mut payload = Vec::with_capacity(2 + msg.len());
    payload.extend_from_slice(&code.as_u16().to_le_bytes());
    payload.extend_from_slice(msg.as_bytes());
    build_packet(PKT_ERROR, session_id, seq, &payload)
}

/// Parse a framed packet.
pub fn parse_packet(bytes: &[u8]) -> Result<(Header, &[u8])> {
    Header::decode(bytes)
}

// ---- TLV codec ----

pub struct TlvWriter<'a> {
    out: &'a mut Vec<u8>,
}

impl<'a> TlvWriter<'a> {
    pub fn new(out: &'a mut Vec<u8>) -> Self {
        Self { out }
    }

    pub fn put(&mut self, tag: u16, value: &[u8]) -> Result<()> {
        if value.len() > u16::MAX as usize {
            return Err(ProtoError::wire(
                ErrorCode::PayloadTooLarge,
                "TLV value > u16",
            ));
        }
        self.out.extend_from_slice(&tag.to_le_bytes());
        self.out
            .extend_from_slice(&(value.len() as u16).to_le_bytes());
        self.out.extend_from_slice(value);
        Ok(())
    }
}

pub struct TlvIter<'a> {
    buf: &'a [u8],
}

impl<'a> TlvIter<'a> {
    pub fn new(buf: &'a [u8]) -> Self {
        Self { buf }
    }
}

impl<'a> Iterator for TlvIter<'a> {
    type Item = Result<(u16, &'a [u8])>;
    fn next(&mut self) -> Option<Self::Item> {
        if self.buf.is_empty() {
            return None;
        }
        if self.buf.len() < 4 {
            let e = ProtoError::wire(ErrorCode::MalformedPacket, "truncated TLV header");
            self.buf = &[];
            return Some(Err(e));
        }
        let tag = u16::from_le_bytes([self.buf[0], self.buf[1]]);
        let len = u16::from_le_bytes([self.buf[2], self.buf[3]]) as usize;
        if self.buf.len() < 4 + len {
            let e = ProtoError::wire(ErrorCode::MalformedPacket, "truncated TLV value");
            self.buf = &[];
            return Some(Err(e));
        }
        let value = &self.buf[4..4 + len];
        self.buf = &self.buf[4 + len..];
        Some(Ok((tag, value)))
    }
}

// ---- HELLO / HELLO_REPLY payloads ----
//
// HELLO payload layout:
//   u8  version
//   u16 suite_count
//   [u16 suite_id] * suite_count
//   u64 caps (LE)
//   TLV*   extensions (MIN_VERSION, MTU_HINT, VENDOR_ID, DEVICE_MODEL, ...)
//
// HELLO_REPLY uses the same layout. On mismatch, the server SHOULD instead
// reply with PKT_ERROR carrying UnsupportedVersion / UnsupportedSuite /
// CapabilityMismatch.

use crate::caps::{cap, SuiteId};

#[derive(Clone, Debug)]
pub struct Hello {
    pub version: u8,
    pub min_version: u8,
    pub suites: Vec<SuiteId>,
    pub caps: cap::Bits,
    pub mtu_hint: Option<u16>,
    pub vendor_id: Option<Vec<u8>>,
    pub device_model: Option<Vec<u8>>,
}

impl Hello {
    pub fn encode(&self) -> Vec<u8> {
        let mut out = Vec::with_capacity(64);
        out.push(self.version);
        out.extend_from_slice(&(self.suites.len() as u16).to_le_bytes());
        for s in &self.suites {
            out.extend_from_slice(&s.to_le_bytes());
        }
        out.extend_from_slice(&self.caps.to_le_bytes());

        // Extensions
        let mut w = TlvWriter::new(&mut out);
        w.put(tlv_tag::MIN_VERSION, &[self.min_version]).unwrap();
        if let Some(mtu) = self.mtu_hint {
            w.put(tlv_tag::MTU_HINT, &mtu.to_le_bytes()).unwrap();
        }
        if let Some(v) = &self.vendor_id {
            w.put(tlv_tag::VENDOR_ID, v).unwrap();
        }
        if let Some(m) = &self.device_model {
            w.put(tlv_tag::DEVICE_MODEL, m).unwrap();
        }
        out
    }

    pub fn decode(bytes: &[u8]) -> Result<Self> {
        if bytes.len() < 1 + 2 {
            return Err(ProtoError::wire(
                ErrorCode::PayloadTooShort,
                "HELLO truncated",
            ));
        }
        let version = bytes[0];
        let n = u16::from_le_bytes([bytes[1], bytes[2]]) as usize;
        let suites_end = 3 + 2 * n;
        if bytes.len() < suites_end + 8 {
            return Err(ProtoError::wire(
                ErrorCode::PayloadTooShort,
                "HELLO suite list truncated",
            ));
        }
        let mut suites = Vec::with_capacity(n);
        for i in 0..n {
            let off = 3 + 2 * i;
            suites.push(u16::from_le_bytes([bytes[off], bytes[off + 1]]));
        }
        let mut caps_bytes = [0u8; 8];
        caps_bytes.copy_from_slice(&bytes[suites_end..suites_end + 8]);
        let caps = u64::from_le_bytes(caps_bytes);

        let mut min_version = version;
        let mut mtu_hint = None;
        let mut vendor_id = None;
        let mut device_model = None;

        for tlv in TlvIter::new(&bytes[suites_end + 8..]) {
            let (tag, value) = tlv?;
            match tag {
                t if t == tlv_tag::MIN_VERSION => {
                    if value.len() != 1 {
                        return Err(ProtoError::wire(
                            ErrorCode::MalformedPacket,
                            "MIN_VERSION must be 1 byte",
                        ));
                    }
                    min_version = value[0];
                }
                t if t == tlv_tag::MTU_HINT => {
                    if value.len() != 2 {
                        return Err(ProtoError::wire(
                            ErrorCode::MalformedPacket,
                            "MTU_HINT must be 2 bytes",
                        ));
                    }
                    mtu_hint = Some(u16::from_le_bytes([value[0], value[1]]));
                }
                t if t == tlv_tag::VENDOR_ID => {
                    vendor_id = Some(value.to_vec());
                }
                t if t == tlv_tag::DEVICE_MODEL => {
                    device_model = Some(value.to_vec());
                }
                _ => { /* Unknown tag: ignore per spec. */ }
            }
        }

        Ok(Self {
            version,
            min_version,
            suites,
            caps,
            mtu_hint,
            vendor_id,
            device_model,
        })
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::caps::SUITE_RISTRETTO255_SHA256;

    #[test]
    fn header_roundtrip() {
        let mut out = Vec::new();
        Header::new(PKT_HELLO, [7u8; 16], 42).encode_into(&mut out);
        let (h, rest) = Header::decode(&out).unwrap();
        assert_eq!(h.pkt_type, PKT_HELLO);
        assert_eq!(h.session_id, [7u8; 16]);
        assert_eq!(h.seq, 42);
        assert!(rest.is_empty());
    }

    #[test]
    fn hello_roundtrip_ignores_unknown_tlv() {
        let hello = Hello {
            version: 2,
            min_version: 2,
            suites: vec![SUITE_RISTRETTO255_SHA256],
            caps: cap::BASELINE,
            mtu_hint: Some(1200),
            vendor_id: Some(b"acme".to_vec()),
            device_model: None,
        };
        let mut bytes = hello.encode();
        // Append an unknown TLV — decoder must skip it.
        let mut w = TlvWriter::new(&mut bytes);
        w.put(0xFFFE, b"future-feature").unwrap();

        let round = Hello::decode(&bytes).unwrap();
        assert_eq!(round.version, 2);
        assert_eq!(round.suites, vec![SUITE_RISTRETTO255_SHA256]);
        assert_eq!(round.mtu_hint, Some(1200));
        assert_eq!(round.vendor_id.as_deref(), Some(&b"acme"[..]));
    }

    #[test]
    fn rejects_old_version() {
        let mut bytes = vec![0x00u8; HEADER_LEN]; // version=0
        bytes[0] = 0x00;
        let e = Header::decode(&bytes).unwrap_err();
        assert_eq!(e.wire_code(), Some(ErrorCode::UnsupportedVersion));
    }
}
