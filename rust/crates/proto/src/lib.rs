//! # proto
//!
//! Transport-agnostic implementation of the ZK-ARCHE v2 IoT authentication
//! protocol. The crate is organized into independent layers so that
//! heterogeneous implementations (different languages, hardware, transports)
//! can interoperate by matching the wire-format spec without sharing code.
//!
//! ## Module map
//!
//! | Module                    | Responsibility                                   |
//! |---------------------------|--------------------------------------------------|
//! | `caps`                    | Protocol version, suite IDs, capability bits     |
//! | `error`                   | Structured errors + wire error codes             |
//! | `transcript`              | Canonical transcript builder + domain separators |
//! | `crypto`                  | Schnorr / rerand / OR-proof / HKDF / HMAC        |
//! | `auth_v3`                 | Non-advertised AUTH-v3 reference primitives      |
//! | `auth_v3_context`         | Draft canonical AUTH-v3 subcontext encoding      |
//! | `auth_v3_context_parser`  | Strict raw AUTH-v3 subcontext parser             |
//! | `auth_v3_iot_core_authz`  | Draft `iot-core` authorization-context schema    |
//! | `replay_continuity`       | Draft fail-closed replay restart state           |
//! | `lineage_replace`         | Wire-neutral lifecycle replacement decisions     |
//! | `lineage_replace_auth_context` | Concrete `iot-core` lifecycle authorization binding |
//! | `lineage_replace_session_binding` | AUTH-v3 lifecycle session/context binding       |
//! | `lineage_replace_possession` | Session-bound lifecycle possession proof-result classifier |
//! | `lineage_replace_bound_auth_context` | Composed possession/session-bound `iot-core` replacement authorization |
//! | `lineage_replace_recovery` | Storage-neutral replacement restart classifier  |
//! | `lineage_replace_faults`  | Deterministic logical storage write-cut model    |
//! | `lineage_replace_freshness` | Durable-generation freshness decision model    |
//! | `lineage_replace_convergence` | Distributed replacement convergence model    |
//! | `lineage_replace_attempt` | Replacement attempt/confirmation binding model  |
//! | `lineage_replace_attempt_evidence` | Current-attempt confirmation provenance   |
//! | `lineage_replace_reconciliation` | Pair reconciliation over durable outcomes |
//! | `lineage_replace_reconciliation_transition` | Guarded exit from reconciliation      |
//! | `wire`                    | Packet header, framing, TLV codec                |
//! | `transport`               | Transport abstraction (UDP, TCP, ...)            |
//! | `store`                   | Credential / registry / replay-cache traits      |
//! | `profile`                 | Timing + resource profiles                       |
//! | `proto`                   | Layer-A state machines (setup, auth)             |
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

pub mod auth_v3;
pub mod auth_v3_context;
pub mod auth_v3_context_parser;
pub mod auth_v3_iot_core_authz;
pub mod caps;
pub mod crypto;
pub mod error;
pub mod lineage_replace;
pub mod lineage_replace_attempt;
pub mod lineage_replace_attempt_evidence;
pub mod lineage_replace_auth_context;
pub mod lineage_replace_bound_auth_context;
pub mod lineage_replace_convergence;
pub mod lineage_replace_faults;
pub mod lineage_replace_freshness;
pub mod lineage_replace_possession;
pub mod lineage_replace_reconciliation;
pub mod lineage_replace_reconciliation_transition;
pub mod lineage_replace_recovery;
pub mod lineage_replace_session_binding;
pub mod profile;
pub mod proto;
pub mod replay_continuity;
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
