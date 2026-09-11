#!/usr/bin/env python3
"""Deterministic mutation qualification for the P2P Common Contract lifecycle corpus."""
from __future__ import annotations

import csv
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
CORPUS = ROOT / "rust/test-vectors/p2p/common-contract-lifecycle-v4.txt"

BOOL_FIELDS = {
    "infrastructure_available",
    "auth_complete",
    "preexisting_trust",
    "authorization_present",
    "authorization_fresh",
    "authorization_generation_bound",
    "authorization_generation_current",
    "revocation_current",
    "revoked",
    "lineage_current",
    "replay_continuity_current",
    "restart_continuity_current",
    "usage_counter_continuity_current",
    "mandatory_floor_compatible",
    "binding_required",
    "binding_valid",
    "trust_mutation_requested",
}

FAIL_CLOSED_MUTATIONS = {
    "auth_complete": False,
    "preexisting_trust": False,
    "authorization_present": False,
    "authorization_fresh": False,
    "authorization_generation_bound": False,
    "authorization_generation_current": False,
    "revocation_current": False,
    "revoked": True,
    "lineage_current": False,
    "replay_continuity_current": False,
    "restart_continuity_current": False,
    "usage_counter_continuity_current": False,
    "mandatory_floor_compatible": False,
    "trust_mutation_requested": True,
}

CROSS_CLASS_DIRECTIONS = {
    ("mcu-core", "linux-edge"),
    ("linux-edge", "mcu-core"),
}


def parse_bool(value: str) -> bool:
    if value == "true":
        return True
    if value == "false":
        return False
    raise ValueError(f"invalid boolean {value!r}")


def classify(row: dict[str, object]) -> str:
    if not row["auth_complete"]:
        return "FAIL_CLOSED"
    if not row["preexisting_trust"]:
        return "FAIL_CLOSED"
    if not row["authorization_present"] or not row["authorization_fresh"]:
        return "FAIL_CLOSED"
    if not row["authorization_generation_bound"]:
        return "FAIL_CLOSED"
    if not row["authorization_generation_current"]:
        return "FAIL_CLOSED"
    if not row["revocation_current"] or row["revoked"]:
        return "FAIL_CLOSED"
    if not row["lineage_current"]:
        return "FAIL_CLOSED"
    if not row["replay_continuity_current"] or not row["restart_continuity_current"]:
        return "FAIL_CLOSED"
    if not row["usage_counter_continuity_current"]:
        return "FAIL_CLOSED"
    if not row["mandatory_floor_compatible"]:
        return "FAIL_CLOSED"
    if row["binding_required"] and not row["binding_valid"]:
        return "FAIL_CLOSED"
    if row["trust_mutation_requested"]:
        return "FAIL_CLOSED"
    return "ESTABLISH"


def load_rows() -> list[dict[str, object]]:
    lines = [line for line in CORPUS.read_text(encoding="utf-8").splitlines() if not line.startswith("#")]
    reader = csv.DictReader(lines, delimiter="|")
    rows: list[dict[str, object]] = []
    for raw in reader:
        row: dict[str, object] = dict(raw)
        for field in BOOL_FIELDS:
            row[field] = parse_bool(str(row[field]))
        rows.append(row)
    return rows


