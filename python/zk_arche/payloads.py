from __future__ import annotations

from dataclasses import dataclass

from .crypto import Point, Scalar, SchnorrProof, SetBranch, decode_scalar, decompress_point
from .crypto import SETUP_CHALLENGE_LEN
from .errors import ErrorCode, ProtoError


def _take(buf: bytes, off: int, n: int, what: str) -> tuple[bytes, int]:
    if len(buf) - off < n:
        raise ProtoError.wire(ErrorCode.PayloadTooShort, f"short read: {what}")
    return buf[off:off+n], off+n


def _take_point(buf: bytes, off: int, what: str) -> tuple[Point, int]:
    b, off = _take(buf, off, 32, what)
    return decompress_point(b, what), off


def _take_scalar(buf: bytes, off: int, what: str) -> tuple[Scalar, int]:
    b, off = _take(buf, off, 32, what)
    return decode_scalar(b, what), off


@dataclass(frozen=True)
class Setup1:
    pairing_token: bytes | None
    device_id: bytes
    device_pub: Point
    client_nonce: bytes
    role_commitment: Point

    def encode(self) -> bytes:
        token = self.pairing_token or b""
        if len(token) > 128:
            raise ProtoError.wire(ErrorCode.MalformedPacket, "pairing token len > 128")
        if len(self.device_id) != 32 or len(self.client_nonce) != 32:
            raise ProtoError.wire(ErrorCode.PayloadTooShort, "SETUP_1 fixed field wrong length")
        return bytes([len(token)]) + token + self.device_id + self.device_pub.to_bytes() + self.client_nonce + self.role_commitment.to_bytes()

    @classmethod
    def decode(cls, data: bytes) -> "Setup1":
        if not data:
            raise ProtoError.wire(ErrorCode.PayloadTooShort, "SETUP_1 empty")
        tlen = data[0]
        if tlen > 128:
            raise ProtoError.wire(ErrorCode.MalformedPacket, "pairing token len > 128")
        off = 1
        token, off = _take(data, off, tlen, "pairing token")
        device_id, off = _take(data, off, 32, "device_id")
        device_pub, off = _take_point(data, off, "device_pub")
        client_nonce, off = _take(data, off, 32, "client_nonce")
        role_commitment, off = _take_point(data, off, "role_commitment")
        return cls(token or None, device_id, device_pub, client_nonce, role_commitment)


@dataclass(frozen=True)
class Setup2:
    server_nonce: bytes
    setup_challenge: bytes
    server_pub: Point
    server_proof: SchnorrProof

    def encode(self) -> bytes:
        if len(self.server_nonce) != 32 or len(self.setup_challenge) != SETUP_CHALLENGE_LEN:
            raise ProtoError.wire(ErrorCode.PayloadTooShort, "SETUP_2 fixed field wrong length")
        return self.server_nonce + self.setup_challenge + self.server_pub.to_bytes() + self.server_proof.a.to_bytes() + self.server_proof.s.to_bytes()

    @classmethod
    def decode(cls, data: bytes) -> "Setup2":
        off = 0
        sn, off = _take(data, off, 32, "server_nonce")
        sc, off = _take(data, off, SETUP_CHALLENGE_LEN, "setup_challenge")
        server_pub, off = _take_point(data, off, "server_pub")
        a, off = _take_point(data, off, "setup a_s")
        s, off = _take_scalar(data, off, "setup s_s")
        return cls(sn, sc, server_pub, SchnorrProof(a, s))


@dataclass(frozen=True)
class Setup3:
    client_proof: SchnorrProof

    def encode(self) -> bytes:
        return self.client_proof.a.to_bytes() + self.client_proof.s.to_bytes()

    @classmethod
    def decode(cls, data: bytes) -> "Setup3":
        off = 0
        a, off = _take_point(data, off, "setup a_c")
        s, off = _take_scalar(data, off, "setup s_c")
        return cls(SchnorrProof(a, s))


