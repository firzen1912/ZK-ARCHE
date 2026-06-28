//! Reference IoT-Auth client.
//!
//! Same command-line surface as the original `client` binary, with an added
//! `--transport {udp,tcp}` flag selecting the transport binding. Both
//! transports drive the same protocol state machines from `proto`.

use std::env;
use std::path::PathBuf;

use curve25519_dalek::ristretto::CompressedRistretto;
use proto::{
    crypto::{derive_device_id, derive_device_scalar, reject_identity},
    error::{ErrorCode, ProtoError},
    proto::{auth::run_auth_client, setup::run_setup_client},
    store::{fs::FsCredentialStore, CredentialStore},
    transport::{tcp::TcpClientTransport, udp::UdpClientTransport},
    Profile, DEFAULT_ALLOWED_ROLES,
};

#[derive(Clone, Copy)]
enum TransportKind {
    Udp,
    Tcp,
}

struct Args {
    server: String,
    transport: TransportKind,
    do_setup: bool,
    pairing_token: Option<String>,
    allow_tofu_setup: bool,
    print_device_identity: bool,
    pin_server_pub: Option<String>,
    state_dir: Option<PathBuf>,
}

fn usage(prog: &str) {
    eprintln!(
"Usage:
  {0} --server 127.0.0.1:4000 [--transport udp|tcp] --setup [--pairing-token TOKEN] [--allow-tofu-setup]
  {0} --server 127.0.0.1:4000 [--transport udp|tcp]
  {0} --pin-server-pub <hex>
  {0} --print-device-identity
  {0} --state-dir ./client-state               (optional; defaults to that folder)",
        prog
    );
}

fn parse_args() -> Result<Args, std::io::Error> {
    let argv: Vec<String> = env::args().collect();
    let prog = argv
        .first()
        .cloned()
        .unwrap_or_else(|| "client".to_string());
    let mut args = Args {
        server: "127.0.0.1:4000".to_string(),
        transport: TransportKind::Udp,
        do_setup: false,
        pairing_token: None,
        allow_tofu_setup: false,
        print_device_identity: false,
        pin_server_pub: None,
        state_dir: None,
    };
    let mut i = 1;
    while i < argv.len() {
        match argv[i].as_str() {
            "--server" => {
                let v = argv.get(i + 1).ok_or_else(|| {
                    std::io::Error::new(std::io::ErrorKind::InvalidInput, "--server missing value")
                })?;
                args.server = v.clone();
                i += 2;
            }
            "--transport" => {
                let v = argv.get(i + 1).ok_or_else(|| {
                    std::io::Error::new(
                        std::io::ErrorKind::InvalidInput,
                        "--transport missing value",
                    )
                })?;
                args.transport = match v.as_str() {
                    "udp" => TransportKind::Udp,
                    "tcp" => TransportKind::Tcp,
                    other => {
                        return Err(std::io::Error::new(
                            std::io::ErrorKind::InvalidInput,
                            format!("--transport must be udp|tcp, got {other}"),
                        ))
                    }
                };
                i += 2;
            }
            "--setup" => {
                args.do_setup = true;
                i += 1;
            }
            "--allow-tofu-setup" => {
                args.allow_tofu_setup = true;
                i += 1;
            }
            "--print-device-identity" => {
                args.print_device_identity = true;
                i += 1;
            }
            "--pairing-token" => {
                let v = argv.get(i + 1).ok_or_else(|| {
                    std::io::Error::new(
                        std::io::ErrorKind::InvalidInput,
                        "--pairing-token missing value",
                    )
                })?;
                args.pairing_token = Some(v.clone());
                i += 2;
            }
            "--pin-server-pub" => {
                let v = argv.get(i + 1).ok_or_else(|| {
                    std::io::Error::new(
                        std::io::ErrorKind::InvalidInput,
                        "--pin-server-pub missing value",
                    )
                })?;
                args.pin_server_pub = Some(v.clone());
                i += 2;
            }
            "--state-dir" => {
                let v = argv.get(i + 1).ok_or_else(|| {
                    std::io::Error::new(
                        std::io::ErrorKind::InvalidInput,
                        "--state-dir missing value",
                    )
                })?;
                args.state_dir = Some(v.into());
                i += 2;
            }
            "-h" | "--help" => {
                usage(&prog);
                std::process::exit(0);
            }
            other => {
                usage(&prog);
                return Err(std::io::Error::new(
                    std::io::ErrorKind::InvalidInput,
                    format!("unknown argument: {other}"),
                ));
            }
        }
    }
    Ok(args)
}

