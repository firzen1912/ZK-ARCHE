from __future__ import annotations

from dataclasses import dataclass
from typing import Iterator

from .caps import MIN_SUPPORTED_VERSION, PROTOCOL_VERSION, SUITE_RISTRETTO255_SHA256, cap
from .errors import ErrorCode, ProtoError

HEADER_LEN = 24
SESSION_ID_LEN = 16
MAX_DATAGRAM = 2048
MAX_PAYLOAD = MAX_DATAGRAM - HEADER_LEN

PKT_HELLO = 0x01
PKT_HELLO_REPLY = 0x02
PKT_SETUP_1 = 0x11
PKT_SETUP_2 = 0x12
PKT_SETUP_3 = 0x13
PKT_SETUP_ACK = 0x14
PKT_AUTH_1 = 0x21
PKT_AUTH_2 = 0x22
PKT_AUTH_3 = 0x23
PKT_AUTH_ACK = 0x24
PKT_ERROR = 0x7F

FLAG_NONE = 0x0000
FLAG_RETRANSMIT = 0x0001


class tlv_tag:
    MIN_VERSION = 0x0001
    SUITE_LIST = 0x0002
    CAPS = 0x0003
    MTU_HINT = 0x0004
    VENDOR_ID = 0x0100
    DEVICE_MODEL = 0x0101


@dataclass(frozen=True)
class Header:
    version: int
    pkt_type: int
    flags: int
    session_id: bytes
    seq: int

    @classmethod
    def new(cls, pkt_type: int, session_id: bytes, seq: int) -> "Header":
        if len(session_id) != SESSION_ID_LEN:
            raise ProtoError.internal("session_id must be 16 bytes")
        return cls(PROTOCOL_VERSION, pkt_type, FLAG_NONE, bytes(session_id), seq)

    def encode(self) -> bytes:
        if len(self.session_id) != SESSION_ID_LEN:
            raise ProtoError.internal("session_id must be 16 bytes")
        return bytes([self.version, self.pkt_type]) + self.flags.to_bytes(2, "little") + self.session_id + self.seq.to_bytes(4, "little")

    @classmethod
    def decode(cls, data: bytes) -> tuple["Header", bytes]:
        if len(data) < HEADER_LEN:
            raise ProtoError.wire(ErrorCode.PayloadTooShort, "datagram < 24B header")
        version = data[0]
        pkt_type = data[1]
        flags = int.from_bytes(data[2:4], "little")
        session_id = data[4:20]
        seq = int.from_bytes(data[20:24], "little")
        if version < MIN_SUPPORTED_VERSION:
            raise ProtoError.wire(ErrorCode.UnsupportedVersion, f"packet version {version} < min supported {MIN_SUPPORTED_VERSION}")
        return cls(version, pkt_type, flags, session_id, seq), data[HEADER_LEN:]


def build_packet(pkt_type: int, session_id: bytes, seq: int, payload: bytes) -> bytes:
    if len(payload) > MAX_PAYLOAD:
        raise ProtoError.wire(ErrorCode.PayloadTooLarge, f"payload {len(payload)} exceeds MAX_PAYLOAD {MAX_PAYLOAD}")
    return Header.new(pkt_type, session_id, seq).encode() + payload


def build_error(session_id: bytes, seq: int, code: ErrorCode, msg: str) -> bytes:
    payload = int(code).to_bytes(2, "little") + msg.encode()
    return build_packet(PKT_ERROR, session_id, seq, payload)


def parse_packet(data: bytes) -> tuple[Header, bytes]:
    return Header.decode(data)


class TlvWriter:
    def __init__(self) -> None:
        self.out = bytearray()

    def put(self, tag: int, value: bytes) -> None:
        if len(value) > 0xFFFF:
            raise ProtoError.wire(ErrorCode.PayloadTooLarge, "TLV value > u16")
        self.out += tag.to_bytes(2, "little")
        self.out += len(value).to_bytes(2, "little")
        self.out += value

    def bytes(self) -> bytes:
        return bytes(self.out)


def iter_tlv(data: bytes) -> Iterator[tuple[int, bytes]]:
    i = 0
    while i < len(data):
        if len(data) - i < 4:
            raise ProtoError.wire(ErrorCode.MalformedPacket, "truncated TLV header")
        tag = int.from_bytes(data[i:i+2], "little")
        n = int.from_bytes(data[i+2:i+4], "little")
        i += 4
        if len(data) - i < n:
            raise ProtoError.wire(ErrorCode.MalformedPacket, "truncated TLV value")
        yield tag, data[i:i+n]
        i += n


@dataclass(frozen=True)
class Hello:
    version: int = PROTOCOL_VERSION
    min_version: int = MIN_SUPPORTED_VERSION
    suites: tuple[int, ...] = (SUITE_RISTRETTO255_SHA256,)
    caps: int = cap.BASELINE
    mtu_hint: int | None = None
    vendor_id: bytes | None = None
    device_model: bytes | None = None

    def encode(self) -> bytes:
        out = bytearray()
        out.append(self.version)
        out += len(self.suites).to_bytes(2, "little")
        for suite in self.suites:
            out += int(suite).to_bytes(2, "little")
        out += int(self.caps).to_bytes(8, "little")
        w = TlvWriter()
        w.put(tlv_tag.MIN_VERSION, bytes([self.min_version]))
        if self.mtu_hint is not None:
            w.put(tlv_tag.MTU_HINT, int(self.mtu_hint).to_bytes(2, "little"))
        if self.vendor_id is not None:
            w.put(tlv_tag.VENDOR_ID, self.vendor_id)
        if self.device_model is not None:
            w.put(tlv_tag.DEVICE_MODEL, self.device_model)
        out += w.bytes()
        return bytes(out)

    @classmethod
    def decode(cls, data: bytes) -> "Hello":
        if len(data) < 3:
            raise ProtoError.wire(ErrorCode.PayloadTooShort, "HELLO truncated")
        version = data[0]
        n = int.from_bytes(data[1:3], "little")
        suites_end = 3 + 2 * n
        if len(data) < suites_end + 8:
            raise ProtoError.wire(ErrorCode.PayloadTooShort, "HELLO suite list truncated")
        suites = tuple(int.from_bytes(data[3 + 2*i:5 + 2*i], "little") for i in range(n))
        caps = int.from_bytes(data[suites_end:suites_end+8], "little")
        min_version = version
        mtu_hint = None
        vendor_id = None
        device_model = None
        for tag, value in iter_tlv(data[suites_end+8:]):
            if tag == tlv_tag.MIN_VERSION:
                if len(value) != 1:
                    raise ProtoError.wire(ErrorCode.MalformedPacket, "MIN_VERSION must be 1 byte")
                min_version = value[0]
            elif tag == tlv_tag.MTU_HINT:
                if len(value) != 2:
                    raise ProtoError.wire(ErrorCode.MalformedPacket, "MTU_HINT must be 2 bytes")
                mtu_hint = int.from_bytes(value, "little")
            elif tag == tlv_tag.VENDOR_ID:
                vendor_id = value
            elif tag == tlv_tag.DEVICE_MODEL:
                device_model = value
        return cls(version, min_version, suites, caps, mtu_hint, vendor_id, device_model)
