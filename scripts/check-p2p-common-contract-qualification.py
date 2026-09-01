#!/usr/bin/env python3
from __future__ import annotations

import csv
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
CORPUS = ROOT / "rust/test-vectors/p2p/common-contract-qualification-v1.txt"
DELEGATION_CORPUS = ROOT / "rust/test-vectors/p2p/local-trust-delegation-v1.txt"
ROADMAP = ROOT / "docs/roadmaps/improvement-roadmap.md"
REGISTRY = ROOT / "spec/registries.md"
DELEGATION_SPEC = ROOT / "spec/p2p-local-trust-and-delegation.md"

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

DELEGATION_EXPECTED = {
    "DLG-001": ("direct-local-trust", "none", "direct-auth", "accept-if-other-authz-checks-pass", "required-unexecuted"),
    "DLG-002": ("transitive-only", "none", "normal-auth", "reject", "required-unexecuted"),
    "DLG-003": ("explicit-delegation", "scope-mismatch", "normal-auth", "reject", "blocked-implementation"),
    "DLG-004": ("explicit-delegation", "audience-mismatch", "normal-auth", "reject", "blocked-implementation"),
    "DLG-005": ("explicit-delegation", "expired-or-epoch-stale", "normal-auth", "reject", "blocked-normative"),
    "DLG-006": ("explicit-delegation", "depth-exceeded", "normal-auth", "reject", "blocked-implementation"),
    "DLG-007": ("explicit-delegation", "revoked", "normal-auth", "reject", "blocked-normative"),
    "DLG-008": ("explicit-delegation", "all-bounds-valid", "normal-auth", "eligible-for-authz-evaluation-not-automatic-trust", "blocked-implementation"),
}
DELEGATION_FIELDS = ["case_id", "trust_basis", "bound_condition", "stimulus", "required_outcome", "evidence_state"]


def fail(message: str) -> None:
    print(f"p2p-common-contract-qualification: FAIL: {message}", file=sys.stderr)
    raise SystemExit(1)


def read_text(path: Path) -> str:
    try:
        return path.read_text(encoding="utf-8")
    except OSError as exc:
        fail(f"cannot read {path.relative_to(ROOT)}: {exc}")


def load_table(path: Path, marker: str, fields: list[str], expected: dict[str, tuple[str, ...]]) -> dict[str, tuple[str, ...]]:
    text = read_text(path)
    lines = text.splitlines()
    if not lines or lines[0] != marker:
        fail(f"missing exact {marker[2:]} corpus marker in {path.relative_to(ROOT)}")
    data_lines = [line for line in lines[1:] if line and not line.startswith("#")]
    reader = csv.DictReader(data_lines, delimiter="|")
    if reader.fieldnames != fields:
        fail(f"unexpected corpus fields in {path.relative_to(ROOT)}: {reader.fieldnames}")
    found: dict[str, tuple[str, ...]] = {}
    for row in reader:
        case_id = row["case_id"]
        if case_id in found:
            fail(f"duplicate case_id {case_id}")
        values = tuple(row[field] for field in fields[1:])
        if any(value is None or value == "" for value in values):
            fail(f"empty field in {case_id}")
        found[case_id] = values
    if found != expected:
        missing = sorted(set(expected) - set(found))
        extra = sorted(set(found) - set(expected))
        changed = sorted(case_id for case_id in set(found) & set(expected) if found[case_id] != expected[case_id])
        fail(f"corpus drift in {path.relative_to(ROOT)} missing={missing} extra={extra} changed={changed}")
    if any(values[-1] == "passed" for values in found.values()):
        fail("static corpus must not self-declare runtime PASS")
    return found


def require_owned_text() -> None:
    roadmap = read_text(ROADMAP)
    registry = read_text(REGISTRY)
    delegation = read_text(DELEGATION_SPEC)
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
    required_delegation = [
        "Trust is local and non-transitive by default.",
        "A trust decision MUST NOT become transitive merely because an accepted peer trusts another peer.",
        "Delegation MUST be explicit, scoped, audience-bound, depth-bounded, validity-bounded, issuer-bound, epoch-bound, and revocable.",
        "A valid delegation MAY make a subject eligible for local authorization evaluation; it MUST NOT create automatic persistent trust.",
        "Normal AUTH remains NO-LEARNING.",
        "The current mandatory Common Contract does not yet claim executable delegation support.",
    ]
    for needle in required_delegation:
        if needle not in delegation:
            fail(f"delegation contract text missing: {needle}")


def main() -> None:
    cases = load_table(CORPUS, "# ZKP2PQUAL/1", FIELDS, EXPECTED)
    delegation_cases = load_table(DELEGATION_CORPUS, "# ZKP2PDELEG/1", DELEGATION_FIELDS, DELEGATION_EXPECTED)
    require_owned_text()
    blocked = sum(1 for values in cases.values() if values[-1].startswith("blocked-"))
    unexecuted = sum(1 for values in cases.values() if values[-1] == "required-unexecuted")
    delegation_blocked = sum(1 for values in delegation_cases.values() if values[-1].startswith("blocked-"))
    print(
        "p2p-common-contract-qualification: PASS "
        f"cases={len(cases)} required_unexecuted={unexecuted} blocked={blocked} "
        f"delegation_cases={len(delegation_cases)} delegation_blocked={delegation_blocked}"
    )


if __name__ == "__main__":
    main()