def main() -> int:
    rows = load_rows()
    assert len(rows) == 25, f"expected 25 canonical rows, got {len(rows)}"

    for row in rows:
        observed = classify(row)
        assert observed == row["expected"], (
            f"canonical classification drift {row['case_id']}: {observed} != {row['expected']}"
        )

    positive = [row for row in rows if row["expected"] == "ESTABLISH"]
    assert positive, "canonical corpus has no establishment baseline"

    # Symmetric assurance requires executable mutation baselines in both
    # constrained<->higher-capability directions. A one-way positive corpus
    # could otherwise keep the aggregate mutation count green while silently
    # dropping reverse-role Common Contract qualification.
    positive_directions = {
        (str(row["peer_a"]), str(row["peer_b"]))
        for row in positive
        if row["peer_a"] != row["peer_b"]
    }
    assert CROSS_CLASS_DIRECTIONS <= positive_directions, (
        "missing positive cross-class direction(s): "
        f"{sorted(CROSS_CLASS_DIRECTIONS - positive_directions)}"
    )

    # Synthetic mutations are necessary but not sufficient evidence that the
    # canonical Rust/C corpus directly exercises lifecycle loss in both role
    # directions. Preserve explicit offline establishment, binding-required
    # establishment, stale authorization-generation rejection, and key-usage
    # continuity rejection for each constrained<->higher-capability direction.
    # This prevents a future corpus edit from keeping generated mutations green
    # while deleting the reverse-role executable evidence actually consumed by
    # both language harnesses.
    for direction in CROSS_CLASS_DIRECTIONS:
        direct = [
            row
            for row in rows
            if (str(row["peer_a"]), str(row["peer_b"])) == direction
        ]
        assert any(
            row["expected"] == "ESTABLISH" and not row["infrastructure_available"]
            for row in direct
        ), f"{direction[0]}->{direction[1]} lacks direct offline establishment evidence"
        assert any(
            row["expected"] == "ESTABLISH"
            and row["binding_required"]
            and row["binding_valid"]
            for row in direct
        ), f"{direction[0]}->{direction[1]} lacks direct required-binding establishment evidence"
        assert any(
            row["expected"] == "FAIL_CLOSED"
            and not row["authorization_generation_current"]
            for row in direct
        ), f"{direction[0]}->{direction[1]} lacks direct stale-generation rejection evidence"
        assert any(
            row["expected"] == "FAIL_CLOSED"
            and not row["usage_counter_continuity_current"]
            for row in direct
        ), f"{direction[0]}->{direction[1]} lacks direct usage-continuity rejection evidence"

    mutation_count = 0
    dimensions: set[str] = set()
    direction_dimensions: dict[tuple[str, str], set[str]] = {
        direction: set() for direction in CROSS_CLASS_DIRECTIONS
    }
    for base in positive:
        direction = (str(base["peer_a"]), str(base["peer_b"]))
        for field, value in FAIL_CLOSED_MUTATIONS.items():
            mutated = dict(base)
            mutated[field] = value
            assert classify(mutated) == "FAIL_CLOSED", (
                f"security mutation escaped fail-closed: {base['case_id']} field={field}"
            )
            mutation_count += 1
            dimensions.add(field)
            if direction in direction_dimensions:
                direction_dimensions[direction].add(field)

        if base["binding_required"]:
            mutated = dict(base)
            mutated["binding_valid"] = False
            assert classify(mutated) == "FAIL_CLOSED", (
                f"binding mutation escaped fail-closed: {base['case_id']}"
            )
            mutation_count += 1
            dimensions.add("binding_valid_when_required")
            if direction in direction_dimensions:
                direction_dimensions[direction].add("binding_valid_when_required")

        infrastructure_flip = dict(base)
        infrastructure_flip["infrastructure_available"] = not bool(base["infrastructure_available"])
        assert classify(infrastructure_flip) == "ESTABLISH", (
            f"optional infrastructure became protocol authority: {base['case_id']}"
        )
        mutation_count += 1
        dimensions.add("infrastructure_non_authority")
        if direction in direction_dimensions:
            direction_dimensions[direction].add("infrastructure_non_authority")

    expected_dimensions = set(FAIL_CLOSED_MUTATIONS) | {
        "binding_valid_when_required",
        "infrastructure_non_authority",
    }
    assert dimensions == expected_dimensions, f"mutation dimension drift: {sorted(dimensions)}"
    for direction, covered in direction_dimensions.items():
        assert covered == expected_dimensions, (
            f"cross-class direction {direction[0]}->{direction[1]} missing mutation dimensions: "
            f"{sorted(expected_dimensions - covered)}"
        )
    assert mutation_count >= 92, f"insufficient mutation breadth: {mutation_count}"
    print(
        "p2p-common-contract-mutations: PASS "
        f"canonical={len(rows)} positive={len(positive)} mutations={mutation_count} "
        f"dimensions={len(dimensions)} cross_class_directions={len(direction_dimensions)}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
