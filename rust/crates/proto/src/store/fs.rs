//! Reference filesystem backend for `CredentialStore`, `RegistryStore`,
//! `ServerKeyStore`, and `ReplayCache`. This is the same on-disk format as
//! the original implementation so running binaries can be upgraded in place.

use std::collections::{HashMap, HashSet};
use std::fs;
use std::io::Write;
#[cfg(unix)]
use std::os::unix::fs::{OpenOptionsExt, PermissionsExt};
use std::path::{Path, PathBuf};

use curve25519_dalek::constants::RISTRETTO_BASEPOINT_POINT;
use curve25519_dalek::ristretto::{CompressedRistretto, RistrettoPoint};
use curve25519_dalek::scalar::Scalar;

use super::{
    CredentialStore, DeviceRecord, RegistryStore, ReplayCache, RoleCredential, ServerKeyStore,
};
use crate::crypto::{
    decode_scalar, decompress_point, encode_role, make_role_commitment, random_bytes_32,
    random_scalar, reject_identity,
};
use crate::error::{ErrorCode, ProtoError, Result};

// ---- Atomic private-file writer (Unix 0600) ----

fn ensure_parent_dir(path: &Path) -> Result<()> {
    if let Some(parent) = path.parent() {
        fs::create_dir_all(parent)
            .map_err(|e| ProtoError::storage(format!("mkdir {}: {e}", parent.display())))?;
    }
    Ok(())
}

pub fn write_private_file_atomic(path: &Path, data: &[u8]) -> Result<()> {
    ensure_parent_dir(path)?;
    let tmp = path.with_extension("tmp");
    #[cfg(unix)]
    {
        let mut f = std::fs::OpenOptions::new()
            .write(true)
            .create(true)
            .truncate(true)
            .mode(0o600)
            .open(&tmp)
            .map_err(|e| ProtoError::storage(format!("open {}: {e}", tmp.display())))?;
        f.write_all(data)
            .map_err(|e| ProtoError::storage(format!("write: {e}")))?;
        f.sync_all()
            .map_err(|e| ProtoError::storage(format!("fsync: {e}")))?;
        fs::set_permissions(&tmp, fs::Permissions::from_mode(0o600))
            .map_err(|e| ProtoError::storage(format!("chmod: {e}")))?;
    }
    #[cfg(not(unix))]
    {
        let mut f = std::fs::OpenOptions::new()
            .write(true)
            .create(true)
            .truncate(true)
            .open(&tmp)
            .map_err(|e| ProtoError::storage(format!("open {}: {e}", tmp.display())))?;
        f.write_all(data)
            .map_err(|e| ProtoError::storage(format!("write: {e}")))?;
        f.sync_all()
            .map_err(|e| ProtoError::storage(format!("fsync: {e}")))?;
    }
    fs::rename(&tmp, path).map_err(|e| ProtoError::storage(format!("rename: {e}")))?;
    Ok(())
}

#[cfg(unix)]
fn verify_private_permissions(path: &Path) -> Result<()> {
    let mode = fs::metadata(path)
        .map_err(|e| ProtoError::storage(format!("stat {}: {e}", path.display())))?
        .permissions()
        .mode()
        & 0o777;
    if mode & 0o077 != 0 {
        return Err(ProtoError::storage(format!(
            "{} must not be group/world accessible (mode {:o})",
            path.display(),
            mode,
        )));
    }
    Ok(())
}
#[cfg(not(unix))]
fn verify_private_permissions(_p: &Path) -> Result<()> {
    Ok(())
}

// ---- CredentialStore ----

pub struct FsCredentialStore {
    pub device_root_file: PathBuf,
    pub server_pub_file: PathBuf,
    pub role_cred_file: PathBuf,
}

impl FsCredentialStore {
    pub fn new_default() -> Self {
        Self {
            device_root_file: "./client-state/device_root.bin".into(),
            server_pub_file: "./client-state/server_pub.bin".into(),
            role_cred_file: "./client-state/role_cred.bin".into(),
        }
    }

    pub fn with_dir(dir: impl AsRef<Path>) -> Self {
        let d = dir.as_ref();
        Self {
            device_root_file: d.join("device_root.bin"),
            server_pub_file: d.join("server_pub.bin"),
            role_cred_file: d.join("role_cred.bin"),
        }
    }
}

impl CredentialStore for FsCredentialStore {
    fn load_or_create_device_root(&mut self, create_if_missing: bool) -> Result<[u8; 32]> {
        if self.device_root_file.exists() {
            verify_private_permissions(&self.device_root_file)?;
            let b = fs::read(&self.device_root_file)
                .map_err(|e| ProtoError::storage(format!("read device_root: {e}")))?;
            if b.len() != 32 {
                return Err(ProtoError::wire(
                    ErrorCode::CredentialMissing,
                    "device_root wrong length",
                ));
            }
            let mut root = [0u8; 32];
            root.copy_from_slice(&b);
            Ok(root)
        } else if create_if_missing {
            let root = random_bytes_32();
            write_private_file_atomic(&self.device_root_file, &root)?;
            Ok(root)
        } else {
            Err(ProtoError::wire(
                ErrorCode::CredentialMissing,
                "device root missing; run --setup first",
            ))
        }
    }

