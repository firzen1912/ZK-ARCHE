from __future__ import annotations

import hashlib
import hmac
from dataclasses import dataclass
from typing import Sequence

from .errors import ErrorCode, ProtoError
from .sodium import IDENTITY, canonical_scalar_bytes, random_bytes_32 as _random_bytes_32, sodium

NONCE_LEN = 32
SETUP_CHALLENGE_LEN = 16


def _require_len(data: bytes, n: int, what: str) -> bytes:
    data = bytes(data)
    if len(data) != n:
        raise ProtoError.wire(ErrorCode.PayloadTooShort, f"{what} must be {n} bytes")
    return data


@dataclass(frozen=True)
class Scalar:
    b: bytes

    def __post_init__(self) -> None:
        object.__setattr__(self, "b", decode_scalar_bytes(self.b, "scalar"))

    @classmethod
    def from_u64(cls, v: int) -> "Scalar":
        if v < 0:
            raise ValueError("scalar cannot be negative")
        return cls((v % (2**252 + 27742317777372353535851937790883648493)).to_bytes(32, "little"))

    @classmethod
    def reduce_wide(cls, wide: bytes) -> "Scalar":
        return cls(sodium().scalar_reduce(_require_len(wide, 64, "wide scalar")))

    @classmethod
    def random(cls) -> "Scalar":
        return cls(sodium().scalar_random())

    def to_bytes(self) -> bytes:
        return self.b

    def __add__(self, other: "Scalar") -> "Scalar":
        return Scalar(sodium().scalar_add(self.b, other.b))

    def __sub__(self, other: "Scalar") -> "Scalar":
        return Scalar(sodium().scalar_sub(self.b, other.b))

    def __mul__(self, other: "Scalar") -> "Scalar":
        return Scalar(sodium().scalar_mul(self.b, other.b))


@dataclass(frozen=True)
class Point:
    b: bytes

    def __post_init__(self) -> None:
        object.__setattr__(self, "b", decompress_point_bytes(self.b, "point"))

    @classmethod
    def base_mul(cls, scalar: Scalar) -> "Point":
        return cls(sodium().scalarmult_base(scalar.b))

    @classmethod
    def from_uniform_bytes(cls, wide: bytes) -> "Point":
        return cls(sodium().from_hash(_require_len(wide, 64, "uniform point hash")))

    def to_bytes(self) -> bytes:
        return self.b

    def __add__(self, other: "Point") -> "Point":
        return Point(sodium().point_add(self.b, other.b))

    def __sub__(self, other: "Point") -> "Point":
        return Point(sodium().point_sub(self.b, other.b))

    def __mul__(self, scalar: Scalar) -> "Point":
        return Point(sodium().scalarmult(scalar.b, self.b))

    def __rmul__(self, scalar: Scalar) -> "Point":
        return self.__mul__(scalar)


@dataclass(frozen=True)
class SchnorrProof:
    a: Point
    s: Scalar


def random_bytes_32() -> bytes:
    return _random_bytes_32()


def random_scalar() -> Scalar:
    return Scalar.random()


def scalar_from_wide_bytes(wide: bytes) -> Scalar:
    return Scalar.reduce_wide(wide)


def decode_scalar_bytes(data: bytes, what: str) -> bytes:
    data = _require_len(data, 32, what)
    if not canonical_scalar_bytes(data):
        raise ProtoError.wire(ErrorCode.NonCanonicalScalar, f"non-canonical scalar: {what}")
    return data


def decode_scalar(data: bytes, what: str) -> Scalar:
    return Scalar(decode_scalar_bytes(data, what))


def decompress_point_bytes(data: bytes, what: str) -> bytes:
    data = _require_len(data, 32, what)
    if not sodium().is_valid_point(data):
        raise ProtoError.wire(ErrorCode.InvalidPoint, f"invalid point: {what}")
    if data == IDENTITY:
        raise ProtoError.wire(ErrorCode.IdentityPoint, f"{what} is the identity point")
    return data


def decompress_point(data: bytes, what: str) -> Point:
    return Point(decompress_point_bytes(data, what))


def reject_identity(p: Point, what: str) -> None:
    if p.b == IDENTITY:
        raise ProtoError.wire(ErrorCode.IdentityPoint, f"{what} is the identity point")


def basepoint_mul(s: Scalar) -> Point:
    return Point.base_mul(s)


def hash_to_point(label: bytes) -> Point:
    h = hashlib.sha512()
    h.update(b"ristretto-hash-to-point-v1")
    h.update(label)
    return Point.from_uniform_bytes(h.digest())


