#!/usr/bin/env python3
"""Cross-module lifecycle invariant audit for canonical decision corpora."""
from __future__ import annotations

import csv
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def decision_rows(path: str) -> dict[str, tuple[str, str]]:
    rows: dict[str, tuple[str, str]] = {}
    for raw in (ROOT / path).read_text(encoding="utf-8").splitlines():
        if raw.startswith("case="):
            parts = raw.split("|")
            rows[parts[0].removeprefix("case=")] = (parts[-2], parts[-1])
    return rows


def require_decision(rows: dict[str, tuple[str, str]], case: str, action: str, reason: str) -> None:
    observed = rows.get(case)
    expected = (action, reason)
    assert observed == expected, f"{case}: expected {expected}, observed {observed}"


def load_p2p() -> dict[str, dict[str, str]]:
    lines = [line for line in (ROOT / "rust/test-vectors/p2p/common-contract-lifecycle-v4.txt").read_text(encoding="utf-8").splitlines() if not line.startswith("#")]
    return {row["case_id"]: row for row in csv.DictReader(lines, delimiter="|")}


def load_revocation_reconciliation() -> dict[str, dict[str, str]]:
    lines = [line for line in (ROOT / "rust/test-vectors/state/revocation-reconciliation-v1.txt").read_text(encoding="utf-8").splitlines() if line and not line.startswith("#")]
    fields = ("case_id", "local_authority", "local_epoch", "update_authority", "update_epoch", "update_kind", "base_epoch", "authenticated", "same_object", "expected", "result_epoch")
    return {row["case_id"]: row for row in csv.DictReader(lines, fieldnames=fields, delimiter="|")}


def check(rows, action, cases):
    for case, reason in cases.items():
        require_decision(rows, case, action, reason)