    fn load_server_pub(&self) -> Result<Option<RistrettoPoint>> {
        if !self.server_pub_file.exists() {
            return Ok(None);
        }
        let b = fs::read(&self.server_pub_file)
            .map_err(|e| ProtoError::storage(format!("read server_pub: {e}")))?;
        if b.len() != 32 {
            return Err(ProtoError::wire(
                ErrorCode::RegistryCorrupt,
                "server_pub wrong length",
            ));
        }
        let mut bb = [0u8; 32];
        bb.copy_from_slice(&b);
        let p = decompress_point(&bb, "pinned server_pub")?;
        reject_identity(&p, "pinned server_pub")?;
        Ok(Some(p))
    }

    fn save_server_pub(&mut self, pubkey: &RistrettoPoint) -> Result<()> {
        write_private_file_atomic(&self.server_pub_file, pubkey.compress().as_bytes())
    }

    fn load_role_credential(&self) -> Result<Option<RoleCredential>> {
        if !self.role_cred_file.exists() {
            return Ok(None);
        }
        verify_private_permissions(&self.role_cred_file)?;
        let data = fs::read(&self.role_cred_file)
            .map_err(|e| ProtoError::storage(format!("read role_cred: {e}")))?;
        if data.len() != 72 {
            return Err(ProtoError::wire(
                ErrorCode::RegistryCorrupt,
                "role_cred wrong length",
            ));
        }

        let mut role_code_bytes = [0u8; 8];
        role_code_bytes.copy_from_slice(&data[0..8]);
        let role_code = u64::from_le_bytes(role_code_bytes);
        let role_scalar = encode_role(role_code);

        let mut blind_bytes = [0u8; 32];
        blind_bytes.copy_from_slice(&data[8..40]);
        let blind = decode_scalar(&blind_bytes, "role blind")?;

        let mut commitment_bytes = [0u8; 32];
        commitment_bytes.copy_from_slice(&data[40..72]);
        let commitment = decompress_point(&commitment_bytes, "role commitment")?;

        let expected = make_role_commitment(&role_scalar, &blind);
        if expected.compress().to_bytes() != commitment.compress().to_bytes() {
            return Err(ProtoError::wire(
                ErrorCode::RegistryCorrupt,
                "role credential commitment mismatch",
            ));
        }

        Ok(Some(RoleCredential {
            role_code,
            blind,
            commitment,
        }))
    }

    fn save_role_credential(&mut self, cred: &RoleCredential) -> Result<()> {
        let mut out = Vec::with_capacity(8 + 32 + 32);
        out.extend_from_slice(&cred.role_code.to_le_bytes());
        out.extend_from_slice(&cred.blind.to_bytes());
        out.extend_from_slice(cred.commitment.compress().as_bytes());
        write_private_file_atomic(&self.role_cred_file, &out)
    }
}

// ---- RegistryStore (server side) ----

pub struct FsRegistryStore {
    pub registry_file: PathBuf,
    pub backup_file: PathBuf,
    cache: HashMap<[u8; 32], DeviceRecord>,
    dirty: bool,
}

impl FsRegistryStore {
    pub fn new_default() -> Result<Self> {
        Self::with_files(
            "./server-state/registry.bin".into(),
            "./server-state/registry.bak".into(),
        )
    }

    pub fn with_files(registry_file: PathBuf, backup_file: PathBuf) -> Result<Self> {
        let cache = load_registry(&registry_file)?;
        Ok(Self {
            registry_file,
            backup_file,
            cache,
            dirty: false,
        })
    }

    pub fn flush(&mut self) -> Result<()> {
        if !self.dirty {
            return Ok(());
        }
        save_registry_atomic(&self.registry_file, &self.backup_file, &self.cache)?;
        self.dirty = false;
        Ok(())
    }
}