def attr_h() -> Point:
    return hash_to_point(b"iot-auth/attr-h/v1")


def encode_role(role_code: int) -> Scalar:
    return Scalar.from_u64(role_code)


def make_role_commitment(role_scalar: Scalar, blind: Scalar) -> Point:
    return basepoint_mul(role_scalar) + attr_h() * blind


def _challenge(domain: bytes, fields: Sequence[tuple[bytes, object]]) -> Scalar:
    from .transcript import TVal, challenge
    conv = []
    for label, value in fields:
        if isinstance(value, Point):
            conv.append((label, TVal.point(value)))
        elif isinstance(value, Scalar):
            conv.append((label, TVal.scalar(value)))
        elif isinstance(value, (bytes, bytearray)):
            conv.append((label, TVal.bytes(bytes(value))))
        elif isinstance(value, int):
            conv.append((label, TVal.u64(value)))
        else:
            raise TypeError(value)
    return challenge(domain, conv)


def compute_pid(device_pub: Point, nonce_c: bytes, eph_c: Point, server_pub: Point) -> bytes:
    from .transcript import T_PID
    nonce_c = _require_len(nonce_c, 32, "nonce_c")
    h = hashlib.sha256()
    h.update(len(T_PID).to_bytes(4, "little"))
    h.update(T_PID)
    h.update(device_pub.b)
    h.update(nonce_c)
    h.update(eph_c.b)
    h.update(server_pub.b)
    return h.digest()


def prove_setup_client_with_rng(rng, x: Scalar, device_id: bytes, device_pub: Point, server_pub: Point, client_nonce: bytes, server_nonce: bytes, setup_challenge: bytes) -> SchnorrProof:
    from . import transcript as tr
    r = scalar_from_wide_bytes(rng(64))
    a = basepoint_mul(r)
    c = _challenge(tr.T_SETUP, [
        (b"role", b"client"), (b"device_id", _require_len(device_id, 32, "device_id")),
        (b"device_pub", device_pub), (b"server_pub", server_pub), (b"a", a),
        (b"client_nonce", _require_len(client_nonce, 32, "client_nonce")),
        (b"server_nonce", _require_len(server_nonce, 32, "server_nonce")),
        (b"setup_challenge", _require_len(setup_challenge, SETUP_CHALLENGE_LEN, "setup_challenge")),
    ])
    return SchnorrProof(a, r + c * x)


def prove_setup_client(x: Scalar, device_id: bytes, device_pub: Point, server_pub: Point, client_nonce: bytes, server_nonce: bytes, setup_challenge: bytes) -> SchnorrProof:
    return prove_setup_client_with_rng(lambda n: _random_bytes_32() + _random_bytes_32(), x, device_id, device_pub, server_pub, client_nonce, server_nonce, setup_challenge)


def verify_setup_client(device_id: bytes, device_pub: Point, server_pub: Point, client_nonce: bytes, server_nonce: bytes, setup_challenge: bytes, proof: SchnorrProof) -> bool:
    from . import transcript as tr
    c = _challenge(tr.T_SETUP, [
        (b"role", b"client"), (b"device_id", _require_len(device_id, 32, "device_id")),
        (b"device_pub", device_pub), (b"server_pub", server_pub), (b"a", proof.a),
        (b"client_nonce", _require_len(client_nonce, 32, "client_nonce")),
        (b"server_nonce", _require_len(server_nonce, 32, "server_nonce")),
        (b"setup_challenge", _require_len(setup_challenge, SETUP_CHALLENGE_LEN, "setup_challenge")),
    ])
    return basepoint_mul(proof.s) == proof.a + device_pub * c


def prove_setup_server_with_rng(rng, server_sk: Scalar, device_id: bytes, device_pub: Point, server_pub: Point, client_nonce: bytes, server_nonce: bytes, setup_challenge: bytes) -> SchnorrProof:
    from . import transcript as tr
    r = scalar_from_wide_bytes(rng(64))
    a = basepoint_mul(r)
    c = _challenge(tr.T_SETUP_SERVER, [
        (b"role", b"server"), (b"device_id", _require_len(device_id, 32, "device_id")),
        (b"device_pub", device_pub), (b"server_pub", server_pub), (b"a", a),
        (b"client_nonce", _require_len(client_nonce, 32, "client_nonce")),
        (b"server_nonce", _require_len(server_nonce, 32, "server_nonce")),
        (b"setup_challenge", _require_len(setup_challenge, SETUP_CHALLENGE_LEN, "setup_challenge")),
    ])
    return SchnorrProof(a, r + c * server_sk)


