from __future__ import annotations

import json
from pathlib import Path

from zk_arche.crypto import (
    Point, Scalar, SchnorrProof, basepoint_mul, compute_pid, derive_kc_keys,
    derive_session_key, hmac_tag, kc_transcript_hash, prove_auth_client_with_rng,
    prove_rerandomization_with_rng, verify_auth_client, verify_rerandomization,
    verify_role_set_membership, scalar_from_wide_bytes,
)
from zk_arche.transcript import Transcript

V = Path(__file__).resolve().parents[1] / "test-vectors" / "0x0001"


def hx(s: str) -> bytes:
    return bytes.fromhex(s)


def scalar(s: str) -> Scalar:
    return Scalar(hx(s))


def point(s: str) -> Point:
    return Point(hx(s))


def fixed_rng(data: bytes):
    pos = 0
    def rng(n: int) -> bytes:
        nonlocal pos
        out = data[pos:pos+n]
        pos += n
        assert len(out) == n
        return out
    return rng


def test_transcript_basic():
    tv = json.loads((V / "transcript.json").read_text())
    t = Transcript(tv["inputs"]["domain"].encode())
    for label, value in tv["inputs"]["fields"]:
        t.append_message(label.encode(), hx(value))
    assert t.as_bytes().hex() == tv["expected"]["transcript_bytes"]
    assert t.challenge_scalar().to_bytes().hex() == tv["expected"]["challenge_scalar"]


def test_pid_basic():
    tv = json.loads((V / "pid.json").read_text())
    i = tv["inputs"]
    pid = compute_pid(point(i["device_pub"]), hx(i["nonce_c"]), point(i["eph_c"]), point(i["server_pub"]))
    assert pid.hex() == tv["expected"]["pid"]


def test_schnorr_auth_client():
    tv = json.loads((V / "schnorr_auth_client.json").read_text())
    i = tv["inputs"]
    proof = prove_auth_client_with_rng(fixed_rng(hx(i["drbg_bytes_for_r"])), scalar(i["x"]), hx(i["pid"]), hx(i["nonce_c"]), point(i["eph_c"]))
    assert proof.a.to_bytes().hex() == tv["expected"]["proof_a"]
    assert proof.s.to_bytes().hex() == tv["expected"]["proof_s"]
    assert verify_auth_client(point(i["device_pub"]), hx(i["pid"]), hx(i["nonce_c"]), point(i["eph_c"]), proof)


def test_rerandomization():
    tv = json.loads((V / "rerandomization.json").read_text())
    i = tv["inputs"]
    stored_c = point(i["stored_c"])
    blind = scalar(i["blind"])
    delta = scalar(i["delta"])
    from zk_arche.crypto import attr_h
    c_prime = stored_c + attr_h() * delta
    assert c_prime.to_bytes().hex() == tv["expected"]["c_prime"]
    proof = prove_rerandomization_with_rng(fixed_rng(hx(i["drbg_bytes_for_r"])), stored_c, c_prime, delta, hx(i["pid"]), hx(i["nonce_c"]), point(i["eph_c"]))
    assert proof.a.to_bytes().hex() == tv["expected"]["proof_a"]
    assert proof.s.to_bytes().hex() == tv["expected"]["proof_s"]
    assert verify_rerandomization(stored_c, c_prime, hx(i["pid"]), hx(i["nonce_c"]), point(i["eph_c"]), proof)


def test_role_set_membership_verify_published_branches():
    tv = json.loads((V / "role_set_membership.json").read_text())
    i = tv["inputs"]
    branches = [(point(b["a"]), scalar(b["c"]), scalar(b["s"])) for b in tv["expected"]["branches"]]
    assert verify_role_set_membership(i["allowed_roles"], point(i["c_prime"]), hx(i["pid"]), hx(i["nonce_c"]), point(i["eph_c"]), branches)


def test_kdf_kc_vector():
    tv = json.loads((V / "kdf_kc.json").read_text())
    i = tv["inputs"]
    e_c_sk = scalar(i["client_eph_sk"])
    e_s_sk = scalar(i["server_eph_sk"])
    eph_c = point(i["eph_c"])
    eph_s = point(i["eph_s"])
    k_c = derive_session_key(e_c_sk, eph_s, hx(i["nonce_c"]), hx(i["nonce_s"]), hx(i["pid"]), eph_c, eph_s)
    k_s = derive_session_key(e_s_sk, eph_c, hx(i["nonce_c"]), hx(i["nonce_s"]), hx(i["pid"]), eph_c, eph_s)
    assert k_c.hex() == tv["expected"]["session_key_from_client"]
    assert k_s.hex() == tv["expected"]["session_key_from_server"]
    th = kc_transcript_hash(hx(i["pid"]), point(i["a_c"]), scalar(i["s_c"]), hx(i["nonce_c"]), eph_c, point(i["server_pub"]), point(i["a_s"]), scalar(i["s_s"]), hx(i["nonce_s"]), eph_s)
    assert th.hex() == tv["expected"]["transcript_hash"]
    k_s2c, k_c2s = derive_kc_keys(k_c, th)
    assert k_s2c.hex() == tv["expected"]["k_s2c"]
    assert k_c2s.hex() == tv["expected"]["k_c2s"]
    assert hmac_tag(k_s2c, b"server finished", th).hex() == tv["expected"]["tag_s"]
    assert hmac_tag(k_c2s, b"client finished", th).hex() == tv["expected"]["tag_c"]
