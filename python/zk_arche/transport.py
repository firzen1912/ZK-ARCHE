from __future__ import annotations

import socket
import struct

from .errors import ProtoError
from .wire import MAX_DATAGRAM


class ClientTransport:
    reliable = False

    def send(self, packet: bytes) -> None: raise NotImplementedError
    def recv(self, timeout: float) -> bytes: raise NotImplementedError
    def is_reliable(self) -> bool: return self.reliable


class UdpClientTransport(ClientTransport):
    reliable = False
    def __init__(self, addr: str) -> None:
        host, port = _split_addr(addr)
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.connect((host, port))
    def send(self, packet: bytes) -> None:
        if len(packet) > MAX_DATAGRAM: raise ProtoError.transport(f"packet {len(packet)} > MAX_DATAGRAM {MAX_DATAGRAM}")
        self.sock.send(packet)
    def recv(self, timeout: float) -> bytes:
        self.sock.settimeout(timeout)
        try: return self.sock.recv(MAX_DATAGRAM)
        except socket.timeout as exc: raise ProtoError.transport("timeout") from exc


class TcpClientTransport(ClientTransport):
    reliable = True
    def __init__(self, addr: str) -> None:
        host, port = _split_addr(addr)
        self.sock = socket.create_connection((host, port))
        self.sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
    def send(self, packet: bytes) -> None:
        _send_framed(self.sock, packet)
    def recv(self, timeout: float) -> bytes:
        self.sock.settimeout(timeout)
        return _recv_framed(self.sock)


def _send_framed(sock: socket.socket, packet: bytes) -> None:
    if len(packet) > MAX_DATAGRAM:
        raise ProtoError.transport(f"packet {len(packet)} > MAX_DATAGRAM {MAX_DATAGRAM}")
    sock.sendall(struct.pack("<I", len(packet)) + packet)


def _recv_exact(sock: socket.socket, n: int) -> bytes:
    out = bytearray()
    while len(out) < n:
        try:
            chunk = sock.recv(n - len(out))
        except socket.timeout as exc:
            raise ProtoError.transport("timeout") from exc
        if not chunk:
            raise ProtoError.transport("tcp peer closed")
        out += chunk
    return bytes(out)


def _recv_framed(sock: socket.socket) -> bytes:
    n = struct.unpack("<I", _recv_exact(sock, 4))[0]
    if n > MAX_DATAGRAM: raise ProtoError.transport(f"tcp frame {n} > MAX_DATAGRAM {MAX_DATAGRAM}")
    return _recv_exact(sock, n)


def _split_addr(addr: str) -> tuple[str, int]:
    if addr.count(":") == 1:
        host, port = addr.rsplit(":", 1)
        return host, int(port)
    raise ValueError("address must be host:port")