fn load_registry(path: &Path) -> Result<HashMap<[u8; 32], DeviceRecord>> {
    if !path.exists() {
        return Ok(HashMap::new());
    }
    let data = fs::read(path).map_err(|e| ProtoError::storage(format!("read registry: {e}")))?;
    if data.len() % 96 != 0 {
        return Err(ProtoError::wire(
            ErrorCode::RegistryCorrupt,
            "registry length not multiple of 96",
        ));
    }
    let mut map = HashMap::new();
    for chunk in data.chunks_exact(96) {
        let mut device_id = [0u8; 32];
        device_id.copy_from_slice(&chunk[0..32]);
        let mut pub_bytes = [0u8; 32];
        pub_bytes.copy_from_slice(&chunk[32..64]);
        let mut rc_bytes = [0u8; 32];
        rc_bytes.copy_from_slice(&chunk[64..96]);

        let pubkey = CompressedRistretto(pub_bytes).decompress().ok_or_else(|| {
            ProtoError::wire(ErrorCode::RegistryCorrupt, "registry pubkey invalid")
        })?;
        let role_commitment = CompressedRistretto(rc_bytes).decompress().ok_or_else(|| {
            ProtoError::wire(ErrorCode::RegistryCorrupt, "registry commitment invalid")
        })?;
        reject_identity(&pubkey, "registry pubkey")?;
        reject_identity(&role_commitment, "registry role commitment")?;
        map.insert(
            device_id,
            DeviceRecord {
                pubkey,
                role_commitment,
            },
        );
    }
    Ok(map)
}

fn save_registry_atomic(
    path: &Path,
    backup: &Path,
    map: &HashMap<[u8; 32], DeviceRecord>,
) -> Result<()> {
    if path.exists() {
        let _ = fs::copy(path, backup);
    }
    let mut out = Vec::with_capacity(map.len() * 96);
    for (id, rec) in map {
        out.extend_from_slice(id);
        out.extend_from_slice(rec.pubkey.compress().as_bytes());
        out.extend_from_slice(rec.role_commitment.compress().as_bytes());
    }
    write_private_file_atomic(path, &out)
}

impl RegistryStore for FsRegistryStore {
    fn lookup_by_device_id(&self, device_id: &[u8; 32]) -> Result<Option<DeviceRecord>> {
        Ok(self.cache.get(device_id).copied())
    }

    fn iter(&self) -> Box<dyn Iterator<Item = ([u8; 32], DeviceRecord)> + '_> {
        Box::new(self.cache.iter().map(|(k, v)| (*k, *v)))
    }

    fn save(&mut self, device_id: [u8; 32], record: DeviceRecord) -> Result<()> {
        self.cache.insert(device_id, record);
        self.dirty = true;
        self.flush()
    }
}

// ---- ServerKeyStore ----

pub struct FsServerKeyStore {
    pub path: PathBuf,
}

impl FsServerKeyStore {
    pub fn new_default() -> Self {
        Self {
            path: "./server-state/server_sk.bin".into(),
        }
    }
    pub fn with_path(path: impl Into<PathBuf>) -> Self {
        Self { path: path.into() }
    }
}

impl ServerKeyStore for FsServerKeyStore {
    fn load_or_create_server_sk(&mut self) -> Result<Scalar> {
        if self.path.exists() {
            verify_private_permissions(&self.path)?;
            let b = fs::read(&self.path)
                .map_err(|e| ProtoError::storage(format!("read server_sk: {e}")))?;
            if b.len() != 32 {
                return Err(ProtoError::wire(
                    ErrorCode::RegistryCorrupt,
                    "server_sk wrong length",
                ));
            }
            let mut bb = [0u8; 32];
            bb.copy_from_slice(&b);
            decode_scalar(&bb, "server_sk")
        } else {
            let sk = random_scalar();
            write_private_file_atomic(&self.path, &sk.to_bytes())?;
            Ok(sk)
        }
    }
}

// ---- ReplayCache ----

pub struct MemoryReplayCache {
    set: HashSet<[u8; 32]>,
    cap: usize,
}

impl MemoryReplayCache {
    pub fn new(cap: usize) -> Self {
        Self {
            set: HashSet::with_capacity(cap.min(1024)),
            cap,
        }
    }
}

impl ReplayCache for MemoryReplayCache {
    fn insert(&mut self, key: [u8; 32]) -> bool {
        if self.set.len() >= self.cap {
            // Simple eviction: drop an arbitrary entry. For production, use
            // a generation-based cache (matches the reference implementation).
            if let Some(&k) = self.set.iter().next() {
                self.set.remove(&k);
            }
        }
        self.set.insert(key)
    }
    fn contains(&self, key: &[u8; 32]) -> bool {
        self.set.contains(key)
    }
}

// Compute a canonical replay key from (pid, nonce_c, eph_c). This is the same
// binding as `T_CLIENT_V2` uses, so no two successful AUTH_1 messages can
// share a key without the client having reused randomness.
pub fn replay_key(pid: &[u8; 32], nonce_c: &[u8; 32], eph_c: &RistrettoPoint) -> [u8; 32] {
    use sha2::{Digest, Sha256};
    let mut h = Sha256::new();
    h.update(b"iot-auth/replay-key/v2");
    h.update(pid);
    h.update(nonce_c);
    h.update(eph_c.compress().as_bytes());
    let mut out = [0u8; 32];
    out.copy_from_slice(&h.finalize());
    out
}

// Silence unused-import warnings when compiling without Unix cfg.
#[allow(dead_code)]
fn _rbp_ref() -> RistrettoPoint {
    RISTRETTO_BASEPOINT_POINT
}
