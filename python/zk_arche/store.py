from __future__ import annotations

import hashlib
import os
from dataclasses import dataclass
from pathlib import Path

from .crypto import Point, Scalar, decode_scalar, decompress_point, derive_device_id, encode_role, make_role_commitment, random_bytes_32, random_scalar
from .errors import ErrorCode, ProtoError


@dataclass(frozen=True)
class RoleCredential:
    role_code: int
    blind: Scalar
    commitment: Point

    def role_scalar(self) -> Scalar:
        return encode_role(self.role_code)


@dataclass(frozen=True)
class DeviceRecord:
    pubkey: Point
    role_commitment: Point


def _ensure_parent(path: Path) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)


def write_private_file_atomic(path: Path, data: bytes) -> None:
    _ensure_parent(path)
    tmp = path.with_suffix(path.suffix + ".tmp")
    flags = os.O_WRONLY | os.O_CREAT | os.O_TRUNC
    fd = os.open(tmp, flags, 0o600)
    try:
        with os.fdopen(fd, "wb") as f:
            f.write(data)
            f.flush()
            os.fsync(f.fileno())
        try:
            os.chmod(tmp, 0o600)
        except OSError:
            pass
        os.replace(tmp, path)
    finally:
        if tmp.exists():
            try:
                tmp.unlink()
            except OSError:
                pass


def verify_private_permissions(path: Path) -> None:
    try:
        mode = path.stat().st_mode & 0o777
    except OSError as exc:
        raise ProtoError.storage(f"stat {path}: {exc}") from exc
    if os.name == "posix" and mode & 0o077:
        raise ProtoError.storage(f"{path} must not be group/world accessible (mode {mode:o})")


class FsCredentialStore:
    def __init__(self, directory: str | os.PathLike[str] = "./client-state") -> None:
        d = Path(directory)
        self.device_root_file = d / "device_root.bin"
        self.server_pub_file = d / "server_pub.bin"
        self.role_cred_file = d / "role_cred.bin"

    def load_or_create_device_root(self, create_if_missing: bool) -> bytes:
        if self.device_root_file.exists():
            verify_private_permissions(self.device_root_file)
            data = self.device_root_file.read_bytes()
            if len(data) != 32:
                raise ProtoError.wire(ErrorCode.CredentialMissing, "device_root wrong length")
            return data
        if create_if_missing:
            root = random_bytes_32()
            write_private_file_atomic(self.device_root_file, root)
            return root
        raise ProtoError.wire(ErrorCode.CredentialMissing, "device root missing; run --setup first")

    def load_server_pub(self) -> Point | None:
        if not self.server_pub_file.exists():
            return None
        data = self.server_pub_file.read_bytes()
        if len(data) != 32:
            raise ProtoError.wire(ErrorCode.RegistryCorrupt, "server_pub wrong length")
        return decompress_point(data, "pinned server_pub")

    def save_server_pub(self, pubkey: Point) -> None:
        write_private_file_atomic(self.server_pub_file, pubkey.to_bytes())

    def load_role_credential(self) -> RoleCredential | None:
        if not self.role_cred_file.exists():
            return None
        verify_private_permissions(self.role_cred_file)
        data = self.role_cred_file.read_bytes()
        if len(data) != 72:
            raise ProtoError.wire(ErrorCode.RegistryCorrupt, "role_cred wrong length")
        role_code = int.from_bytes(data[:8], "little")
        blind = decode_scalar(data[8:40], "role blind")
        commitment = decompress_point(data[40:72], "role commitment")
        expected = make_role_commitment(encode_role(role_code), blind)
        if expected.to_bytes() != commitment.to_bytes():
            raise ProtoError.wire(ErrorCode.RegistryCorrupt, "role credential commitment mismatch")
        return RoleCredential(role_code, blind, commitment)

    def save_role_credential(self, cred: RoleCredential) -> None:
        data = int(cred.role_code).to_bytes(8, "little") + cred.blind.to_bytes() + cred.commitment.to_bytes()
        write_private_file_atomic(self.role_cred_file, data)


