#!/usr/bin/env python3
"""Transparent ZK-ARCHE UART <-> UDP byte bridge.

UART framing is exactly: u16 big-endian packet length, followed by the raw
ZK-ARCHE packet. The bridge does not parse identity, roles, proofs, or session
state and therefore is not an authentication authority.
"""

from __future__ import annotations

import argparse
import socket
import struct
import sys

import serial

MAX_DATAGRAM = 2048


def read_exact(port: serial.Serial, size: int) -> bytes:
    chunks: list[bytes] = []
    remaining = size
    while remaining:
        chunk = port.read(remaining)
        if not chunk:
            raise TimeoutError(f"UART timeout with {remaining} bytes remaining")
        chunks.append(chunk)
        remaining -= len(chunk)
    return b"".join(chunks)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--serial", required=True, help="e.g. /dev/serial0 or COM6")
    parser.add_argument("--baud", type=int, default=921600)
    parser.add_argument("--server", default="127.0.0.1:4040")
    parser.add_argument("--timeout", type=float, default=5.0)
    args = parser.parse_args()

    host, sep, port_text = args.server.rpartition(":")
    if not sep or not host:
        parser.error("--server must be HOST:PORT")
    udp_target = (host, int(port_text))

    with serial.Serial(args.serial, args.baud, timeout=args.timeout) as uart:
        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as udp:
            udp.settimeout(args.timeout)
            print(
                f"transparent bridge: {args.serial}@{args.baud} <-> "
                f"udp://{udp_target[0]}:{udp_target[1]}",
                flush=True,
            )
            while True:
                try:
                    frame_len = struct.unpack(">H", read_exact(uart, 2))[0]
                    if frame_len == 0 or frame_len > MAX_DATAGRAM:
                        raise ValueError(f"invalid MCU frame length {frame_len}")
                    request = read_exact(uart, frame_len)

                    # One UDP retry mirrors the host client's bounded retry
                    # without interpreting or modifying the packet.
                    response: bytes | None = None
                    for _ in range(2):
                        udp.sendto(request, udp_target)
                        try:
                            response, _ = udp.recvfrom(MAX_DATAGRAM)
                            break
                        except socket.timeout:
                            continue
                    if response is None:
                        raise TimeoutError("ZK-ARCHE responder timed out")
                    if len(response) > MAX_DATAGRAM:
                        raise ValueError("oversized UDP response")

                    uart.write(struct.pack(">H", len(response)))
                    uart.write(response)
                    uart.flush()
                except KeyboardInterrupt:
                    return 0
                except (TimeoutError, ValueError, OSError, serial.SerialException) as exc:
                    print(f"bridge error: {exc}", file=sys.stderr, flush=True)
                    uart.reset_input_buffer()


if __name__ == "__main__":
    raise SystemExit(main())
