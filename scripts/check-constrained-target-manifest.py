#!/usr/bin/env python3
"""Fail-closed structural validation for TD-002 constrained-target evidence manifests."""

from __future__ import annotations

import json
import re
import sys
from pathlib import Path
from typing import Any

SCHEMA = "ZKARCHE-CONSTRAINED-TARGET/1"
MEASUREMENT_KEYS = (
    "wire_bytes",
    "static_ram_bytes",
    "peak_stack_bytes",
    "peak_heap_bytes",
    "flash_bytes",
    "latency_us",
    "cpu_cycles",
)
NONEMPTY_PATHS = (
    ("target", "family"),
    ("target", "board"),
    ("target", "board_revision"),
    ("target", "architecture"),
    ("target", "execution_environment"),
    ("implementation", "commit_sha"),
    ("implementation", "lane"),
    ("implementation", "toolchain"),
    ("implementation", "build_profile"),
    ("crypto_context", "backend"),
    ("crypto_context", "entropy_source"),
    ("crypto_context", "drbg_reseed_posture"),
    ("crypto_context", "key_generation_mode"),
    ("crypto_context", "key_storage_location"),
    ("crypto_context", "secure_boot_state"),
    ("crypto_context", "debug_state"),
    ("lifecycle_assumptions", "restart"),
    ("lifecycle_assumptions", "rollback"),
    ("lifecycle_assumptions", "clone"),
    ("lifecycle_assumptions", "reprovision"),
)


def fail(message: str) -> "NoReturn":
    raise SystemExit(f"constrained-target-manifest: FAIL: {message}")


def get_path(doc: dict[str, Any], path: tuple[str, str]) -> Any:
    section, key = path
    value = doc.get(section)
    if not isinstance(value, dict) or key not in value:
        fail(f"missing {section}.{key}")
    return value[key]


def main() -> None:
    if len(sys.argv) != 2:
        fail("usage: check-constrained-target-manifest.py MANIFEST.json")

    path = Path(sys.argv[1])
    try:
        doc = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        fail(str(exc))
    if not isinstance(doc, dict):
        fail("top-level JSON value must be an object")
    if doc.get("schema") != SCHEMA:
        fail(f"schema must be {SCHEMA}")

    status = doc.get("evidence_status")
    physical = doc.get("physical_target_executed")
    measurements = doc.get("measurements")
    if not isinstance(measurements, dict):
        fail("measurements must be an object")
    for key in MEASUREMENT_KEYS:
        if key not in measurements:
            fail(f"missing measurements.{key}")

    if status == "unmeasured":
        if physical is not False:
            fail("unmeasured manifest must set physical_target_executed=false")
        non_null = [key for key in MEASUREMENT_KEYS if measurements[key] is not None]
        if non_null:
            fail("unmeasured manifest must keep measurement values null: " + ", ".join(non_null))
        print("constrained-target-manifest: PASS status=unmeasured measurements=0")
        return

    if status != "measured":
        fail("evidence_status must be 'unmeasured' or 'measured'")
    if physical is not True:
        fail("measured manifest requires physical_target_executed=true")

    for field_path in NONEMPTY_PATHS:
        value = get_path(doc, field_path)
        if not isinstance(value, str) or not value.strip():
            fail("measured manifest requires non-empty " + ".".join(field_path))

    commit_sha = get_path(doc, ("implementation", "commit_sha"))
    if not re.fullmatch(r"[0-9a-f]{40}", commit_sha):
        fail("implementation.commit_sha must be a full lowercase 40-hex Git SHA")

    accel = get_path(doc, ("crypto_context", "accelerator_used"))
    if not isinstance(accel, bool):
        fail("crypto_context.accelerator_used must be boolean")

    for key in MEASUREMENT_KEYS:
        value = measurements[key]
        if not isinstance(value, (int, float)) or isinstance(value, bool) or value < 0:
            fail(f"measured manifest requires non-negative numeric measurements.{key}")

    if measurements["wire_bytes"] <= 0 or measurements["flash_bytes"] <= 0 or measurements["latency_us"] <= 0:
        fail("wire_bytes, flash_bytes, and latency_us must be greater than zero for measured evidence")

    print(f"constrained-target-manifest: PASS status=measured target={get_path(doc, ('target', 'board'))} commit={commit_sha}")


if __name__ == "__main__":
    main()
