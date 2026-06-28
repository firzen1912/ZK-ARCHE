from __future__ import annotations

import pytest

from zk_arche.crypto import (
    Point,
    Scalar,
    attr_h,
    basepoint_mul,
    compute_pid,
    decode_scalar,
    decompress_point,
    derive_session_key,
    make_role_commitment,
    prove_auth_client_with_rng,
    prove_rerandomization_with_rng,
    prove_role_set_membership_with_rng,
    scalar_from_wide_bytes,
    verify_auth_client,
    verify_rerandomization,
    verify_role_set_membership,
)
from zk_arche.errors import ProtoError
from zk_arche.payloads import Auth1, Auth2, Auth3, Setup1, Setup2, Setup3
from zk_arche.store import MemoryReplayCache, replay_key
from zk_arche.crypto import SchnorrProof


def scalar(seed: int) -> Scalar:
    return scalar_from_wide_bytes(bytes([seed]) * 64)


def point(seed: int) -> Point:
    return basepoint_mul(scalar(seed))


def proof(seed: int) -> SchnorrProof:
    return SchnorrProof(point(seed), scalar(seed + 1))


def fixed_rng():
    data = bytes((i * 73 + 19) % 256 for i in range(4096))
    pos = 0

    def rng(n: int) -> bytes:
        nonlocal pos
        out = bytearray()
        while len(out) < n:
            take = min(n - len(out), len(data) - (pos % len(data)))
            start = pos % len(data)
            out.extend(data[start:start + take])
            pos += take
        return bytes(out)

    return rng


def assert_rejects_trailing(decoder, encoded: bytes) -> None:
    with pytest.raises(ProtoError):
        decoder(encoded + b"\xa5")


def test_rejects_invalid_points_scalars_and_trailing_payload_bytes():
    with pytest.raises(ProtoError):
        decompress_point(b"\x00" * 32, "identity")
    with pytest.raises(ProtoError):
        decompress_point(b"\xff" * 32, "invalid point")
    with pytest.raises(ProtoError):
        decode_scalar(b"\xff" * 32, "non-canonical scalar")

    assert_rejects_trailing(
        Setup1.decode,
        Setup1(b"token", b"\x01" * 32, point(2), b"\x03" * 32, point(4)).encode(),
    )
    assert_rejects_trailing(
        Setup2.decode,
        Setup2(b"\x05" * 32, b"\x06" * 16, point(7), proof(8)).encode(),
    )
    assert_rejects_trailing(Setup3.decode, Setup3(proof(10)).encode())
    assert_rejects_trailing(
        Auth1.decode,
        Auth1(
            b"\x0b" * 32,
            proof(12),
            b"\x0d" * 32,
            point(14),
            point(15),
            proof(16),
            [(point(17), scalar(18), scalar(19))],
        ).encode(),
    )
    assert_rejects_trailing(
        Auth2.decode,
        Auth2(point(20), proof(21), b"\x16" * 32, point(23), b"\x18" * 32).encode(),
    )
    assert_rejects_trailing(Auth3.decode, Auth3(b"\x19" * 32).encode())


def test_auth_proofs_are_bound_to_pid_nonce_ephemeral_key_and_public_key():
    x = scalar(31)
    device_pub = basepoint_mul(x)
    server_pub = point(32)
    eph_secret = scalar(33)
    eph_c = basepoint_mul(eph_secret)
    nonce_c = b"\x22" * 32
    pid = compute_pid(device_pub, nonce_c, eph_c, server_pub)
    client_proof = prove_auth_client_with_rng(fixed_rng(), x, pid, nonce_c, eph_c)

    assert verify_auth_client(device_pub, pid, nonce_c, eph_c, client_proof)
    assert not verify_auth_client(device_pub, bytes([pid[0] ^ 1]) + pid[1:], nonce_c, eph_c, client_proof)
    assert not verify_auth_client(device_pub, pid, nonce_c[:7] + bytes([nonce_c[7] ^ 0x80]) + nonce_c[8:], eph_c, client_proof)
    assert not verify_auth_client(point(35), pid, nonce_c, eph_c, client_proof)
    assert not verify_auth_client(device_pub, pid, nonce_c, point(36), client_proof)
    assert not verify_auth_client(device_pub, pid, nonce_c, eph_c, SchnorrProof(client_proof.a + point(37), client_proof.s))
    assert not verify_auth_client(device_pub, pid, nonce_c, eph_c, SchnorrProof(client_proof.a, client_proof.s + Scalar.from_u64(1)))


def test_role_proofs_session_keys_and_replay_are_bound_to_auth_transcript():
    allowed = [1, 2, 3]
    blind = scalar(41)
    delta = scalar(42)
    stored_c = make_role_commitment(Scalar.from_u64(2), blind)
    c_prime = stored_c + attr_h() * delta
    blind_prime = blind + delta
    pid = b"\x2b" * 32
    nonce_c = b"\x2c" * 32
    eph_c = point(45)

    rerand = prove_rerandomization_with_rng(fixed_rng(), stored_c, c_prime, delta, pid, nonce_c, eph_c)
    assert verify_rerandomization(stored_c, c_prime, pid, nonce_c, eph_c, rerand)
    assert not verify_rerandomization(stored_c, c_prime, bytes([pid[0] ^ 1]) + pid[1:], nonce_c, eph_c, rerand)

    branches = prove_role_set_membership_with_rng(fixed_rng(), allowed, c_prime, 2, blind_prime, pid, nonce_c, eph_c)
    assert verify_role_set_membership(allowed, c_prime, pid, nonce_c, eph_c, branches)
    assert not verify_role_set_membership([1, 3, 4], c_prime, pid, nonce_c, eph_c, branches)
    assert not verify_role_set_membership(allowed, c_prime, pid, nonce_c, eph_c, branches[:2])
    tampered = list(branches)
    a, c, s = tampered[0]
    tampered[0] = (a, c + Scalar.from_u64(1), s)
    assert not verify_role_set_membership(allowed, c_prime, pid, nonce_c, eph_c, tampered)

    client_eph = scalar(51)
    server_eph = scalar(52)
    ec = basepoint_mul(client_eph)
    es = basepoint_mul(server_eph)
    nonce_s = b"\x35" * 32
    key_c = derive_session_key(client_eph, es, nonce_c, nonce_s, pid, ec, es)
    key_s = derive_session_key(server_eph, ec, nonce_c, nonce_s, pid, ec, es)
    assert key_c == key_s
    assert key_c != derive_session_key(client_eph, es, nonce_c, bytes([nonce_s[0] ^ 1]) + nonce_s[1:], pid, ec, es)

    cache = MemoryReplayCache(16)
    key = replay_key(pid, nonce_c, eph_c)
    assert cache.insert(key)
    assert not cache.insert(key)
