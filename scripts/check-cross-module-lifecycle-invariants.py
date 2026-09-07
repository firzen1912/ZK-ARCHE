#!/usr/bin/env python3
"""Cross-module lifecycle invariant audit for canonical decision corpora."""
from __future__ import annotations

import csv
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def decision_rows(path: str) -> dict[str, tuple[str, str]]:
    rows: dict[str, tuple[str, str]] = {}
    for raw in (ROOT / path).read_text(encoding="utf-8").splitlines():
        if not raw.startswith("case="):
            continue
        parts = raw.split("|")
        rows[parts[0].removeprefix("case=")] = (parts[-2], parts[-1])
    return rows


def require_decision(rows: dict[str, tuple[str, str]], case: str, action: str, reason: str) -> None:
    expected = (action, reason)
    observed = rows.get(case)
    assert observed == expected, f"{case}: expected {expected}, observed {observed}"


def load_p2p() -> dict[str, dict[str, str]]:
    path = ROOT / "rust/test-vectors/p2p/common-contract-lifecycle-v4.txt"
    lines = [line for line in path.read_text(encoding="utf-8").splitlines() if not line.startswith("#")]
    return {row["case_id"]: row for row in csv.DictReader(lines, delimiter="|")}


def load_revocation_reconciliation() -> dict[str, dict[str, str]]:
    path = ROOT / "rust/test-vectors/state/revocation-reconciliation-v1.txt"
    lines = [
        line
        for line in path.read_text(encoding="utf-8").splitlines()
        if line and not line.startswith("#")
    ]
    fields = (
        "case_id",
        "local_authority",
        "local_epoch",
        "update_authority",
        "update_epoch",
        "update_kind",
        "base_epoch",
        "authenticated",
        "same_object",
        "expected",
        "result_epoch",
    )
    return {
        row["case_id"]: row
        for row in csv.DictReader(lines, fieldnames=fields, delimiter="|")
    }


