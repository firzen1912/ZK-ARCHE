//! Concrete UDP binding for the `Transport` / `ClientTransport` traits.
//!
//! This is the reference binding. Other environments (CoAP, BLE, serial,
//! in-process test harness) can implement the same traits and the protocol
//! state machines will drive them unchanged.

use std::io::ErrorKind;
use std::net::{SocketAddr, ToSocketAddrs, UdpSocket};
use std::time::Duration;

use crate::error::{ProtoError, Result};
use crate::transport::{ClientTransport, Transport};
use crate::wire::MAX_DATAGRAM;

/// UDP client socket pre-bound to one peer. Marks `is_reliable() = false` so
/// the state machines apply retries + backoff.
pub struct UdpClientTransport {
    sock: UdpSocket,
}

impl UdpClientTransport {
    pub fn connect<A: ToSocketAddrs>(addr: A) -> Result<Self> {
        let sock = UdpSocket::bind("0.0.0.0:0")
            .map_err(|e| ProtoError::transport(format!("udp bind: {e}")))?;
        sock.connect(addr)
            .map_err(|e| ProtoError::transport(format!("udp connect: {e}")))?;
        Ok(Self { sock })
    }
}

impl ClientTransport for UdpClientTransport {
    fn send(&mut self, packet: &[u8]) -> Result<()> {
        if packet.len() > MAX_DATAGRAM {
            return Err(ProtoError::transport(format!(
                "packet {} > MAX_DATAGRAM {MAX_DATAGRAM}",
                packet.len()
            )));
        }
        self.sock
            .send(packet)
            .map_err(|e| ProtoError::transport(format!("udp send: {e}")))?;
        Ok(())
    }

    fn recv(&mut self, timeout: Duration) -> Result<Vec<u8>> {
        self.sock.set_read_timeout(Some(timeout)).ok();
        let mut buf = [0u8; MAX_DATAGRAM];
        match self.sock.recv(&mut buf) {
            Ok(n) => Ok(buf[..n].to_vec()),
            Err(e) if e.kind() == ErrorKind::WouldBlock || e.kind() == ErrorKind::TimedOut => {
                Err(ProtoError::transport("timeout"))
            }
            Err(e) => Err(ProtoError::transport(format!("udp recv: {e}"))),
        }
    }

    fn max_datagram(&self) -> usize {
        MAX_DATAGRAM
    }
    fn is_reliable(&self) -> bool {
        false
    }
}

/// UDP server transport. Multi-peer: each `recv` returns the originating
/// `SocketAddr`; `send(&addr, ...)` targets a specific peer.
pub struct UdpServerTransport {
    sock: UdpSocket,
}

impl UdpServerTransport {
    pub fn bind<A: ToSocketAddrs>(addr: A) -> Result<Self> {
        let sock =
            UdpSocket::bind(addr).map_err(|e| ProtoError::transport(format!("udp bind: {e}")))?;
        Ok(Self { sock })
    }

    pub fn local_addr(&self) -> Result<SocketAddr> {
        self.sock
            .local_addr()
            .map_err(|e| ProtoError::transport(format!("udp local_addr: {e}")))
    }
}

impl Transport for UdpServerTransport {
    type Addr = SocketAddr;

    fn send(&mut self, addr: &SocketAddr, packet: &[u8]) -> Result<()> {
        if packet.len() > MAX_DATAGRAM {
            return Err(ProtoError::transport(format!(
                "packet {} > MAX_DATAGRAM {MAX_DATAGRAM}",
                packet.len()
            )));
        }
        self.sock
            .send_to(packet, addr)
            .map_err(|e| ProtoError::transport(format!("udp send_to: {e}")))?;
        Ok(())
    }

    fn recv(&mut self, timeout: Duration) -> Result<(Vec<u8>, SocketAddr)> {
        self.sock.set_read_timeout(Some(timeout)).ok();
        let mut buf = [0u8; MAX_DATAGRAM];
        match self.sock.recv_from(&mut buf) {
            Ok((n, src)) => Ok((buf[..n].to_vec(), src)),
            Err(e) if e.kind() == ErrorKind::WouldBlock || e.kind() == ErrorKind::TimedOut => {
                Err(ProtoError::transport("timeout"))
            }
            Err(e) => Err(ProtoError::transport(format!("udp recv_from: {e}"))),
        }
    }

    fn max_datagram(&self) -> usize {
        MAX_DATAGRAM
    }
    fn is_reliable(&self) -> bool {
        false
    }
}
