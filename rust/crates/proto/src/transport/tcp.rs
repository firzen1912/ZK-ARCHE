//! Concrete TCP binding.
//!
//! TCP is a byte stream, so we add a trivial `u32` little-endian length prefix
//! ahead of each framed packet. This preserves message boundaries so the
//! transport trait's "one frame in, one frame out" contract is honored.
//!
//! Since TCP already delivers in order and reliably, `is_reliable()` returns
//! `true`, and the state machines skip the retry/backoff loop.

use std::io::{Read, Write};
use std::net::{SocketAddr, TcpListener, TcpStream, ToSocketAddrs};
use std::time::Duration;

use crate::error::{ProtoError, Result};
use crate::transport::{ClientTransport, Transport};
use crate::wire::MAX_DATAGRAM;

fn send_framed(stream: &mut TcpStream, packet: &[u8]) -> Result<()> {
    if packet.len() > MAX_DATAGRAM {
        return Err(ProtoError::transport(format!(
            "packet {} > MAX_DATAGRAM {MAX_DATAGRAM}", packet.len())));
    }
    let len = (packet.len() as u32).to_le_bytes();
    stream.write_all(&len).map_err(|e| ProtoError::transport(format!("tcp write len: {e}")))?;
    stream.write_all(packet).map_err(|e| ProtoError::transport(format!("tcp write: {e}")))?;
    stream.flush().map_err(|e| ProtoError::transport(format!("tcp flush: {e}")))?;
    Ok(())
}

fn recv_framed(stream: &mut TcpStream) -> Result<Vec<u8>> {
    let mut len_buf = [0u8; 4];
    stream.read_exact(&mut len_buf)
        .map_err(|e| ProtoError::transport(format!("tcp read len: {e}")))?;
    let len = u32::from_le_bytes(len_buf) as usize;
    if len > MAX_DATAGRAM {
        return Err(ProtoError::transport(format!("tcp frame {len} > MAX_DATAGRAM {MAX_DATAGRAM}")));
    }
    let mut buf = vec![0u8; len];
    stream.read_exact(&mut buf)
        .map_err(|e| ProtoError::transport(format!("tcp read body: {e}")))?;
    Ok(buf)
}

/// TCP client transport.
pub struct TcpClientTransport {
    stream: TcpStream,
}

impl TcpClientTransport {
    pub fn connect<A: ToSocketAddrs>(addr: A) -> Result<Self> {
        let stream = TcpStream::connect(addr)
            .map_err(|e| ProtoError::transport(format!("tcp connect: {e}")))?;
        stream.set_nodelay(true).ok();
        Ok(Self { stream })
    }
}

impl ClientTransport for TcpClientTransport {
    fn send(&mut self, packet: &[u8]) -> Result<()> {
        send_framed(&mut self.stream, packet)
    }

    fn recv(&mut self, timeout: Duration) -> Result<Vec<u8>> {
        self.stream.set_read_timeout(Some(timeout)).ok();
        recv_framed(&mut self.stream)
    }

    fn max_datagram(&self) -> usize { MAX_DATAGRAM }
    fn is_reliable(&self) -> bool   { true }
}

/// Per-connection TCP server transport. A listener loop creates one of these
/// for each accepted client.
pub struct TcpServerPeer {
    stream: TcpStream,
    peer:   SocketAddr,
}

impl TcpServerPeer {
    pub fn peer_addr(&self) -> SocketAddr { self.peer }
}

impl Transport for TcpServerPeer {
    type Addr = SocketAddr;

    fn send(&mut self, _addr: &SocketAddr, packet: &[u8]) -> Result<()> {
        // TCP is point-to-point; ignore addr parameter.
        send_framed(&mut self.stream, packet)
    }

    fn recv(&mut self, timeout: Duration) -> Result<(Vec<u8>, SocketAddr)> {
        self.stream.set_read_timeout(Some(timeout)).ok();
        let bytes = recv_framed(&mut self.stream)?;
        Ok((bytes, self.peer))
    }

    fn is_reliable(&self) -> bool { true }
}

/// TCP listener helper.
pub struct TcpServerListener {
    listener: TcpListener,
}

impl TcpServerListener {
    pub fn bind<A: ToSocketAddrs>(addr: A) -> Result<Self> {
        let listener = TcpListener::bind(addr)
            .map_err(|e| ProtoError::transport(format!("tcp bind: {e}")))?;
        Ok(Self { listener })
    }

    pub fn local_addr(&self) -> Result<SocketAddr> {
        self.listener.local_addr()
            .map_err(|e| ProtoError::transport(format!("tcp local_addr: {e}")))
    }

    /// Accept the next incoming connection and wrap it as a `TcpServerPeer`.
    pub fn accept(&self) -> Result<TcpServerPeer> {
        let (stream, peer) = self.listener.accept()
            .map_err(|e| ProtoError::transport(format!("tcp accept: {e}")))?;
        stream.set_nodelay(true).ok();
        Ok(TcpServerPeer { stream, peer })
    }
}
