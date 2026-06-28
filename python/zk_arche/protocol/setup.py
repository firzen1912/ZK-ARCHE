from __future__ import annotations

import hmac
from dataclasses import dataclass

from ..crypto import (
    SETUP_CHALLENGE_LEN, basepoint_mul, derive_device_id, derive_device_scalar,
    encode_role, make_role_commitment, prove_setup_client, prove_setup_server,
    random_bytes_32, random_scalar, reject_identity, verify_setup_client, verify_setup_server,
)
from ..errors import ErrorCode, ProtoError
from ..payloads import Setup1, Setup2, Setup3, decode_ack, encode_ack
from ..profile import Profile
from ..store import DeviceRecord, FsCredentialStore, FsRegistryStore, FsServerKeyStore, RoleCredential
from ..transport import ClientTransport
from ..wire import PKT_SETUP_1, PKT_SETUP_2, PKT_SETUP_3, PKT_SETUP_ACK, build_packet
from .common import rand_session_id, send_expect


def run_setup_client(transport: ClientTransport, store: FsCredentialStore, profile: Profile, pairing_token: str | None = None, allow_tofu: bool = False) -> None:
    root = store.load_or_create_device_root(True)
    device_id = derive_device_id(root)
    x = derive_device_scalar(root)
    device_pub = basepoint_mul(x)
    reject_identity(device_pub, "device_pub")

    role_cred = store.load_role_credential()
    if role_cred is None:
        role_code = 1
        blind = random_scalar()
        commitment = make_role_commitment(encode_role(role_code), blind)
        role_cred = RoleCredential(role_code, blind, commitment)
        store.save_role_credential(role_cred)

    pinned = store.load_server_pub()
    if pinned is None and not allow_tofu:
        raise ProtoError.wire(ErrorCode.CredentialMissing, "no pinned server key; pin OOB or pass --allow-tofu-setup (lab-only)")

    session_id = rand_session_id()
    client_nonce = random_bytes_32()
    s1 = Setup1(pairing_token.encode() if pairing_token else None, device_id, device_pub, client_nonce, role_cred.commitment)
    s2 = Setup2.decode(send_expect(transport, profile, PKT_SETUP_1, session_id, 0, s1.encode(), PKT_SETUP_2))

    if pinned is not None and not hmac.compare_digest(pinned.to_bytes(), s2.server_pub.to_bytes()):
        raise ProtoError.wire(ErrorCode.PeerKeyMismatch, "server key does not match pinned value")

    if not verify_setup_server(s2.server_pub, device_id, device_pub, client_nonce, s2.server_nonce, s2.setup_challenge, s2.server_proof):
        raise ProtoError.wire(ErrorCode.ProofVerifyFailed, "server setup proof failed verification")

    client_proof = prove_setup_client(x, device_id, device_pub, s2.server_pub, client_nonce, s2.server_nonce, s2.setup_challenge)
    ack = send_expect(transport, profile, PKT_SETUP_3, session_id, 1, Setup3(client_proof).encode(), PKT_SETUP_ACK)
    decode_ack(ack)
    store.save_server_pub(s2.server_pub)


@dataclass(frozen=True)
class PendingSetup:
    session_id: bytes
    device_id: bytes
    device_pub: object
    client_nonce: bytes
    server_nonce: bytes
    setup_challenge: bytes
    role_commitment: object
    response_packet: bytes


def handle_setup_1(key_store: FsServerKeyStore, session_id: bytes, seq: int, payload: bytes, require_pairing_token: str | None = None) -> PendingSetup:
    s1 = Setup1.decode(payload)
    if require_pairing_token is not None:
        provided = s1.pairing_token or b""
        if not hmac.compare_digest(provided, require_pairing_token.encode()):
            raise ProtoError.wire(ErrorCode.PairingTokenInvalid, "pairing token mismatch")
    server_sk = key_store.load_or_create_server_sk()
    server_pub = basepoint_mul(server_sk)
    server_nonce = random_bytes_32()
    sc = random_bytes_32()[:SETUP_CHALLENGE_LEN]
    server_proof = prove_setup_server(server_sk, s1.device_id, s1.device_pub, server_pub, s1.client_nonce, server_nonce, sc)
    s2 = Setup2(server_nonce, sc, server_pub, server_proof)
    response_packet = build_packet(PKT_SETUP_2, session_id, seq, s2.encode())
    return PendingSetup(session_id, s1.device_id, s1.device_pub, s1.client_nonce, server_nonce, sc, s1.role_commitment, response_packet)


def handle_setup_3(registry: FsRegistryStore, pending: PendingSetup, server_pub, session_id: bytes, seq: int, payload: bytes) -> bytes:
    s3 = Setup3.decode(payload)
    if not verify_setup_client(pending.device_id, pending.device_pub, server_pub, pending.client_nonce, pending.server_nonce, pending.setup_challenge, s3.client_proof):
        raise ProtoError.wire(ErrorCode.ProofVerifyFailed, "client setup proof failed verification")
    registry.save(pending.device_id, DeviceRecord(pending.device_pub, pending.role_commitment))
    return build_packet(PKT_SETUP_ACK, session_id, seq, encode_ack())
