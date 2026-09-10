#!/usr/bin/env python3
from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RUST = ROOT / "rust/crates/proto/src/caps.rs"
C_HDR = ROOT / "c/include/auth/auth.h"
SPEC = ROOT / "spec/registries.md"

RUST_VERSION_RE = re.compile(r"^pub const (PROTOCOL_VERSION|MIN_SUPPORTED_VERSION): u8 = 0x([0-9A-Fa-f]{2});$")
C_VERSION_RE = re.compile(r"^#define AUTH_(PROTOCOL_VERSION|MIN_SUPPORTED_VERSION)\s+0x([0-9A-Fa-f]{2})$")
RUST_SUITE_RE = re.compile(r"^pub const (SUITE_[A-Z0-9_]+): SuiteId = 0x([0-9A-Fa-f]{4});$")
C_SUITE_RE = re.compile(r"^#define AUTH_(SUITE_[A-Z0-9_]+)\s+\(\(auth_suite_t\)0x([0-9A-Fa-f]{4})\)$")
RUST_CAP_RE = re.compile(r"^\s*pub const ([A-Z0-9_]+): Bits = 1 << ([0-9]+);$")
C_CAP_RE = re.compile(r"^#define AUTH_CAP_([A-Z0-9_]+)\s+\(1ULL << ([0-9]+)\)$")
SPEC_VERSION_RE = re.compile(r"^\| `0x([0-9A-Fa-f]{2})` \| ([^|]+) \| (stable|draft|reserved|deprecated) \|")
SPEC_SUITE_RE = re.compile(r"^\| `0x([0-9A-Fa-f]{4})` \| ([A-Z0-9-]+) \| (stable|draft|reserved|deprecated) \|")
SPEC_CAP_RE = re.compile(r"^\| ([0-9]+) \| `0x[0-9A-Fa-f]{16}` \| `([A-Z0-9_]+)` \| ([^|]+) \|")


def fail(message: str) -> None:
    print(f"core-registry-parity: FAIL: {message}", file=sys.stderr)
    raise SystemExit(1)


def section(text: str, start: str, end: str) -> str:
    try:
        return text.split(start, 1)[1].split(end, 1)[0]
    except IndexError:
        fail(f"missing spec section boundary: {start!r} -> {end!r}")


def parse_unique(lines: list[str], pattern: re.Pattern[str], label: str) -> dict[str, int]:
    out: dict[str, int] = {}
    for line in lines:
        match = pattern.match(line)
        if not match:
            continue
        name = match.group(1)
        value = int(match.group(2), 16 if "0x" in pattern.pattern else 10)
        if name in out:
            fail(f"duplicate {label} name {name}")
        out[name] = value
    return out


def main() -> None:
    for path in (RUST, C_HDR, SPEC):
        if not path.is_file():
            fail(f"missing required input {path.relative_to(ROOT)}")

    rust_lines = RUST.read_text(encoding="utf-8").splitlines()
    c_lines = C_HDR.read_text(encoding="utf-8").splitlines()
    spec_text = SPEC.read_text(encoding="utf-8")

    rust_versions: dict[str, int] = {}
    for line in rust_lines:
        m = RUST_VERSION_RE.match(line)
        if m:
            rust_versions[m.group(1)] = int(m.group(2), 16)
    c_versions: dict[str, int] = {}
    for line in c_lines:
        m = C_VERSION_RE.match(line)
        if m:
            c_versions[m.group(1)] = int(m.group(2), 16)
    if rust_versions != c_versions:
        fail(f"Rust/C protocol-version drift: rust={rust_versions}, c={c_versions}")
    if set(rust_versions) != {"PROTOCOL_VERSION", "MIN_SUPPORTED_VERSION"}:
        fail(f"missing version constants: {rust_versions}")

    version_section = section(spec_text, "## 3. Protocol Version Registry (`u8`)", "## 4. Cryptographic Suite Registry (`u16`)")
    stable_versions: set[int] = set()
    for line in version_section.splitlines():
        m = SPEC_VERSION_RE.match(line)
        if m and m.group(3) == "stable":
            stable_versions.add(int(m.group(1), 16))
    if stable_versions != {rust_versions["PROTOCOL_VERSION"]}:
        fail(f"spec/implementation stable-version drift: spec={sorted(stable_versions)}, implementation={rust_versions['PROTOCOL_VERSION']}")
    if rust_versions["MIN_SUPPORTED_VERSION"] > rust_versions["PROTOCOL_VERSION"]:
        fail("minimum supported version exceeds current protocol version")

    rust_suites: dict[str, int] = {}
    for line in rust_lines:
        m = RUST_SUITE_RE.match(line)
        if m:
            rust_suites[m.group(1)] = int(m.group(2), 16)
    c_suites: dict[str, int] = {}
    for line in c_lines:
        m = C_SUITE_RE.match(line)
        if m:
            c_suites[m.group(1)] = int(m.group(2), 16)
    if rust_suites != c_suites:
        fail(f"Rust/C suite allocation drift: rust={rust_suites}, c={c_suites}")

    suite_section = section(spec_text, "## 4. Cryptographic Suite Registry (`u16`)", "## 5. Protocol/Security Profile Registry (`u16`)")
    stable_suite_values: set[int] = set()
    for line in suite_section.splitlines():
        m = SPEC_SUITE_RE.match(line)
        if m and m.group(3) == "stable":
            stable_suite_values.add(int(m.group(1), 16))
    if stable_suite_values != set(rust_suites.values()):
        fail(f"spec/implementation stable-suite drift: spec={sorted(stable_suite_values)}, implementation={sorted(rust_suites.values())}")

    rust_caps: dict[str, int] = {}
    for line in rust_lines:
        m = RUST_CAP_RE.match(line)
        if m:
            rust_caps[m.group(1)] = int(m.group(2))
    c_caps: dict[str, int] = {}
    for line in c_lines:
        m = C_CAP_RE.match(line)
        if m:
            c_caps[m.group(1)] = int(m.group(2))
    if rust_caps != c_caps:
        fail(f"Rust/C capability allocation drift: rust={rust_caps}, c={c_caps}")

    cap_section = section(spec_text, "### 6.1 Existing protocol-managed bits", "### 6.2 Raw advertisement vs selected security capabilities")
    spec_caps: dict[str, int] = {}
    for line in cap_section.splitlines():
        m = SPEC_CAP_RE.match(line)
        if not m:
            continue
        bit = int(m.group(1))
        name = m.group(2)
        state = m.group(3).strip()
        if not state.startswith("stable"):
            continue
        if name in spec_caps:
            fail(f"duplicate spec capability name {name}")
        spec_caps[name] = bit
    if spec_caps != rust_caps:
        fail(f"spec/implementation stable-capability drift: spec={spec_caps}, implementation={rust_caps}")

    print(
        "core-registry-parity: PASS "
        f"version=0x{rust_versions['PROTOCOL_VERSION']:02x} "
        f"suites={len(rust_suites)} capabilities={len(rust_caps)}"
    )


if __name__ == "__main__":
    main()
