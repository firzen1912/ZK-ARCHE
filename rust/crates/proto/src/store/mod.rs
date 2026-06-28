//! Storage abstraction.
//!
//! Protocol state machines never touch the filesystem, flash, or any specific
//! backend directly. They go through these traits. A reference filesystem
//! backend is provided behind the `fs-store` feature; embedded targets can
//! implement secure-element, TPM-backed, or flash backends by implementing
//! the same traits.

use curve25519_dalek::ristretto::RistrettoPoint;

use crate::crypto::SchnorrProof;
use crate::error::Result;

/// Client-side credential store. Holds the device root secret, the pinned
/// server public key, and the role credential.
pub trait CredentialStore {
    /// Returns the 32-byte device root secret, creating one on first use if
    /// `create_if_missing` is set. The caller is responsible for zeroizing.
    fn load_or_create_device_root(&mut self, create_if_missing: bool) -> Result<[u8; 32]>;

    /// Returns the pinned server static public key if one is set, else `None`.
    fn load_server_pub(&self) -> Result<Option<RistrettoPoint>>;

    /// Pins the server static public key (replaces any previous value).
    fn save_server_pub(&mut self, pubkey: &RistrettoPoint) -> Result<()>;

    /// Loads the role credential (role_code, blind, commitment).
    fn load_role_credential(&self) -> Result<Option<RoleCredential>>;

    /// Saves or updates the role credential.
    fn save_role_credential(&mut self, cred: &RoleCredential) -> Result<()>;
}

/// Server-side device registry: maps device_id -> (device_pub, role_commitment).
pub trait RegistryStore {
    fn lookup_by_device_id(&self, device_id: &[u8; 32]) -> Result<Option<DeviceRecord>>;

    /// Scan all enrolled records. Used by the server during AUTH_1 to find the
    /// device whose stored `(pubkey, commitment)` produces the observed pid
    /// and re-randomization proof. A database-backed implementation MAY
    /// short-circuit with an index (e.g., if the client includes a hint TLV).
    fn iter(&self) -> Box<dyn Iterator<Item = ([u8; 32], DeviceRecord)> + '_>;

    fn save(&mut self, device_id: [u8; 32], record: DeviceRecord) -> Result<()>;
}

/// Replay cache: records accepted `(pid, nonce_c, eph_c)` tuples hashed to a
/// 32-byte key. The server uses this to reject replayed AUTH_1 messages even
/// within the session TTL.
pub trait ReplayCache {
    /// Returns true if the key was newly inserted; false if it was already
    /// present (i.e., the incoming AUTH_1 is a replay).
    fn insert(&mut self, key: [u8; 32]) -> bool;

    fn contains(&self, key: &[u8; 32]) -> bool;
}

// ---- Shared record types ----

use curve25519_dalek::scalar::Scalar;

#[derive(Clone, Debug)]
pub struct RoleCredential {
    pub role_code: u64,
    pub blind: Scalar,
    pub commitment: RistrettoPoint,
}

impl RoleCredential {
    pub fn role_scalar(&self) -> Scalar {
        crate::crypto::encode_role(self.role_code)
    }
}

#[derive(Clone, Copy, Debug)]
pub struct DeviceRecord {
    pub pubkey: RistrettoPoint,
    pub role_commitment: RistrettoPoint,
}

// ---- Server-side key store ----

pub trait ServerKeyStore {
    /// Load or create the server static secret scalar.
    fn load_or_create_server_sk(&mut self) -> Result<Scalar>;
}

// Opaque helper so library users can convert stored proofs through the wire
// (kept here so the trait and wire layer agree on shapes).
pub type SetupProofBytes = ([u8; 32], [u8; 32]); // (a_compressed, s_bytes)

impl From<&SchnorrProof> for SetupProofBytes {
    fn from(p: &SchnorrProof) -> Self {
        (p.a.compress().to_bytes(), p.s.to_bytes())
    }
}

pub mod fs;
