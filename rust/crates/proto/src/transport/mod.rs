//! Transport abstraction.
//!
//! The protocol state machines in `proto::*` speak in framed datagrams only.
//! A concrete transport wires those datagrams to the outside world: UDP, TCP
//! length-prefixed streams, CoAP, BLE L2CAP, a serial gateway, even
//! in-process mpsc channels for tests.
//!
//! ## Rules for implementors
//!
//! 1. `send` transmits one **complete** framed packet to the peer. The packet
//!    already includes the 24-byte header from `wire::Header`.
//! 2. `recv` returns one **complete** framed packet, blocking up to `timeout`.
//!    Streaming transports (TCP) MUST do their own length-prefixing so the
//!    byte boundary of a frame is preserved.
//! 3. A transport MUST NOT interpret or modify the payload — it is an opaque
//!    byte buffer.
//! 4. Errors caused by the underlying medium (connection reset, timeout, ...)
//!    return `ProtoError::Transport`. Use `ProtoError::Wire` only for
//!    framing-level problems the peer can act on.
//!
//! Reliability (ack / retransmit / dedup) is *not* part of this trait. The
//! request/response state machines handle it by talking to this interface
//! symmetrically for both UDP and TCP. For UDP, the `udp` module provides a
//! `ReliableUdp` wrapper that layers retries, backoff, and dedup on top.

use std::time::Duration;

use crate::error::Result;

/// A bidirectional, message-oriented transport between two protocol peers.
///
/// `Addr` identifies the other endpoint. For a client, this is typically a
/// socket address (UDP/TCP) or a URI (CoAP). For a server, it identifies the
/// particular client that sent the last received packet.
pub trait Transport {
    type Addr: Clone + Eq + std::hash::Hash + std::fmt::Debug + Send;

    /// Send one framed packet to `addr`.
    fn send(&mut self, addr: &Self::Addr, packet: &[u8]) -> Result<()>;

    /// Receive one framed packet. Returns `(packet_bytes, from_addr)`.
    /// Blocks up to `timeout`. Returns `ErrorCode::Unspecified` wrapped in
    /// `ProtoError::Transport` with message "timeout" if the deadline elapses
    /// with no packet.
    fn recv(&mut self, timeout: Duration) -> Result<(Vec<u8>, Self::Addr)>;

    /// Hint for the maximum single-datagram payload this transport supports.
    /// UDP: path MTU. TCP: very large (use the protocol's MAX_DATAGRAM). CoAP:
    /// typically 1152. Embedded: whatever the link allows.
    fn max_datagram(&self) -> usize {
        crate::wire::MAX_DATAGRAM
    }

    /// True if the transport delivers packets reliably and in order without
    /// additional logic (TCP, QUIC streams). False for UDP / CoAP unreliable.
    /// The state machines use this to decide whether to rely on the
    /// `ReliableUdp` retry layer or skip it.
    fn is_reliable(&self) -> bool {
        false
    }
}

/// Convenience: a client-side transport that is pre-bound to one peer, so
/// callers can `.send(&packet)` without repeating the address.
pub trait ClientTransport {
    fn send(&mut self, packet: &[u8]) -> Result<()>;
    fn recv(&mut self, timeout: Duration) -> Result<Vec<u8>>;
    fn max_datagram(&self) -> usize {
        crate::wire::MAX_DATAGRAM
    }
    fn is_reliable(&self) -> bool {
        false
    }
}

#[cfg(feature = "udp-transport")]
pub mod udp;

pub mod tcp;
