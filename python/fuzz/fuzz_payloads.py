#!/usr/bin/env python3
"""Fuzz harness for the ZK-ARCHE payload codecs (SETUP_1/2/3, AUTH_1/2/3, ACK).

Mirrors c/fuzz/fuzz_payloads.c and rust/fuzz/fuzz_targets/auth_payloads.rs: feed
arbitrary bytes through every payload decoder and require that only
malformed-input errors (ProtoError / ValueError) are raised. Any other
exception is a finding.

Run with Atheris (libFuzzer-backed):

    python -m pip install atheris
    python fuzz/fuzz_payloads.py

`test_one_input` is plain Python with no Atheris dependency, so it can also be
driven by unit tests or a custom corpus runner.
"""

from __future__ import annotations

import sys

from zk_arche.errors import ProtoError
from zk_arche.payloads import (
    Auth1,
    Auth2,
    Auth3,
    Setup1,
    Setup2,
    Setup3,
    decode_ack,
)


def test_one_input(data: bytes) -> None:
    for decoder in (
        Setup1.decode,
        Setup2.decode,
        Setup3.decode,
        Auth1.decode,
        Auth2.decode,
        Auth3.decode,
        decode_ack,
    ):
        try:
            decoder(data)
        except (ProtoError, ValueError):
            pass


def main(argv: list[str]) -> None:
    import atheris  # lazy import so the module is usable without Atheris

    atheris.Setup(argv, lambda data: test_one_input(bytes(data)))
    atheris.Fuzz()


if __name__ == "__main__":
    main(sys.argv)
