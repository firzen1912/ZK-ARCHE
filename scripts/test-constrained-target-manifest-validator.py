#!/usr/bin/env python3
"""Self-test the TD-002 constrained-target manifest validator with synthetic fixtures.

These fixtures are validator tests only. They are created in a temporary directory and
MUST NOT be retained or interpreted as physical-target evidence.
"""

from __future__ import annotations

import copy
import json
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
VALIDATOR = ROOT / "scripts" / "check-constrained-target-manifest.py"
TEMPLATE = ROOT / "evidence" / "constrained-target" / "manifest-template.json"


def run(doc: dict) -> subprocess.CompletedProcess[str]:
    with tempfile.NamedTemporaryFile("w", suffix=".json", encoding="utf-8", delete=False) as handle:
        json.dump(doc, handle)
        path = Path(handle.name)
    try:
        return subprocess.run(
            [sys.executable, str(VALIDATOR), str(path)],
            cwd=ROOT,
            text=True,
            capture_output=True,
            check=False,
        )
    finally:
        path.unlink(missing_ok=True)


def expect_pass(name: str, doc: dict) -> None:
    result = run(doc)
    if result.returncode != 0:
        raise SystemExit(f"{name}: expected PASS, got rc={result.returncode}: {result.stdout}{result.stderr}")


def expect_fail(name: str, doc: dict, needle: str) -> None:
    result = run(doc)
    output = result.stdout + result.stderr
    if result.returncode == 0:
        raise SystemExit(f"{name}: expected FAIL but validator passed")
    if needle not in output:
        raise SystemExit(f"{name}: expected failure containing {needle!r}, got: {output}")


def synthetic_measured(template: dict) -> dict:
    doc = copy.deepcopy(template)
    doc["evidence_status"] = "measured"
    doc["physical_target_executed"] = True
    doc["target"].update(
        family="synthetic-test-target",
        board="synthetic-board",
        board_revision="test-only",
        architecture="synthetic-arch",
        peer_class="mcu-core",
        cpu_clock_hz=1_000_000,
        ram_bytes=65_536,
        flash_bytes_available=262_144,
        execution_environment="validator-self-test-only",
        power_mode="synthetic",
    )
    doc["implementation"].update(
        commit_sha="0" * 40,
        lane="validator-self-test",
        protocol_operation="synthetic-operation",
        profile="synthetic-profile",
        toolchain="synthetic-toolchain",
        compiler_version="0",
        compiler_flags=[],
        build_profile="synthetic",
        firmware_artifact_sha256="0" * 64,
    )
    doc["crypto_context"].update(
        backend="synthetic",
        backend_version="0",
        accelerator_used=False,
        entropy_source="synthetic-not-real",
        drbg_reseed_posture="synthetic-not-real",
        key_generation_mode="synthetic-not-real",
        key_storage_location="synthetic-not-real",
        secure_boot_state="synthetic-not-real",
        debug_state="synthetic-not-real",
    )
    doc["lifecycle_assumptions"].update(
        restart="synthetic",
        rollback="synthetic",
        clone="synthetic",
        reprovision="synthetic",
        persistent_state_backend="synthetic",
        rollback_protection_mechanism="synthetic",
    )
    doc["measurement_method"].update(
        instrumentation="synthetic-validator-fixture",
        timer_or_counter="synthetic",
        memory_method="synthetic",
        warmup_iterations=1,
        sample_count=2,
        cold_boot_each_sample=False,
        transport_context="synthetic",
        notes="Validator self-test only; not physical evidence.",
    )
    doc["measurements"].update(
        wire_bytes={"request": 10, "response": 20, "total": 30},
        static_ram_bytes=1024,
        peak_stack_bytes=2048,
        peak_heap_bytes=1024,
        flash_bytes=4096,
        latency_us={"min": 10, "median": 20, "p95": 30, "max": 40},
        cpu_cycles={"min": 10, "median": 20, "p95": 30, "max": 40},
    )
    doc["qualification"].update(
        restart_test_executed=True,
        rollback_test_executed=True,
        replay_rejection_test_executed=True,
        rng_health_or_failure_test_executed=True,
        result="PASS",
    )
    doc["provenance"].update(
        operator="synthetic-validator-self-test",
        measured_at_utc="2000-01-01T00:00:00Z",
        raw_evidence_refs=["synthetic://validator-self-test/not-physical-evidence"],
        notes="Synthetic validator fixture only. MUST NOT be retained as TD-002 evidence.",
    )
    return doc


def main() -> None:
    template = json.loads(TEMPLATE.read_text(encoding="utf-8"))
    expect_pass("unmeasured-template", template)

    dishonest = copy.deepcopy(template)
    dishonest["measurements"]["peak_stack_bytes"] = 1
    expect_fail("unmeasured-observation", dishonest, "peak_stack_bytes=null")

    dishonest = copy.deepcopy(template)
    dishonest["qualification"]["restart_test_executed"] = True
    expect_fail("unmeasured-qualification", dishonest, "restart_test_executed=false")

    dishonest = copy.deepcopy(template)
    dishonest["provenance"]["raw_evidence_refs"] = ["synthetic://not-evidence"]
    expect_fail("unmeasured-provenance", dishonest, "raw_evidence_refs empty")

    measured = synthetic_measured(template)
    expect_pass("synthetic-measured-shape", measured)

    invalid = copy.deepcopy(measured)
    invalid["measurements"]["wire_bytes"]["total"] = 31
    expect_fail("wire-accounting", invalid, "total must equal request + response")

    invalid = copy.deepcopy(measured)
    invalid["measurements"]["peak_stack_bytes"] = invalid["target"]["ram_bytes"] + 1
    expect_fail("ram-bound", invalid, "peak_stack_bytes cannot exceed target.ram_bytes")

    invalid = copy.deepcopy(measured)
    invalid["measurements"]["latency_us"] = {"min": 10, "median": 30, "p95": 20, "max": 40}
    expect_fail("range-order", invalid, "min <= median <= p95 <= max")

    invalid = copy.deepcopy(measured)
    invalid["qualification"]["rollback_test_executed"] = False
    expect_fail("pass-requires-qualification", invalid, "PASS requires all qualification flags true")

    invalid = copy.deepcopy(measured)
    invalid["implementation"]["commit_sha"] = "0" * 39
    expect_fail("commit-provenance", invalid, "full lowercase 40-hex Git SHA")

    print("constrained-target-manifest-validator-self-test: PASS cases=10 synthetic_physical_evidence=0")


if __name__ == "__main__":
    main()
