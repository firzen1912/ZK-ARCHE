#!/usr/bin/env python3
"""Fail-closed validator for constrained DATA audit-chain persistence evidence."""

from __future__ import annotations

import json
import re
import sys
from pathlib import Path
from typing import Any, NoReturn

SCHEMA = "ZKARCHE-CONSTRAINED-DATA-AUDIT-STORAGE/1"

OBSERVATION_KEYS = (
    "audit_state_bytes",
    "audit_append_scratch_bytes",
    "audit_bytes_written_per_append",
    "audit_append_latency_us",
    "audit_restart_recovery_us",
)

REQUIRED_CONTEXT = (
    ("target", "family"),
    ("target", "board"),
    ("implementation", "commit_sha"),
    ("implementation", "lane"),
    ("storage", "backend"),
    ("storage", "persistence_medium"),
    ("storage", "audit_record_format"),
    ("storage", "audit_atomicity"),
    ("storage", "audit_restart_policy"),
    ("storage", "audit_rollback_anchor"),
    ("storage", "audit_rollback_policy"),
    ("storage", "audit_power_loss_model"),
)

EXECUTION_FLAGS = (
    "audit_append_test_executed",
    "audit_restart_test_executed",
    "audit_rollback_test_executed",
    "audit_power_loss_test_executed",
)


def fail(message: str) -> NoReturn:
    raise SystemExit(f"constrained-data-audit-storage: FAIL: {message}")


def get_path(doc: dict[str, Any], path: tuple[str, str]) -> Any:
    section, key = path
    value = doc.get(section)
    if not isinstance(value, dict) or key not in value:
        fail(f"missing {section}.{key}")
    return value[key]


def main() -> None:
    if len(sys.argv) != 2:
        fail("usage: check-constrained-data-audit-storage.py MANIFEST.json")

    path = Path(sys.argv[1])
    try:
        doc = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        fail(str(exc))
    if not isinstance(doc, dict):
        fail("top-level JSON value must be an object")
    if doc.get("schema") != SCHEMA:
        fail(f"schema must be {SCHEMA}")

    observations = doc.get("observations")
    if not isinstance(observations, dict):
        fail("observations must be an object")
    for key in OBSERVATION_KEYS:
        if key not in observations:
            fail(f"missing observations.{key}")

    status = doc.get("evidence_status")
    physical = doc.get("physical_target_executed")

    if status == "unmeasured":
        if physical is not False:
            fail("unmeasured manifest must set physical_target_executed=false")
        for flag in EXECUTION_FLAGS:
            if doc.get(flag) is not False:
                fail(f"unmeasured manifest must set {flag}=false")
        non_null = [key for key in OBSERVATION_KEYS if observations[key] is not None]
        if non_null:
            fail("unmeasured manifest must keep observations null: " + ", ".join(non_null))
        print("constrained-data-audit-storage: PASS status=unmeasured observations=0")
        return

    if status != "measured":
        fail("evidence_status must be 'unmeasured' or 'measured'")
    if physical is not True:
        fail("measured manifest requires physical_target_executed=true")
    for flag in EXECUTION_FLAGS:
        if doc.get(flag) is not True:
            fail(f"measured manifest requires {flag}=true")

    for field_path in REQUIRED_CONTEXT:
        value = get_path(doc, field_path)
        if not isinstance(value, str) or not value.strip():
            fail("measured manifest requires non-empty " + ".".join(field_path))

    commit_sha = get_path(doc, ("implementation", "commit_sha"))
    if not re.fullmatch(r"[0-9a-f]{40}", commit_sha):
        fail("implementation.commit_sha must be a full lowercase 40-hex Git SHA")

    for key in OBSERVATION_KEYS:
        value = observations[key]
        if not isinstance(value, (int, float)) or isinstance(value, bool) or value <= 0:
            fail(f"measured manifest requires positive numeric observations.{key}")

    print(
        "constrained-data-audit-storage: PASS "
        f"status=measured target={get_path(doc, ('target', 'board'))} "
        f"commit={commit_sha}"
    )


if __name__ == "__main__":
    main()
