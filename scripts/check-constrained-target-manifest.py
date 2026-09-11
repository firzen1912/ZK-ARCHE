#!/usr/bin/env python3
"""Fail-closed structural validation for TD-002 constrained-target evidence manifests."""

from __future__ import annotations

from datetime import datetime
import json
import re
import sys
from pathlib import Path
from typing import Any, NoReturn

SCHEMA = "ZKARCHE-CONSTRAINED-TARGET/2"
SCALAR_MEASUREMENTS = (
    "static_ram_bytes",
    "peak_stack_bytes",
    "peak_heap_bytes",
    "flash_bytes",
)
RANGE_MEASUREMENTS = ("latency_us", "cpu_cycles")
RANGE_KEYS = ("min", "median", "p95", "max")
WIRE_KEYS = ("request", "response", "total")
QUALIFICATION_FLAGS = (
    "restart_test_executed",
    "rollback_test_executed",
    "replay_rejection_test_executed",
    "rng_health_or_failure_test_executed",
)
NONEMPTY_PATHS = (
    ("target", "family"),
    ("target", "board"),
    ("target", "board_revision"),
    ("target", "architecture"),
    ("target", "execution_environment"),
    ("target", "power_mode"),
    ("implementation", "commit_sha"),
    ("implementation", "lane"),
    ("implementation", "protocol_operation"),
    ("implementation", "profile"),
    ("implementation", "toolchain"),
    ("implementation", "compiler_version"),
    ("implementation", "build_profile"),
    ("implementation", "firmware_artifact_sha256"),
    ("crypto_context", "backend"),
    ("crypto_context", "backend_version"),
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
    ("lifecycle_assumptions", "persistent_state_backend"),
    ("lifecycle_assumptions", "rollback_protection_mechanism"),
    ("measurement_method", "instrumentation"),
    ("measurement_method", "timer_or_counter"),
    ("measurement_method", "memory_method"),
    ("measurement_method", "transport_context"),
)


def fail(message: str) -> NoReturn:
    raise SystemExit(f"constrained-target-manifest: FAIL: {message}")


def section(doc: dict[str, Any], name: str) -> dict[str, Any]:
    value = doc.get(name)
    if not isinstance(value, dict):
        fail(f"{name} must be an object")
    return value


def get_path(doc: dict[str, Any], path: tuple[str, str]) -> Any:
    group, key = path
    value = section(doc, group)
    if key not in value:
        fail(f"missing {group}.{key}")
    return value[key]


def numeric(value: Any, label: str, *, positive: bool = False, integral: bool = False) -> None:
    if integral:
        if not isinstance(value, int) or isinstance(value, bool) or value < 0:
            fail(f"{label} must be a non-negative integer")
    elif not isinstance(value, (int, float)) or isinstance(value, bool) or value < 0:
        fail(f"{label} must be a non-negative number")
    if positive and value <= 0:
        fail(f"{label} must be greater than zero")


def require_utc_timestamp(value: Any, label: str) -> None:
    if not isinstance(value, str) or not re.fullmatch(r"\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}(?:\.\d+)?Z", value):
        fail(f"{label} must be a UTC timestamp in YYYY-MM-DDTHH:MM:SS[.fraction]Z form")
    try:
        datetime.fromisoformat(value[:-1] + "+00:00")
    except ValueError:
        fail(f"{label} must contain a valid UTC calendar date and time")


def require_null_measurements(measurements: dict[str, Any]) -> None:
    wire = measurements.get("wire_bytes")
    if not isinstance(wire, dict) or any(wire.get(key) is not None for key in WIRE_KEYS):
        fail("unmeasured manifest must keep all measurements.wire_bytes values null")
    for key in SCALAR_MEASUREMENTS:
        if measurements.get(key) is not None:
            fail(f"unmeasured manifest must keep measurements.{key}=null")
    for key in RANGE_MEASUREMENTS:
        value = measurements.get(key)
        if not isinstance(value, dict) or any(value.get(item) is not None for item in RANGE_KEYS):
            fail(f"unmeasured manifest must keep all measurements.{key} values null")


def validate_unmeasured(doc: dict[str, Any], measurements: dict[str, Any]) -> None:
    if doc.get("physical_target_executed") is not False:
        fail("unmeasured manifest must set physical_target_executed=false")
    require_null_measurements(measurements)
    method = section(doc, "measurement_method")
    for key in ("warmup_iterations", "sample_count"):
        value = method.get(key)
        if value != 0 or isinstance(value, bool):
            fail(f"unmeasured manifest must keep measurement_method.{key}=0")
    if method.get("cold_boot_each_sample") is not False:
        fail("unmeasured manifest must keep measurement_method.cold_boot_each_sample=false")
    qualification = section(doc, "qualification")
    for key in QUALIFICATION_FLAGS:
        if qualification.get(key) is not False:
            fail(f"unmeasured manifest must keep qualification.{key}=false")
    if qualification.get("result") != "UNMEASURED":
        fail("unmeasured qualification.result must be UNMEASURED")
    provenance = section(doc, "provenance")
    if provenance.get("measured_at_utc") not in {"", None}:
        fail("unmeasured manifest must not claim provenance.measured_at_utc")
    refs = provenance.get("raw_evidence_refs")
    if refs != []:
        fail("unmeasured manifest must keep provenance.raw_evidence_refs empty")
    print("constrained-target-manifest: PASS status=unmeasured measurements=0 qualification=0 provenance=0")


