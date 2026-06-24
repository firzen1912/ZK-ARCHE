# Local Validation Runbook

This runbook gives the exact order to validate both the Rust and C implementations.

## 0. Recommended environment
Use Ubuntu 22.04/24.04, Debian, Kali, or Raspberry Pi OS. Run from the project root.

```bash
sudo apt-get update
sudo apt-get install -y build-essential pkg-config libsodium-dev clang llvm lld make git curl unzip
```

Install Rust:

```bash
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
. "$HOME/.cargo/env"
rustup toolchain install stable
rustup component add rustfmt clippy
cargo install cargo-fuzz
```

## 1. Validate the Rust implementation

```bash
cd ZK-ARCHE-Rust
./scripts/ci-rust.sh | tee ../evidence-rust-ci.log
```

Fuzzing:

```bash
cd ZK-ARCHE-Rust
cargo fuzz run wire_parse -- -max_total_time=300
cargo fuzz run auth_payloads -- -max_total_time=300
```

Expected result: no panic, no crash, no sanitizer finding.

## 2. Validate the C implementation

```bash
cd ZK-ARCHE-C
./scripts/ci-c.sh | tee ../evidence-c-ci.log
```

Fuzzing:

```bash
cd ZK-ARCHE-C
make fuzz-wire
make fuzz-payloads
```

Expected result: no crash, no ASan finding, no UBSan finding.

## 3. Replay tests
Use `security/REPLAY_TEST_PLAN.md` to implement automated tests. The minimum acceptable evidence is one passing test for each negative and positive replay case.

## 4. TOFU hardening decision
For research/lab mode, document TOFU as allowed. For production mode, disable TOFU and require one of:

- pinned server public key,
- signed onboarding token,
- QR-code provisioning,
- manufacturer-installed trust anchor,
- secure element provisioning.

## 5. Formal model
Install ProVerif or Tamarin separately. Start with:

```bash
proverif models/proverif/zk_arche_auth_skeleton.pv
```

The symbolic model validates abstract authentication and secrecy properties only. The role-membership proof still requires a separate cryptographic proof.

## 6. Evidence checklist
Store these logs with your project report:

- Rust CI log.
- C CI log.
- Fuzzing logs.
- Replay test results.
- RNG/side-channel checklist per IoT platform.
- ProVerif/Tamarin output.
- External review notes for role proof.
