//! Helpers shared between the setup and auth state machines.

use crate::error::{ErrorCode, ProtoError, Result};
use crate::profile::Profile;
use crate::transport::ClientTransport;
use crate::wire::{build_packet, parse_packet, Header, PKT_ERROR, SESSION_ID_LEN};

/// Run one reliable request/response round-trip over a `ClientTransport`.
///
/// * For reliable transports (TCP) a single `send`/`recv` pair is used. Stray
///   packets (mismatched session_id/seq) are discarded.
/// * For unreliable transports (UDP), the request is retransmitted with
///   exponential backoff up to `profile.max_retries`.
/// * `PKT_ERROR` responses are decoded back into structured `ProtoError`s.
pub(super) fn send_expect<T: ClientTransport>(
    transport:     &mut T,
    profile:       &Profile,
    pkt_type:      u8,
    session_id:    &[u8; SESSION_ID_LEN],
    seq:           u32,
    payload:       &[u8],
    expected_resp: u8,
) -> Result<Vec<u8>> {
    let packet = build_packet(pkt_type, session_id, seq, payload);

    if transport.is_reliable() {
        transport.send(&packet)?;
        loop {
            let bytes = transport.recv(profile.io_timeout)?;
            let (hdr, resp) = parse_packet(&bytes)?;
            if hdr.session_id != *session_id || hdr.seq != seq {
                continue;
            }
            return handle_response_header(hdr, resp, expected_resp);
        }
    }

    let mut last_err: Option<ProtoError> = None;
    for attempt in 0..=profile.max_retries {
        let shift = (attempt as u32).min(profile.max_backoff_shift);
        let timeout = profile.retransmit_timeout
            .checked_mul(1u32 << shift)
            .unwrap_or(profile.retransmit_timeout);
        transport.send(&packet)?;
        match transport.recv(timeout) {
            Ok(bytes) => {
                let (hdr, resp) = match parse_packet(&bytes) {
                    Ok(v) => v,
                    Err(e) => { last_err = Some(e); continue; }
                };
                if hdr.session_id != *session_id || hdr.seq != seq { continue; }
                return handle_response_header(hdr, resp, expected_resp);
            }
            Err(e) => { last_err = Some(e); continue; }
        }
    }
    Err(last_err.unwrap_or_else(|| ProtoError::transport("retries exhausted")))
}

fn handle_response_header(hdr: Header, resp: &[u8], expected: u8) -> Result<Vec<u8>> {
    if hdr.pkt_type == PKT_ERROR {
        return Err(ProtoError::from_wire_payload(resp));
    }
    if hdr.pkt_type != expected {
        return Err(ProtoError::wire(
            ErrorCode::UnknownPacketType,
            format!("expected 0x{expected:02x}, got 0x{:02x}", hdr.pkt_type),
        ));
    }
    Ok(resp.to_vec())
}

/// Helper: generate a random 16-byte session id.
pub(super) fn rand_session_id() -> [u8; SESSION_ID_LEN] {
    let b = crate::crypto::random_bytes_32();
    let mut out = [0u8; SESSION_ID_LEN];
    out.copy_from_slice(&b[..SESSION_ID_LEN]);
    out
}
