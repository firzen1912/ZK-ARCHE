#!/usr/bin/env python3
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
CORPUS = ROOT / "rust/test-vectors/state/auth-terminal-flight-v1.txt"
RUST = ROOT / "rust/crates/server/src/main.rs"
C = ROOT / "c/bin/server.c"
SPEC = ROOT / "spec/auth-terminal-flight-disposition.md"

REQUIRED_CASES = {
    "success_consumes": ("absent", "fresh_auth1"),
    "failure_consumes": ("absent", "fresh_auth1"),
    "same_session_after_failure": ("absent", "unknown_session"),
    "exact_cached_duplicate": ("absent", "cached_response"),
    "fresh_session_restart": ("new", "auth1_normal_checks"),
}


def fail(msg: str) -> None:
    print(f"auth-terminal-flight-contract: FAIL: {msg}", file=sys.stderr)
    raise SystemExit(1)


def read(path: Path) -> str:
    try:
        return path.read_text(encoding="utf-8")
    except OSError as exc:
        fail(f"cannot read {path.relative_to(ROOT)}: {exc}")


def parse_corpus(text: str):
    cases = {}
    for lineno, raw in enumerate(text.splitlines(), 1):
        line = raw.strip()
        if not line or line.startswith("#"):
            continue
        parts = [p.strip() for p in line.split("|")]
        if len(parts) != 5:
            fail(f"corpus line {lineno}: expected 5 fields, got {len(parts)}")
        case, terminal_result, pending_after, followup, note = parts
        if case in cases:
            fail(f"duplicate corpus case {case}")
        cases[case] = (terminal_result, pending_after, followup, note)
    return cases


def require_order(text: str, first: str, second: str, owner: str) -> None:
    a = text.find(first)
    b = text.find(second)
    if a < 0:
        fail(f"{owner}: missing marker {first!r}")
    if b < 0:
        fail(f"{owner}: missing marker {second!r}")
    if a >= b:
        fail(f"{owner}: expected {first!r} before {second!r}")


def main() -> None:
    corpus = parse_corpus(read(CORPUS))
    if set(corpus) != set(REQUIRED_CASES):
        fail(f"corpus cases drifted: got {sorted(corpus)}, expected {sorted(REQUIRED_CASES)}")
    for case, (expected_pending, expected_followup) in REQUIRED_CASES.items():
        _, pending_after, followup, _ = corpus[case]
        if (pending_after, followup) != (expected_pending, expected_followup):
            fail(
                f"{case}: got pending={pending_after} followup={followup}, "
                f"expected {expected_pending}/{expected_followup}"
            )

    rust = read(RUST)
    c = read(C)
    spec = read(SPEC)

    require_order(
        rust,
        "state.response_cache.get(&(hdr.session_id, hdr.seq))",
        "PKT_AUTH_3 =>",
        "rust",
    )
    if "sessions.remove(session_id)" not in rust:
        fail("rust: terminal-session helper no longer removes pending state")
    require_order(
        rust,
        "PKT_AUTH_3 => match take_terminal_session(&mut state.auth_sessions, &hdr.session_id)",
        "Some(pending) => handle_auth_3(&pending, hdr.session_id, hdr.seq, payload)",
        "rust",
    )

    require_order(
        c,
        "err = auth_server_handle_auth3(&pending,",
        "(void)auth_session_table_release(&S->sessions, slot);",
        "c",
    )
    require_order(
        c,
        "(void)auth_session_table_release(&S->sessions, slot);",
        "if (err) goto error_reply;",
        "c",
    )

    required_spec = [
        "MUST consume that pending AUTH session before terminal cryptographic/context verification is evaluated",
        "MUST NOT be restored",
        "MUST begin a fresh AUTH exchange from `AUTH_1` using a fresh `session_id`",
    ]
    for marker in required_spec:
        if marker not in spec:
            fail(f"spec: missing normative marker {marker!r}")

    print(f"auth-terminal-flight-contract: PASS cases={len(corpus)}")


if __name__ == "__main__":
    main()
