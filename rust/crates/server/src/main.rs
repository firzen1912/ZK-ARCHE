//! Reference IoT-Auth server.
//!
//! Dispatches incoming packets to the state-machine handlers in
//! `proto::proto`, over either UDP or TCP, selectable via
//! `--transport {udp,tcp,both}`. Maintains:
//!
//! * setup session cache (SETUP_1 -> SETUP_3 continuity)
//! * auth  session cache (AUTH_1  -> AUTH_3  continuity)
//! * cached per-(session_id, seq) responses for retransmit idempotency (UDP)
//! * replay cache for AUTH_1 keyed on H(pid || nonce_c || eph_c)

use std::collections::HashMap;
use std::env;
use std::net::SocketAddr;
use std::path::PathBuf;
use std::sync::{Arc, Mutex};
use std::thread;
use std::time::{Duration, Instant};

use curve25519_dalek::constants::RISTRETTO_BASEPOINT_POINT;
use curve25519_dalek::ristretto::RistrettoPoint;

use proto::{
    error::{ErrorCode, ProtoError},
    proto::{
        auth::{handle_auth_1, handle_auth_3, PendingAuth},
        setup::{handle_setup_1, handle_setup_3, PendingSetup},
    },
    store::{
        fs::{FsRegistryStore, FsServerKeyStore, MemoryReplayCache},
        ServerKeyStore,
    },
    transport::{
        tcp::{TcpServerListener, TcpServerPeer},
        udp::UdpServerTransport,
        Transport,
    },
    wire::{
        build_error, parse_packet, PKT_AUTH_1, PKT_AUTH_3, PKT_SETUP_1, PKT_SETUP_3,
        SESSION_ID_LEN,
    },
    Profile, DEFAULT_ALLOWED_ROLES,
};

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
enum TransportKind {
    Udp,
    Tcp,
    Both,
}

struct ServerArgs {
    bind:                  String,
    transport:             TransportKind,
    state_dir:             Option<PathBuf>,
    require_pairing_token: Option<String>,
}

fn usage(prog: &str) {
    eprintln!(
"Usage:
  {0} --bind 0.0.0.0:4000 [--transport udp|tcp|both] \\
      [--state-dir ./server-state] \\
      [--require-pairing-token TOKEN]

  The --bind address is used for both UDP and TCP when --transport=both.",
        prog
    );
}

fn parse_args() -> Result<ServerArgs, std::io::Error> {
    let argv: Vec<String> = env::args().collect();
    let prog = argv.get(0).cloned().unwrap_or_else(|| "server".to_string());
    let mut out = ServerArgs {
        bind:                  "0.0.0.0:4000".to_string(),
        transport:             TransportKind::Udp,
        state_dir:             None,
        require_pairing_token: None,
    };
    let mut i = 1;
    while i < argv.len() {
        match argv[i].as_str() {
            "--bind" => {
                out.bind = argv.get(i + 1).ok_or_else(|| {
                    std::io::Error::new(std::io::ErrorKind::InvalidInput, "--bind missing value")
                })?.clone();
                i += 2;
            }
            "--transport" => {
                let v = argv.get(i + 1).ok_or_else(|| {
                    std::io::Error::new(std::io::ErrorKind::InvalidInput, "--transport missing value")
                })?;
                out.transport = match v.as_str() {
                    "udp"  => TransportKind::Udp,
                    "tcp"  => TransportKind::Tcp,
                    "both" => TransportKind::Both,
                    other => return Err(std::io::Error::new(
                        std::io::ErrorKind::InvalidInput,
                        format!("--transport must be udp|tcp|both, got {other}"),
                    )),
                };
                i += 2;
            }
            "--state-dir" => {
                out.state_dir = Some(argv.get(i + 1).ok_or_else(|| {
                    std::io::Error::new(std::io::ErrorKind::InvalidInput, "--state-dir missing value")
                })?.into());
                i += 2;
            }
            "--require-pairing-token" => {
                out.require_pairing_token = Some(argv.get(i + 1).ok_or_else(|| {
                    std::io::Error::new(std::io::ErrorKind::InvalidInput, "--require-pairing-token missing value")
                })?.clone());
                i += 2;
            }
            "-h" | "--help" => { usage(&prog); std::process::exit(0); }
            other => {
                usage(&prog);
                return Err(std::io::Error::new(
                    std::io::ErrorKind::InvalidInput,
                    format!("unknown arg: {other}"),
                ));
            }
        }
    }
    Ok(out)
}

fn main() {
    if let Err(e) = run() {
        eprintln!("server error: {e}");
        std::process::exit(1);
    }
}

// ---- Shared state ----

#[derive(Clone)]
struct CachedResponse {
    peer:       SocketAddr,
    packet:     Vec<u8>,
    created_at: Instant,
}

