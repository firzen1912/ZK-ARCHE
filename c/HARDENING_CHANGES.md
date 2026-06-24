# Hardening Changes for ZK-ARCHE C

This bundle adds a practical assurance layer for the C implementation:

- Local validation runbook.
- Reproducible build/CI script.
- Security hardening plan.
- Threat model.
- Security goals.
- Assurance status matrix.
- Side-channel and RNG checklist.
- Replay-cache test plan.
- ProVerif skeleton model.
- External review brief for the role-membership proof.
- Parser fuzzing harnesses.

The added material improves engineering assurance but does not by itself make the protocol production-certified. Keep build logs, fuzz logs, formal-model outputs, and review notes under `evidence/`.
