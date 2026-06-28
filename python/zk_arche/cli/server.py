from __future__ import annotations

import argparse
import socket
import threading
import time
from dataclasses import dataclass, field

from .. import DEFAULT_ALLOWED_ROLES
from ..crypto import basepoint_mul
from ..errors import ErrorCode, ProtoError
from ..profile import Profile
from ..protocol.auth import PendingAuth, handle_auth_1, handle_auth_3
from ..protocol.setup import PendingSetup, handle_setup_1, handle_setup_3
from ..store import FsRegistryStore, FsServerKeyStore, MemoryReplayCache
from ..transport import _recv_framed, _send_framed, _split_addr
from ..wire import PKT_AUTH_1, PKT_AUTH_3, PKT_SETUP_1, PKT_SETUP_3, build_error, parse_packet


@dataclass
class CachedResponse:
    peer: object
    packet: bytes
    created_at: float


@dataclass
class ServerState:
    profile: Profile
    registry: FsRegistryStore
    key_store: FsServerKeyStore
    require_pairing_token: str | None = None
    setup_sessions: dict[bytes, PendingSetup] = field(default_factory=dict)
    auth_sessions: dict[bytes, PendingAuth] = field(default_factory=dict)
    response_cache: dict[tuple[bytes, int], CachedResponse] = field(default_factory=dict)
    replay: MemoryReplayCache = field(default_factory=lambda: MemoryReplayCache(4096))
    lock: threading.Lock = field(default_factory=threading.Lock)

    @property
    def server_pub(self):
        return basepoint_mul(self.key_store.load_or_create_server_sk())


def dispatch_packet(state: ServerState, peer: object, data: bytes) -> bytes | None:
    try:
        hdr, payload = parse_packet(data)
    except ProtoError as exc:
        print(f"{peer}: framing error: {exc}")
        return None
    with state.lock:
        return _dispatch_packet_locked(state, peer, hdr, payload)


def _dispatch_packet_locked(state: ServerState, peer: object, hdr, payload: bytes) -> bytes | None:
    cached = state.response_cache.get((hdr.session_id, hdr.seq))
    if cached is not None and cached.peer == peer:
        return cached.packet
    try:
        if hdr.pkt_type == PKT_SETUP_1:
            p = handle_setup_1(state.key_store, hdr.session_id, hdr.seq, payload, state.require_pairing_token)
            state.setup_sessions[hdr.session_id] = p
            resp = p.response_packet
        elif hdr.pkt_type == PKT_SETUP_3:
            p = state.setup_sessions.get(hdr.session_id)
            if p is None:
                raise ProtoError.wire(ErrorCode.UnknownSession, "no setup session for SETUP_3")
            resp = handle_setup_3(state.registry, p, state.server_pub, hdr.session_id, hdr.seq, payload)
            state.setup_sessions.pop(hdr.session_id, None)
        elif hdr.pkt_type == PKT_AUTH_1:
            p = handle_auth_1(state.key_store, state.registry, state.replay, DEFAULT_ALLOWED_ROLES, hdr.session_id, hdr.seq, payload)
            state.auth_sessions[hdr.session_id] = p
            resp = p.response_packet
        elif hdr.pkt_type == PKT_AUTH_3:
            p = state.auth_sessions.get(hdr.session_id)
            if p is None:
                raise ProtoError.wire(ErrorCode.UnknownSession, "no auth session for AUTH_3")
            resp = handle_auth_3(p, hdr.session_id, hdr.seq, payload)
            state.auth_sessions.pop(hdr.session_id, None)
        else:
            raise ProtoError.wire(ErrorCode.UnknownPacketType, f"unknown packet type 0x{hdr.pkt_type:02x}")
    except ProtoError as exc:
        code = exc.wire_code() or ErrorCode.Unspecified
        resp = build_error(hdr.session_id, hdr.seq, code, exc.msg)
    state.response_cache[(hdr.session_id, hdr.seq)] = CachedResponse(peer, resp, time.time())
    if len(state.response_cache) > state.profile.max_cached_responses:
        oldest = min(state.response_cache, key=lambda k: state.response_cache[k].created_at)
        state.response_cache.pop(oldest, None)
    return resp


def run_udp(bind: str, state: ServerState) -> None:
    host, port = _split_addr(bind)
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((host, port))
    print(f"Server listening UDP on {host}:{port}")
    while True:
        data, peer = sock.recvfrom(2048)
        resp = dispatch_packet(state, peer, data)
        if resp:
            sock.sendto(resp, peer)


def handle_tcp_conn(conn: socket.socket, peer, state: ServerState) -> None:
    with conn:
        while True:
            try:
                data = _recv_framed(conn)
            except Exception:
                return
            resp = dispatch_packet(state, peer, data)
            if resp:
                _send_framed(conn, resp)


def run_tcp(bind: str, state: ServerState) -> None:
    host, port = _split_addr(bind)
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sock.bind((host, port))
    sock.listen()
    print(f"Server listening TCP on {host}:{port}")
    while True:
        conn, peer = sock.accept()
        conn.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
        threading.Thread(target=handle_tcp_conn, args=(conn, peer, state), daemon=True).start()


def build_parser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(prog="zk-arche-server")
    p.add_argument("--bind", default="0.0.0.0:4000")
    p.add_argument("--transport", choices=["udp", "tcp", "both"], default="udp")
    p.add_argument("--state-dir", default="./server-state")
    p.add_argument("--require-pairing-token")
    return p


def main(argv: list[str] | None = None) -> int:
    args = build_parser().parse_args(argv)
    state = ServerState(
        profile=Profile.standard(),
        registry=FsRegistryStore.with_dir(args.state_dir),
        key_store=FsServerKeyStore.with_dir(args.state_dir),
        require_pairing_token=args.require_pairing_token,
    )
    # Ensure key exists and pubkey is valid before accepting traffic.
    _ = state.server_pub
    if args.transport == "udp":
        run_udp(args.bind, state)
    elif args.transport == "tcp":
        run_tcp(args.bind, state)
    else:
        threading.Thread(target=run_udp, args=(args.bind, state), daemon=True).start()
        run_tcp(args.bind, state)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
