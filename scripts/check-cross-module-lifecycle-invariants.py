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
    path = ROOT / "rust/test-vectors/p2p/common-contract-lifecycle-v2.txt"
    lines = [line for line in path.read_text(encoding="utf-8").splitlines() if not line.startswith("#")]
    return {row["case_id"]: row for row in csv.DictReader(lines, delimiter="|")}


def main() -> int:
    enrollment = decision_rows("rust/test-vectors/state/enrollment-grant-v3.txt")
    require_decision(enrollment, "ENR3-001", "ISSUE", "CURRENT")
    require_decision(enrollment, "ENR3-006", "DENY", "COMMISSIONER_AUTHORIZATION_STALE")
    require_decision(enrollment, "ENR3-008", "DENY", "ENROLLMENT_REPLAY_DETECTED")
    require_decision(enrollment, "ENR3-016", "DENY", "REVOCATION_STALE")
    require_decision(enrollment, "ENR3-017", "DENY", "LINEAGE_STALE")
    require_decision(enrollment, "ENR3-019", "DENY", "ROLLBACK_SUSPECTED")
    require_decision(enrollment, "ENR3-020", "DENY", "COMMISSIONER_AUTHORIZATION_GENERATION_STALE")

    resumption = decision_rows("rust/test-vectors/state/resumption-authorization-v4.txt")
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

    delegation = decision_rows("rust/test-vectors/p2p/bounded-delegation-v2.txt")
    require_decision(delegation, "DEL2-001", "ACCEPT", "CURRENT")
    require_decision(delegation, "DEL2-012", "DENY", "REVOCATION_STALE")
    require_decision(delegation, "DEL2-014", "DENY", "LINEAGE_STALE")
    require_decision(delegation, "DEL2-018", "DENY", "ROLLBACK_SUSPECTED")

    p2p = load_p2p()
    assert p2p["XC2-001"]["expected"] == "ESTABLISH"
    for case in ("XC2-008", "XC2-009", "XC2-010", "XC2-011", "XC2-012", "XC2-013", "XC2-017"):
        assert p2p[case]["expected"] == "FAIL_CLOSED", f"{case}: expected FAIL_CLOSED"

    assert delegation["DEL2-001"] == ("ACCEPT", "CURRENT")
    for case in ("XC2-008", "XC2-009", "XC2-010", "XC2-011", "XC2-012", "XC2-013"):
        assert p2p[case]["expected"] == "FAIL_CLOSED", f"{case}: delegation must not repair lifecycle state"

    offline = p2p["XC2-002"]
    online = p2p["XC2-004"]
    compared = ("peer_a", "peer_b", "auth_complete", "preexisting_trust", "authorization_present", "authorization_fresh", "authorization_generation_current", "revocation_current", "revoked", "lineage_current", "replay_continuity_current", "restart_continuity_current", "mandatory_floor_compatible", "binding_required", "binding_valid", "trust_mutation_requested", "expected")
    assert all(offline[field] == online[field] for field in compared)
    assert offline["infrastructure_available"] == "false"
    assert online["infrastructure_available"] == "true"
    assert offline["expected"] == online["expected"] == "ESTABLISH"

    print("cross-module-lifecycle-invariants: PASS surfaces=6 authz_generation=7 revocation=5 lineage=5 replay_restart=5 transport_non_authority=2 infrastructure_non_authority=1 delegation_non_repair=6")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
