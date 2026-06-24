//! # proto
//!
//! Transport-agnostic implementation of the ZK-ARCHE v2 IoT authentication
//! protocol. The crate is organized into independent layers so that
//! heterogeneous implementations (different languages, hardware, transports)
//! can interoperate by matching the wire-format spec without sharing code.
//!
//! ## Module map
//!
//! | Module        | Responsibility                                   |
//! |---------------|--------------------------------------------------|
//! | `caps`        | Protocol version, suite IDs, capability bits     |
//! | `error`       | Structured errors + wire error codes             |
//! | `transcript`  | Canonical transcript builder + domain separators |
//! | `crypto`      | Schnorr / rerand / OR-proof / HKDF / HMAC        |
//! | `wire`        | Packet header, framing, TLV codec                |
//! | `transport`   | Transport abstraction (UDP, TCP, ...)            |
//! | `store`       | Credential / registry / replay-cache traits      |
//! | `profile`     | Timing + resource profiles                       |
//! | `proto`       | Layer-A state machines (setup, auth)             |
//!
//! ## Protocol summary
//!
//! * **Setup** (enrollment): client proves knowledge of `x` such that
//!   `device_pub = g^x`, server proves knowledge of its static secret. The
//!   client pins the server's raw public key.
//! * **Auth**: every session derives `pid = H(device_pub || nonce_c || eph_c
//!   || server_pub)` and binds every transcript to `pid` (not `device_id`).
//!   The client additionally re-randomizes its role commitment and produces
//!   a CDS-OR set-membership proof that the committed role lies in the
//!   allowed set. Both sides run key confirmation.
//!
//! See `spec/iot-auth-wire-spec.docx` for the full specification.

pub mod caps;
pub mod crypto;
pub mod error;
pub mod profile;
pub mod proto;
pub mod store;
pub mod transcript;
pub mod transport;
pub mod wire;

// Convenience re-exports.
pub use error::{ErrorCode, ProtoError, Result};
pub use profile::{Profile, ProfileKind};

/// Default list of allowed role codes for the CDS role-set proof. Both peers
/// MUST agree on this list; the online proof reveals only that the committed
/// role lies in this set, not which one.
pub const DEFAULT_ALLOWED_ROLES: &[u64] = &[1u64, 2u64];
