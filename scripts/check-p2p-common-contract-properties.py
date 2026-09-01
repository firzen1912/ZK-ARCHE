#!/usr/bin/env python3
from __future__ import annotations

import csv
import itertools
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MATRIX = ROOT / "rust/test-vectors/p2p/common-contract-decision-v1.txt"

FIELDS = [
    "case_id", "peer_a", "peer_b", "infrastructure_available", "auth_valid",
    "authorization_fresh", "revocation_fresh", "holder_revoked", "lineage_current",
    "mandatory_floor_compatible", "binding_required", "binding_valid", "expected",
]
PEER_CLASSES = ("mcu-core", "linux-edge")
SUCCESS = "MUTUAL_AUTH_LOCAL_DECISION"
FAIL = "FAIL_CLOSED"


def fail(message: str) -> None:
    print(f"p2p-common-contract-properties: FAIL: {message}", file=sys.stderr)
    raise SystemExit(1)


def classify(
    auth_valid: bool,
    authorization_fresh: bool,
    revocation_fresh: bool,
    holder_revoked: bool,
    lineage_current: bool,
    mandatory_floor_compatible: bool,
    binding_required: bool,
    binding_valid: bool,
) -> str:
    if not auth_valid:
        return FAIL
    if not mandatory_floor_compatible:
        return FAIL
    if not revocation_fresh or holder_revoked:
        return FAIL
    if not lineage_current or not authorization_fresh:
        return FAIL
    if binding_required and not binding_valid:
        return FAIL
    return SUCCESS


def classify_context(
    peer_a: str,
    peer_b: str,
    infrastructure_available: bool,
    security_state: tuple[bool, ...],
) -> str:
    if peer_a not in PEER_CLASSES or peer_b not in PEER_CLASSES:
        fail(f"unknown peer context {peer_a}->{peer_b}")
    if not isinstance(infrastructure_available, bool):
        fail("infrastructure availability must be Boolean")
    # Peer class and optional infrastructure are intentionally validated as
    # context but excluded from protocol authority. The mandatory decision is
    # determined solely by the local security evidence tuple.
    return classify(*security_state)


def parse_bool(case_id: str, field: str, value: str) -> bool:
    if value == "true":
        return True
    if value == "false":
        return False
    fail(f"{case_id}: {field} must be true/false, got {value!r}")


def load_canonical_cases() -> list[dict[str, str]]:
    try:
        lines = MATRIX.read_text(encoding="utf-8").splitlines()
    except OSError as exc:
        fail(f"cannot read canonical matrix: {exc}")
    if not lines or lines[0] != "# ZKP2PDECISION/1":
        fail("missing exact ZKP2PDECISION/1 marker")
    data = [line for line in lines[1:] if line and not line.startswith("#")]
    reader = csv.DictReader(data, delimiter="|")
    if reader.fieldnames != FIELDS:
        fail(f"unexpected canonical fields: {reader.fieldnames}")
    rows = list(reader)
    if not rows:
        fail("canonical matrix is empty")
    return rows


def validate_canonical(rows: list[dict[str, str]]) -> None:
    seen: set[str] = set()
    for row in rows:
        cid = row["case_id"]
        if cid in seen:
            fail(f"duplicate canonical case_id {cid}")
        seen.add(cid)
        if row["peer_a"] not in PEER_CLASSES or row["peer_b"] not in PEER_CLASSES:
            fail(f"{cid}: unknown peer class")
        values = tuple(
            parse_bool(cid, field, row[field])
            for field in (
                "auth_valid", "authorization_fresh", "revocation_fresh", "holder_revoked",
                "lineage_current", "mandatory_floor_compatible", "binding_required", "binding_valid",
            )
        )
        infrastructure_available = parse_bool(
            cid, "infrastructure_available", row["infrastructure_available"]
        )
        actual = classify_context(
            row["peer_a"], row["peer_b"], infrastructure_available, values
        )
        if row["expected"] != actual:
            fail(f"{cid}: expected={row['expected']} computed={actual}")


def main() -> None:
    rows = load_canonical_cases()
    validate_canonical(rows)

    state_count = 0
    success_count = 0
    failure_count = 0

    # Exhaust the complete Boolean decision surface for both infrastructure
    # states and every constrained/edge peer pairing. Peer class and optional
    # infrastructure are valid context, but neither may become protocol
    # authority for an otherwise identical local security-evidence tuple.
    peer_pairs = tuple(itertools.product(PEER_CLASSES, repeat=2))
    for peer_a, peer_b in peer_pairs:
        for security_state in itertools.product((False, True), repeat=8):
            offline = classify_context(peer_a, peer_b, False, security_state)
            online = classify_context(peer_a, peer_b, True, security_state)
            if offline != online:
                fail(
                    "infrastructure availability changed authority decision "
                    f"for {peer_a}->{peer_b} state={security_state}"
                )

            # Peer class/order must not create a weaker authentication model.
            baseline = offline
            for other_a, other_b in peer_pairs:
                other = classify_context(other_a, other_b, False, security_state)
                if baseline != other:
                    fail(
                        "peer class changed mandatory-floor decision "
                        f"{peer_a}->{peer_b} versus {other_a}->{other_b} state={security_state}"
                    )

            auth, authz, revfresh, revoked, lineage, floor_ok, binding_required, binding_valid = security_state
            outcome = offline
            state_count += 2  # offline + online
            if outcome == SUCCESS:
                success_count += 2
                if not (auth and authz and revfresh and not revoked and lineage and floor_ok):
                    fail(f"success escaped a mandatory fail-closed guard: {security_state}")
                if binding_required and not binding_valid:
                    fail(f"success escaped required-binding validation: {security_state}")
            else:
                failure_count += 2

            # Optional binding metadata cannot become an implicit authority.
            if not binding_required:
                with_invalid_binding = classify(
                    auth, authz, revfresh, revoked, lineage, floor_ok, False, False
                )
                with_valid_binding = classify(
                    auth, authz, revfresh, revoked, lineage, floor_ok, False, True
                )
                if with_invalid_binding != with_valid_binding:
                    fail(f"optional binding validity changed decision: {security_state}")

    if state_count != 2048:
        fail(f"unexpected exhaustive state count {state_count}, expected 2048")
    if success_count != 24 or failure_count != 2024:
        fail(
            f"unexpected decision distribution success={success_count} failure={failure_count}; "
            "classifier semantics drifted"
        )

    print(
        "p2p-common-contract-properties: PASS "
        f"canonical={len(rows)} exhaustive_states={state_count} "
        f"success={success_count} fail_closed={failure_count}"
    )


if __name__ == "__main__":
    main()