def main() -> int:
    association = decision_rows("rust/test-vectors/state/association-admission-v4.txt")
    require_decision(association, "ASC4-001", "ESTABLISH", "CURRENT")
    require_decision(association, "ASC4-006", "FAIL_CLOSED", "AUTHORIZATION_GENERATION_UNBOUND")
    require_decision(association, "ASC4-007", "FAIL_CLOSED", "AUTHORIZATION_GENERATION_STALE")
    require_decision(association, "ASC4-008", "FAIL_CLOSED", "REVOCATION_STALE")
    require_decision(association, "ASC4-010", "FAIL_CLOSED", "LINEAGE_STALE")
    require_decision(association, "ASC4-011", "FAIL_CLOSED", "REPLAY_CONTINUITY_STALE")
    require_decision(association, "ASC4-012", "FAIL_CLOSED", "RESTART_CONTINUITY_STALE")
    require_decision(association, "ASC4-013", "FAIL_CLOSED", "USAGE_COUNTER_CONTINUITY_STALE")
    require_decision(association, "ASC4-016", "FAIL_CLOSED", "ROLLBACK_SUSPECTED")

    enrollment = decision_rows("rust/test-vectors/state/enrollment-grant-v3.txt")
    require_decision(enrollment, "ENR3-001", "ISSUE", "CURRENT")
    require_decision(enrollment, "ENR3-006", "DENY", "COMMISSIONER_AUTHORIZATION_STALE")
    require_decision(enrollment, "ENR3-008", "DENY", "ENROLLMENT_REPLAY_DETECTED")
    require_decision(enrollment, "ENR3-016", "DENY", "REVOCATION_STALE")
    require_decision(enrollment, "ENR3-017", "DENY", "LINEAGE_STALE")
    require_decision(enrollment, "ENR3-019", "DENY", "ROLLBACK_SUSPECTED")
    require_decision(enrollment, "ENR3-020", "DENY", "COMMISSIONER_AUTHORIZATION_GENERATION_STALE")

    resumption = decision_rows("rust/test-vectors/state/resumption-authorization-v5.txt")
    require_decision(resumption, "current", "RESUME", "CURRENT")
    require_decision(resumption, "authz-stale", "FULL_AUTH_REQUIRED", "AUTHORIZATION_STALE")
    require_decision(resumption, "authz-generation-unbound", "FULL_AUTH_REQUIRED", "AUTHORIZATION_GENERATION_UNBOUND")
    require_decision(resumption, "authz-generation-stale", "FULL_AUTH_REQUIRED", "AUTHORIZATION_GENERATION_STALE")
    require_decision(resumption, "revocation-stale", "REJECT", "REVOCATION_STALE")
    require_decision(resumption, "revoked", "REJECT", "REVOKED")
    require_decision(resumption, "lineage-stale", "REJECT", "LINEAGE_STALE")
    require_decision(resumption, "restart-stale", "REJECT", "RESTART_CONTINUITY_STALE")
    require_decision(resumption, "usage-counter-continuity-stale", "REJECT", "USAGE_COUNTER_CONTINUITY_STALE")
    require_decision(resumption, "rollback", "REJECT", "ROLLBACK_SUSPECTED")
    require_decision(resumption, "privacy-identifier-state-stale", "FULL_AUTH_REQUIRED", "PRIVACY_IDENTIFIER_STATE_STALE")
    require_decision(resumption, "repeated-identifier-linkable", "FULL_AUTH_REQUIRED", "REPEATED_IDENTIFIER_LINKABLE")
    require_decision(resumption, "privacy-stale-at-reuse-limit", "FULL_AUTH_REQUIRED", "PRIVACY_IDENTIFIER_STATE_STALE")
    require_decision(resumption, "repeated-id-at-reuse-limit", "FULL_AUTH_REQUIRED", "REPEATED_IDENTIFIER_LINKABLE")
    require_decision(resumption, "privacy-stale-with-epoch-stale", "FULL_AUTH_REQUIRED", "PRIVACY_IDENTIFIER_STATE_STALE")
    require_decision(resumption, "repeated-id-with-epoch-stale", "FULL_AUTH_REQUIRED", "REPEATED_IDENTIFIER_LINKABLE")
    require_decision(resumption, "privacy-stale-with-binding-mismatch", "FULL_AUTH_REQUIRED", "PRIVACY_IDENTIFIER_STATE_STALE")
    require_decision(resumption, "repeated-id-with-profile-mismatch", "FULL_AUTH_REQUIRED", "REPEATED_IDENTIFIER_LINKABLE")

    transport = decision_rows("rust/test-vectors/state/transport-continuation-v3.txt")
    require_decision(transport, "steady", "CONTINUE", "CURRENT")
    require_decision(transport, "route-changed", "CONTINUE", "CURRENT")
    require_decision(transport, "connection-changed", "CONTINUE", "CURRENT")
    require_decision(transport, "replay-stale", "REJECT", "REPLAY_CONTINUITY_STALE")
    require_decision(transport, "usage-counter-stale", "REJECT", "USAGE_COUNTER_CONTINUITY_STALE")
    require_decision(transport, "authorization-generation-unbound", "FULL_AUTH_REQUIRED", "AUTHORIZATION_GENERATION_UNBOUND")
    require_decision(transport, "authorization-generation-stale", "FULL_AUTH_REQUIRED", "AUTHORIZATION_GENERATION_STALE")
    require_decision(transport, "address-as-identity", "REJECT", "TRANSPORT_ADDRESS_AS_IDENTITY")
    require_decision(transport, "metadata-as-authority", "REJECT", "TRANSPORT_METADATA_AS_AUTHORITY")

    data = decision_rows("rust/test-vectors/state/data-release-authorization-v4.txt")
    require_decision(data, "current", "RELEASE", "CURRENT")
    require_decision(data, "authorization-stale", "DENY", "AUTHORIZATION_STALE")
    require_decision(data, "authorization-generation-unbound", "DENY", "AUTHORIZATION_GENERATION_UNBOUND")
    require_decision(data, "authorization-generation-stale", "DENY", "AUTHORIZATION_GENERATION_STALE")
    require_decision(data, "revocation-stale", "DENY", "REVOCATION_STALE")
    require_decision(data, "revoked", "DENY", "REVOKED")
    require_decision(data, "lineage-stale", "DENY", "LINEAGE_STALE")
    require_decision(data, "binding-invalid", "DENY", "CHANNEL_BINDING_MISSING_OR_INVALID")
    require_decision(data, "release-replay", "DENY", "RELEASE_REPLAY_DETECTED")
    require_decision(data, "rollback", "DENY", "ROLLBACK_SUSPECTED")

    delegation = decision_rows("rust/test-vectors/p2p/bounded-delegation-v3.txt")
    require_decision(delegation, "DEL3-001", "ACCEPT", "CURRENT")
    require_decision(delegation, "DEL3-011", "DENY", "AUTHORIZATION_GENERATION_UNBOUND")
    require_decision(delegation, "DEL3-012", "DENY", "AUTHORIZATION_GENERATION_STALE")
    require_decision(delegation, "DEL3-014", "DENY", "REVOCATION_STALE")
    require_decision(delegation, "DEL3-016", "DENY", "LINEAGE_STALE")
    require_decision(delegation, "DEL3-020", "DENY", "ROLLBACK_SUSPECTED")

    revocation = load_revocation_reconciliation()
    for case in ("bootstrap-full", "full-advance", "diff-next"):
        row = revocation[case]
        assert row["authenticated"] == "1", f"{case}: incorporated update must be authenticated"
        assert row["expected"] == "APPLY", f"{case}: expected authoritative incorporation"
        assert int(row["result_epoch"]) > int(row["local_epoch"]), f"{case}: epoch must advance"

    for case in ("unauthenticated-full", "unauthenticated-diff"):
        row = revocation[case]
        assert row["expected"] == "REJECT_UNAUTHENTICATED", case
        assert row["result_epoch"] == row["local_epoch"], f"{case}: rejection must preserve epoch"

    # A newly incorporated revocation epoch must not be a bookkeeping-only event:
    # every retained-authority surface must already have a fail-closed path for
    # stale/revoked lifecycle state before it can authorize protected work again.
    require_decision(association, "ASC4-008", "FAIL_CLOSED", "REVOCATION_STALE")
    require_decision(enrollment, "ENR3-016", "DENY", "REVOCATION_STALE")
    require_decision(resumption, "revocation-stale", "REJECT", "REVOCATION_STALE")
    require_decision(resumption, "revoked", "REJECT", "REVOKED")
    require_decision(data, "revocation-stale", "DENY", "REVOCATION_STALE")
    require_decision(data, "revoked", "DENY", "REVOKED")
    require_decision(delegation, "DEL3-014", "DENY", "REVOCATION_STALE")

    p2p = load_p2p()
    assert p2p["XC4-001"]["expected"] == "ESTABLISH"
    for case in ("XC4-008", "XC4-009", "XC4-010", "XC4-011", "XC4-012", "XC4-013", "XC4-017", "XC4-021"):
        assert p2p[case]["expected"] == "FAIL_CLOSED", f"{case}: expected FAIL_CLOSED"

    for case in ("XC4-008", "XC4-009", "XC4-010", "XC4-011", "XC4-012", "XC4-013", "XC4-021"):
        assert p2p[case]["expected"] == "FAIL_CLOSED", f"{case}: delegation must not repair lifecycle state"

    for case in ("XC4-022", "XC4-023", "XC4-024"):
        assert p2p[case]["usage_counter_continuity_current"] == "false", case
        assert p2p[case]["expected"] == "FAIL_CLOSED", f"{case}: usage-counter continuity must fail closed"

    offline = p2p["XC4-002"]
    online = p2p["XC4-004"]
    compared = ("peer_a","peer_b","auth_complete","preexisting_trust","authorization_present","authorization_fresh","authorization_generation_bound","authorization_generation_current","revocation_current","revoked","lineage_current","replay_continuity_current","restart_continuity_current","usage_counter_continuity_current","mandatory_floor_compatible","binding_required","binding_valid","trust_mutation_requested","expected")
    assert all(offline[field] == online[field] for field in compared)
    assert offline["infrastructure_available"] == "false"
    assert online["infrastructure_available"] == "true"
    assert offline["expected"] == online["expected"] == "ESTABLISH"

    print("cross-module-lifecycle-invariants: PASS surfaces=8 authz_generation=12 revocation=13 revocation_ingestion=5 lineage=6 replay_restart=7 usage_counter=6 privacy_resumption=8 transport_non_authority=2 infrastructure_non_authority=1 delegation_non_repair=7")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
