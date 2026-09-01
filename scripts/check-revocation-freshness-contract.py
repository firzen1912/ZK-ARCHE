#!/usr/bin/env python3
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
SPEC = ROOT / "spec/revocation-convergence-and-stale-authorization.md"
CORPUS = ROOT / "rust/test-vectors/state/revocation-freshness-v1.txt"

EXPECTED = {
    "current-authorized": "AUTHORIZED",
    "offline-within-bound": "AUTHORIZED",
    "explicit-revocation": "REVOKED",
    "stale-view": "STALE_VIEW",
    "old-epoch": "ROLLBACK_OR_OLD_EPOCH",
    "invalid-view": "INVALID_VIEW",
    "stale-lineage": "STALE_LINEAGE",
    "scope-denied": "SCOPE_DENIED",
}

REQUIRED_SPEC = [
    "Authentication, authorization, and trust mutation remain distinct.",
    "MUST fail closed",
    "MUST NOT be silently converted to authorization",
    "Always-online infrastructure MUST NOT be required",
    "Core AUTH remains NO-LEARNING.",
    "Local trust remains non-transitive.",
    "A non-`AUTHORIZED` decision",
    "view_epoch < required_min_epoch",
    "max_staleness",
]


def fail(msg: str) -> None:
    print(f"revocation-freshness-contract: FAIL: {msg}", file=sys.stderr)
    raise SystemExit(1)


def parse_bool(value: str, case_id: str, field: str) -> bool:
    if value not in {"true", "false"}:
        fail(f"{case_id}: {field} must be true/false")
    return value == "true"


def classify(parts):
    case_id = parts[0]
    integrity = parse_bool(parts[1], case_id, "view_integrity_valid")
    view_epoch = int(parts[2])
    required = int(parts[3])
    age = int(parts[4])
    max_staleness = int(parts[5])
    revoked = parse_bool(parts[6], case_id, "holder_revoked")
    lineage_current = parse_bool(parts[7], case_id, "lineage_current")
    scope_authorized = parse_bool(parts[8], case_id, "scope_authorized")
    if min(view_epoch, required, age, max_staleness) < 0:
        fail(f"{case_id}: numeric fields must be non-negative")
    if not integrity:
        return "INVALID_VIEW"
    if view_epoch < required:
        return "ROLLBACK_OR_OLD_EPOCH"
    if age > max_staleness:
        return "STALE_VIEW"
    if revoked:
        return "REVOKED"
    if not lineage_current:
        return "STALE_LINEAGE"
    if not scope_authorized:
        return "SCOPE_DENIED"
    return "AUTHORIZED"


def main() -> None:
    if not SPEC.is_file() or not CORPUS.is_file():
        fail("required spec/corpus file missing")
    spec = SPEC.read_text(encoding="utf-8")
    for marker in REQUIRED_SPEC:
        if marker not in spec:
            fail(f"normative marker missing: {marker!r}")

    lines = [ln.strip() for ln in CORPUS.read_text(encoding="utf-8").splitlines() if ln.strip() and not ln.startswith("#")]
    seen = {}
    for line in lines:
        parts = line.split("|")
        if len(parts) != 10:
            fail(f"malformed corpus row: {line}")
        case_id, expected = parts[0], parts[9]
        if case_id in seen:
            fail(f"duplicate case: {case_id}")
        observed = classify(parts)
        if expected != observed:
            fail(f"{case_id}: expected field {expected}, classifier yields {observed}")
        seen[case_id] = expected

    if seen != EXPECTED:
        missing = sorted(set(EXPECTED) - set(seen))
        extra = sorted(set(seen) - set(EXPECTED))
        drift = sorted(k for k in set(seen) & set(EXPECTED) if seen[k] != EXPECTED[k])
        fail(f"corpus drift missing={missing} extra={extra} changed={drift}")

    print(f"revocation-freshness-contract: PASS cases={len(seen)} authorized=2 denied={len(seen)-2}")


if __name__ == "__main__":
    main()
