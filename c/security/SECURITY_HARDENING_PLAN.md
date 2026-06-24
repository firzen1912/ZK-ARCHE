# Security Hardening Plan

## Phase 1: Reproducible builds
- Pin toolchains.
- Use lock files.
- Run all compiler warnings as errors where practical.
- Add AddressSanitizer/UBSan for C.
- Store clean build logs.

## Phase 2: Parser hardening
- Fuzz header parsing, TLV parsing, setup payloads, auth payloads, and ACK/error payloads.
- Add corpus seeds from valid test vectors.
- Add negative tests for truncated, oversized, duplicated, non-canonical, and out-of-order packets.

## Phase 3: Replay and state-machine tests
- Implement replay-cache tests from `REPLAY_TEST_PLAN.md`.
- Test stale sessions, duplicate nonces, duplicate ephemeral keys, duplicate sequence numbers, and retransmission semantics.

## Phase 4: Provisioning hardening
- Treat TOFU as lab-only.
- Prefer pinned server public key, signed onboarding token, QR-code provisioning, or manufacturing-time provisioning.
- Fail closed if the provisioning trust anchor is absent in production mode.

## Phase 5: Platform assurance
- Complete `SIDE_CHANNEL_RNG_CHECKLIST.md` for Raspberry Pi, ESP32, STM32/FreeRTOS, Zephyr, or any selected target.
- Confirm RNG, zeroization, flash protection, debug lockout, and secure boot posture.

## Phase 6: Formal and external review
- Complete a symbolic ProVerif/Tamarin model for authentication and key secrecy.
- Write a separate cryptographic proof for Schnorr and role-membership proof components.
- Send `EXTERNAL_REVIEW_BRIEF.md` and relevant source files to an external reviewer.
