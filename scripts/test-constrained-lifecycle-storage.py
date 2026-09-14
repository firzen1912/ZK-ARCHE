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


def measured_claim() -> dict:
    doc = copy.deepcopy(base)
    doc["evidence_status"] = "measured"
    doc["physical_target_executed"] = True
    for flag in EXECUTION_FLAGS:
        doc[flag] = True
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

fake = measured_claim()
r = run(fake)
if r.returncode == 0 or "target.family" not in r.stderr:
    raise SystemExit(
        "constrained target validator self-test: context-free measured claim was accepted"
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

print(
    "constrained-target-manifest-self-test: PASS "
    f"negative_cases={3 + 1 + len(EXECUTION_FLAGS)}"
)
