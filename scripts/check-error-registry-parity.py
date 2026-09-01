#!/usr/bin/env python3
from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RUST = ROOT / "rust/crates/proto/src/error.rs"
C_HDR = ROOT / "c/include/auth/auth.h"
CORPUS = ROOT / "rust/test-vectors/wire/error-code-normalization-v1.txt"

RUST_RE = re.compile(r"^\s*([A-Za-z][A-Za-z0-9_]*)\s*=\s*0x([0-9A-Fa-f]{4}),\s*$")
C_RE = re.compile(r"^\s*(AUTH_[A-Z0-9_]+)\s*=\s*0x([0-9A-Fa-f]{4}),?\s*$")
CASE_RE = re.compile(r"^case=([^|]+)\|([0-9A-Fa-f]{4})\|([0-9A-Fa-f]{4})$")
LOCAL_C = {0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005}
UNSPECIFIED = 0x7FFF


def fail(message: str) -> None:
    print(f"error-registry-parity: FAIL: {message}", file=sys.stderr)
    raise SystemExit(1)


def parse_rust() -> dict[int, str]:
    out: dict[int, str] = {}
    for line in RUST.read_text(encoding="utf-8").splitlines():
        match = RUST_RE.match(line)
        if not match:
            continue
        value = int(match.group(2), 16)
        if value in out:
            fail(f"duplicate Rust wire value 0x{value:04x}")
        out[value] = match.group(1)
    return out


def parse_c() -> dict[int, str]:
    out: dict[int, str] = {}
    for line in C_HDR.read_text(encoding="utf-8").splitlines():
        match = C_RE.match(line)
        if not match:
            continue
        value = int(match.group(2), 16)
        if value in LOCAL_C:
            continue
        if value in out:
            fail(f"duplicate C wire value 0x{value:04x}")
        out[value] = match.group(1)
    return out


def parse_corpus() -> list[tuple[str, int, int]]:
    rows: list[tuple[str, int, int]] = []
    seen_ids: set[str] = set()
    for raw in CORPUS.read_text(encoding="utf-8").splitlines():
        line = raw.strip()
        if not line or line.startswith("#") or line == "version=1":
            continue
        match = CASE_RE.match(line)
        if not match:
            fail(f"malformed corpus line: {line!r}")
        case_id = match.group(1)
        if case_id in seen_ids:
            fail(f"duplicate corpus case id {case_id}")
        seen_ids.add(case_id)
        rows.append((case_id, int(match.group(2), 16), int(match.group(3), 16)))
    return rows


def main() -> None:
    for path in (RUST, C_HDR, CORPUS):
        if not path.is_file():
            fail(f"missing required input {path.relative_to(ROOT)}")

    rust = parse_rust()
    c = parse_c()
    if set(rust) != set(c):
        only_rust = [f"0x{x:04x}" for x in sorted(set(rust) - set(c))]
        only_c = [f"0x{x:04x}" for x in sorted(set(c) - set(rust))]
        fail(f"Rust/C wire allocation drift; rust_only={only_rust}, c_only={only_c}")
    if UNSPECIFIED not in rust:
        fail("0x7fff UNSPECIFIED is missing from the registered wire set")

    rows = parse_corpus()
    identity: dict[int, str] = {}
    fallback_count = 0
    for case_id, received, normalized in rows:
        if received in rust:
            if normalized != received:
                fail(f"{case_id}: registered 0x{received:04x} must normalize to itself")
            if received in identity:
                fail(f"{case_id}: duplicate identity corpus coverage for 0x{received:04x}")
            identity[received] = case_id
        else:
            fallback_count += 1
            if normalized != UNSPECIFIED:
                fail(f"{case_id}: unregistered 0x{received:04x} must normalize to 0x7fff")

    missing = sorted(set(rust) - set(identity))
    if missing:
        fail("registered wire values missing identity corpus cases: " + ", ".join(f"0x{x:04x}" for x in missing))
    if fallback_count == 0:
        fail("corpus has no unregistered-value fallback cases")

    print(
        "error-registry-parity: PASS "
        f"registered={len(rust)} corpus_cases={len(rows)} fallback_cases={fallback_count}"
    )


if __name__ == "__main__":
    main()