def prove_setup_server(server_sk: Scalar, device_id: bytes, device_pub: Point, server_pub: Point, client_nonce: bytes, server_nonce: bytes, setup_challenge: bytes) -> SchnorrProof:
    return prove_setup_server_with_rng(lambda n: _random_bytes_32() + _random_bytes_32(), server_sk, device_id, device_pub, server_pub, client_nonce, server_nonce, setup_challenge)


def verify_setup_server(server_pub: Point, device_id: bytes, device_pub: Point, client_nonce: bytes, server_nonce: bytes, setup_challenge: bytes, proof: SchnorrProof) -> bool:
    from . import transcript as tr
    c = _challenge(tr.T_SETUP_SERVER, [
        (b"role", b"server"), (b"device_id", _require_len(device_id, 32, "device_id")),
        (b"device_pub", device_pub), (b"server_pub", server_pub), (b"a", proof.a),
        (b"client_nonce", _require_len(client_nonce, 32, "client_nonce")),
        (b"server_nonce", _require_len(server_nonce, 32, "server_nonce")),
        (b"setup_challenge", _require_len(setup_challenge, SETUP_CHALLENGE_LEN, "setup_challenge")),
    ])
    return basepoint_mul(proof.s) == proof.a + server_pub * c


def prove_auth_client_with_rng(rng, x: Scalar, pid: bytes, nonce_c: bytes, eph_c: Point) -> SchnorrProof:
    from . import transcript as tr
    pubkey = basepoint_mul(x)
    r = scalar_from_wide_bytes(rng(64))
    a = basepoint_mul(r)
    c = _challenge(tr.T_CLIENT_V2, [(b"pid", _require_len(pid, 32, "pid")), (b"pubkey", pubkey), (b"a", a), (b"nonce_c", _require_len(nonce_c, 32, "nonce_c")), (b"eph_c", eph_c)])
    return SchnorrProof(a, r + c * x)


def prove_auth_client(x: Scalar, pid: bytes, nonce_c: bytes, eph_c: Point) -> SchnorrProof:
    return prove_auth_client_with_rng(lambda n: _random_bytes_32() + _random_bytes_32(), x, pid, nonce_c, eph_c)


def verify_auth_client(device_pub: Point, pid: bytes, nonce_c: bytes, eph_c: Point, proof: SchnorrProof) -> bool:
    from . import transcript as tr
    c = _challenge(tr.T_CLIENT_V2, [(b"pid", _require_len(pid, 32, "pid")), (b"pubkey", device_pub), (b"a", proof.a), (b"nonce_c", _require_len(nonce_c, 32, "nonce_c")), (b"eph_c", eph_c)])
    return basepoint_mul(proof.s) == proof.a + device_pub * c


def prove_auth_server_with_rng(rng, server_sk: Scalar, nonce_s: bytes, eph_s: Point) -> SchnorrProof:
    from . import transcript as tr
    r = scalar_from_wide_bytes(rng(64))
    a = basepoint_mul(r)
    server_pub = basepoint_mul(server_sk)
    c = _challenge(tr.T_SERVER, [(b"pubkey", server_pub), (b"a", a), (b"nonce_s", _require_len(nonce_s, 32, "nonce_s")), (b"eph_s", eph_s)])
    return SchnorrProof(a, r + c * server_sk)


def prove_auth_server(server_sk: Scalar, nonce_s: bytes, eph_s: Point) -> SchnorrProof:
    return prove_auth_server_with_rng(lambda n: _random_bytes_32() + _random_bytes_32(), server_sk, nonce_s, eph_s)


def verify_auth_server(server_pub: Point, nonce_s: bytes, eph_s: Point, proof: SchnorrProof) -> bool:
    from . import transcript as tr
    c = _challenge(tr.T_SERVER, [(b"pubkey", server_pub), (b"a", proof.a), (b"nonce_s", _require_len(nonce_s, 32, "nonce_s")), (b"eph_s", eph_s)])
    return basepoint_mul(proof.s) == proof.a + server_pub * c


def rerandomize_commitment(stored_c: Point, stored_blind: Scalar) -> tuple[Point, Scalar, Scalar]:
    delta = random_scalar()
    c_prime = stored_c + attr_h() * delta
    blind_prime = stored_blind + delta
    return c_prime, blind_prime, delta