def validate_measured(doc: dict[str, Any], measurements: dict[str, Any]) -> None:
    if doc.get("physical_target_executed") is not True:
        fail("measured manifest requires physical_target_executed=true")
    for field_path in NONEMPTY_PATHS:
        value = get_path(doc, field_path)
        if not isinstance(value, str) or not value.strip():
            fail("measured manifest requires non-empty " + ".".join(field_path))
    target = section(doc, "target")
    for key in ("cpu_clock_hz", "ram_bytes", "flash_bytes_available"):
        numeric(target.get(key), f"target.{key}", positive=True, integral=True)
    commit_sha = get_path(doc, ("implementation", "commit_sha"))
    if not re.fullmatch(r"[0-9a-f]{40}", commit_sha):
        fail("implementation.commit_sha must be a full lowercase 40-hex Git SHA")
    artifact_sha = get_path(doc, ("implementation", "firmware_artifact_sha256"))
    if not re.fullmatch(r"[0-9a-f]{64}", artifact_sha):
        fail("implementation.firmware_artifact_sha256 must be lowercase 64-hex")
    implementation = section(doc, "implementation")
    compiler_flags = implementation.get("compiler_flags")
    if not isinstance(compiler_flags, list) or not all(isinstance(item, str) for item in compiler_flags):
        fail("implementation.compiler_flags must be an array of strings")
    accel = get_path(doc, ("crypto_context", "accelerator_used"))
    if not isinstance(accel, bool):
        fail("crypto_context.accelerator_used must be boolean")
    method = section(doc, "measurement_method")
    for key in ("warmup_iterations", "sample_count"):
        value = method.get(key)
        if not isinstance(value, int) or isinstance(value, bool) or value < 0:
            fail(f"measurement_method.{key} must be a non-negative integer")
    if method["sample_count"] <= 0:
        fail("measurement_method.sample_count must be greater than zero")
    if not isinstance(method.get("cold_boot_each_sample"), bool):
        fail("measurement_method.cold_boot_each_sample must be boolean")
    wire = measurements.get("wire_bytes")
    if not isinstance(wire, dict):
        fail("measurements.wire_bytes must be an object")
    for key in WIRE_KEYS:
        numeric(wire.get(key), f"measurements.wire_bytes.{key}", integral=True)
    numeric(wire["total"], "measurements.wire_bytes.total", positive=True, integral=True)
    if wire["request"] + wire["response"] != wire["total"]:
        fail("measurements.wire_bytes.total must equal request + response")
    for key in SCALAR_MEASUREMENTS:
        numeric(measurements.get(key), f"measurements.{key}", positive=(key == "flash_bytes"), integral=True)
    for key in ("static_ram_bytes", "peak_stack_bytes", "peak_heap_bytes"):
        if measurements[key] > target["ram_bytes"]:
            fail(f"measurements.{key} cannot exceed target.ram_bytes")
    if measurements["flash_bytes"] > target["flash_bytes_available"]:
        fail("measurements.flash_bytes cannot exceed target.flash_bytes_available")
    for key in RANGE_MEASUREMENTS:
        value = measurements.get(key)
        if not isinstance(value, dict):
            fail(f"measurements.{key} must be an object")
        for item in RANGE_KEYS:
            numeric(value.get(item), f"measurements.{key}.{item}", integral=(key == "cpu_cycles"))
        if not (value["min"] <= value["median"] <= value["p95"] <= value["max"]):
            fail(f"measurements.{key} must satisfy min <= median <= p95 <= max")
    if measurements["latency_us"]["median"] <= 0:
        fail("measurements.latency_us.median must be greater than zero")
    qualification = section(doc, "qualification")
    for key in QUALIFICATION_FLAGS:
        if not isinstance(qualification.get(key), bool):
            fail(f"qualification.{key} must be boolean")
    if qualification.get("result") not in {"PASS", "FAIL"}:
        fail("measured qualification.result must be PASS or FAIL")
    if qualification["result"] == "PASS":
        failed_flags = [key for key in QUALIFICATION_FLAGS if not qualification[key]]
        if failed_flags:
            fail("measured PASS requires all qualification flags true: " + ", ".join(failed_flags))
    provenance = section(doc, "provenance")
    operator = provenance.get("operator")
    if not isinstance(operator, str) or not operator.strip():
        fail("measured manifest requires non-empty provenance.operator")
    require_utc_timestamp(provenance.get("measured_at_utc"), "provenance.measured_at_utc")
    refs = provenance.get("raw_evidence_refs")
    if not isinstance(refs, list) or not refs or not all(isinstance(item, str) and item.strip() for item in refs):
        fail("measured manifest requires at least one non-empty provenance.raw_evidence_refs entry")
    print(f"constrained-target-manifest: PASS status=measured target={get_path(doc, ('target', 'board'))} commit={commit_sha}")


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
    measurements = section(doc, "measurements")
    status = doc.get("evidence_status")
    if status == "unmeasured":
        validate_unmeasured(doc, measurements)
        return
    if status != "measured":
        fail("evidence_status must be 'unmeasured' or 'measured'")
    validate_measured(doc, measurements)


if __name__ == "__main__":
    main()
