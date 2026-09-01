#!/usr/bin/env python3
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
CORPUS = ROOT / "rust/test-vectors/state/auth-trust-boundary-v1.txt"
RUST = ROOT / "rust/crates/server/src/main.rs"
C = ROOT / "c/bin/server.c"
SPEC = ROOT / "spec/auth-trust-mutation-boundary.md"

REQUIRED_CASES = {
    "auth1_accept": ("auth", "unchanged", "explicit_trust_operation"),
    "auth1_reject": ("auth", "unchanged", "none"),
    "auth3_accept": ("auth", "unchanged", "explicit_trust_operation"),
    "auth3_reject": ("auth", "unchanged", "fresh_auth1_or_none"),
    "setup3_accept": ("setup", "explicit_mutation", "registry_persist"),
}


def fail(msg: str) -> None:
    print(f"auth-trust-boundary: FAIL: {msg}", file=sys.stderr)
    raise SystemExit(1)


def read(path: Path) -> str:
    try:
        return path.read_text(encoding="utf-8")
    except OSError as exc:
        fail(f"cannot read {path.relative_to(ROOT)}: {exc}")


def between(text: str, start: str, end: str, owner: str) -> str:
    a = text.find(start)
    if a < 0:
        fail(f"{owner}: missing start marker {start!r}")
    b = text.find(end, a + len(start))
    if b < 0:
        fail(f"{owner}: missing end marker {end!r}")
    return text[a:b]


def parse_corpus(text: str):
    cases = {}
    for lineno, raw in enumerate(text.splitlines(), 1):
        line = raw.strip()
        if not line or line.startswith("#"):
            continue
        parts = [p.strip() for p in line.split("|")]
        if len(parts) != 5:
            fail(f"corpus line {lineno}: expected 5 fields, got {len(parts)}")
        case, phase, trust_effect, followup, note = parts
        if case in cases:
            fail(f"duplicate corpus case {case}")
        cases[case] = (phase, trust_effect, followup, note)
    return cases


def main() -> None:
    corpus = parse_corpus(read(CORPUS))
    if set(corpus) != set(REQUIRED_CASES):
        fail(f"corpus cases drifted: got {sorted(corpus)}, expected {sorted(REQUIRED_CASES)}")
    for case, expected in REQUIRED_CASES.items():
        observed = corpus[case][:3]
        if observed != expected:
            fail(f"{case}: got {observed}, expected {expected}")

    rust = read(RUST)
    c = read(C)
    spec = read(SPEC)

    rust_auth1 = between(rust, "PKT_AUTH_1 => {", "PKT_AUTH_3 =>", "rust AUTH_1")
    rust_auth3 = between(rust, "PKT_AUTH_3 =>", "other =>", "rust AUTH_3")
    rust_setup3 = between(rust, "PKT_SETUP_3 =>", "PKT_AUTH_1 =>", "rust SETUP_3")

    if "&state.registry" not in rust_auth1:
        fail("rust AUTH_1: expected immutable registry lookup owner")
    if "&mut state.registry" in rust_auth1 or "&mut state.registry" in rust_auth3:
        fail("rust AUTH: persistent registry became mutable in normal AUTH")
    if "handle_auth_3(" not in rust_auth3:
        fail("rust AUTH_3: terminal handler marker missing")
    if "handle_setup_3(" not in rust_setup3 or "&mut state.registry" not in rust_setup3:
        fail("rust SETUP_3: explicit registry mutation control path drifted")

    c_auth1 = between(c, "case AUTH_PKT_AUTH_1: {", "case AUTH_PKT_AUTH_3: {", "c AUTH_1")
    c_auth3 = between(c, "case AUTH_PKT_AUTH_3: {", "default:", "c AUTH_3")
    c_setup3 = between(c, "case AUTH_PKT_SETUP_3: {", "case AUTH_PKT_AUTH_1: {", "c SETUP_3")
    c_scan = between(c, "static int try_handle_auth1(", "/* ---- Dispatch one incoming packet ---- */", "c AUTH_1 scan")

    for owner, block in (("c AUTH_1", c_auth1), ("c AUTH_3", c_auth3), ("c AUTH_1 scan", c_scan)):
        for marker in ("auth_registry_put(", "auth_registry_save("):
            if marker in block:
                fail(f"{owner}: trust mutation marker {marker!r} appeared in normal AUTH")
    if "try_handle_auth1(" not in c_auth1 or "auth_server_handle_auth1_guarded(" not in c_scan:
        fail("c AUTH_1: guarded read-only candidate-scan path drifted")
    if "auth_server_handle_auth3(" not in c_auth3:
        fail("c AUTH_3: terminal handler marker missing")
    if "auth_registry_put(" not in c_setup3 or "auth_registry_save(" not in c_setup3:
        fail("c SETUP_3: explicit registry mutation control path drifted")

    required_spec = [
        "MUST NOT create, replace, delete, or otherwise mutate persistent trust/enrollment registry state",
        "MUST NOT be interpreted as an implicit enrollment, trust grant, or application authorization grant",
        "MUST occur only through an explicit trust-management operation",
    ]
    for marker in required_spec:
        if marker not in spec:
            fail(f"spec: missing normative marker {marker!r}")

    print(f"auth-trust-boundary: PASS cases={len(corpus)}")


if __name__ == "__main__":
    main()
