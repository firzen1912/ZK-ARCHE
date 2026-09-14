#!/usr/bin/env python3
from __future__ import annotations

import copy
import json
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
VALIDATOR = ROOT / "scripts/check-constrained-lifecycle-storage.py"
TEMPLATE = ROOT / "evidence/constrained-target/lifecycle-storage-template.json"

EXECUTION_FLAGS = (
    "restart_test_executed",
    "rollback_test_executed",
    "entropy_path_exercised",
    "key_storage_path_exercised",
    "revocation_reconciliation_test_executed",
    "revocation_restart_test_executed",
    "revocation_rollback_test_executed",
    "revocation_power_loss_test_executed",
    "authorization_generation_test_executed",
    "authorization_generation_restart_test_executed",
    "authorization_generation_rollback_test_executed",
    "authorization_generation_power_loss_test_executed",
    "enrollment_replay_test_executed",
    "enrollment_restart_test_executed",
    "enrollment_rollback_test_executed",
    "enrollment_power_loss_test_executed",
)

CONTEXT_SECTIONS = (
    "target",
    "implementation",
    "toolchain",
    "crypto_execution",
    "entropy",
    "rng_adapter",
    "key_storage",
    "boot_debug",
    "storage",
    "transport",
)


def run(doc: dict) -> subprocess.CompletedProcess[str]:
    with tempfile.NamedTemporaryFile("w", suffix=".json", encoding="utf-8", delete=False) as f:
        json.dump(doc, f)
        path = Path(f.name)
    try:
        return subprocess.run(
            [sys.executable, str(VALIDATOR), str(path)],
            text=True,
            capture_output=True,
        )
    finally:
        path.unlink(missing_ok=True)


def execution_only_claim() -> dict:
    doc = copy.deepcopy(base)
    doc["evidence_status"] = "measured"
    doc["physical_target_executed"] = True
    for flag in EXECUTION_FLAGS:
        doc[flag] = True
    return doc


def measured_claim() -> dict:
    """Return a synthetic, internally valid measured fixture; it is never retained as evidence."""
    doc = execution_only_claim()
    for section in CONTEXT_SECTIONS:
        for key, value in doc[section].items():
            if value == "":
                doc[section][key] = f"self-test-{section}-{key}"
    doc["implementation"]["commit_sha"] = "0" * 40
    doc["rng_adapter"]["max_request_bytes"] = 1
    doc["transport"]["mtu_bytes"] = 1
    for observation in doc["observations"]:
        doc["observations"][observation] = 1
    return doc


base = json.loads(TEMPLATE.read_text(encoding="utf-8"))
r = run(base)
if r.returncode != 0 or "PASS status=unmeasured" not in r.stdout:
    raise SystemExit("constrained target validator self-test: template did not pass honestly")

for observation in (
    "auth_latency_us",
    "enrollment_replay_state_bytes",
    "authorization_generation_state_bytes",
):
    fake = copy.deepcopy(base)
    fake["observations"][observation] = 1
    r = run(fake)
    if r.returncode == 0:
        raise SystemExit(
            "constrained target validator self-test: "
            f"fabricated unmeasured observation {observation} was accepted"
        )

fake = execution_only_claim()
r = run(fake)
if r.returncode == 0 or "target.family" not in r.stderr:
    raise SystemExit(
        "constrained target validator self-test: context-free measured claim was accepted"
    )

r = run(measured_claim())
if r.returncode != 0 or "PASS status=measured" not in r.stdout:
    raise SystemExit(
        "constrained target validator self-test: synthetic complete measured fixture did not pass"
    )

for flag in EXECUTION_FLAGS:
    fake = measured_claim()
    fake[flag] = False
    r = run(fake)
    if r.returncode == 0 or flag not in r.stderr:
        raise SystemExit(
            "constrained target validator self-test: "
            f"measured claim did not require {flag}"
        )

fake = measured_claim()
fake["implementation"]["commit_sha"] = "deadbeef"
r = run(fake)
if r.returncode == 0 or "implementation.commit_sha" not in r.stderr:
    raise SystemExit(
        "constrained target validator self-test: abbreviated commit provenance was accepted"
    )

for section, key in (
    ("rng_adapter", "max_request_bytes"),
    ("transport", "mtu_bytes"),
):
    fake = measured_claim()
    fake[section][key] = 0
    r = run(fake)
    if r.returncode == 0 or f"{section}.{key}" not in r.stderr:
        raise SystemExit(
            "constrained target validator self-test: "
            f"non-positive measured context {section}.{key} was accepted"
        )

for observation, value in (
    ("heap_peak_bytes", -1),
    ("auth_latency_us", 0),
):
    fake = measured_claim()
    fake["observations"][observation] = value
    r = run(fake)
    if r.returncode == 0 or f"observations.{observation}" not in r.stderr:
        raise SystemExit(
            "constrained target validator self-test: "
            f"invalid measured observation {observation}={value} was accepted"
        )

print(
    "constrained-target-manifest-self-test: PASS "
    f"negative_cases={3 + 1 + len(EXECUTION_FLAGS) + 1 + 2 + 2}"
)
