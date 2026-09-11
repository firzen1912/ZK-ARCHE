#!/usr/bin/env python3
from __future__ import annotations

import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
CORPUS = ROOT / "rust/test-vectors/state/association-admission-v4.txt"
EXPECTED_CASES = 21
EXPECTED_FIELDS = 19  # case id + 16 facts + decision + reason

# Canonical v4 fact positions after case id. Keep these names centralized so
# governance checks remain readable when protecting authorization/revocation
# semantics from accidental corpus drift.
FACT_INDEX = {
    "authorization_present": 3,
    "authorization_current": 4,
    "authorization_generation_bound": 5,
    "authorization_generation_current": 6,
    "revocation_view_current": 7,
    "revoked": 8,
    "lineage_current": 9,
}


def fail(message: str) -> None:
    print(f"association-admission-corpus-governance: FAIL: {message}", file=sys.stderr)
    raise SystemExit(1)


def load_cases() -> dict[str, list[str]]:
    lines = [line.strip() for line in CORPUS.read_text(encoding="utf-8").splitlines() if line.strip()]
    if not lines or lines[0] != "version=4":
        fail("expected canonical association-admission corpus version=4")

    cases: dict[str, list[str]] = {}
    for line in lines[1:]:
        if not line.startswith("case="):
            fail(f"unexpected corpus line: {line}")
        fields = line.removeprefix("case=").split("|")
        if len(fields) != EXPECTED_FIELDS:
            fail(f"{fields[0] if fields else '<unknown>'}: expected {EXPECTED_FIELDS} fields, got {len(fields)}")
        case_id = fields[0]
        if case_id in cases:
            fail(f"duplicate case id {case_id}")
        cases[case_id] = fields

    if len(cases) != EXPECTED_CASES:
        fail(f"expected {EXPECTED_CASES} cases, got {len(cases)}")
    return cases


def require_lifecycle_case(
    cases: dict[str, list[str]],
    case_id: str,
    *,
    expected_reason: str,
    facts: dict[str, str],
) -> None:
    fields = cases.get(case_id)
    if fields is None:
        fail(f"missing required governance case {case_id}")
    for name, expected in facts.items():
        index = FACT_INDEX[name]
        if fields[index] != expected:
            fail(f"{case_id}: {name} drifted from {expected}")
    if fields[-2] != "FAIL_CLOSED" or fields[-1] != expected_reason:
        fail(f"{case_id}: expected FAIL_CLOSED/{expected_reason}, got {fields[-2]}/{fields[-1]}")


def require_case(
    cases: dict[str, list[str]],
    case_id: str,
    *,
    rollback_suspected: str,
    trust_mutation_requested: str,
    expected_reason: str,
) -> None:
    fields = cases.get(case_id)
    if fields is None:
        fail(f"missing required governance case {case_id}")

    # v4 tail layout: binding_required, binding_valid, rollback_suspected,
    # trust_mutation_requested, decision, reason.
    if fields[15] != rollback_suspected:
        fail(f"{case_id}: rollback_suspected drifted from {rollback_suspected}")
    if fields[16] != trust_mutation_requested:
        fail(f"{case_id}: trust_mutation_requested drifted from {trust_mutation_requested}")
    if fields[-2] != "FAIL_CLOSED" or fields[-1] != expected_reason:
        fail(f"{case_id}: expected FAIL_CLOSED/{expected_reason}, got {fields[-2]}/{fields[-1]}")


def main() -> None:
    cases = load_cases()

    # Preserve the authorization/revocation/lineage boundary as separate
    # fail-closed evidence. A future corpus cleanup must not silently merge
    # stale grants, generation binding, revocation freshness, revocation, or
    # lineage into a weaker generic authorization failure.
    require_lifecycle_case(
        cases,
        "ASC4-004",
        expected_reason="AUTHORIZATION_MISSING",
        facts={"authorization_present": "0"},
    )
    require_lifecycle_case(
        cases,
        "ASC4-005",
        expected_reason="AUTHORIZATION_STALE",
        facts={"authorization_present": "1", "authorization_current": "0"},
    )
    require_lifecycle_case(
        cases,
        "ASC4-006",
        expected_reason="AUTHORIZATION_GENERATION_UNBOUND",
        facts={"authorization_current": "1", "authorization_generation_bound": "0"},
    )
    require_lifecycle_case(
        cases,
        "ASC4-007",
        expected_reason="AUTHORIZATION_GENERATION_STALE",
        facts={"authorization_generation_bound": "1", "authorization_generation_current": "0"},
    )
    require_lifecycle_case(
        cases,
        "ASC4-008",
        expected_reason="REVOCATION_STALE",
        facts={"authorization_generation_current": "1", "revocation_view_current": "0", "revoked": "0"},
    )
    require_lifecycle_case(
        cases,
        "ASC4-009",
        expected_reason="REVOKED",
        facts={"revocation_view_current": "1", "revoked": "1"},
    )
    require_lifecycle_case(
        cases,
        "ASC4-010",
        expected_reason="LINEAGE_STALE",
        facts={"revocation_view_current": "1", "revoked": "0", "lineage_current": "0"},
    )

    # Preserve both isolated negatives and compound precedence negatives. This
    # prevents a future cleanup from replacing precedence evidence with a
    # simpler isolated case (or vice versa).
    require_case(
        cases,
        "ASC4-016",
        rollback_suspected="1",
        trust_mutation_requested="0",
        expected_reason="ROLLBACK_SUSPECTED",
    )
    require_case(
        cases,
        "ASC4-017",
        rollback_suspected="0",
        trust_mutation_requested="1",
        expected_reason="TRUST_MUTATION_REQUESTED",
    )
    require_case(
        cases,
        "ASC4-018",
        rollback_suspected="1",
        trust_mutation_requested="1",
        expected_reason="ROLLBACK_SUSPECTED",
    )
    require_case(
        cases,
        "ASC4-019",
        rollback_suspected="0",
        trust_mutation_requested="1",
        expected_reason="TRUST_MUTATION_REQUESTED",
    )

    # Compound cases must remain genuinely compound, with upstream lifecycle
    # facts also invalid, so they continue exercising terminal-reason
    # precedence rather than silently collapsing into duplicate isolated cases.
    for case_id in ("ASC4-018", "ASC4-019"):
        fields = cases[case_id]
        if all(value == "1" for value in fields[1:8]):
            fail(f"{case_id}: compound precedence coverage collapsed to an isolated case")

    print(
        "association-admission-corpus-governance: PASS "
        f"version=4 cases={len(cases)} authorization_revocation=7 isolated=2 compound=2"
    )


if __name__ == "__main__":
    main()
