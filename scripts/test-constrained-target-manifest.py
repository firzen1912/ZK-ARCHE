#!/usr/bin/env python3
"""Negative/self-test coverage for the TD-002 constrained-target manifest gate.

All measured values in this file are synthetic validator fixtures only. They are
created in a temporary directory and MUST NOT be interpreted or retained as
physical-target evidence.
"""

from __future__ import annotations

import copy
import json
from pathlib import Path
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
CHECKER = ROOT / "scripts" / "check-constrained-target-manifest.py"
TEMPLATE = ROOT / "evidence" / "constrained-target" / "manifest-template.json"


def fail(message: str) -> None:
    raise SystemExit(f"constrained-target-manifest-self-test: FAIL: {message}")


def run_case(name: str, doc: dict, *, expect_ok: bool, needle: str | None = None) -> None:
    with tempfile.TemporaryDirectory(prefix="zkarche-target-manifest-") as tmp:
        path = Path(tmp) / f"{name}.json"
        path.write_text(json.dumps(doc, indent=2) + "\n", encoding="utf-8")
        result = subprocess.run(
            [sys.executable, str(CHECKER), str(path)],
            cwd=ROOT,
            text=True,
            capture_output=True,
            check=False,
        )
    output = result.stdout + result.stderr
    if expect_ok and result.returncode != 0:
        fail(f"{name} unexpectedly rejected: {output.strip()}")
    if not expect_ok and result.returncode == 0:
        fail(f"{name} unexpectedly accepted")
    if needle is not None and needle not in output:
        fail(f"{name} did not report expected diagnostic {needle!r}: {output.strip()}")


def measured_fixture(template: dict) -> dict:
    doc = copy.deepcopy(template)
    doc["evidence_status"] = "measured"
    doc["physical_target_executed"] = True
    doc["target"].update(
        {
            "family": "synthetic-self-test",
            "board": "synthetic-self-test",
            "board_revision": "synthetic-self-test",
            "architecture": "synthetic-self-test",
            "peer_class": "mcu-core",
            "cpu_clock_hz": 1,
            "ram_bytes": 4096,
            "flash_bytes_available": 8192,
            "execution_environment": "synthetic-self-test",
            "power_mode": "synthetic-self-test",
        }
    )
    doc["implementation"].update(
        {
            "commit_sha": "0" * 40,
            "lane": "synthetic-self-test",
            "protocol_operation": "AUTH",
            "profile": "iot-core",
            "toolchain": "synthetic-self-test",
            "compiler_version": "synthetic-self-test",
            "compiler_flags": [],
            "build_profile": "synthetic-self-test",
            "firmware_artifact_sha256": "0" * 64,
        }
    )
    doc["crypto_context"].update(
        {
            "backend": "synthetic-self-test",
            "backend_version": "synthetic-self-test",
            "accelerator_used": False,
            "entropy_source": "synthetic-self-test",
            "drbg_reseed_posture": "synthetic-self-test",
            "key_generation_mode": "synthetic-self-test",
            "key_storage_location": "synthetic-self-test",
            "secure_boot_state": "synthetic-self-test",
            "debug_state": "synthetic-self-test",
        }
    )
    doc["lifecycle_assumptions"].update(
        {
            "restart": "synthetic-self-test",
            "rollback": "synthetic-self-test",
            "clone": "synthetic-self-test",
            "reprovision": "synthetic-self-test",
            "persistent_state_backend": "synthetic-self-test",
            "rollback_protection_mechanism": "synthetic-self-test",
        }
    )
    doc["measurement_method"].update(
        {
            "instrumentation": "synthetic-self-test",
            "timer_or_counter": "synthetic-self-test",
            "memory_method": "synthetic-self-test",
            "warmup_iterations": 1,
            "sample_count": 1,
            "cold_boot_each_sample": False,
            "transport_context": "synthetic-self-test",
            "notes": "Ephemeral validator fixture; not physical evidence.",
        }
    )
    doc["measurements"].update(
        {
            "wire_bytes": {"request": 1, "response": 1, "total": 2},
            "static_ram_bytes": 1,
            "peak_stack_bytes": 1,
            "peak_heap_bytes": 1,
            "flash_bytes": 1,
            "latency_us": {"min": 1, "median": 1, "p95": 1, "max": 1},
            "cpu_cycles": {"min": 1, "median": 1, "p95": 1, "max": 1},
        }
    )
    doc["qualification"].update(
        {
            "restart_test_executed": True,
            "rollback_test_executed": True,
            "replay_rejection_test_executed": True,
            "rng_health_or_failure_test_executed": True,
            "result": "PASS",
        }
    )
    doc["provenance"].update(
        {
            "operator": "synthetic-self-test",
            "measured_at_utc": "2000-01-01T00:00:00Z",
            "raw_evidence_refs": ["synthetic://not-retained"],
            "notes": "Ephemeral validator fixture; not physical evidence.",
        }
    )
    return doc


