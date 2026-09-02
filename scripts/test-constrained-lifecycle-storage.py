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


base = json.loads(TEMPLATE.read_text(encoding="utf-8"))
r = run(base)
if r.returncode != 0 or "PASS status=unmeasured" not in r.stdout:
    raise SystemExit("constrained target validator self-test: template did not pass honestly")

fake = copy.deepcopy(base)
fake["observations"]["auth_latency_us"] = 1
r = run(fake)
if r.returncode == 0:
    raise SystemExit(
        "constrained target validator self-test: fabricated unmeasured observation was accepted"
    )

fake = copy.deepcopy(base)
fake["observations"]["enrollment_replay_state_bytes"] = 1
r = run(fake)
if r.returncode == 0:
    raise SystemExit(
        "constrained target validator self-test: fabricated enrollment replay observation was accepted"
    )

fake = copy.deepcopy(base)
fake["observations"]["authorization_generation_state_bytes"] = 1
r = run(fake)
if r.returncode == 0:
    raise SystemExit(
        "constrained target validator self-test: fabricated authorization-generation observation was accepted"
    )

fake = copy.deepcopy(base)
fake["evidence_status"] = "measured"
fake["physical_target_executed"] = True
fake["restart_test_executed"] = True
fake["rollback_test_executed"] = True
fake["entropy_path_exercised"] = True
fake["key_storage_path_exercised"] = True
fake["authorization_generation_test_executed"] = True
fake["authorization_generation_power_loss_test_executed"] = True
fake["enrollment_replay_test_executed"] = True
fake["enrollment_power_loss_test_executed"] = True
r = run(fake)
if r.returncode == 0:
    raise SystemExit(
        "constrained target validator self-test: context-free measured claim was accepted"
    )

fake = copy.deepcopy(base)
fake["evidence_status"] = "measured"
fake["physical_target_executed"] = True
fake["restart_test_executed"] = True
fake["rollback_test_executed"] = True
fake["entropy_path_exercised"] = True
fake["key_storage_path_exercised"] = True
fake["authorization_generation_test_executed"] = False
fake["authorization_generation_power_loss_test_executed"] = True
fake["enrollment_replay_test_executed"] = True
fake["enrollment_power_loss_test_executed"] = True
r = run(fake)
if r.returncode == 0:
    raise SystemExit(
        "constrained target validator self-test: measured claim without authorization-generation test was accepted"
    )

fake = copy.deepcopy(base)
fake["evidence_status"] = "measured"
fake["physical_target_executed"] = True
fake["restart_test_executed"] = True
fake["rollback_test_executed"] = True
fake["entropy_path_exercised"] = True
fake["key_storage_path_exercised"] = True
fake["authorization_generation_test_executed"] = True
fake["authorization_generation_power_loss_test_executed"] = False
fake["enrollment_replay_test_executed"] = True
fake["enrollment_power_loss_test_executed"] = True
r = run(fake)
if r.returncode == 0:
    raise SystemExit(
        "constrained target validator self-test: measured claim without authorization-generation power-loss test was accepted"
    )

fake = copy.deepcopy(base)
fake["evidence_status"] = "measured"
fake["physical_target_executed"] = True
fake["restart_test_executed"] = True
fake["rollback_test_executed"] = True
fake["entropy_path_exercised"] = True
fake["key_storage_path_exercised"] = True
fake["authorization_generation_test_executed"] = True
fake["authorization_generation_power_loss_test_executed"] = True
fake["enrollment_replay_test_executed"] = False
fake["enrollment_power_loss_test_executed"] = True
r = run(fake)
if r.returncode == 0:
    raise SystemExit(
        "constrained target validator self-test: measured claim without enrollment replay test was accepted"
    )

print("constrained-target-manifest-self-test: PASS negative_cases=7")
