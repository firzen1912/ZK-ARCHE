# Assurance and Validation

This document is the unified home for the security, hardening, local validation, replay, review, and deterministic-vector guidance that used to be duplicated under the C and Rust lanes.

## Hardening Summary

The repository contains a practical assurance layer for both implementation lanes:

- Parent-level and lane-level CI scripts.
- Security hardening plan.
- Threat model.
- Security goals and claim boundaries.
- Assurance status matrix.
- Side-channel and RNG checklist.
- Replay-cache test plan.
- ProVerif skeleton models.
- External review brief for the role-membership proof.
- Parser fuzzing harnesses.
- Deterministic test-vector DRBG specification.

This material improves engineering assurance, but it does not by itself make the protocol production-certified, formally verified, side-channel resistant, externally reviewed, or field-ready.

## Local Validation Runbook

Use Ubuntu 22.04/24.04, Debian, Kali, or Raspberry Pi OS for the standard local validation path.

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

Run both lanes from the repository root:

```bash
./scripts/ci-all.sh
```

Run only Rust:

```bash
./scripts/ci-rust.sh
```

Run only C:

```bash
./scripts/ci-c.sh
```

Run the C vector harness against the Rust vector corpus:

```bash
cd c
make
./build/tests/test_vectors ../rust/test-vectors/0x0001
```

Rust fuzzing:

```bash
cd rust
cargo fuzz run wire_parse -- -max_total_time=300
cargo fuzz run auth_payloads -- -max_total_time=300
```

C fuzzing:

```bash
cd c
make fuzz-wire
make fuzz-payloads
```

Formal model starting points:

```bash
proverif rust/models/proverif/zk_arche_auth_skeleton.pv
proverif c/models/proverif/zk_arche_auth_skeleton.pv
```

The symbolic models validate abstract authentication and secrecy properties only. The role-membership proof still requires separate cryptographic proof and external review.

## Evidence Policy

CI and fuzz scripts write lane-local logs under `rust/evidence/` and `c/evidence/`; the parent wrapper writes `evidence/ci-all.log`. Generated evidence logs are ignored by default unless a release or review process intentionally checks in a curated artifact.

Collect these artifacts for review packages:

- Rust CI log.
- C CI log.
- Cross-language vector test output.
- Fuzzing logs and crash triage notes.
- Replay test results.
- RNG and side-channel checklist per target platform.
- ProVerif or Tamarin output.
- External role-proof review notes.

Do not store secrets, private keys, pairing tokens, or live fleet credentials in evidence folders.

## Assurance Status

| Assurance item | Current state | Required evidence |
|---|---|---|
| Reproducible build | Build scripts added | Passing local or CI logs for Rust and C |
| Unit tests | Existing tests plus plans | `cargo test` and `make test` output |
| Cross-language vectors | Rust vectors and C harness present | Passing C vector harness against `rust/test-vectors/0x0001/` |
| Parser fuzzing | Harnesses added | Crash-free fuzz logs and corpus notes |
| Replay resistance | Test plan defined | Automated tests for duplicate, reordered, stale, and retransmitted messages |
| TOFU hardening | Guidance defined | Provisioning flow with pinned key, QR code, signed token, or manufacturing secret |
| Side-channel/RNG | Checklist defined | Per-board review record |
| Formal model | Skeleton models added | ProVerif/Tamarin output and model review |
| Role proof review | Brief defined | External cryptographer or security reviewer feedback |

## Threat Model

This threat model covers the ZK-ARCHE setup and authentication protocols for IoT devices, gateways, and servers. It applies to both Rust and C implementations.

Assets:

- Device long-term secret scalar.
- Device public identity and pseudonymous authentication identifier.
- Server long-term secret scalar and pinned server public key.
- Session keys derived after authentication.
- Role credential and role commitment/blinding value.
- Replay cache state and sequence/session identifiers.
- Pairing token or onboarding material, if used.

Trust assumptions:

- Ristretto255 discrete logarithm is infeasible for the adversary.
- SHA-256, HMAC, and HKDF are secure for their intended uses.
- Each platform RNG provides cryptographically strong entropy.
- Long-term private keys are protected against direct extraction.
- The verifier uses the expected registered public key and role policy for the device.

Adversary model:

The network adversary may eavesdrop, replay, drop, delay, reorder, modify, and inject packets. The adversary may run fake clients and fake servers, capture old transcripts, attempt malformed packet attacks, and try downgrade, unknown-key-share, and man-in-the-middle attacks.

Out of scope unless explicitly evaluated:

- Physical key extraction from flash or RAM.
- Power or EM side-channel attacks.
- Fault injection against scalar arithmetic or RNG.
- Compromise of the server database.
- Malicious firmware installed before protocol execution.

Security boundaries:

- Protocol logic is portable.
- Transport, storage, and RNG are platform-specific trust boundaries.
- TOFU onboarding is a lower-assurance mode and must not be treated as equivalent to authenticated provisioning.

## Security Goals and Claim Boundaries

Required goals:

1. Device authentication: only a device knowing the registered private scalar can authenticate for the corresponding public key.
2. Server authentication: the client authenticates the server against a pinned or provisioned server public key.
3. Key secrecy: established session keys are unknown to passive and active network attackers.
4. Key confirmation: both peers confirm they derived the same session keys over the same transcript.
5. Replay resistance: old `AUTH_1`, `AUTH_2`, and `AUTH_3` messages are rejected or handled idempotently as retransmissions.
6. Transcript binding: challenges, nonces, ephemeral keys, roles, and identities are bound into Fiat-Shamir, HKDF, and HMAC contexts with domain separation.
7. Role authorization: the server accepts only if the role proof satisfies an allowed role set.
8. Role privacy: the role proof should reveal membership in an allowed set without unnecessarily revealing the exact role.

Current assurance claim:

The implementation should be described as a research prototype with structured transcripts and standard primitives. It is not yet formally verified, side-channel evaluated, externally audited, production certified, or field-ready.

Claims that must not yet be made:

- Do not claim formal proof unless the symbolic model and cryptographic proof are completed.
- Do not claim production readiness without fuzzing, replay testing, side-channel review, and external cryptographic review.
- Do not claim secure onboarding when TOFU is enabled.

## Security Hardening Plan

Phase 1: reproducible builds.

- Pin toolchains where practical.
- Use lock files.
- Run compiler warnings as errors where practical.
- Add AddressSanitizer and UBSan for C.
- Store clean build logs.

Phase 2: parser hardening.

- Fuzz header parsing, TLV parsing, setup payloads, auth payloads, and ACK/error payloads.
- Add corpus seeds from valid test vectors.
- Add negative tests for truncated, oversized, duplicated, non-canonical, and out-of-order packets.

Phase 3: replay and state-machine tests.

- Implement replay-cache tests from this document.
- Test stale sessions, duplicate nonces, duplicate ephemeral keys, duplicate sequence numbers, and retransmission semantics.

Phase 4: provisioning hardening.

- Treat TOFU as lab-only.
- Prefer pinned server public key, signed onboarding token, QR-code provisioning, or manufacturing-time provisioning.
- Fail closed if the provisioning trust anchor is absent in production mode.

Phase 5: platform assurance.

- Complete the side-channel and RNG checklist for Raspberry Pi, ESP32, STM32/FreeRTOS, Zephyr, or any selected target.
- Confirm RNG, zeroization, flash protection, debug lockout, and secure boot posture.

Phase 6: formal and external review.

- Complete a symbolic ProVerif/Tamarin model for authentication and key secrecy.
- Write a separate cryptographic proof for Schnorr and role-membership proof components.
- Send the external review brief and relevant source files to an external reviewer.

## Replay-Cache Test Plan

Required negative tests:

1. Reuse the same `AUTH_1` packet with identical `pid || nonce_c || eph_c`; the verifier must reject or treat it as an idempotent retransmission only if the session state explicitly allows it.
2. Replay `AUTH_1` after a completed session; the verifier must reject it.
3. Replay `AUTH_2` to a different client session; the client must reject it because transcript/HMAC verification fails.
4. Replay `AUTH_3` after `AUTH_ACK`; the server must reject or idempotently return cached ACK only for the same session/sequence.
5. Change `seq` while keeping old payload; transcript or state verification must reject.
6. Reorder `AUTH_2` before `AUTH_1`; the receiver must reject due to missing state.
7. Reuse `nonce_c` with a new `eph_c`; policy should reject or log as suspicious.
8. Reuse `eph_c` with a new `nonce_c`; policy should reject or log as suspicious.

Required positive tests:

1. A valid first authentication succeeds.
2. A retransmission marked with the retransmit flag for the same `(session_id, seq)` receives the same cached response if retransmission support is implemented.
3. Fresh authentication with fresh nonce and ephemeral key succeeds.

Evidence to collect:

- Test name.
- Expected result.
- Actual result.
- Packet fields replayed or modified.
- Whether the receiver returned an error, ignored the packet, or returned a cached response.

## Side-Channel and RNG Checklist

Use this checklist for each deployment target.

RNG:

- Identify the RNG source used by the platform.
- Verify it is cryptographically secure and seeded before use.
- Run a boot-time sanity check that fails closed if RNG is unavailable.
- Never reuse Schnorr nonces or ephemeral Diffie-Hellman secrets.

Secret handling:

- Zeroize private scalars, nonce material, blinding values, and session keys after use.
- Avoid logging secrets, scalars, nonces, MAC keys, or derived session keys.
- Disable core dumps on production Linux targets.
- Protect flash/NVS storage using hardware-backed protection where available.

Timing and power:

- Use constant-time crypto libraries for scalar arithmetic and MAC verification.
- Avoid branching on secret data in protocol glue code.
- On MCU targets, evaluate power/EM leakage if physical attackers are in scope.

Debug interfaces:

- Disable JTAG, SWD, and UART debug access for production devices.
- Disable verbose protocol tracing in production firmware.
- Use secure boot and signed firmware where possible.

Platform notes:

- Raspberry Pi/Linux gateway: use OS RNG, file permissions, no core dumps, and service sandboxing.
- ESP32: use esp_random/mbedTLS or libsodium-compatible RNG, NVS encryption, flash encryption, and secure boot.
- STM32/FreeRTOS: use a hardware RNG peripheral, flash readout protection, explicit zeroization, and network-stack fuzzing.
- Zephyr: use `sys_csrand_get`, settings/NVS storage, stack canaries, and MPU where available.

## External Review Brief

Review objective:

Evaluate whether the role-membership proof is complete, sound, zero-knowledge, transcript-bound, replay-resistant, and safe under the selected role encoding.

Materials to provide reviewers:

- Protocol specification and wire format.
- Source files implementing role encoding, role commitment, rerandomization, and set-membership proof verification.
- Threat model and security goals.
- Test vectors for valid and invalid role proofs.

Reviewer questions:

1. Is the commitment construction binding and hiding under the stated assumptions?
2. Is the OR-proof challenge construction sound and domain separated?
3. Can a prover mix branches from different role sets or transcripts?
4. Does branch ordering leak role information?
5. Are invalid points, non-canonical scalars, or identity elements rejected consistently?
6. Does the proof remain zero-knowledge against a malicious verifier?
7. Is replay prevented by transcript, nonce, and session binding?
8. Does the implementation avoid accidentally revealing the real branch through timing or error behavior?

Expected output:

A short review memo that classifies findings as critical, high, medium, low, or informational, with recommended fixes.

## Test-Vector DRBG Specification

All probabilistic fields in the suite `0x0001` vectors are derived from a single deterministic bit generator so conforming implementations can reproduce them byte-for-byte.

Seed:

```text
DRBG_SEED = b"iot-auth/test-vectors/v1        "  (exactly 32 bytes)
```

Generator:

ChaCha20 with the seed as the 256-bit key, a 96-bit all-zero nonce, and block counter starting at 0. Output is the keystream. This matches `rand_chacha::ChaCha20Rng::from_seed` RFC-compatible mode.

Scalar sampling:

```text
rand_scalar(rng):
    bytes = rng.next_bytes(64)
    return Scalar::from_bytes_mod_order_wide(bytes)
```

The 64 uniform bytes are reduced modulo the ristretto255 scalar group order using the standard wide reduction used by libsodium, ed25519-donna, and curve25519-dalek.

Consumption order:

- `schnorr_auth_client.json`: 64 bytes for `r`.
- `rerandomization.json`: 64 bytes for `r`.
- `role_set_membership.json`: for each branch in `0..n`, consume 64 bytes (`w`) for the true index, otherwise consume 128 bytes (`c_i`, `s_i`).
- `kdf_kc.json`: bytes `0..64` for client Schnorr `r`; bytes `64..128` for server Schnorr `r`.

A reproducing implementation that uses the same seed, the same generator, and consumes bytes in the same order will obtain the same `(a, s)` for every proof published in this corpus.
