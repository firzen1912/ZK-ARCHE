#!/usr/bin/env python3
from __future__ import annotations

import csv
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MATRIX = ROOT / "rust/test-vectors/p2p/common-contract-decision-v2.txt"
P2P_CORPUS = ROOT / "rust/test-vectors/p2p/common-contract-qualification-v1.txt"
REVOCATION_SPEC = ROOT / "spec/revocation-convergence-and-stale-authorization.md"
BIND_SPEC = ROOT / "spec/transport-binding-and-adapter-authority.md"
ROADMAP = ROOT / "docs/roadmaps/improvement-roadmap.md"

FIELDS = [
    "case_id", "peer_a", "peer_b", "infrastructure_available", "auth_valid",
    "authorization_fresh", "authorization_generation_bound", "authorization_generation_current",
    "revocation_fresh", "holder_revoked", "lineage_current", "mandatory_floor_compatible",
    "binding_required", "binding_valid", "expected",
]
PEER_CLASSES = {"mcu-core", "linux-edge"}
OUTCOMES = {"MUTUAL_AUTH_LOCAL_DECISION", "FAIL_CLOSED"}

def fail(message: str) -> None:
    print(f"p2p-common-contract-decision: FAIL: {message}", file=sys.stderr)
    raise SystemExit(1)

def parse_bool(case_id: str, field: str, value: str) -> bool:
    if value == "true":
        return True
    if value == "false":
        return False
    fail(f"{case_id}: {field} must be true/false, got {value!r}")

def classify(row: dict[str, str]) -> str:
    cid = row["case_id"]
    auth_valid = parse_bool(cid, "auth_valid", row["auth_valid"])
    authorization_fresh = parse_bool(cid, "authorization_fresh", row["authorization_fresh"])
    generation_bound = parse_bool(cid, "authorization_generation_bound", row["authorization_generation_bound"])
    generation_current = parse_bool(cid, "authorization_generation_current", row["authorization_generation_current"])
    revocation_fresh = parse_bool(cid, "revocation_fresh", row["revocation_fresh"])
    holder_revoked = parse_bool(cid, "holder_revoked", row["holder_revoked"])
    lineage_current = parse_bool(cid, "lineage_current", row["lineage_current"])
    floor_ok = parse_bool(cid, "mandatory_floor_compatible", row["mandatory_floor_compatible"])
    binding_required = parse_bool(cid, "binding_required", row["binding_required"])
    binding_valid = parse_bool(cid, "binding_valid", row["binding_valid"])
    parse_bool(cid, "infrastructure_available", row["infrastructure_available"])

    if not auth_valid or not floor_ok:
        return "FAIL_CLOSED"
    if not generation_bound or not generation_current:
        return "FAIL_CLOSED"
    if not revocation_fresh or holder_revoked:
        return "FAIL_CLOSED"
    if not lineage_current or not authorization_fresh:
        return "FAIL_CLOSED"
    if binding_required and not binding_valid:
        return "FAIL_CLOSED"
    return "MUTUAL_AUTH_LOCAL_DECISION"

def require_text(path: Path, needles: list[str]) -> None:
    try:
        text = path.read_text(encoding="utf-8")
    except OSError as exc:
        fail(f"cannot read {path.relative_to(ROOT)}: {exc}")
    for needle in needles:
        if needle not in text:
            fail(f"{path.relative_to(ROOT)} missing required text: {needle}")

def main() -> None:
    try:
        lines = MATRIX.read_text(encoding="utf-8").splitlines()
    except OSError as exc:
        fail(f"cannot read matrix: {exc}")
    if not lines or lines[0] != "# ZKP2PDECISION/2":
        fail("missing exact ZKP2PDECISION/2 marker")
    data = [line for line in lines[1:] if line and not line.startswith("#")]
    reader = csv.DictReader(data, delimiter="|")
    if reader.fieldnames != FIELDS:
        fail(f"unexpected fields: {reader.fieldnames}")

    seen: set[str] = set()
    cross_class = 0
    offline_accept = 0
    negative = 0
    generation_negative = 0
    for row in reader:
        cid = row["case_id"]
        if cid in seen:
            fail(f"duplicate case_id {cid}")
        seen.add(cid)
        if row["peer_a"] not in PEER_CLASSES or row["peer_b"] not in PEER_CLASSES:
            fail(f"{cid}: unknown peer class")
        if row["expected"] not in OUTCOMES:
            fail(f"{cid}: invalid expected outcome {row['expected']}")
        actual = classify(row)
        if actual != row["expected"]:
            fail(f"{cid}: expected={row['expected']} computed={actual}")
        if row["peer_a"] != row["peer_b"]:
            cross_class += 1
        if row["infrastructure_available"] == "false" and actual == "MUTUAL_AUTH_LOCAL_DECISION":
            offline_accept += 1
        if actual == "FAIL_CLOSED":
            negative += 1
        if row["authorization_generation_bound"] == "false" or row["authorization_generation_current"] == "false":
            if actual != "FAIL_CLOSED":
                fail(f"{cid}: stale/unbound authorization generation must fail closed")
            generation_negative += 1

    if len(seen) < 16 or cross_class < 12 or offline_accept < 5 or negative < 10 or generation_negative < 4:
        fail(
            f"insufficient coverage cases={len(seen)} cross_class={cross_class} "
            f"offline_accept={offline_accept} negative={negative} generation_negative={generation_negative}"
        )

    require_text(P2P_CORPUS, [
        "P2P-002|mcu-core|linux-edge", "P2P-003|linux-edge|mcu-core",
        "P2P-006|any|any|infrastructure-loss", "P2P-009|any|any|none|stale-beyond-permitted-freshness",
    ])
    require_text(REVOCATION_SPEC, [
        "Offline operation is permitted while the local revocation/authorization view remains within the profile's declared freshness bound",
        "absent explicit profile text, the required behavior is fail closed",
    ])
    require_text(BIND_SPEC, [
        "MUST NOT be treated as protocol identity.",
        "It is not sufficient authentication, authorization, trust mutation, or resumption authorization by itself.",
    ])
    require_text(ROADMAP, [
        "Asymmetric computation is acceptable; asymmetric authentication assurance is not.",
        "no hidden CA/cloud/gateway dependency in the core path", "profile/capability downgrade-resistance tests",
    ])
    print(
        "p2p-common-contract-decision: PASS "
        f"cases={len(seen)} cross_class={cross_class} offline_accept={offline_accept} "
        f"negative={negative} generation_negative={generation_negative}"
    )

if __name__ == "__main__":
    main()
