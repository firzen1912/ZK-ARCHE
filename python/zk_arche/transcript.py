from __future__ import annotations

import hashlib
from dataclasses import dataclass
from typing import Iterable, Literal

from .crypto import Point, Scalar, scalar_from_wide_bytes

T_SETUP = b"setup_client_schnorr_v1"
T_SETUP_SERVER = b"setup_server_schnorr_v1"
T_SERVER = b"server_schnorr_v1"
T_PID = b"iot-auth/pid/v1"
T_CLIENT_V2 = b"client_schnorr_v2"
T_KC_V2 = b"kc_v2"
T_ROLE_SET = b"client_role_set_v1"
T_ROLE_RERAND = b"client_role_rerand_v1"


@dataclass(frozen=True)
class TVal:
    kind: Literal["bytes", "u8", "u64", "point", "scalar"]
    value: object

    @classmethod
    def bytes(cls, value: bytes) -> "TVal":
        return cls("bytes", bytes(value))

    @classmethod
    def u8(cls, value: int) -> "TVal":
        return cls("u8", int(value))

    @classmethod
    def u64(cls, value: int) -> "TVal":
        return cls("u64", int(value))

    @classmethod
    def point(cls, value: Point) -> "TVal":
        return cls("point", value)

    @classmethod
    def scalar(cls, value: Scalar) -> "TVal":
        return cls("scalar", value)


class Transcript:
    def __init__(self, domain: bytes) -> None:
        if len(domain) > 255:
            raise ValueError("domain too long")
        self._buf = bytearray()
        self._buf.append(len(domain))
        self._buf += domain

    def append_message(self, label: bytes, msg: bytes) -> None:
        if len(label) > 255:
            raise ValueError("label too long")
        self._buf.append(len(label))
        self._buf += label
        self._buf += len(msg).to_bytes(4, "little")
        self._buf += msg

    def append_u8(self, label: bytes, v: int) -> None:
        self.append_message(label, bytes([v & 0xFF]))

    def append_u64(self, label: bytes, v: int) -> None:
        self.append_message(label, int(v).to_bytes(8, "little"))

    def append_point(self, label: bytes, p: Point) -> None:
        self.append_message(label, p.to_bytes())

    def append_scalar(self, label: bytes, s: Scalar) -> None:
        self.append_message(label, s.to_bytes())

    def challenge_scalar(self) -> Scalar:
        return scalar_from_wide_bytes(hashlib.sha512(self._buf).digest())

    def hash_sha256(self) -> bytes:
        return hashlib.sha256(self._buf).digest()

    def as_bytes(self) -> bytes:
        return bytes(self._buf)


def build(domain: bytes, fields: Iterable[tuple[bytes, TVal]]) -> Transcript:
    t = Transcript(domain)
    for label, v in fields:
        if v.kind == "bytes":
            t.append_message(label, v.value)  # type: ignore[arg-type]
        elif v.kind == "u8":
            t.append_u8(label, int(v.value))
        elif v.kind == "u64":
            t.append_u64(label, int(v.value))
        elif v.kind == "point":
            t.append_point(label, v.value)  # type: ignore[arg-type]
        elif v.kind == "scalar":
            t.append_scalar(label, v.value)  # type: ignore[arg-type]
        else:
            raise ValueError(v.kind)
    return t


def challenge(domain: bytes, fields: Iterable[tuple[bytes, TVal]]) -> Scalar:
    return build(domain, fields).challenge_scalar()


def compute_pid(device_pub: Point, nonce_c: bytes, eph_c: Point, server_pub: Point) -> bytes:
    from .crypto import compute_pid as _compute_pid
    return _compute_pid(device_pub, nonce_c, eph_c, server_pub)
