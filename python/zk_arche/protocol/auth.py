from __future__ import annotations

import hmac
from dataclasses import dataclass
from typing import Sequence

from ..crypto import (
    basepoint_mul, compute_pid, derive_device_id, derive_device_scalar, derive_kc_keys,
    derive_session_key, hmac_tag, kc_transcript_hash, prove_auth_client, prove_auth_server,
    prove_rerandomization, prove_role_set_membership, random_bytes_32, random_scalar,
    reject_identity, rerandomize_commitment, verify_auth_client, verify_auth_server,
    verify_rerandomization, verify_role_set_membership,
)
from ..errors import ErrorCode, ProtoError
from ..payloads import Auth1, Auth2, Auth3, decode_ack, encode_ack
from ..profile import Profile
from ..store import FsCredentialStore, FsRegistryStore, FsServerKeyStore, MemoryReplayCache, replay_key
from ..transport import ClientTransport
from ..wire import PKT_AUTH_1, PKT_AUTH_2, PKT_AUTH_3, PKT_AUTH_ACK, build_packet
from .common import rand_session_id, send_expect


def run_auth_client(transport: ClientTransport, store: FsCredentialStore, profile: Profile, allowed_roles: Sequence[int]) -> bytes:
    root = store.load_or_create_device_root(False)
    _device_id = derive_device_id(root)
    x = derive_device_scalar(root)
    device_pub = basepoint_mul(x)
    reject_identity(device_pub, "device_pub")
    server_pub = store.load_server_pub()
    if server_pub is None:
        raise ProtoError.wire(ErrorCode.CredentialMissing, "no pinned server key; run --setup first")
    role = store.load_role_credential()
    if role is None:
        raise ProtoError.wire(ErrorCode.CredentialMissing, "role credential missing")

    nonce_c = random_bytes_32()
    eph_secret = random_scalar()
    eph_c = basepoint_mul(eph_secret)
    pid = compute_pid(device_pub, nonce_c, eph_c, server_pub)
    client_proof = prove_auth_client(x, pid, nonce_c, eph_c)
    c_prime, blind_prime, delta = rerandomize_commitment(role.commitment, role.blind)
    rerand_proof = prove_rerandomization(role.commitment, c_prime, delta, pid, nonce_c, eph_c)
    branches = prove_role_set_membership(allowed_roles, c_prime, role.role_code, blind_prime, pid, nonce_c, eph_c)
    a1 = Auth1(pid, client_proof, nonce_c, eph_c, c_prime, rerand_proof, branches)
    session_id = rand_session_id()
    a2 = Auth2.decode(send_expect(transport, profile, PKT_AUTH_1, session_id, 0, a1.encode(), PKT_AUTH_2))

    if not hmac.compare_digest(a2.server_pub.to_bytes(), server_pub.to_bytes()):
        raise ProtoError.wire(ErrorCode.PeerKeyMismatch, "server pubkey mismatch")
    if not verify_auth_server(a2.server_pub, a2.nonce_s, a2.eph_s, a2.server_proof):
        raise ProtoError.wire(ErrorCode.ProofVerifyFailed, "server auth proof invalid")
    session_key = derive_session_key(eph_secret, a2.eph_s, nonce_c, a2.nonce_s, pid, eph_c, a2.eph_s)
    th = kc_transcript_hash(pid, client_proof.a, client_proof.s, nonce_c, eph_c, a2.server_pub, a2.server_proof.a, a2.server_proof.s, a2.nonce_s, a2.eph_s)
    k_s2c, k_c2s = derive_kc_keys(session_key, th)
    expected_tag_s = hmac_tag(k_s2c, b"server finished", th)
    if not hmac.compare_digest(expected_tag_s, a2.tag_s):
        raise ProtoError.wire(ErrorCode.KeyConfirmFailed, "server finished tag mismatch")
    tag_c = hmac_tag(k_c2s, b"client finished", th)
    ack = send_expect(transport, profile, PKT_AUTH_3, session_id, 1, Auth3(tag_c).encode(), PKT_AUTH_ACK)
    decode_ack(ack)
    return session_key


@dataclass(frozen=True)
class PendingAuth:
    session_id: bytes
    pid: bytes
    device_id: bytes
    expected_tag_c: bytes
    response_packet: bytes


def handle_auth_1(key_store: FsServerKeyStore, registry: FsRegistryStore, replay: MemoryReplayCache, allowed_roles: Sequence[int], session_id: bytes, seq: int, payload: bytes) -> PendingAuth:
    a1 = Auth1.decode(payload)
    rkey = replay_key(a1.pid, a1.nonce_c, a1.eph_c)
    if replay.contains(rkey):
        raise ProtoError.wire(ErrorCode.ReplayDetected, "AUTH_1 replay")
    server_sk = key_store.load_or_create_server_sk()
    server_pub = basepoint_mul(server_sk)

    found = None
    for device_id, rec in registry.iter():
        if compute_pid(rec.pubkey, a1.nonce_c, a1.eph_c, server_pub) == a1.pid:
            found = (device_id, rec)
            break
    if found is None:
        raise ProtoError.wire(ErrorCode.UnknownDevice, "no enrolled device matches pid")
    device_id, rec = found
    if not verify_auth_client(rec.pubkey, a1.pid, a1.nonce_c, a1.eph_c, a1.client_proof):
        raise ProtoError.wire(ErrorCode.ProofVerifyFailed, "client auth proof invalid")
    if not verify_rerandomization(rec.role_commitment, a1.c_prime, a1.pid, a1.nonce_c, a1.eph_c, a1.rerand_proof):
        raise ProtoError.wire(ErrorCode.ProofVerifyFailed, "rerand proof invalid")
    if not verify_role_set_membership(allowed_roles, a1.c_prime, a1.pid, a1.nonce_c, a1.eph_c, a1.branches):
        raise ProtoError.wire(ErrorCode.RoleNotPermitted, "role set proof invalid")

    nonce_s = random_bytes_32()
    eph_s_sk = random_scalar()
    eph_s = basepoint_mul(eph_s_sk)
    server_proof = prove_auth_server(server_sk, nonce_s, eph_s)
    session_key = derive_session_key(eph_s_sk, a1.eph_c, a1.nonce_c, nonce_s, a1.pid, a1.eph_c, eph_s)
    th = kc_transcript_hash(a1.pid, a1.client_proof.a, a1.client_proof.s, a1.nonce_c, a1.eph_c, server_pub, server_proof.a, server_proof.s, nonce_s, eph_s)
    k_s2c, k_c2s = derive_kc_keys(session_key, th)
    tag_s = hmac_tag(k_s2c, b"server finished", th)
    expected_tag_c = hmac_tag(k_c2s, b"client finished", th)
    response_packet = build_packet(PKT_AUTH_2, session_id, seq, Auth2(server_pub, server_proof, nonce_s, eph_s, tag_s).encode())
    replay.insert(rkey)
    return PendingAuth(session_id, a1.pid, device_id, expected_tag_c, response_packet)


def handle_auth_3(pending: PendingAuth, session_id: bytes, seq: int, payload: bytes) -> bytes:
    a3 = Auth3.decode(payload)
    if not hmac.compare_digest(a3.tag_c, pending.expected_tag_c):
        raise ProtoError.wire(ErrorCode.KeyConfirmFailed, "client finished tag mismatch")
    return build_packet(PKT_AUTH_ACK, session_id, seq, encode_ack())
