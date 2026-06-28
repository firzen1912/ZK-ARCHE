from __future__ import annotations

from ..crypto import random_bytes_32
from ..errors import ErrorCode, ProtoError
from ..profile import Profile
from ..transport import ClientTransport
from ..wire import PKT_ERROR, SESSION_ID_LEN, Header, build_packet, parse_packet


def rand_session_id() -> bytes:
    return random_bytes_32()[:SESSION_ID_LEN]


def send_expect(transport: ClientTransport, profile: Profile, pkt_type: int, session_id: bytes, seq: int, payload: bytes, expected_resp: int) -> bytes:
    packet = build_packet(pkt_type, session_id, seq, payload)
    if transport.is_reliable():
        transport.send(packet)
        while True:
            hdr, resp = parse_packet(transport.recv(profile.io_timeout))
            if hdr.session_id != session_id or hdr.seq != seq:
                continue
            return _handle_response_header(hdr, resp, expected_resp)
    last_err: ProtoError | None = None
    for attempt in range(profile.max_retries + 1):
        shift = min(attempt, profile.max_backoff_shift)
        timeout = profile.retransmit_timeout * (1 << shift)
        transport.send(packet)
        try:
            hdr, resp = parse_packet(transport.recv(timeout))
            if hdr.session_id != session_id or hdr.seq != seq:
                continue
            return _handle_response_header(hdr, resp, expected_resp)
        except ProtoError as exc:
            last_err = exc
    raise last_err or ProtoError.transport("retries exhausted")


def _handle_response_header(hdr: Header, resp: bytes, expected: int) -> bytes:
    if hdr.pkt_type == PKT_ERROR:
        raise ProtoError.from_wire_payload(resp)
    if hdr.pkt_type != expected:
        raise ProtoError.wire(ErrorCode.UnknownPacketType, f"expected 0x{expected:02x}, got 0x{hdr.pkt_type:02x}")
    return bytes(resp)
