#!/usr/bin/env python3
"""Guard lifecycle invalidation requirements for wire-neutral LINEAGE_REPLACE planners."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

REQUIRED = {
    "session_keys": ("invalidate_session_keys", "invalidate_session_keys"),
    "resumption": ("invalidate_resumption", "invalidate_resumption"),
    "authorization_cache": ("invalidate_authorization_cache", "invalidate_authorization_cache"),
    "attribution_cache": ("invalidate_attribution_cache", "invalidate_attribution_cache"),
    "channel_binding": ("invalidate_channel_binding", "invalidate_channel_binding"),
    "replay_state": ("invalidate_replay_state", "invalidate_replay_state"),
}


def require(source: str, needle: str, label: str) -> None:
    if needle not in source:
        raise AssertionError(f"missing {label}: {needle}")


def main() -> int:
    rust = (ROOT / "rust/crates/proto/src/lineage_replace.rs").read_text(encoding="utf-8")
    c = (ROOT / "c/src/proto/lineage_replace.c").read_text(encoding="utf-8")

    for label, (rust_field, c_field) in REQUIRED.items():
        require(rust, f"{rust_field}: true", f"Rust {label} invalidation")
        require(c, c_field, f"C {label} invalidation")

    require(rust, "LineageReplaceState::ContinuityBroken", "Rust interrupted replacement fail-closed state")
    require(c, "LINEAGE_REPLACE_STATE_CONTINUITY_BROKEN", "C interrupted replacement fail-closed state")
    require(rust, "LineageReplaceEvent::Interrupt", "Rust interruption transition")
    require(c, "LINEAGE_REPLACE_EVENT_INTERRUPT", "C interruption transition")

    print(
        "lineage-replace-lifecycle-invariants: PASS "
        f"dependent_state_classes={len(REQUIRED)} interrupt_fail_closed=2"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
