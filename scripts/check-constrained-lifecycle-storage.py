#!/usr/bin/env python3
"""Fail-closed validator for constrained target lifecycle/storage evidence."""

from __future__ import annotations

import json
import re
import sys
from pathlib import Path
from typing import Any, NoReturn

SCHEMA = "ZKARCHE-CONSTRAINED-LIFECYCLE-STORAGE/2"

OBSERVATION_KEYS = (
    "wire_bytes_auth_exchange",
    "stack_high_water_bytes",
    "heap_peak_bytes",
    "static_ram_bytes",
    "flash_text_rodata_bytes",
    "persistent_state_bytes",
    "revocation_view_bytes",
    "authorization_view_bytes",
    "auth_latency_us",
    "update_latency_us",
    "restart_recovery_us",
    "bytes_written_per_update",
)

REQUIRED_CONTEXT = (
    ("target", "family"),
    ("target", "board"),
    ("target", "board_revision"),
    ("implementation", "commit_sha"),
    ("implementation", "lane"),
    ("toolchain", "compiler"),
    ("toolchain", "compiler_version"),
    ("toolchain", "build_profile"),
    ("crypto_execution", "library"),
    ("crypto_execution", "library_version"),
    ("crypto_execution", "accelerator_path"),
    ("crypto_execution", "software_fallback"),
    ("entropy", "source"),
    ("entropy", "health_test_posture"),
    ("entropy", "drbg"),
    ("entropy", "reseed_policy"),
    ("key_storage", "representation"),
    ("key_storage", "location"),
    ("key_storage", "zeroization_posture"),
    ("boot_debug", "secure_boot_state"),
    ("boot_debug", "debug_state"),
    ("storage", "backend"),
    ("storage", "persistence_medium"),
    ("storage", "atomic_update_strategy"),
    ("storage", "monotonic_freshness_source"),
    ("storage", "rollback_detection"),
    ("storage", "power_loss_model"),
    ("transport", "kind"),
    ("transport", "mtu_bytes"),
    ("transport", "reliability"),
)


def fail(message: str) -> NoReturn:
    raise SystemExit(f"constrained-lifecycle-storage: FAIL: {message}")


def get_path(doc: dict[str, Any], path: tuple[str, str]) -> Any:
    section, key = path
    value = doc.get(section)
    if not isinstance(value, dict) or key not in value:
        fail(f"missing {section}.{key}")
    return value[key]


def require_nonempty_context(doc: dict[str, Any]) -> None:
    for field_path in REQUIRED_CONTEXT:
        value = get_path(doc, field_path)
        if field_path == ("transport", "mtu_bytes"):
            if not isinstance(value, int) or isinstance(value, bool) or value <= 0:
                fail("measured manifest requires positive integer transport.mtu_bytes")
            continue
        if not isinstance(value, str) or not value.strip():
            fail("measured manifest requires non-empty " + ".".join(field_path))


def main() -> None:
    if len(sys.argv) != 2:
        fail("usage: check-constrained-lifecycle-storage.py MANIFEST.json")

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
        for flag in (
            "restart_test_executed",
            "rollback_test_executed",
            "entropy_path_exercised",
            "key_storage_path_exercised",
        ):
            if doc.get(flag) is not False:
                fail(f"unmeasured manifest must set {flag}=false")
        non_null = [key for key in OBSERVATION_KEYS if observations[key] is not None]
        if non_null:
            fail("unmeasured manifest must keep observations null: " + ", ".join(non_null))
        print("constrained-lifecycle-storage: PASS status=unmeasured observations=0")
        return

    if status != "measured":
        fail("evidence_status must be 'unmeasured' or 'measured'")
    if physical is not True:
        fail("measured manifest requires physical_target_executed=true")
    for flag in (
        "restart_test_executed",
        "rollback_test_executed",
        "entropy_path_exercised",
        "key_storage_path_exercised",
    ):
        if doc.get(flag) is not True:
            fail(f"measured manifest requires {flag}=true")

    require_nonempty_context(doc)

    commit_sha = get_path(doc, ("implementation", "commit_sha"))
    if not re.fullmatch(r"[0-9a-f]{40}", commit_sha):
        fail("implementation.commit_sha must be a full lowercase 40-hex Git SHA")

    for key in OBSERVATION_KEYS:
        value = observations[key]
        if not isinstance(value, (int, float)) or isinstance(value, bool) or value < 0:
            fail(f"measured manifest requires non-negative numeric observations.{key}")

    for key in (
        "wire_bytes_auth_exchange",
        "stack_high_water_bytes",
        "static_ram_bytes",
        "flash_text_rodata_bytes",
        "persistent_state_bytes",
        "auth_latency_us",
        "update_latency_us",
        "restart_recovery_us",
    ):
        if observations[key] <= 0:
            fail(f"observations.{key} must be greater than zero for measured evidence")

    print(
        "constrained-lifecycle-storage: PASS "
        f"status=measured target={get_path(doc, ('target', 'board'))} "
        f"commit={commit_sha}"
    )


if __name__ == "__main__":
    main()
