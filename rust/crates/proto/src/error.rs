//! Structured error codes for the IoT-Auth protocol.
//!
//! Wire-level codes are stable `u16`s so that a C / Python / Go / embedded
//! implementation can decode `PKT_ERROR` payloads identically. The high byte
//! identifies the category, the low byte identifies the specific condition.
//!
//! | Range         | Category                      |
//! |---------------|-------------------------------|
//! | 0x0100-0x01FF | Version / capability          |
//! | 0x0200-0x02FF | Packet framing / parsing      |
//! | 0x0300-0x03FF | Cryptographic validation      |
//! | 0x0400-0x04FF | Session / replay              |
//! | 0x0500-0x05FF | Authorization                 |
//! | 0x0600-0x06FF | Rate limiting / resource      |
//! | 0x0700-0x07FF | Storage / backend             |
//! | 0x7FFF        | Unspecified / internal        |
//!
//! This mapping is part of the wire-format specification (see `spec/`).

use std::fmt;

/// Wire-format error code. Transmitted in the payload of a `PKT_ERROR` frame as
/// `u16` little-endian followed by an optional UTF-8 message.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
#[repr(u16)]
pub enum ErrorCode {
    // Version / capability
    UnsupportedVersion = 0x0101,
    UnsupportedSuite = 0x0102,
    CapabilityMismatch = 0x0103,

    // Packet framing
    MalformedPacket = 0x0201,
    UnknownPacketType = 0x0202,
    PayloadTooLarge = 0x0203,
    PayloadTooShort = 0x0204,
    InvalidEncoding = 0x0205,

    // Cryptographic validation
    InvalidPoint = 0x0301,
    NonCanonicalScalar = 0x0302,
    IdentityPoint = 0x0303,
    ProofVerifyFailed = 0x0304,
    KeyConfirmFailed = 0x0305,
    PeerKeyMismatch = 0x0306,

    // Session / replay
    UnknownSession = 0x0401,
    SessionExpired = 0x0402,
    ReplayDetected = 0x0403,
    SequenceOutOfOrder = 0x0404,

    // Authorization
    UnknownDevice = 0x0501,
    DeviceNotEnrolled = 0x0502,
    RoleNotPermitted = 0x0503,
    PairingTokenInvalid = 0x0504,

    // Rate limiting / resource
    RateLimited = 0x0601,
    ServerBusy = 0x0602,
    TooManyActive = 0x0603,

    // Storage / backend
    StorageFailure = 0x0701,
    CredentialMissing = 0x0702,
    RegistryCorrupt = 0x0703,

    Unspecified = 0x7FFF,
}

impl ErrorCode {
    pub fn as_u16(self) -> u16 {
        self as u16
    }

    pub fn from_u16(code: u16) -> Self {
        match code {
            0x0101 => Self::UnsupportedVersion,
            0x0102 => Self::UnsupportedSuite,
            0x0103 => Self::CapabilityMismatch,
            0x0201 => Self::MalformedPacket,
            0x0202 => Self::UnknownPacketType,
            0x0203 => Self::PayloadTooLarge,
            0x0204 => Self::PayloadTooShort,
            0x0205 => Self::InvalidEncoding,
            0x0301 => Self::InvalidPoint,
            0x0302 => Self::NonCanonicalScalar,
            0x0303 => Self::IdentityPoint,
            0x0304 => Self::ProofVerifyFailed,
            0x0305 => Self::KeyConfirmFailed,
            0x0306 => Self::PeerKeyMismatch,
            0x0401 => Self::UnknownSession,
            0x0402 => Self::SessionExpired,
            0x0403 => Self::ReplayDetected,
            0x0404 => Self::SequenceOutOfOrder,
            0x0501 => Self::UnknownDevice,
            0x0502 => Self::DeviceNotEnrolled,
            0x0503 => Self::RoleNotPermitted,
            0x0504 => Self::PairingTokenInvalid,
            0x0601 => Self::RateLimited,
            0x0602 => Self::ServerBusy,
            0x0603 => Self::TooManyActive,
            0x0701 => Self::StorageFailure,
            0x0702 => Self::CredentialMissing,
            0x0703 => Self::RegistryCorrupt,
            _ => Self::Unspecified,
        }
    }
}

impl fmt::Display for ErrorCode {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "0x{:04x}", self.as_u16())
    }
}

/// Top-level protocol error. Every fallible path in this crate returns this.
/// Any wire-transmittable error carries an `ErrorCode` that can be serialized
/// into a `PKT_ERROR` payload for cross-implementation debuggability.
#[derive(thiserror::Error, Debug)]
pub enum ProtoError {
    #[error("wire error {code}: {msg}")]
    Wire { code: ErrorCode, msg: String },

    #[error("transport error: {0}")]
    Transport(String),

    #[error("storage error: {0}")]
    Storage(String),

    #[error("internal error: {0}")]
    Internal(String),
}

impl ProtoError {
    pub fn wire(code: ErrorCode, msg: impl Into<String>) -> Self {
        Self::Wire {
            code,
            msg: msg.into(),
        }
    }

    pub fn transport(msg: impl Into<String>) -> Self {
        Self::Transport(msg.into())
    }
    pub fn storage(msg: impl Into<String>) -> Self {
        Self::Storage(msg.into())
    }
    pub fn internal(msg: impl Into<String>) -> Self {
        Self::Internal(msg.into())
    }

    /// Extract the wire code, if this error can be transmitted on the wire.
    pub fn wire_code(&self) -> Option<ErrorCode> {
        match self {
            Self::Wire { code, .. } => Some(*code),
            _ => None,
        }
    }

    /// Serialize into a `PKT_ERROR` payload: `code_le(2) || utf8_msg`.
    pub fn to_wire_payload(&self) -> Vec<u8> {
        let (code, msg) = match self {
            Self::Wire { code, msg } => (*code, msg.as_str()),
            Self::Transport(m) => (ErrorCode::Unspecified, m.as_str()),
            Self::Storage(m) => (ErrorCode::StorageFailure, m.as_str()),
            Self::Internal(m) => (ErrorCode::Unspecified, m.as_str()),
        };
        let mut out = Vec::with_capacity(2 + msg.len());
        out.extend_from_slice(&code.as_u16().to_le_bytes());
        out.extend_from_slice(msg.as_bytes());
        out
    }

    /// Decode a `PKT_ERROR` payload.
    pub fn from_wire_payload(bytes: &[u8]) -> Self {
        if bytes.len() < 2 {
            return Self::wire(ErrorCode::MalformedPacket, "error payload too short");
        }
        let code = ErrorCode::from_u16(u16::from_le_bytes([bytes[0], bytes[1]]));
        let msg = std::str::from_utf8(&bytes[2..]).unwrap_or("<non-utf8>");
        Self::Wire {
            code,
            msg: msg.into(),
        }
    }
}

impl From<std::io::Error> for ProtoError {
    fn from(e: std::io::Error) -> Self {
        Self::transport(e.to_string())
    }
}

pub type Result<T> = std::result::Result<T, ProtoError>;
