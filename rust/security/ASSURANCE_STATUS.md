# Assurance Status

| Assurance item | Current state | Required evidence |
|---|---|---|
| Reproducible build | Build scripts added | Passing local/CI logs for Rust and C |
| Unit tests | Existing tests plus plans | `cargo test` and `make test` output |
| Parser fuzzing | Harnesses added | Crash-free fuzz logs and corpus |
| Replay resistance | Test plan added | Automated tests for duplicate, reordered, stale, and retransmitted messages |
| TOFU hardening | Guidance added | Provisioning flow with pinned key, QR code, signed token, or manufacturing secret |
| Side-channel/RNG | Checklist added | Per-board review record |
| Formal model | Skeleton added | ProVerif/Tamarin verification output and model review |
| Role proof review | Brief added | External cryptographer/security reviewer feedback |

## Evidence folder recommendation
Create an `evidence/` folder locally and store build logs, fuzzing logs, ProVerif/Tamarin outputs, and reviewer notes. Do not store secrets or private keys in this folder.