def prove_rerandomization_with_rng(rng, stored_c: Point, c_prime: Point, delta: Scalar, pid: bytes, nonce_c: bytes, eph_c: Point) -> SchnorrProof:
    from . import transcript as tr
    h = attr_h()
    r = scalar_from_wide_bytes(rng(64))
    a = h * r
    c = _challenge(tr.T_ROLE_RERAND, [(b"pid", _require_len(pid, 32, "pid")), (b"nonce_c", _require_len(nonce_c, 32, "nonce_c")), (b"eph_c", eph_c), (b"stored_c", stored_c), (b"c_prime", c_prime), (b"a", a)])
    return SchnorrProof(a, r + c * delta)


def prove_rerandomization(stored_c: Point, c_prime: Point, delta: Scalar, pid: bytes, nonce_c: bytes, eph_c: Point) -> SchnorrProof:
    return prove_rerandomization_with_rng(lambda n: _random_bytes_32() + _random_bytes_32(), stored_c, c_prime, delta, pid, nonce_c, eph_c)


def verify_rerandomization(stored_c: Point, c_prime: Point, pid: bytes, nonce_c: bytes, eph_c: Point, proof: SchnorrProof) -> bool:
    from . import transcript as tr
    h = attr_h()
    c = _challenge(tr.T_ROLE_RERAND, [(b"pid", _require_len(pid, 32, "pid")), (b"nonce_c", _require_len(nonce_c, 32, "nonce_c")), (b"eph_c", eph_c), (b"stored_c", stored_c), (b"c_prime", c_prime), (b"a", proof.a)])
    return h * proof.s == proof.a + (c_prime - stored_c) * c

SetBranch = tuple[Point, Scalar, Scalar]


def prove_role_set_membership_with_rng(rng, allowed_roles: Sequence[int], c_prime: Point, role_code: int, blind_prime: Scalar, pid: bytes, nonce_c: bytes, eph_c: Point) -> list[SetBranch]:
    from . import transcript as tr
    h = attr_h()
    n = len(allowed_roles)
    if role_code not in allowed_roles:
        raise ProtoError.wire(ErrorCode.RoleNotPermitted, "role not in allowed_roles")
    true_index = list(allowed_roles).index(role_code)
    y_points = [c_prime - basepoint_mul(Scalar.from_u64(r)) for r in allowed_roles]
    zero = Scalar.from_u64(0)
    a_points = [None] * n
    c_vals = [zero] * n
    s_vals = [zero] * n
    w_true = zero
    for i in range(n):
        if i == true_index:
            w = scalar_from_wide_bytes(rng(64))
            a_points[i] = h * w
            w_true = w
        else:
            c_i = scalar_from_wide_bytes(rng(64))
            s_i = scalar_from_wide_bytes(rng(64))
            a_points[i] = h * s_i - y_points[i] * c_i
            c_vals[i] = c_i
            s_vals[i] = s_i
    t = tr.Transcript(tr.T_ROLE_SET)
    t.append_message(b"pid", _require_len(pid, 32, "pid"))
    t.append_message(b"nonce_c", _require_len(nonce_c, 32, "nonce_c"))
    t.append_point(b"eph_c", eph_c)
    t.append_point(b"c_prime", c_prime)
    for i, r in enumerate(allowed_roles):
        t.append_message(f"r_{i}".encode(), int(r).to_bytes(8, "little"))
    for i, a in enumerate(a_points):
        if a is None:
            raise ProtoError.internal("role-set proof branch was not constructed")
        t.append_point(f"A_{i}".encode(), a)
    master_c = t.challenge_scalar()
    sum_sim = zero
    for i, c_i in enumerate(c_vals):
        if i != true_index:
            sum_sim = sum_sim + c_i
    c_true = master_c - sum_sim
    s_true = w_true + c_true * blind_prime
    c_vals[true_index] = c_true
    s_vals[true_index] = s_true
    branches: list[SetBranch] = []
    for i, a in enumerate(a_points):
        if a is None:
            raise ProtoError.internal("role-set proof branch was not constructed")
        branches.append((a, c_vals[i], s_vals[i]))
    return branches


def prove_role_set_membership(allowed_roles: Sequence[int], c_prime: Point, role_code: int, blind_prime: Scalar, pid: bytes, nonce_c: bytes, eph_c: Point) -> list[SetBranch]:
    return prove_role_set_membership_with_rng(lambda n: _random_bytes_32() + _random_bytes_32(), allowed_roles, c_prime, role_code, blind_prime, pid, nonce_c, eph_c)