fn main() {
    if let Err(e) = run() {
        eprintln!("client error: {e}");
        std::process::exit(1);
    }
}

fn run() -> Result<(), Box<dyn std::error::Error>> {
    let args = parse_args()?;
    let mut store = match &args.state_dir {
        Some(d) => FsCredentialStore::with_dir(d),
        None => FsCredentialStore::new_default(),
    };

    // Out-of-band pin first (no network required).
    if let Some(hex_str) = args.pin_server_pub {
        let decoded = hex::decode(&hex_str).map_err(|_| {
            ProtoError::wire(ErrorCode::InvalidEncoding, "--pin-server-pub is not hex")
        })?;
        if decoded.len() != 32 {
            return Err(ProtoError::wire(
                ErrorCode::InvalidPoint,
                "--pin-server-pub must be 32 bytes",
            )
            .into());
        }
        let mut bytes = [0u8; 32];
        bytes.copy_from_slice(&decoded);
        let p = CompressedRistretto(bytes).decompress().ok_or_else(|| {
            ProtoError::wire(ErrorCode::InvalidPoint, "not a valid Ristretto point")
        })?;
        reject_identity(&p, "pinned server pub")?;
        store.save_server_pub(&p)?;
        println!("Pinned server pubkey out-of-band.");
        return Ok(());
    }

    if args.print_device_identity {
        let mut root = store.load_or_create_device_root(false)?;
        let device_id = derive_device_id(&root);
        let x = derive_device_scalar(&root);
        use curve25519_dalek::constants::RISTRETTO_BASEPOINT_POINT;
        let device_pub = RISTRETTO_BASEPOINT_POINT * x;
        // zeroize
        {
            use zeroize::Zeroize;
            root.zeroize();
        }
        println!(
            "{} {}",
            hex::encode(device_id),
            hex::encode(device_pub.compress().to_bytes()),
        );
        return Ok(());
    }

    let profile = Profile::standard();
    match args.transport {
        TransportKind::Udp => {
            let mut tr = UdpClientTransport::connect(&args.server)?;
            if args.do_setup {
                run_setup_client(
                    &mut tr,
                    &mut store,
                    &profile,
                    args.pairing_token.as_deref(),
                    args.allow_tofu_setup,
                )?;
                println!("Client[SETUP]: Enrollment OK over UDP.");
            } else {
                let key = run_auth_client(&mut tr, &mut store, &profile, DEFAULT_ALLOWED_ROLES)?;
                println!(
                    "Client[AUTH]: OK over UDP. session_key={}",
                    hex::encode(&key[..8])
                );
            }
        }
        TransportKind::Tcp => {
            let mut tr = TcpClientTransport::connect(&args.server)?;
            if args.do_setup {
                run_setup_client(
                    &mut tr,
                    &mut store,
                    &profile,
                    args.pairing_token.as_deref(),
                    args.allow_tofu_setup,
                )?;
                println!("Client[SETUP]: Enrollment OK over TCP.");
            } else {
                let key = run_auth_client(&mut tr, &mut store, &profile, DEFAULT_ALLOWED_ROLES)?;
                println!(
                    "Client[AUTH]: OK over TCP. session_key={}",
                    hex::encode(&key[..8])
                );
            }
        }
    }
    Ok(())
}
