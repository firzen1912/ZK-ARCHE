from __future__ import annotations

from zk_arche.caps import SUITE_RISTRETTO255_SHA256, cap
from zk_arche.crypto import Scalar, SchnorrProof, basepoint_mul
from zk_arche.payloads import Auth1, Setup1, decode_ack, encode_ack
from zk_arche.wire import Header, Hello, PKT_HELLO, SESSION_ID_LEN


def test_header_roundtrip():
    h = Header.new(PKT_HELLO, bytes([7]) * SESSION_ID_LEN, 42)
    h2, rest = Header.decode(h.encode())
    assert rest == b""
    assert h2.pkt_type == PKT_HELLO
    assert h2.session_id == bytes([7]) * SESSION_ID_LEN
    assert h2.seq == 42


def test_hello_roundtrip_ignores_unknown_tlv():
    hello = Hello(2, 2, (SUITE_RISTRETTO255_SHA256,), cap.BASELINE, 1200, b"acme", None)
    data = hello.encode() + (0xFFFE).to_bytes(2, "little") + (3).to_bytes(2, "little") + b"xyz"
    got = Hello.decode(data)
    assert got.version == 2
    assert got.suites == (SUITE_RISTRETTO255_SHA256,)
    assert got.mtu_hint == 1200
    assert got.vendor_id == b"acme"


def test_setup1_roundtrip():
    p = basepoint_mul(Scalar.from_u64(1))
    msg = Setup1(b"abc", bytes([1]) * 32, p, bytes([2]) * 32, p)
    got = Setup1.decode(msg.encode())
    assert got.pairing_token == b"abc"
    assert got.device_id == bytes([1]) * 32


def test_auth1_roundtrip():
    p = basepoint_mul(Scalar.from_u64(1))
    proof = SchnorrProof(p, Scalar.from_u64(7))
    msg = Auth1(bytes([9])*32, proof, bytes([3])*32, p, p, proof, [(p, Scalar.from_u64(1), Scalar.from_u64(2)), (p, Scalar.from_u64(3), Scalar.from_u64(4))])
    got = Auth1.decode(msg.encode())
    assert len(got.branches) == 2
    assert got.nonce_c == bytes([3])*32


def test_ack():
    decode_ack(encode_ack())