def main() -> None:
    template = json.loads(TEMPLATE.read_text(encoding="utf-8"))

    run_case("unmeasured-template", template, expect_ok=True, needle="status=unmeasured")

    valid = measured_fixture(template)
    run_case("synthetic-measured", valid, expect_ok=True, needle="peer_class=mcu-core")

    unknown_class = copy.deepcopy(valid)
    unknown_class["target"]["peer_class"] = "mcu-weak"
    run_case(
        "unknown-peer-class",
        unknown_class,
        expect_ok=False,
        needle="target.peer_class must be one of",
    )

    missing_class = copy.deepcopy(valid)
    del missing_class["target"]["peer_class"]
    run_case(
        "missing-peer-class",
        missing_class,
        expect_ok=False,
        needle="missing target.peer_class",
    )

    unmeasured_unknown_class = copy.deepcopy(template)
    unmeasured_unknown_class["target"]["peer_class"] = "mcu-weak"
    run_case(
        "unmeasured-unknown-peer-class",
        unmeasured_unknown_class,
        expect_ok=False,
        needle="unmeasured target.peer_class",
    )

    incomplete_pass = copy.deepcopy(valid)
    incomplete_pass["qualification"]["rollback_test_executed"] = False
    run_case(
        "pass-without-rollback-test",
        incomplete_pass,
        expect_ok=False,
        needle="measured PASS requires all qualification flags true",
    )

    wire_mismatch = copy.deepcopy(valid)
    wire_mismatch["measurements"]["wire_bytes"]["total"] = 3
    run_case(
        "wire-total-mismatch",
        wire_mismatch,
        expect_ok=False,
        needle="wire_bytes.total must equal request + response",
    )

    ram_overrun = copy.deepcopy(valid)
    ram_overrun["measurements"]["peak_stack_bytes"] = valid["target"]["ram_bytes"] + 1
    run_case(
        "ram-overrun",
        ram_overrun,
        expect_ok=False,
        needle="peak_stack_bytes cannot exceed target.ram_bytes",
    )

    invalid_commit = copy.deepcopy(valid)
    invalid_commit["implementation"]["commit_sha"] = "deadbeef"
    run_case(
        "invalid-commit-sha",
        invalid_commit,
        expect_ok=False,
        needle="commit_sha must be a full lowercase 40-hex Git SHA",
    )

    zero_cpu_clock = copy.deepcopy(valid)
    zero_cpu_clock["target"]["cpu_clock_hz"] = 0
    run_case(
        "zero-cpu-clock",
        zero_cpu_clock,
        expect_ok=False,
        needle="target.cpu_clock_hz must be greater than zero",
    )

    flash_overrun = copy.deepcopy(valid)
    flash_overrun["measurements"]["flash_bytes"] = valid["target"]["flash_bytes_available"] + 1
    run_case(
        "flash-overrun",
        flash_overrun,
        expect_ok=False,
        needle="measurements.flash_bytes cannot exceed target.flash_bytes_available",
    )

    invalid_timestamp = copy.deepcopy(valid)
    invalid_timestamp["provenance"]["measured_at_utc"] = "2000-02-31T00:00:00Z"
    run_case(
        "invalid-measurement-timestamp",
        invalid_timestamp,
        expect_ok=False,
        needle="must contain a valid UTC calendar date and time",
    )

    unordered_latency = copy.deepcopy(valid)
    unordered_latency["measurements"]["latency_us"] = {
        "min": 1,
        "median": 4,
        "p95": 3,
        "max": 5,
    }
    run_case(
        "unordered-latency-range",
        unordered_latency,
        expect_ok=False,
        needle="measurements.latency_us must satisfy min <= median <= p95 <= max",
    )

    print("constrained-target-manifest-self-test: PASS cases=13 physical_evidence_claimed=0")


if __name__ == "__main__":
    main()