@dataclass(frozen=True)
class Auth1:
    pid: bytes
    client_proof: SchnorrProof
    nonce_c: bytes
    eph_c: Point
    c_prime: Point
    rerand_proof: SchnorrProof
    branches: list[SetBranch]

    def encode(self) -> bytes:
        if len(self.pid) != 32 or len(self.nonce_c) != 32:
            raise ProtoError.wire(ErrorCode.PayloadTooShort, "AUTH_1 fixed field wrong length")
        out = bytearray()
        out += self.pid
        out += self.client_proof.a.to_bytes() + self.client_proof.s.to_bytes()
        out += self.nonce_c
        out += self.eph_c.to_bytes()
        out += self.c_prime.to_bytes()
        out += self.rerand_proof.a.to_bytes() + self.rerand_proof.s.to_bytes()
        out += len(self.branches).to_bytes(2, "little")
        for a, c, s in self.branches:
            out += a.to_bytes() + c.to_bytes() + s.to_bytes()
        return bytes(out)

    @classmethod
    def decode(cls, data: bytes) -> "Auth1":
        off = 0
        pid, off = _take(data, off, 32, "pid")
        ac, off = _take_point(data, off, "a_c")
        sc, off = _take_scalar(data, off, "s_c")
        nc, off = _take(data, off, 32, "nonce_c")
        eph_c, off = _take_point(data, off, "eph_c")
        c_prime, off = _take_point(data, off, "c_prime")
        rand_a, off = _take_point(data, off, "rerand_a")
        rand_s, off = _take_scalar(data, off, "rerand_s")
        n_b, off = _take(data, off, 2, "branch count")
        n = int.from_bytes(n_b, "little")
        if len(data) - off < 96 * n:
            raise ProtoError.wire(ErrorCode.PayloadTooShort, "branch payload truncated")
        branches = []
        for _ in range(n):
            a, off = _take_point(data, off, "branch A")
            c, off = _take_scalar(data, off, "branch c")
            s, off = _take_scalar(data, off, "branch s")
            branches.append((a, c, s))
        return cls(pid, SchnorrProof(ac, sc), nc, eph_c, c_prime, SchnorrProof(rand_a, rand_s), branches)


@dataclass(frozen=True)
class Auth2:
    server_pub: Point
    server_proof: SchnorrProof
    nonce_s: bytes
    eph_s: Point
    tag_s: bytes

    def encode(self) -> bytes:
        if len(self.nonce_s) != 32 or len(self.tag_s) != 32:
            raise ProtoError.wire(ErrorCode.PayloadTooShort, "AUTH_2 fixed field wrong length")
        return self.server_pub.to_bytes() + self.server_proof.a.to_bytes() + self.server_proof.s.to_bytes() + self.nonce_s + self.eph_s.to_bytes() + self.tag_s

    @classmethod
    def decode(cls, data: bytes) -> "Auth2":
        off = 0
        sp, off = _take_point(data, off, "server_pub")
        a, off = _take_point(data, off, "a_s")
        s, off = _take_scalar(data, off, "s_s")
        ns, off = _take(data, off, 32, "nonce_s")
        es, off = _take_point(data, off, "eph_s")
        tag, off = _take(data, off, 32, "tag_s")
        return cls(sp, SchnorrProof(a, s), ns, es, tag)


@dataclass(frozen=True)
class Auth3:
    tag_c: bytes

    def encode(self) -> bytes:
        if len(self.tag_c) != 32:
            raise ProtoError.wire(ErrorCode.PayloadTooShort, "AUTH_3 tag wrong length")
        return self.tag_c

    @classmethod
    def decode(cls, data: bytes) -> "Auth3":
        tag, _ = _take(data, 0, 32, "tag_c")
        return cls(tag)


ACK_BYTE = 0x01


def encode_ack() -> bytes:
    return bytes([ACK_BYTE])


def decode_ack(data: bytes) -> None:
    if data != bytes([ACK_BYTE]):
        raise ProtoError.wire(ErrorCode.MalformedPacket, "bad ACK")