/// All state shared between the UDP and TCP dispatchers. Wrapped in a single
/// `Mutex` for simplicity; a production deployment would split by concern.
struct ServerState {
    profile:               Profile,
    server_pub:            RistrettoPoint,
    registry:              FsRegistryStore,
    key_store:             FsServerKeyStore,
    setup_sessions:        HashMap<[u8; SESSION_ID_LEN], PendingSetup>,
    auth_sessions:         HashMap<[u8; SESSION_ID_LEN], PendingAuth>,
    response_cache:        HashMap<([u8; SESSION_ID_LEN], u32), CachedResponse>,
    replay:                MemoryReplayCache,
    require_pairing_token: Option<String>,
    last_gc:               Instant,
}

/// Dispatch one already-received packet. Returns the response bytes to send
/// back to the peer. This is the single entry point shared by the UDP and
/// TCP drivers — proof that Layer A (state machines) and Layer B (framing)
/// are fully transport-independent.
fn dispatch_packet(
    state: &mut ServerState,
    peer:  SocketAddr,
    bytes: &[u8],
) -> Option<Vec<u8>> {
    let (hdr, payload) = match parse_packet(bytes) {
        Ok(v) => v,
        Err(e) => {
            eprintln!("{peer}: framing error: {e}");
            return None;
        }
    };

    // Idempotent retransmit: if we have a cached response for this
    // (session_id, seq) and peer matches, replay verbatim. (Useful on UDP;
    // harmless on TCP since TCP won't retransmit at the app layer.)
    if let Some(cached) = state.response_cache.get(&(hdr.session_id, hdr.seq)) {
        if cached.peer == peer {
            return Some(cached.packet.clone());
        }
    }

    let result: Result<Vec<u8>, ProtoError> = match hdr.pkt_type {
        PKT_SETUP_1 => {
            let pending = handle_setup_1(
                &mut state.key_store,
                hdr.session_id, hdr.seq, payload,
                state.require_pairing_token.as_deref(),
            );
            pending.map(|p| {
                let resp = p.response_packet.clone();
                state.setup_sessions.insert(hdr.session_id, p);
                resp
            })
        }

        PKT_SETUP_3 => {
            match state.setup_sessions.get(&hdr.session_id) {
                None => Err(ProtoError::wire(
                    ErrorCode::UnknownSession,
                    "no setup session for SETUP_3",
                )),
                Some(pending) => {
                    let r = handle_setup_3(
                        &mut state.registry, pending, &state.server_pub,
                        hdr.session_id, hdr.seq, payload,
                    );
                    if r.is_ok() { state.setup_sessions.remove(&hdr.session_id); }
                    r
                }
            }
        }

        PKT_AUTH_1 => {
            let pending = handle_auth_1(
                &mut state.key_store, &state.registry, &mut state.replay,
                DEFAULT_ALLOWED_ROLES,
                hdr.session_id, hdr.seq, payload,
            );
            pending.map(|p| {
                let resp = p.response_packet.clone();
                state.auth_sessions.insert(hdr.session_id, p);
                resp
            })
        }

        PKT_AUTH_3 => {
            match state.auth_sessions.get(&hdr.session_id) {
                None => Err(ProtoError::wire(
                    ErrorCode::UnknownSession,
                    "no auth session for AUTH_3",
                )),
                Some(pending) => {
                    let r = handle_auth_3(pending, hdr.session_id, hdr.seq, payload);
                    if r.is_ok() { state.auth_sessions.remove(&hdr.session_id); }
                    r
                }
            }
        }

        other => Err(ProtoError::wire(
            ErrorCode::UnknownPacketType,
            format!("server does not handle packet type 0x{other:02x}"),
        )),
    };

    let response = match result {
        Ok(pkt) => pkt,
        Err(e)  => {
            let (code, msg) = wire_code_and_msg(&e);
            eprintln!("{peer}: {code}: {msg}");
            build_error(&hdr.session_id, hdr.seq, code, msg)
        }
    };

    // Cache for idempotent retransmit.
    state.response_cache.insert((hdr.session_id, hdr.seq), CachedResponse {
        peer, packet: response.clone(), created_at: Instant::now(),
    });

    Some(response)
}

fn wire_code_and_msg(e: &ProtoError) -> (ErrorCode, &str) {
    match e {
        ProtoError::Wire { code, msg } => (*code, msg.as_str()),
        ProtoError::Storage(m)         => (ErrorCode::StorageFailure, m.as_str()),
        ProtoError::Transport(m)       => (ErrorCode::Unspecified,    m.as_str()),
        ProtoError::Internal(m)        => (ErrorCode::Unspecified,    m.as_str()),
    }
}

fn gc_if_due(state: &mut ServerState) {
    let now = Instant::now();
    if now.duration_since(state.last_gc) < Duration::from_secs(5) {
        return;
    }
    state.last_gc = now;
    let ttl = state.profile.session_ttl;
    state.response_cache.retain(|_, v| now.duration_since(v.created_at) <= ttl);

    let cap = state.profile.max_active_sessions;
    while state.setup_sessions.len() > cap {
        if let Some(k) = state.setup_sessions.keys().next().copied() {
            state.setup_sessions.remove(&k);
        } else { break; }
    }
    while state.auth_sessions.len() > cap {
        if let Some(k) = state.auth_sessions.keys().next().copied() {
            state.auth_sessions.remove(&k);
        } else { break; }
    }
}

