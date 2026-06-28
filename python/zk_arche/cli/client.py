from __future__ import annotations

import argparse
import binascii
import sys

from .. import DEFAULT_ALLOWED_ROLES
from ..crypto import basepoint_mul, derive_device_id, derive_device_scalar
from ..profile import Profile
from ..protocol.auth import run_auth_client
from ..protocol.setup import run_setup_client
from ..store import FsCredentialStore
from ..transport import TcpClientTransport, UdpClientTransport


def build_parser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(prog="zk-arche-client")
    p.add_argument("--server", default="127.0.0.1:4000")
    p.add_argument("--transport", choices=["udp", "tcp"], default="udp")
    p.add_argument("--state-dir", default="./client-state")
    p.add_argument("--setup", action="store_true")
    p.add_argument("--pairing-token")
    p.add_argument("--allow-tofu-setup", action="store_true")
    p.add_argument("--derive-from-root", help="32-byte hex root; prints device_id and device_pub")
    return p


def main(argv: list[str] | None = None) -> int:
    args = build_parser().parse_args(argv)
    if args.derive_from_root:
        root = bytes.fromhex(args.derive_from_root)
        if len(root) != 32:
            raise SystemExit("--derive-from-root must be 32 bytes hex")
        device_id = derive_device_id(root)
        device_pub = basepoint_mul(derive_device_scalar(root))
        print(device_id.hex(), device_pub.to_bytes().hex())
        return 0

    store = FsCredentialStore(args.state_dir)
    profile = Profile.standard()
    transport = UdpClientTransport(args.server) if args.transport == "udp" else TcpClientTransport(args.server)
    if args.setup:
        run_setup_client(transport, store, profile, args.pairing_token, args.allow_tofu_setup)
        print(f"Client[SETUP]: Enrollment OK over {args.transport.upper()}.")
    else:
        key = run_auth_client(transport, store, profile, DEFAULT_ALLOWED_ROLES)
        print(f"Client[AUTH]: OK over {args.transport.upper()}. session_key={key[:8].hex()}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