class FsRegistryStore:
    def __init__(self, registry_file: str | os.PathLike[str] = "./server-state/registry.bin", backup_file: str | os.PathLike[str] | None = None) -> None:
        self.registry_file = Path(registry_file)
        self.backup_file = Path(backup_file) if backup_file is not None else self.registry_file.with_suffix(".bak")
        self.cache = self._load_registry(self.registry_file)
        self.dirty = False

    @classmethod
    def with_dir(cls, directory: str | os.PathLike[str]) -> "FsRegistryStore":
        d = Path(directory)
        return cls(d / "registry.bin", d / "registry.bak")

    @staticmethod
    def _load_registry(path: Path) -> dict[bytes, DeviceRecord]:
        if not path.exists():
            return {}
        data = path.read_bytes()
        if len(data) % 96 != 0:
            raise ProtoError.wire(ErrorCode.RegistryCorrupt, "registry length not multiple of 96")
        out: dict[bytes, DeviceRecord] = {}
        for i in range(0, len(data), 96):
            device_id = data[i:i+32]
            pubkey = decompress_point(data[i+32:i+64], "registry pubkey")
            role_commitment = decompress_point(data[i+64:i+96], "registry role commitment")
            out[device_id] = DeviceRecord(pubkey, role_commitment)
        return out

    def flush(self) -> None:
        if not self.dirty:
            return
        if self.registry_file.exists():
            try:
                _ensure_parent(self.backup_file)
                self.backup_file.write_bytes(self.registry_file.read_bytes())
            except OSError:
                pass
        data = bytearray()
        for device_id, rec in self.cache.items():
            data += device_id + rec.pubkey.to_bytes() + rec.role_commitment.to_bytes()
        write_private_file_atomic(self.registry_file, bytes(data))
        self.dirty = False

    def lookup_by_device_id(self, device_id: bytes) -> DeviceRecord | None:
        return self.cache.get(bytes(device_id))

    def iter(self):
        return list(self.cache.items())

    def save(self, device_id: bytes, record: DeviceRecord) -> None:
        if len(device_id) != 32:
            raise ProtoError.wire(ErrorCode.PayloadTooShort, "device_id must be 32 bytes")
        self.cache[bytes(device_id)] = record
        self.dirty = True
        self.flush()


class FsServerKeyStore:
    def __init__(self, path: str | os.PathLike[str] = "./server-state/server_sk.bin") -> None:
        self.path = Path(path)

    @classmethod
    def with_dir(cls, directory: str | os.PathLike[str]) -> "FsServerKeyStore":
        return cls(Path(directory) / "server_sk.bin")

    def load_or_create_server_sk(self) -> Scalar:
        if self.path.exists():
            verify_private_permissions(self.path)
            data = self.path.read_bytes()
            if len(data) != 32:
                raise ProtoError.wire(ErrorCode.RegistryCorrupt, "server_sk wrong length")
            return decode_scalar(data, "server_sk")
        sk = random_scalar()
        write_private_file_atomic(self.path, sk.to_bytes())
        return sk


class MemoryReplayCache:
    def __init__(self, cap: int) -> None:
        self.cap = cap
        self._set: set[bytes] = set()

    def insert(self, key: bytes) -> bool:
        key = bytes(key)
        already = key in self._set
        if len(self._set) >= self.cap and not already:
            self._set.pop()
        self._set.add(key)
        return not already

    def contains(self, key: bytes) -> bool:
        return bytes(key) in self._set


def replay_key(pid: bytes, nonce_c: bytes, eph_c: Point) -> bytes:
    h = hashlib.sha256()
    h.update(b"iot-auth/replay-key/v2")
    h.update(pid)
    h.update(nonce_c)
    h.update(eph_c.to_bytes())
    return h.digest()