def main() -> int:
    association = decision_rows("rust/test-vectors/state/association-admission-v4.txt")
    check(association, "FAIL_CLOSED", {
        "ASC4-006":"AUTHORIZATION_GENERATION_UNBOUND", "ASC4-007":"AUTHORIZATION_GENERATION_STALE",
        "ASC4-008":"REVOCATION_STALE", "ASC4-010":"LINEAGE_STALE", "ASC4-011":"REPLAY_CONTINUITY_STALE",
        "ASC4-012":"RESTART_CONTINUITY_STALE", "ASC4-013":"USAGE_COUNTER_CONTINUITY_STALE", "ASC4-016":"ROLLBACK_SUSPECTED",
    })
    require_decision(association, "ASC4-001", "ESTABLISH", "CURRENT")

    enrollment = decision_rows("rust/test-vectors/state/enrollment-grant-v4.txt")
    check(enrollment, "DENY", {
        "ENR4-006":"COMMISSIONER_AUTHORIZATION_STALE", "ENR4-008":"ENROLLMENT_REPLAY_DETECTED",
        "ENR4-016":"REVOCATION_STALE", "ENR4-017":"LINEAGE_STALE", "ENR4-019":"ROLLBACK_SUSPECTED",
        "ENR4-020":"COMMISSIONER_AUTHORIZATION_GENERATION_STALE", "ENR4-021":"COMMISSIONER_AUTHORIZATION_GENERATION_UNBOUND",
        "ENR4-022":"ROLLBACK_SUSPECTED", "ENR4-023":"COMMISSIONER_UNAUTHENTICATED",
        "ENR4-024":"COMMISSIONER_AUTHORIZATION_GENERATION_UNBOUND", "ENR4-025":"COMMISSIONER_REVOKED",
        "ENR4-026":"AUTHORITY_ESCALATION", "ENR4-027":"EPOCH_STALE", "ENR4-028":"REVOCATION_STALE",
        "ENR4-029":"COMMISSIONER_UNAUTHORIZED", "ENR4-030":"COMMISSIONER_AUTHORIZATION_GENERATION_STALE",
        "ENR4-031":"NORMAL_AUTH_FORBIDDEN", "ENR4-032":"COMMISSIONER_REVOKED", "ENR4-033":"LINEAGE_STALE",
    })
    require_decision(enrollment, "ENR4-001", "ISSUE", "CURRENT")

    resumption = decision_rows("rust/test-vectors/state/resumption-authorization-v5.txt")
    require_decision(resumption, "current", "RESUME", "CURRENT")
    check(resumption, "FULL_AUTH_REQUIRED", {
        "authz-stale":"AUTHORIZATION_STALE", "authz-generation-unbound":"AUTHORIZATION_GENERATION_UNBOUND",
        "authz-generation-stale":"AUTHORIZATION_GENERATION_STALE", "privacy-identifier-state-stale":"PRIVACY_IDENTIFIER_STATE_STALE",
        "repeated-identifier-linkable":"REPEATED_IDENTIFIER_LINKABLE", "privacy-stale-at-reuse-limit":"PRIVACY_IDENTIFIER_STATE_STALE",
        "repeated-id-at-reuse-limit":"REPEATED_IDENTIFIER_LINKABLE", "privacy-stale-with-epoch-stale":"PRIVACY_IDENTIFIER_STATE_STALE",
        "repeated-id-with-epoch-stale":"REPEATED_IDENTIFIER_LINKABLE", "privacy-stale-with-binding-mismatch":"PRIVACY_IDENTIFIER_STATE_STALE",
        "repeated-id-with-profile-mismatch":"REPEATED_IDENTIFIER_LINKABLE",
    })
    check(resumption, "REJECT", {
        "revocation-stale":"REVOCATION_STALE", "revoked":"REVOKED", "lineage-stale":"LINEAGE_STALE",
        "restart-stale":"RESTART_CONTINUITY_STALE", "usage-counter-continuity-stale":"USAGE_COUNTER_CONTINUITY_STALE", "rollback":"ROLLBACK_SUSPECTED",
        "rollback-with-authz-stale-and-binding-mismatch":"ROLLBACK_SUSPECTED",
        "restart-stale-at-reuse-limit-and-binding-mismatch":"RESTART_CONTINUITY_STALE",
        "usage-continuity-stale-with-generation-stale-and-binding-mismatch":"USAGE_COUNTER_CONTINUITY_STALE",
        "session-invalidated-with-profile-and-binding-mismatch":"SESSION_INVALIDATED",
        "revocation-stale-with-generation-profile-and-binding-mismatch":"REVOCATION_STALE",
        "revoked-with-generation-and-authz-stale":"REVOKED",
        "lineage-stale-with-generation-and-binding-mismatch":"LINEAGE_STALE",
        "revocation-stale-at-reuse-limit":"REVOCATION_STALE", "revoked-with-authz-stale":"REVOKED",
        "lineage-stale-with-binding-mismatch":"LINEAGE_STALE",
    })

    transport = decision_rows("rust/test-vectors/state/transport-continuation-v3.txt")
    for case in ("steady", "route-changed", "connection-changed"):
        require_decision(transport, case, "CONTINUE", "CURRENT")
    check(transport, "REJECT", {"replay-stale":"REPLAY_CONTINUITY_STALE", "usage-counter-stale":"USAGE_COUNTER_CONTINUITY_STALE", "address-as-identity":"TRANSPORT_ADDRESS_AS_IDENTITY", "metadata-as-authority":"TRANSPORT_METADATA_AS_AUTHORITY"})
    check(transport, "FULL_AUTH_REQUIRED", {"authorization-generation-unbound":"AUTHORIZATION_GENERATION_UNBOUND", "authorization-generation-stale":"AUTHORIZATION_GENERATION_STALE"})

    data = decision_rows("rust/test-vectors/state/data-release-authorization-v4.txt")
    require_decision(data, "current", "RELEASE", "CURRENT")
    check(data, "DENY", {"authorization-stale":"AUTHORIZATION_STALE", "authorization-generation-unbound":"AUTHORIZATION_GENERATION_UNBOUND", "authorization-generation-stale":"AUTHORIZATION_GENERATION_STALE", "revocation-stale":"REVOCATION_STALE", "revoked":"REVOKED", "lineage-stale":"LINEAGE_STALE", "binding-invalid":"CHANNEL_BINDING_MISSING_OR_INVALID", "release-replay":"RELEASE_REPLAY_DETECTED", "rollback":"ROLLBACK_SUSPECTED"})

    delegation = decision_rows("rust/test-vectors/p2p/bounded-delegation-v3.txt")
    require_decision(delegation, "DEL3-001", "ACCEPT", "CURRENT")
    check(delegation, "DENY", {"DEL3-011":"AUTHORIZATION_GENERATION_UNBOUND", "DEL3-012":"AUTHORIZATION_GENERATION_STALE", "DEL3-014":"REVOCATION_STALE", "DEL3-016":"LINEAGE_STALE", "DEL3-020":"ROLLBACK_SUSPECTED"})

    revocation = load_revocation_reconciliation()
    for case in ("bootstrap-full", "full-advance", "diff-next"):
        row = revocation[case]
        assert row["authenticated"] == "1" and row["expected"] == "APPLY" and int(row["result_epoch"]) > int(row["local_epoch"]), case
    for case in ("unauthenticated-full", "unauthenticated-diff"):
        row = revocation[case]
        assert row["expected"] == "REJECT_UNAUTHENTICATED" and row["result_epoch"] == row["local_epoch"], case

    p2p = load_p2p()
    assert p2p["XC4-001"]["expected"] == "ESTABLISH"
    for case in ("XC4-008", "XC4-009", "XC4-010", "XC4-011", "XC4-012", "XC4-013", "XC4-017", "XC4-021"):
        assert p2p[case]["expected"] == "FAIL_CLOSED", case
    for case in ("XC4-022", "XC4-023", "XC4-024"):
        assert p2p[case]["usage_counter_continuity_current"] == "false" and p2p[case]["expected"] == "FAIL_CLOSED", case

    offline, online = p2p["XC4-002"], p2p["XC4-004"]
    compared = ("peer_a","peer_b","auth_complete","preexisting_trust","authorization_present","authorization_fresh","authorization_generation_bound","authorization_generation_current","revocation_current","revoked","lineage_current","replay_continuity_current","restart_continuity_current","usage_counter_continuity_current","mandatory_floor_compatible","binding_required","binding_valid","trust_mutation_requested","expected")
    assert all(offline[field] == online[field] for field in compared)
    assert offline["infrastructure_available"] == "false" and online["infrastructure_available"] == "true"
    assert offline["expected"] == online["expected"] == "ESTABLISH"

    print("cross-module-lifecycle-invariants: PASS surfaces=8 authz_generation=13 enrollment_compound=12 revocation=13 revocation_ingestion=5 lineage=6 replay_restart=7 usage_counter=6 privacy_resumption=8 terminal_resumption=10 transport_non_authority=2 infrastructure_non_authority=1 delegation_non_repair=7")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
