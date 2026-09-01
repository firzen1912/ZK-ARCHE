#!/usr/bin/env python3
from __future__ import annotations

import csv
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
CORPUS = ROOT / "rust/test-vectors/p2p/common-contract-qualification-v1.txt"
ROADMAP = ROOT / "docs/roadmaps/improvement-roadmap.md"
REGISTRY = ROOT / "spec/registries.md"

EXPECTED = {
    "P2P-001": ("mcu-core", "mcu-core", "none", "local-authorized", "normal-auth", "mutual-auth-local-decision", "required-unexecuted"),
    "P2P-002": ("mcu-core", "linux-edge", "none", "local-authorized", "normal-auth", "mutual-auth-local-decision", "required-unexecuted"),
    "P2P-003": ("linux-edge", "mcu-core", "none", "local-authorized", "normal-auth", "mutual-auth-local-decision", "required-unexecuted"),
    "P2P-004": ("any", "any", "none", "unauthorized", "normal-auth", "fail-closed", "required-unexecuted"),
    "P2P-005": ("peer-a", "peer-c", "none", "transitive-only", "normal-auth", "fail-closed-without-explicit-bounded-delegation", "required-unexecuted"),
    "P2P-006": ("any", "any", "infrastructure-loss", "local-authorized", "normal-auth", "remains-possible-with-sufficient-local-state", "required-unexecuted"),
    "P2P-007": ("mcu-core", "linux-edge", "none", "local-authorized", "extra-optional-capabilities", "select-common-floor-or-fail-closed", "required-unexecuted"),
    "P2P-008": ("any", "any", "none", "local-authorized", "transport-address-change", "transport-address-not-protocol-identity", "required-unexecuted"),
    "P2P-009": ("any", "any", "none", "stale-beyond-permitted-freshness", "normal-auth", "fail-closed-or-restricted-per-profile", "blocked-normative"),
    "P2P-010": ("any", "any", "none", "local-authorized", "incompatible-mandatory-floor", "fail-closed", "required-unexecuted"),
}
FIELDS = ["case_id", "peer_a", "peer_b", "infrastructure", "trust_precondition", "stimulus", "required_outcome", "evidence_state"]


def fail(message: str) -> None:
    print(f"p2p-common-contract-qualification: FAIL: {message}", file=sys.stderr)
    raise SystemExit(1)


def read_text(path: Path) -> str:
    try:
        return path.read_text(encoding="utf-8")
    except OSError as exc:
        fail(f"cannot read {path.relative_to(ROOT)}: {exc}")


def load_corpus() -> dict[str, tuple[str, ...]]:
    text = read_text(CORPUS)
    lines = text.splitlines()
    if not lines or lines[0] != "# ZKP2PQUAL/1":
        fail("missing exact ZKP2PQUAL/1 corpus marker")
    data_lines = [line for line in lines[1:] if line and not line.startswith("#")]
    reader = csv.DictReader(data_lines, delimiter="|")
    if reader.fieldnames != FIELDS:
        fail(f"unexpected corpus fields: {reader.fieldnames}")
    found: dict[str, tuple[str, ...]] = {}
    for row in reader:
        case_id = row["case_id"]
        if case_id in found:
            fail(f"duplicate case_id {case_id}")
        values = tuple(row[field] for field in FIELDS[1:])
        if any(value is None or value == "" for value in values):
            fail(f"empty field in {case_id}")
        found[case_id] = values
    if found != EXPECTED:
        missing = sorted(set(EXPECTED) - set(found))
        extra = sorted(set(found) - set(EXPECTED))
        changed = sorted(case_id for case_id in set(found) & set(EXPECTED) if found[case_id] != EXPECTED[case_id])
        fail(f"corpus drift missing={missing} extra={extra} changed={changed}")
    if any(values[-1] == "passed" for values in found.values()):
        fail("static corpus must not self-declare runtime PASS")
    return found


def require_owned_text() -> None:
    roadmap = read_text(ROADMAP)
    registry = read_text(REGISTRY)
    required_roadmap = [
        "Trust is local and non-transitive by default.",
        "Basic P2P authentication between already-authorized peers must not require:",
        "Asymmetric computation is acceptable; asymmetric authentication assurance is not.",
        "MCU↔MCU and MCU↔edge bidirectional interoperability",
        "no hidden CA/cloud/gateway dependency in the core path",
        "profile/capability downgrade-resistance tests",
    ]
    for needle in required_roadmap:
        if needle not in roadmap:
            fail(f"roadmap ownership text missing: {needle}")
    if "| `0x0003` | `p2p-iot-core` | draft |" not in registry:
        fail("p2p-iot-core must remain an explicitly draft registry allocation")
    if "All currently named protocol/security profiles remain `draft`; none of the numeric profile IDs above is production-selectable" not in registry:
        fail("registry no longer preserves non-selectable draft-profile boundary")


def main() -> None:
    cases = load_corpus()
    require_owned_text()
    blocked = sum(1 for values in cases.values() if values[-1] == "blocked-normative")
    unexecuted = sum(1 for values in cases.values() if values[-1] == "required-unexecuted")
    print(f"p2p-common-contract-qualification: PASS cases={len(cases)} required_unexecuted={unexecuted} blocked_normative={blocked}")


if __name__ == "__main__":
    main()
