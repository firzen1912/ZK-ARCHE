from __future__ import annotations

from zk_arche import DEFAULT_ALLOWED_ROLES
from zk_arche.crypto import basepoint_mul, derive_device_id, derive_device_scalar, encode_role, make_role_commitment, random_scalar
from zk_arche.payloads import Auth1, Auth2, Auth3, Setup1, Setup2, Setup3, decode_ack
from zk_arche.protocol.auth import handle_auth_1, handle_auth_3
from zk_arche.protocol.setup import handle_setup_1, handle_setup_3
from zk_arche.store import FsCredentialStore, FsRegistryStore, FsServerKeyStore, MemoryReplayCache, RoleCredential
from zk_arche.wire import parse_packet
from zk_arche.crypto import (
    compute_pid, derive_kc_keys, derive_session_key, hmac_tag, kc_transcript_hash,
    prove_auth_client, prove_rerandomization, prove_role_set_membership,
    prove_setup_client, rerandomize_commitment, random_bytes_32,
)


def test_setup_and_auth_handlers(tmp_path):
    client_store = FsCredentialStore(tmp_path / "client")
    registry = FsRegistryStore.with_dir(tmp_path / "server")
    key_store = FsServerKeyStore.with_dir(tmp_path / "server")
    server_pub = basepoint_mul(key_store.load_or_create_server_sk())

    root = client_store.load_or_create_device_root(True)
    device_id = derive_device_id(root)
    x = derive_device_scalar(root)
    device_pub = basepoint_mul(x)
    blind = random_scalar()
    role = RoleCredential(1, blind, make_role_commitment(encode_role(1), blind))
    client_store.save_role_credential(role)

    sid = b"S" * 16
    nc = random_bytes_32()
    p1 = Setup1(None, device_id, device_pub, nc, role.commitment).encode()
    pending_setup = handle_setup_1(key_store, sid, 0, p1)
    _hdr, s2_bytes = parse_packet(pending_setup.response_packet)
    s2 = Setup2.decode(s2_bytes)
    s3 = Setup3(prove_setup_client(x, device_id, device_pub, s2.server_pub, nc, s2.server_nonce, s2.setup_challenge)).encode()
    ack_packet = handle_setup_3(registry, pending_setup, server_pub, sid, 1, s3)
    _hdr, ack = parse_packet(ack_packet)
    decode_ack(ack)

    nonce_c = random_bytes_32()
    eph_secret = random_scalar()
    eph_c = basepoint_mul(eph_secret)
    pid = compute_pid(device_pub, nonce_c, eph_c, server_pub)
    client_proof = prove_auth_client(x, pid, nonce_c, eph_c)
    c_prime, blind_prime, delta = rerandomize_commitment(role.commitment, role.blind)
    rerand_proof = prove_rerandomization(role.commitment, c_prime, delta, pid, nonce_c, eph_c)
    branches = prove_role_set_membership(DEFAULT_ALLOWED_ROLES, c_prime, role.role_code, blind_prime, pid, nonce_c, eph_c)
    a1 = Auth1(pid, client_proof, nonce_c, eph_c, c_prime, rerand_proof, branches).encode()

    replay = MemoryReplayCache(16)
    pending_auth = handle_auth_1(key_store, registry, replay, DEFAULT_ALLOWED_ROLES, sid, 0, a1)
    _hdr, a2_bytes = parse_packet(pending_auth.response_packet)
    a2 = Auth2.decode(a2_bytes)
    session_key = derive_session_key(eph_secret, a2.eph_s, nonce_c, a2.nonce_s, pid, eph_c, a2.eph_s)
    th = kc_transcript_hash(pid, client_proof.a, client_proof.s, nonce_c, eph_c, a2.server_pub, a2.server_proof.a, a2.server_proof.s, a2.nonce_s, a2.eph_s)
    _k_s2c, k_c2s = derive_kc_keys(session_key, th)
    ack_packet = handle_auth_3(pending_auth, sid, 1, Auth3(hmac_tag(k_c2s, b"client finished", th)).encode())
    _hdr, ack = parse_packet(ack_packet)
    decode_ack(ack)
