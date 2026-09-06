#!/usr/bin/env python3
"""Qualify normalized revocation-view reconciliation semantics.

This is a wire-neutral oracle for zk214. It assumes the incoming view has
already passed structural validation. Authentication is represented explicitly
so structurally valid but unauthenticated state can never advance local
revocation authority.
"""

from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
CORPUS = ROOT / "rust/test-vectors/state/revocation-reconciliation-v1.txt"


@dataclass(frozen=True)
class Case:
    case_id: str
    local_authority: str | None
    local_epoch: int
    update_authority: str
    update_epoch: int
    update_kind: str
    base_epoch: int
    authenticated: bool
    same_object: bool
    expected: str
    result_epoch: int


def classify(case: Case) -> tuple[str, int]:
    if not case.authenticated:
        return "REJECT_UNAUTHENTICATED", case.local_epoch

    if case.local_authority is not None and case.update_authority != case.local_authority:
        return "REJECT_AUTHORITY", case.local_epoch

    if case.update_epoch < case.local_epoch:
        return "REJECT_ROLLBACK", case.local_epoch

    if case.update_epoch == case.local_epoch and case.local_epoch != 0:
        if case.same_object:
            return "NOOP_DUPLICATE", case.local_epoch
        return "REJECT_CONFLICT", case.local_epoch

    if case.same_object:
        raise ValueError(f"{case.case_id}: same_object only valid at the incorporated epoch")

    if case.update_kind == "FULL":
        if case.base_epoch != 0:
            raise ValueError(f"{case.case_id}: structurally invalid FULL base_epoch")
        return "APPLY", case.update_epoch

    if case.update_kind == "DIFF":
        if case.local_authority is None or case.base_epoch != case.local_epoch:
            return "REJECT_BASE_EPOCH", case.local_epoch
        return "APPLY", case.update_epoch

    raise ValueError(f"{case.case_id}: unknown update_kind {case.update_kind}")


def parse_bool(value: str, case_id: str) -> bool:
    if value == "0":
        return False
    if value == "1":
        return True
    raise ValueError(f"{case_id}: expected boolean 0/1, got {value!r}")


def load_cases() -> list[Case]:
    cases: list[Case] = []
    for line_no, raw in enumerate(CORPUS.read_text(encoding="utf-8").splitlines(), 1):
        line = raw.strip()
        if not line or line.startswith("#"):
            continue
        fields = line.split("|")
        if len(fields) != 11:
            raise ValueError(f"line {line_no}: expected 11 fields, got {len(fields)}")
        case_id = fields[0]
        cases.append(
            Case(
                case_id=case_id,
                local_authority=None if fields[1] == "-" else fields[1],
                local_epoch=int(fields[2]),
                update_authority=fields[3],
                update_epoch=int(fields[4]),
                update_kind=fields[5],
                base_epoch=int(fields[6]),
                authenticated=parse_bool(fields[7], case_id),
                same_object=parse_bool(fields[8], case_id),
                expected=fields[9],
                result_epoch=int(fields[10]),
            )
        )
    return cases


def main() -> int:
    cases = load_cases()
    if not cases:
        raise SystemExit("revocation reconciliation corpus is empty")

    seen: set[str] = set()
    failures: list[str] = []
    for case in cases:
        if case.case_id in seen:
            failures.append(f"duplicate case id: {case.case_id}")
            continue
        seen.add(case.case_id)
        actual, result_epoch = classify(case)
        if (actual, result_epoch) != (case.expected, case.result_epoch):
            failures.append(
                f"{case.case_id}: expected {case.expected}/{case.result_epoch}, "
                f"got {actual}/{result_epoch}"
            )

    required = {
        "APPLY",
        "NOOP_DUPLICATE",
        "REJECT_UNAUTHENTICATED",
        "REJECT_AUTHORITY",
        "REJECT_ROLLBACK",
        "REJECT_BASE_EPOCH",
        "REJECT_CONFLICT",
    }
    covered = {case.expected for case in cases}
    missing = sorted(required - covered)
    if missing:
        failures.append("missing decision coverage: " + ", ".join(missing))

    if failures:
        for failure in failures:
            print(f"FAIL: {failure}")
        return 1

    print(f"revocation reconciliation: PASS ({len(cases)} cases)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