// ---- Drivers ----

fn run_udp(bind: &str, state: Arc<Mutex<ServerState>>) -> Result<(), ProtoError> {
    let mut transport = UdpServerTransport::bind(bind)?;
    println!("  [UDP] listening on {}", transport.local_addr()?);

    loop {
        match transport.recv(Duration::from_secs(1)) {
            Ok((bytes, peer)) => {
                let resp = {
                    let mut s = state.lock().unwrap();
                    gc_if_due(&mut s);
                    dispatch_packet(&mut s, peer, &bytes)
                };
                if let Some(resp) = resp {
                    if let Err(e) = transport.send(&peer, &resp) {
                        eprintln!("udp send to {peer}: {e}");
                    }
                }
            }
            Err(e) => {
                let msg = e.to_string();
                if msg.contains("timeout") {
                    let mut s = state.lock().unwrap();
                    gc_if_due(&mut s);
                    continue;
                }
                eprintln!("udp recv: {e}");
            }
        }
    }
}

fn run_tcp(bind: &str, state: Arc<Mutex<ServerState>>) -> Result<(), ProtoError> {
    let listener = TcpServerListener::bind(bind)?;
    println!("  [TCP] listening on {}", listener.local_addr()?);

    loop {
        match listener.accept() {
            Ok(peer_transport) => {
                let state = state.clone();
                thread::spawn(move || handle_tcp_connection(peer_transport, state));
            }
            Err(e) => {
                eprintln!("tcp accept: {e}");
            }
        }
    }
}

/// One thread per TCP connection: read framed packets sequentially, dispatch
/// through the shared state, send the response back. The connection stays
/// open for the duration of the flow (SETUP or AUTH is 2 round-trips each),
/// then closes when the client disconnects or we hit `io_timeout`.
fn handle_tcp_connection(mut transport: TcpServerPeer, state: Arc<Mutex<ServerState>>) {
    let peer = transport.peer_addr();
    let io_timeout = {
        let s = state.lock().unwrap();
        s.profile.io_timeout
    };

    loop {
        let bytes = match transport.recv(io_timeout) {
            Ok((b, _)) => b,
            Err(e) => {
                let msg = e.to_string();
                let is_clean_close = msg.contains("timeout")
                    || msg.contains("UnexpectedEof")
                    || msg.contains("eof")
                    || msg.contains("failed to fill whole buffer");
                if !is_clean_close {
                    eprintln!("tcp recv from {peer}: {e}");
                }
                return;
            }
        };

        let response = {
            let mut s = state.lock().unwrap();
            gc_if_due(&mut s);
            dispatch_packet(&mut s, peer, &bytes)
        };
        if let Some(resp) = response {
            if let Err(e) = transport.send(&peer, &resp) {
                eprintln!("tcp send to {peer}: {e}");
                return;
            }
        }
    }
}

// ---- Entry point ----

fn run() -> Result<(), Box<dyn std::error::Error>> {
    let args = parse_args()?;
    let profile = Profile::standard();

    // Storage backends.
    let (registry_path, registry_bak, sk_path) = match &args.state_dir {
        Some(d) => (
            d.join("registry.bin"),
            d.join("registry.bak"),
            d.join("server_sk.bin"),
        ),
        None => (
            "./server-state/registry.bin".into(),
            "./server-state/registry.bak".into(),
            "./server-state/server_sk.bin".into(),
        ),
    };
    let registry = FsRegistryStore::with_files(registry_path, registry_bak)?;
    let mut key_store = FsServerKeyStore::with_path(sk_path);
    let server_sk = key_store.load_or_create_server_sk()?;
    let server_pub = RISTRETTO_BASEPOINT_POINT * server_sk;

    let state = Arc::new(Mutex::new(ServerState {
        profile,
        server_pub,
        registry,
        key_store,
        setup_sessions:        HashMap::new(),
        auth_sessions:         HashMap::new(),
        response_cache:        HashMap::new(),
        replay:                MemoryReplayCache::new(profile.max_cached_responses.saturating_mul(2)),
        require_pairing_token: args.require_pairing_token,
        last_gc:               Instant::now(),
    }));

    println!(
        "server v{} starting. server_pub={}",
        env!("CARGO_PKG_VERSION"),
        hex::encode(server_pub.compress().to_bytes()),
    );

    match args.transport {
        TransportKind::Udp => { run_udp(&args.bind, state)?; }
        TransportKind::Tcp => { run_tcp(&args.bind, state)?; }
        TransportKind::Both => {
            let bind_udp = args.bind.clone();
            let bind_tcp = args.bind.clone();
            let s_udp = state.clone();
            let s_tcp = state.clone();
            let t_udp = thread::spawn(move || run_udp(&bind_udp, s_udp));
            let t_tcp = thread::spawn(move || run_tcp(&bind_tcp, s_tcp));
            let _ = t_udp.join();
            let _ = t_tcp.join();
        }
    }
    Ok(())
}