def verify_role_set_membership(allowed_roles: Sequence[int], c_prime: Point, pid: bytes, nonce_c: bytes, eph_c: Point, branches: Sequence[SetBranch]) -> bool:
    from . import transcript as tr
    h = attr_h()
    n = len(allowed_roles)
    if len(branches) != n:
        return False
    y_points = [c_prime - basepoint_mul(Scalar.from_u64(r)) for r in allowed_roles]
    t = tr.Transcript(tr.T_ROLE_SET)
    t.append_message(b"pid", _require_len(pid, 32, "pid"))
    t.append_message(b"nonce_c", _require_len(nonce_c, 32, "nonce_c"))
    t.append_point(b"eph_c", eph_c)
    t.append_point(b"c_prime", c_prime)
    for i, r in enumerate(allowed_roles):
        t.append_message(f"r_{i}".encode(), int(r).to_bytes(8, "little"))
    for i, (a, _c, _s) in enumerate(branches):
        t.append_point(f"A_{i}".encode(), a)
    master_c = t.challenge_scalar()
    sum_c = Scalar.from_u64(0)
    for _a, c, _s in branches:
        sum_c = sum_c + c
    if sum_c != master_c:
        return False
    for i, (a, c, s) in enumerate(branches):
        if h * s != a + y_points[i] * c:
            return False
    return True


def hkdf_sha256(ikm: bytes, salt: bytes | None, info: bytes, length: int) -> bytes:
    if salt is None:
        salt = b"\x00" * hashlib.sha256().digest_size
    prk = hmac.new(salt, ikm, hashlib.sha256).digest()
    okm = b""
    t = b""
    counter = 1
    while len(okm) < length:
        t = hmac.new(prk, t + info + bytes([counter]), hashlib.sha256).digest()
        okm += t
        counter += 1
    return okm[:length]


def derive_session_key(eph_secret: Scalar, peer_eph_pub: Point, nonce_c: bytes, nonce_s: bytes, pid: bytes, eph_c: Point, eph_s: Point) -> bytes:
    shared = peer_eph_pub * eph_secret
    salt = _require_len(nonce_c, 32, "nonce_c") + _require_len(nonce_s, 32, "nonce_s")
    info = b"session key v2" + _require_len(pid, 32, "pid") + eph_c.b + eph_s.b
    return hkdf_sha256(shared.b, salt, info, 32)


def kc_transcript_hash(pid: bytes, a_c: Point, s_c: Scalar, nonce_c: bytes, eph_c: Point, server_pub: Point, a_s: Point, s_s: Scalar, nonce_s: bytes, eph_s: Point) -> bytes:
    from . import transcript as tr
    t = tr.Transcript(tr.T_KC_V2)
    t.append_message(b"pid", _require_len(pid, 32, "pid"))
    t.append_point(b"a_c", a_c)
    t.append_scalar(b"s_c", s_c)
    t.append_message(b"nonce_c", _require_len(nonce_c, 32, "nonce_c"))
    t.append_point(b"eph_c", eph_c)
    t.append_point(b"server_pub", server_pub)
    t.append_point(b"a_s", a_s)
    t.append_scalar(b"s_s", s_s)
    t.append_message(b"nonce_s", _require_len(nonce_s, 32, "nonce_s"))
    t.append_point(b"eph_s", eph_s)
    return t.hash_sha256()


def derive_kc_keys(session_key: bytes, th: bytes) -> tuple[bytes, bytes]:
    session_key = _require_len(session_key, 32, "session_key")
    th = _require_len(th, 32, "transcript_hash")
    return hkdf_sha256(session_key, th, b"kc s2c", 32), hkdf_sha256(session_key, th, b"kc c2s", 32)


def hmac_tag(key: bytes, label: bytes, th: bytes) -> bytes:
    return hmac.new(_require_len(key, 32, "key"), label + _require_len(th, 32, "transcript_hash"), hashlib.sha256).digest()


def derive_device_id(root: bytes) -> bytes:
    root = _require_len(root, 32, "device root")
    h = hashlib.blake2b(digest_size=32)
    h.update(b"device-id")
    h.update(root)
    return h.digest()


def derive_device_scalar(root: bytes) -> Scalar:
    root = _require_len(root, 32, "device root")
    h = hashlib.blake2b(digest_size=64)
    h.update(b"device-auth-v1")
    h.update(root)
    return scalar_from_wide_bytes(h.digest())
