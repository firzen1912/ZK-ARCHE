#!/usr/bin/env python3
"""Fuzz harness for the ZK-ARCHE wire codec (header + TLV + Hello).

Mirrors c/fuzz/fuzz_wire.c and rust/fuzz/fuzz_targets/wire_parse.rs: feed
arbitrary bytes through the parsers and require that only malformed-input
errors (ProtoError / ValueError) are raised. Any other exception is a finding.

Run with Atheris (libFuzzer-backed):

    python -m pip install atheris
    python fuzz/fuzz_wire.py

`test_one_input` is plain Python with no Atheris dependency, so it can also be
driven by unit tests or a custom corpus runner.
"""

from __future__ import annotations

import sys

from zk_arche import wire
from zk_arche.errors import ProtoError


def test_one_input(data: bytes) -> None:
    try:
        _header, payload = wire.parse_packet(data)
        for _tag, _value in wire.iter_tlv(payload):
            pass
    except (ProtoError, ValueError):
        pass

    try:
        wire.Hello.decode(data)
    except (ProtoError, ValueError):
        pass


def main(argv: list[str]) -> None:
    import atheris  # lazy import so the module is usable without Atheris

    atheris.Setup(argv, lambda data: test_one_input(bytes(data)))
    atheris.Fuzz()


if __name__ == "__main__":
    main(sys.argv)
