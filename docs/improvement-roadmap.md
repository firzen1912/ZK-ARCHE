# ZK-ARCHE Unified Rust/C/Python Improvement Roadmap

This roadmap describes an evidence-gated improvement path for the unified ZK-ARCHE Rust, C, and Python repository. It is designed for ordinary maintenance work, automated-agent assistance, and checkpoint-style review when security-sensitive implementation changes are proposed.

The roadmap is evidence-gated, not calendar-gated. It does not claim production readiness, formal verification, side-channel resistance, replay-resistance completeness, IoT field readiness, external cryptographic review, or certification unless the required evidence exists as checked-in artifacts.

## Current Baseline

The repository contains three implementation lanes:

| Lane | Path | Role |
|---|---|---|
| Rust reference | `rust/` | Cargo workspace, protocol library, binaries, canonical deterministic test vectors, fuzz targets, ProVerif skeleton, security docs |
| C implementation | `c/` | libsodium-based C11 implementation, public headers, unit/e2e/vector tests, fuzz harnesses, ProVerif skeleton, security docs |
| Python reference | `python/` | readable reference implementation, CLI harness, mirrored vector fixtures, state-machine tests, protocol experimentation lane |

Rust test vectors under `rust/test-vectors/0x0001/` are the primary byte-level interop anchor. The C harness consumes them directly; the Python lane currently carries mirrored fixtures under `python/test-vectors/0x0001/` and must preserve the same semantics.

## Non-Negotiable Boundaries

- Do not change cryptographic primitives, transcript domain separators, packet formats, suite identifiers, test-vector meanings, replay semantics, or wire compatibility without explicit checkpoint-style review and evidence.
- Do not claim production-ready cryptographic security, side-channel resistance, memory-safety completeness, formal verification, replay-resistance completeness, external review completion, IoT field readiness, or certification without checked-in evidence.
- Rust, C, and Python must remain byte-compatible at the vector, transcript, wire-header, TLV, proof, KDF, MAC, and protocol-state-machine boundaries where each lane implements the feature. Python is a reference/interoperability lane, not an `iot-core` constrained-device target.
- Normal `AUTH` must remain proof of prior enrollment. Unknown-device self-registration inside `AUTH` is not allowed unless a future checkpoint explicitly proves equivalent authorization, replay, privacy, and abuse resistance.
- `iot-core` must remain ZK-minimal: no general-purpose zkSNARK/STARK/Plonk/Groth16 prover, large anonymous-credential prover, post-quantum hybrid, large Merkle tree, certificate-chain parser, or heap-heavy policy engine may become mandatory for MCU-class devices.
- Protected telemetry/data-sovereignty features must be encrypted-by-default. Any plaintext protected-data path must be explicitly marked public/test-only and must fail CI for sovereignty profiles.
- Per-device data sovereignty claims require checked-in evidence for device-controlled encryption, policy-bound release, auditability, revocation behavior, replay resistance, and target profile footprint.
- The C implementation must preserve strict warnings and sanitizer-clean validation as release-gate evidence.
- Rust code must preserve `cargo fmt`, `cargo check`, `cargo test`, and `cargo clippy -D warnings` evidence as release-gate evidence.
- Fuzz targets and formal models are evidence producers, not proof of complete security by themselves.
- Automated agents may improve clarity, tests, scripts, and validation, but must not weaken assurance-status truthfulness.



## IoT Capability and Assurance Contract

This roadmap must stay implementable across heterogeneous IoT targets rather than drifting toward gateway-only assumptions.

Target classes:

| Class | Representative devices | Expected role |
|---|---|---|
| MCU-core | STM32-class bare-metal/RTOS targets | constrained client, minimal storage, no heap-heavy protocol requirements |
| MCU-plus | ESP32-S3-class RTOS targets | constrained client, optional commissioner, small registry only if explicitly benchmarked |
| Linux-edge | Raspberry Pi-class Linux targets | client, server, commissioner, test harness, local gateway |
| Accelerated-edge | Jetson Orin-class Linux targets | gateway/server, large-registry benchmarks, fuzz/formal/review artifact generation |

Profile policy:

- `iot-core` features must fit a classical, low-memory client/server profile using fixed-size buffers where practical, bounded parsing, deterministic error handling, and no mandatory dynamic allocation in C hot paths.
- `iot-edge` features may require Linux-class storage, larger registries, and more benchmarking infrastructure, but must not become mandatory for `iot-core` authentication.
- Heavy cryptographic upgrades, privacy-credential experiments, and post-quantum experiments must be suite-negotiated and optional until target-specific byte, RAM, CPU, and interop evidence exists.
- Any feature that cannot fit `iot-core` must provide a safe fallback or be explicitly marked `iot-edge-only`; it cannot be a hidden dependency of baseline `AUTH`.
- UDP profiles should preserve the current small-datagram design target. Any feature that exceeds the configured datagram budget must require TCP, fragmentation, or an explicit gateway-only profile.
- Hardware-specific acceleration is allowed only as an optimization. Correctness and security semantics must not depend on vendor-specific acceleration.

Minimum metrics every new protocol feature must report before moving beyond design:

| Metric | Required evidence |
|---|---|
| Wire cost | measured bytes for each new message and total exchange, with `R=1`, `R=4`, and `R=8` role branches where relevant |
| RAM profile | peak stack/heap estimate or measurement for C/Rust implementations on constrained builds |
| CPU profile | benchmark against the current baseline on at least one constrained-class and one Linux-edge-class target, or an explicit evidence gap |
| Registry scaling | lookup cost for 1k, 10k, and 100k records where server-side lookup is affected |
| Replay behavior | duplicate, reordered, stale, cross-session, and wrong-sequence negative tests |
| Transcript binding | mutation tests proving each negotiated security field is covered by key confirmation or an equivalent binder |
| Interop | Rust/C byte-level vectors before compatibility is claimed; Python follows the same vector suite where available |
| Failure behavior | malformed input, oversized input, storage exhaustion, RNG failure, clock skew, and restart behavior are explicit tests or documented gaps |

IoT-readiness claim rule:

```text
No roadmap item may claim IoT field readiness merely because it builds on a workstation.
An IoT-readiness claim requires checked-in target profile notes, byte/RAM/CPU measurements,
negative security tests, and implementation gap disclosure.
```

## Review Policy

| Work type | Required review posture |
|---|---|
| Documentation cleanup, roadmap alignment, validation script wrappers | Lightweight review is acceptable |
| CI repair without semantic protocol changes | Normal review followed by final verification |
| Replay tests, wire parsing, transcript generation, proof verification, KDF/MAC behavior, RNG/DRBG handling | Checkpoint-style review |
| C memory-safety changes, unsafe Rust changes, sanitizer findings, fuzz-crash fixes | Checkpoint-style review |
| Cross-language compatibility changes | Checkpoint-style review |
| Release-candidate gate review | Independent final verification |

## Phase Map

| Range | Status | Purpose |
|---|---|---|
| zk201 | Planned | Combined repository baseline, graph build, validation inventory |
| zk202 | Planned | Parent-level CI wrappers and evidence log normalization |
| zk203 | Planned | Replay-test automation and negative-case coverage |
| zk204 | Planned | Fuzzing automation, corpus layout, crash triage workflow |
| zk205 | Planned | Rust/C cross-language vector parity and interop validation |
| zk206 | Planned | TOFU/provisioning hardening design and evidence checklist |
| zk207 | Planned | Formal-model expansion and traceability to implementation assertions |
| zk208 | Planned | Side-channel and RNG evidence checklist enforcement |
| zk209 | Planned | External review package and reproducibility bundle |
| zk210 | Planned | Release-candidate evidence gate, without self-awarded production claims |
| zk211 | Planned | Late enrollment design using signed one-time `EnrollmentGrant` objects |
| zk212 | Planned | Delegated commissioner enrollment after successful `AUTH` |
| zk213 | Planned | Authenticated rekey and re-registration for existing devices |
| zk214 | Planned | Enrollment replay, abuse, and authorization evidence |
| zk215 | Planned | Privacy-preserving credential upgrade research for role authorization |
| zk216 | Planned | IoT profile matrix, byte/RAM/CPU benchmark harness, and assurance gate normalization |
| zk217 | Planned | AUTH transcript v3 with complete security-context binding and mutation tests |
| zk218 | Planned | Strict AUTH state-machine, sequence, session, and transport-binding validation |
| zk219 | Planned | Stateless `AUTH_RETRY` cookies and unauthenticated-work throttling |
| zk220 | Planned | Optional encrypted lookup hints for `O(1)` server registry lookup without cleartext identity |
| zk221 | Planned | Replay-safe session resumption for constrained devices, with no unsafe 0-RTT writes |
| zk222 | Planned | AUTH metrics CI covering wire cost, CPU/RAM, registry scaling, and negative security tests |
| zk223 | Planned | Reviewed anonymous-credential migration evaluation as optional, not `iot-core` mandatory |
| zk224 | Planned | Optional post-quantum hybrid suite research with explicit MTU/RAM/profile limits |
| zk225 | Planned | Python lane integration, parent-level CI, and Rust-vector semantic alignment |
| zk226 | Planned | RFC-style specification package skeleton and normative registry discipline |
| zk227 | Planned | EDHOC/CoAP/OSCORE-inspired constrained profile research without mandatory CBOR/COSE dependency |
| zk228 | Planned | TLS/mTLS-style channel-binding profile using exporter-bound transcripts where TLS already exists |
| zk229 | Planned | DTLS-style datagram profile for retry cookies, anti-amplification, replay windows, and lossy-link retransmission |
| zk230 | Planned | Protocol-suite decomposition into CORE, AUTH, BIND, ENROLL, and DATA documents |
| zk231 | Planned | Per-device data sovereignty architecture with device-local root secrets and encrypted-by-default telemetry |
| zk232 | Planned | ZK-minimal proof-carrying data profile for small devices |
| zk233 | Planned | `ZK-ARCHE-DATA` minimal data commit, release request, release proof, wrapped-key, and audit flow |
| zk234 | Planned | Policy-bound release tokens, recipient/purpose binding, and revocable epochs |
| zk235 | Planned | Local audit hash-chain and gateway transparency-log bridge |
| zk236 | Planned | Sovereignty CI gates for plaintext prevention, replay rejection, vector parity, fuzzing, and footprint budgets |
| zk237 | Planned | Channel-bound sovereignty mode for EDHOC/OSCORE, TLS/mTLS, and DTLS deployments |
| zk238 | Planned | Advanced anonymous-credential and zkSNARK sovereignty research kept outside `iot-core` |

## zk201 — Baseline

Goal: make the unified repository navigable and establish baseline evidence without modifying protocol semantics.

Required artifacts:

| Artifact | Path |
|---|---|
| Combined repo README | `README.md` |
| Combined roadmap | `docs/improvement-roadmap.md` |
| Rust lane preserved | `rust/` |
| C lane preserved | `c/` |
| Parent validation scripts | `scripts/` |

Acceptance gates:

- Both implementation lanes are present and independently buildable.
- The Rust test-vector path is documented as the C interop anchor.
- No production/security-certification claims are introduced.

Suggested task:

```text
Inspect the unified ZK-ARCHE Rust/C/Python workspace. Identify validation commands, security docs, test-vector anchors, fuzz targets, and formal-model files. Do not change protocol semantics.
```

## zk202 — Parent CI and Evidence Normalization

Goal: make local validation repeatable from the parent repository.

Required artifacts:

| Artifact | Path |
|---|---|
| Rust wrapper | `scripts/ci-rust.sh` |
| C wrapper | `scripts/ci-c.sh` |
| Combined wrapper | `scripts/ci-all.sh` |
| Rust evidence log | `rust/evidence/rust-ci.log` |
| C evidence log | `c/evidence/c-ci.log` |
| Parent summary log | `evidence/ci-all.log` |

Acceptance gates:

- Rust wrapper delegates to `rust/scripts/ci-rust.sh`.
- C wrapper delegates to `c/scripts/ci-c.sh`.
- Combined wrapper records versions and timestamps.
- Failures are visible and not converted into false passes.
- No protocol semantics are changed.

Suggested task:

```text
Improve parent-level CI wrappers and evidence logging only. Do not modify cryptographic code, packet formats, transcript construction, or test-vector meanings.
```

## zk203 — Replay-Test Automation

Goal: convert the existing replay-test plans into executable or clearly tracked evidence.

Required coverage themes:

- duplicate message replay;
- reordered message replay;
- stale session replay;
- retransmitted packet handling;
- response cache/idempotency behavior;
- cross-transport replay behavior for UDP and TCP;
- C/Rust parity for accepted and rejected replay cases.

Acceptance gates:

- Replay tests are automated where practical.
- Manual gaps are recorded as explicit TODO evidence, not silent passes.
- Rust and C behavior is compared where vectors or harnesses allow comparison.
- Any semantic change requires checkpoint-style review and final verification.

Suggested task:

```text
Implement automated replay evidence for duplicate, reordered, stale, and retransmitted protocol messages across Rust and C where practical. Preserve wire compatibility and document any unimplemented manual evidence gaps.
```

## zk204 — Fuzzing Automation and Crash Triage

Goal: make fuzz targets easy to run and triage.

Required artifacts:

| Artifact | Path |
|---|---|
| Rust fuzz target inventory | `rust/fuzz/` |
| C fuzz target inventory | `c/fuzz/` |
| Fuzz runbook | `docs/fuzzing-runbook.md` |
| Crash triage template | `docs/fuzz-crash-triage.md` |

Acceptance gates:

- Fuzz commands are documented per lane.
- Corpus/crash output directories are ignored or separated from source.
- Crashes become reproducible tests before being claimed fixed.
- Fuzzing is not described as complete verification.

## zk205 — Cross-Language Vector and Interop Parity

Goal: prove byte-level compatibility between Rust and C using deterministic vectors and runnable interop checks.

Required evidence:

- C vector harness against `rust/test-vectors/0x0001/`;
- Rust vector regeneration command and diff procedure;
- transcript byte parity;
- PID parity;
- Schnorr/auth proof parity;
- role membership/rerandomization/KDF/MAC parity;
- wire-header and TLV parity;
- transport-level interop evidence where practical.

Acceptance gates:

- Any vector change is reviewed as a protocol-impacting change.
- Both implementations agree on generated/parsed bytes.
- Interop failures block release-candidate status.

## zk206 — TOFU and Provisioning Hardening

Goal: make enrollment/provisioning trust assumptions explicit and testable.

Required artifacts:

- TOFU threat analysis;
- provisioning state-machine assumptions;
- key-pinning evidence plan;
- rollback/re-enrollment behavior;
- operator-visible warnings for insecure bootstrap modes.

Acceptance gates:

- TOFU is not presented as equivalent to pre-provisioned trust.
- Insecure bootstrap modes are explicit and auditable.
- Any change to enrollment trust semantics requires checkpoint-style review.

## zk207 — Formal Model Expansion

Goal: expand formal models without overstating proof coverage.

Required artifacts:

- ProVerif model inventory;
- model-to-code traceability map;
- assumptions file;
- commands to reproduce model runs;
- known limitations section.

Acceptance gates:

- Formal model claims are scoped to modeled properties only.
- Unmodeled implementation behavior remains marked as unproven.
- Proof artifacts do not replace implementation tests.

## zk208 — Side-Channel and RNG Evidence

Goal: enforce truthful side-channel and RNG evidence tracking.

Required artifacts:

- RNG source inventory;
- deterministic test-only DRBG boundary;
- constant-time comparison inventory;
- secret-handling checklist;
- sanitizer and valgrind-style evidence where available;
- limitations and external-review requirements.

Acceptance gates:

- Test-only deterministic randomness cannot enter production paths.
- Side-channel resistance is not claimed without evidence.
- C memory handling and Rust secret handling are reviewed in checkpoint-style review.

## zk209 — External Review Package

Goal: prepare a reproducible package for external cryptographic/security review.

Required artifacts:

- architecture summary;
- threat model;
- security goals;
- wire spec;
- test-vector bundle;
- validation logs;
- fuzzing status;
- formal-model status;
- open questions and known limitations.

Acceptance gates:

- Package is reproducible from a clean checkout.
- Known gaps are visible.
- External review is not marked complete until review records exist.

## zk210 — Release-Candidate Evidence Gate

Goal: produce a release-candidate readiness decision without self-awarding unsupported claims.

Minimum gates:

- Rust CI evidence is current.
- C CI evidence is current.
- Cross-language vector parity evidence is current.
- Replay tests are automated or manual gaps are explicitly documented.
- Fuzzing status is current and crash triage is clean or explicitly blocked.
- Formal model status is current and scoped.
- Side-channel/RNG checklist is current.
- External-review status is truthful.

Blocked claims unless externally supported:

- production-ready cryptographic security;
- complete replay resistance;
- complete side-channel resistance;
- formal verification of the full implementation;
- IoT field readiness;
- certification;
- external review completion.


## zk211 — Late Enrollment with Signed Enrollment Grants

Goal: allow devices to be registered after the original setup/provisioning window without allowing unauthenticated self-registration through normal `AUTH`.

Design principle:

```text
Normal authentication proves prior enrollment.
Late registration remains a separate, explicitly authorized enrollment act.
```

Required design artifacts:

| Artifact | Suggested path |
|---|---|
| Late-enrollment design note | `docs/late-enrollment-design.md` |
| Enrollment grant wire/TLV extension | `docs/enrollment-grant-wire.md` |
| Grant threat model addendum | `security/ENROLLMENT_GRANT_THREAT_MODEL.md` |
| Grant replay-test plan | `security/ENROLLMENT_GRANT_REPLAY_TEST_PLAN.md` |
| Rust/C/Python compatibility vectors | `rust/test-vectors/0x0001/enrollment_grant.json` |

Proposed `EnrollmentGrant` fields:

```text
EnrollmentGrant {
  version
  suite_id
  server_pub_hash
  device_pub_hash        optional for operator-mediated pairing; recommended when known
  role_policy
  role_commitment_policy optional
  not_before
  expires_at
  max_uses               default 1
  nonce
  issuer_id
  issuer_signature
}
```

Candidate behavior:

- Extend `SETUP_1` so the client may present either the existing lab/demo pairing token or a structured `EnrollmentGrant`.
- The server writes a registry record only after grant validation and device proof-of-possession both succeed.
- Grant validation must bind the grant to the expected suite, server/domain, role policy, validity window, issuer, and replay state.
- The normal `AUTH_1` path must not auto-register an unknown device.

Acceptance gates:

- Expired, not-yet-valid, wrong-server, wrong-suite, wrong-role, malformed, unsigned, and replayed grants are rejected.
- The device proves knowledge of the private key corresponding to the enrolled device public key.
- A used-grant store exists and is covered by negative tests.
- Rust, C, and Python agree on serialized grant bytes, transcript binding, and rejection cases before compatibility is claimed.
- Legacy pairing-token behavior remains clearly marked as lab/demo unless separately hardened.

Suggested task:

```text
Design and implement a signed one-time EnrollmentGrant path for SETUP. Preserve normal AUTH as prior-enrollment proof only. Add negative tests for expired, wrong-server, wrong-role, malformed, unsigned, and replayed grants across Rust, C, and Python where available.
```

## zk212 — Delegated Commissioner Enrollment

Goal: allow an already-authenticated device or operator-controlled commissioner to authorize registration of a new device without sharing secrets or directly editing the registry.

Candidate flow:

```text
existing device -> AUTH -> server
existing device -> ENROLL_GRANT_REQ(new_device_pub_hash, requested_role, validity_window)
server -> ENROLL_GRANT_RESP(signed one-time EnrollmentGrant)
new device -> SETUP using EnrollmentGrant
```

Required design decisions:

- Define which roles may request grants, for example `admin`, `commissioner`, or another explicit role bit.
- Bind grant issuance to the authenticated session transcript and audit log.
- Decide whether the grant may be targeted to a known `device_pub_hash` or may be operator-mediated with a short pairing window.
- Define rate limits, per-issuer quotas, and revocation behavior for unused grants.
- Define an operator-visible audit record for grant issuance, grant use, and grant rejection.

Acceptance gates:

- A non-commissioner authenticated device cannot mint enrollment grants.
- A commissioner cannot request a grant outside its permitted role policy.
- A grant issued in one server/domain cannot be used in another.
- Grant issuance is auditable without leaking unnecessary device-linkage metadata.
- Existing device secrets are never transferred to the new device.

Suggested task:

```text
Add a delegated commissioner enrollment design and, if approved, implement ENROLL_GRANT_REQ/RESP as an authenticated post-AUTH operation. Enforce role authorization, server/domain binding, audit records, and single-use grant replay protection.
```

## zk213 — Authenticated Rekey and Re-Registration

Goal: allow an already-enrolled device to rotate or replace its device key without performing a full operator-assisted setup and without creating an ambiguous duplicate identity.

Candidate flow:

```text
device -> AUTH
device -> REKEY_1(new_device_pub, new_role_commitment, old_to_new_binding_proof)
server -> REKEY_2(acceptance, registry_update_commitment)
```

Required proof obligations:

- The device must prove control of the existing credential through the authenticated session.
- The device must prove knowledge of the new device private key.
- The rekey transcript must be bound to the current authenticated session key and server identity.
- The server must atomically tombstone or revoke the old registry entry and activate the new entry.

Registry behavior:

```text
old_device_id -> revoked or tombstoned
new_device_id -> DeviceRecord(new_pub, new_role_commitment, enrollment_epoch, rekey_parent)
```

Acceptance gates:

- Replay of an old rekey request does not roll the device backward.
- Concurrent rekey attempts have deterministic, test-covered behavior.
- Failed rekey attempts do not leave partial registry state.
- Rekey does not silently expand role privileges.
- Rust, C, and Python either implement the same semantics or explicitly mark implementation gaps.

Suggested task:

```text
Design authenticated REKEY_1/REKEY_2 messages that rotate a device key after AUTH. Bind the rekey transcript to the session key, prove possession of the new key, atomically tombstone the old registry entry, and add rollback/replay/concurrency negative tests.
```

## zk214 — Enrollment Abuse, Replay, and Operational Controls

Goal: prevent late enrollment from becoming a denial-of-service, privilege-escalation, or privacy-regression path.

Required coverage themes:

- grant replay and duplicate use;
- grant stuffing and registry-spam resistance;
- rate limiting by issuer, role, source, and server policy;
- storage exhaustion behavior;
- revocation of unused grants;
- downgrade behavior from `EnrollmentGrant` to lab pairing token;
- audit-log privacy and retention;
- server lookup scalability after many late-enrolled devices.

Acceptance gates:

- Replayed grants and duplicated setup attempts are rejected consistently.
- The server can reject or throttle grant validation before expensive cryptographic or registry work when safe.
- Operator-visible logs distinguish grant issuance, successful enrollment, rejection, and revocation.
- Privacy claims are not expanded unless unlinkability and metadata-retention evidence exists.
- Large-registry behavior remains measured because `AUTH_1` lookup is intentionally `O(n)` while PID hides identity.

Suggested task:

```text
Build late-enrollment abuse and replay tests covering grant duplication, registry spam, invalid issuer floods, downgrade attempts, and large-registry lookup behavior. Add operational controls without changing AUTH privacy semantics.
```

## zk215 — Privacy-Preserving Credential Upgrade Research

Goal: evaluate whether role authorization should migrate from the current custom role-membership proof toward a reviewed privacy-preserving credential system.

Research candidates:

- BBS-style unlinkable credentials for selective disclosure and role authorization;
- Privacy Pass-style issuance/redemption separation for unlinkable authorization tokens;
- Direct Anonymous Attestation-style device authorization for hardware-backed deployments;
- certificate enrollment only for non-anonymous operational modes.

Acceptance gates before any migration:

- A concrete threat model explains what privacy or assurance property improves over the current proof.
- Wire compatibility impact is isolated behind a new suite identifier or negotiated capability.
- Deterministic test vectors exist before implementation compatibility is claimed.
- External cryptographic review is required before production claims.
- The existing custom proof remains marked security-sensitive until reviewed or replaced.

Suggested task:

```text
Research privacy-preserving credential options for future role authorization. Produce a design comparison covering BBS-style credentials, Privacy Pass-style tokens, and DAA-style approaches, with suite negotiation and vector requirements before any protocol migration.
```


## zk216 — IoT Profile Matrix and Benchmark Harness

Goal: make every future protocol improvement measurable on constrained and heterogeneous IoT targets before it is treated as baseline behavior.

Target outcome:

```text
iot-core: constrained classical AUTH profile for STM32/ESP32-S3-class targets
iot-edge: Linux gateway/server profile for Raspberry Pi/Jetson Orin-class targets
research-only: optional suites that are not baseline until measured and reviewed
```

Required artifacts:

| Artifact | Suggested path |
|---|---|
| IoT profile matrix | `docs/iot-profile-matrix.md` |
| AUTH byte-cost report | `evidence/auth-wire-cost.md` |
| Constrained benchmark harness | `bench/auth_iot_profile/` |
| C bounded-buffer audit notes | `c/security/BOUNDED_BUFFER_AUDIT.md` |
| Rust allocation/profile notes | `rust/security/IOT_PROFILE_NOTES.md` |
| Target evidence template | `docs/iot-target-evidence-template.md` |

Baseline byte model to verify in CI:

```text
AUTH_1 packet = 282 + 96R bytes
Full AUTH exchange = 579 + 96R bytes
R=1: AUTH_1 378 B, full exchange 675 B
R=4: AUTH_1 666 B, full exchange 963 B
R=8: AUTH_1 1050 B, full exchange 1347 B
```

Profile gates:

- `iot-core` classical AUTH must remain within the configured small-datagram budget, with `R=4` as the preferred constrained default and `R=8` as an explicitly measured upper profile.
- C hot-path parsing and protocol state handling must use bounded lengths and predictable failure paths.
- Rust and C must report wire size deltas for every AUTH extension.
- Gateway-only features must be marked as such and must not be silently enabled for MCU profiles.
- No feature may require a wall-clock source for security unless clock-skew and no-clock fallback behavior are specified.
- No feature may require persistent storage writes during every AUTH unless wear, failure atomicity, and replay impact are documented.

Acceptance gates:

- A target profile table exists for STM32-class, ESP32-S3-class, Raspberry Pi-class, and Jetson Orin-class deployments.
- Each AUTH improvement has a byte/RAM/CPU estimate before implementation and measured evidence after implementation.
- Missing hardware measurements are marked as evidence gaps, not passes.
- `iot-core` remains usable without post-quantum, anonymous-credential, or large-registry extensions.

Suggested task:

```text
Add an IoT profile matrix and AUTH benchmark harness. Measure or estimate wire bytes, RAM, CPU, storage writes, and registry lookup costs for constrained and Linux-edge profiles. Do not claim IoT field readiness without target evidence.
```

## zk217 — AUTH Transcript v3 and Complete Context Binding

Goal: harden AUTH key confirmation against downgrade, transcript-splicing, unknown-key-share, cross-suite, and cross-transport confusion while adding no extra round trips and negligible wire cost.

Design principle:

```text
Every negotiated or security-relevant byte that affects AUTH semantics must be covered
by the transcript hash, finished MAC, session binder, or an explicit equivalent check.
```

Transcript fields to bind:

- protocol version and suite identifier;
- HELLO and HELLO_REPLY negotiated capabilities;
- profile identifier, including `iot-core`, `iot-edge`, and any research suite bit;
- packet type, sequence, and session identifier;
- client and server ephemeral public keys;
- pinned server public key or server identity hash;
- allowed-role policy hash and role-proof parameters;
- transport binding label such as `udp`, `tcp`, `ble`, or `serial`;
- deployment/domain identifier;
- complete canonical AUTH payloads except fields that are intentionally computed over the transcript, such as final MAC fields.

IoT constraints:

- Must use the existing hash/KDF stack where possible.
- Must not add new public-key operations.
- Must not add additional AUTH messages.
- Must remain constant-memory with streaming or bounded transcript hashing for C.

Acceptance gates:

- Flipping any transcript-bound field causes key confirmation or binder verification to fail.
- Downgrade tests reject removed capabilities, changed suite IDs, changed role-policy hashes, changed profile IDs, and changed transport labels.
- Rust and C transcript bytes match deterministic vectors.
- Python vectors are updated after Rust/C parity is stable.
- No production claim is made until mutation-test coverage is checked in.

Suggested task:

```text
Implement AUTH transcript v3 by binding HELLO, AUTH headers, suite/profile IDs, role policy, transport label, server identity, and canonical payload bytes into key confirmation. Add mutation tests proving each security-relevant field is covered.
```

## zk218 — Strict AUTH State-Machine and Sequence Validation

Goal: make replay resistance and retransmission behavior a precise state machine rather than an emergent property of handlers.

Required rules:

```text
AUTH_1: accepted only in unauthenticated/pending state with expected sequence
AUTH_2: accepted only by the originating client session and expected server identity
AUTH_3: accepted only for a pending session_id, expected client address/binding, and expected sequence
AUTH_ACK: accepted only after AUTH_3 validation and expected sequence
```

Additional checks:

- reject wrong packet type for the current state;
- reject wrong or reused sequence values;
- reject cross-session `AUTH_3` replay;
- reject stale `AUTH_1` when nonce/session freshness has expired;
- bind UDP peer address unless explicit migration support is negotiated;
- define TCP connection reuse semantics separately from UDP datagram retransmission;
- define server restart replay behavior and persistent-vs-memory replay-cache policy.

IoT constraints:

- State entries must be fixed-size and evictable under memory pressure.
- Constrained clients must not need to store unbounded transcript histories.
- Server response caches must have bounded entry count, entry size, and TTL.

Acceptance gates:

- Duplicate, reordered, stale, wrong-sequence, wrong-session, wrong-address, and cross-transport replay tests are present.
- Memory exhaustion of pending sessions fails closed and does not corrupt established sessions.
- Rust/C behavior matches for accepted retransmission and rejected replay cases.
- Formal model notes identify which state-machine rules are modeled and which remain implementation-only tests.

Suggested task:

```text
Tighten AUTH state-machine checks for session_id, sequence, packet type, address binding, retransmission, and restart behavior. Add negative tests for duplicate, reordered, stale, wrong-sequence, wrong-session, wrong-address, and cross-transport packets.
```

## zk219 — Stateless AUTH_RETRY Cookies and Unauthenticated-Work Throttling

Goal: prevent unauthenticated peers from forcing expensive registry scans, proof verification, allocations, or response amplification before basic source validation.

Candidate flow:

```text
client -> AUTH_1 without retry cookie
server -> AUTH_RETRY(cookie, expires_at, retry_profile)
client -> AUTH_1 with retry cookie
server -> expensive lookup and proof verification only after cookie validation
```

Candidate cookie construction:

```text
retry_cookie = MAC(server_retry_secret,
                   peer_binding || suite_id || profile_id || session_id || nonce_c || eph_c || expires_at)
```

Policy:

- normal local deployments may make retry optional;
- high-load, WAN-facing, or UDP deployments may require retry before expensive work;
- retry verification must happen before registry scan, proof verification, or large allocation;
- retry secrets must rotate without invalidating all in-flight clients abruptly;
- source binding must be configurable for NAT-heavy deployments.

IoT constraints:

- Cookie verification must be MAC-only and fixed-memory.
- Cookie state must be stateless or bounded; no per-client allocation is allowed before cookie validation.
- Constrained clients must support one retry without treating it as fatal.

Metrics and gates:

| Metric | Target |
|---|---|
| Invalid unauthenticated `AUTH_1` | O(1) MAC/check path, no registry scan |
| Pre-validation amplification | bounded by configured policy, with a 3x-style target for UDP deployments |
| Server memory before cookie validation | no per-client heap allocation beyond bounded parse buffer |
| Replay of old retry cookie | rejected after expiry or retry-secret rotation window |

Acceptance gates:

- Invalid random AUTH floods do not trigger registry scan or proof verification when retry-required policy is active.
- Replayed, expired, wrong-source, wrong-suite, wrong-profile, and tampered cookies are rejected.
- Retry behavior is covered for UDP and TCP profiles.
- The fallback path remains available for closed local deployments that do not need retry.

Suggested task:

```text
Add an AUTH_RETRY cookie design and implementation option. Verify retry cookies before registry scans or proof verification, keep the path stateless or bounded, and add negative tests for expired, tampered, wrong-source, wrong-suite, and replayed cookies.
```

## zk220 — Optional Encrypted Lookup Hints for Scalable AUTH

Goal: reduce server registry lookup from intentional `O(n)` scanning to optional `O(1)` lookup without exposing a stable cleartext device identifier on the wire.

Design principle:

```text
The server may receive an encrypted, randomized lookup hint that only the server can open.
The network must not receive a stable cleartext device identifier.
The existing scan path remains the privacy-preserving compatibility fallback.
```

Candidate construction for `iot-core`:

- use the existing Ristretto255 scalar multiplication and HKDF/AEAD stack rather than mandating a separate HPKE dependency;
- derive a one-time hint encryption key from client ephemeral secret and a server lookup public key;
- encrypt `device_id || registry_epoch || random_padding` with transcript-bound AAD;
- randomize every hint so repeated AUTH attempts are not linkable by the network;
- bind the encrypted hint and server lookup public key hash into the AUTH transcript.

Server behavior:

```text
if encrypted_lookup_hint is present and valid:
    decrypt hint
    lookup candidate registry record directly
    verify PID and role proof against that record
else:
    use existing O(n) PID scan path
```

IoT constraints:

- Must fit the configured AUTH_1 datagram budget for `iot-core` profiles, or be disabled on constrained UDP.
- Must reuse existing cryptographic primitives where possible.
- Must remain optional; constrained devices may continue using the scan path.
- The server-side registry index must be bounded and crash-safe.

Metrics and gates:

| Metric | Target |
|---|---|
| Lookup cost with valid hint | O(1) candidate lookup plus normal proof verification |
| Lookup cost without hint | existing O(n) fallback, still measured |
| AUTH_1 size increase | explicitly measured; target roughly one small AEAD-sealed identifier block, not a large certificate chain |
| Network unlinkability | repeated hints for the same device differ across sessions |

Acceptance gates:

- Repeated AUTH attempts from the same device produce different encrypted hints.
- Tampered hints fall back or reject according to policy without accepting wrong records.
- Decrypted hint mismatch does not bypass PID/proof verification.
- Large-registry benchmarks include 1k, 10k, and 100k records on Linux-edge server profiles.
- MCU clients can disable hints and remain compatible.

Suggested task:

```text
Design optional encrypted lookup hints using existing curve/KDF/AEAD primitives where possible. Preserve the current O(n) scan fallback, bind hints into the transcript, and benchmark 1k/10k/100k-record lookup behavior.
```

## zk221 — Replay-Safe Session Resumption for Constrained Devices

Goal: reduce repeated AUTH cost and latency for devices that reconnect frequently without allowing replayed tickets or unsafe early writes.

Candidate full-auth output:

```text
AuthTicket {
  version
  suite_id
  profile_id
  device_record_hash
  role_policy_hash
  registry_epoch
  ticket_secret
  not_before
  expires_at
  single_use_nonce_or_counter
  server_ticket_mac
}
```

Candidate resumed flow:

```text
AUTH_RESUME_1(ticket, nonce_c, fresh eph_c, binder)
AUTH_RESUME_2(nonce_s, fresh eph_s, server_binder, optional_new_ticket)
```

Policy:

- prefer 1-RTT resumption for baseline ZK-ARCHE;
- do not allow general-purpose state-changing 0-RTT writes;
- bind ticket use to suite, profile, server identity, registry epoch, role policy, and transcript;
- revoke or reject tickets when the device record, role policy, or registry epoch changes;
- use single-use tickets where persistent replay tracking is available, or short-lived bounded tickets where not.

IoT constraints:

- Ticket storage on constrained clients must be small and optional.
- Server ticket validation should be stateless or bounded, using encrypted/self-authenticating tickets where appropriate.
- Flash-write frequency must be controlled; per-AUTH persistent writes are not acceptable for low-end flash unless wear impact is documented.

Metrics and gates:

| Metric | Target |
|---|---|
| Resumed AUTH bytes | lower than full AUTH for the same profile; target below 600 B where feasible |
| Extra client persistent storage | one or few bounded tickets, profile-configured |
| Replay behavior | reused tickets rejected or bounded by documented short TTL policy |
| Privilege freshness | role-policy or registry changes invalidate stale tickets |

Acceptance gates:

- Replayed, expired, wrong-server, wrong-suite, wrong-profile, wrong-registry-epoch, and wrong-role-policy tickets are rejected.
- Resumption never expands privileges beyond the original authenticated role policy.
- Resumption transcript mutation tests exist.
- Full AUTH remains available and required when tickets are absent or invalid.

Suggested task:

```text
Design replay-safe 1-RTT AUTH resumption with self-authenticating or bounded tickets. Bind tickets to suite, profile, server identity, registry epoch, role policy, and transcript. Reject unsafe 0-RTT writes.
```

## zk222 — AUTH Metrics CI and Security-Assurance Dashboard

Goal: make AUTH security and performance regressions visible as checked-in evidence rather than informal observations.

Required CI metrics:

```text
auth_full_bytes_R1
auth_full_bytes_R4
auth_full_bytes_R8
auth_retry_cookie_bytes
auth_lookup_hint_bytes
auth_resume_bytes
auth_server_lookup_records_1k
auth_server_lookup_records_10k
auth_server_lookup_records_100k
auth_invalid_auth1_cost
auth_replay_auth1_rejected
auth_replay_auth3_rejected
auth_wrong_seq_rejected
auth_cross_session_rejected
auth_transcript_mutation_rejected
auth_role_privacy_repeated_sessions_unlinkable
auth_resume_replay_rejected
auth_peak_stack_c_iot_core
auth_peak_heap_c_iot_core
auth_rust_alloc_profile_iot_core
```

Required artifacts:

| Artifact | Suggested path |
|---|---|
| AUTH metrics schema | `docs/auth-metrics-schema.md` |
| Latest AUTH metrics | `evidence/auth-metrics.json` |
| Security dashboard summary | `evidence/auth-security-dashboard.md` |
| Negative-test inventory | `security/AUTH_NEGATIVE_TEST_INVENTORY.md` |
| Target benchmark logs | `evidence/targets/` |

Acceptance gates:

- Wire-size metrics fail CI on unreviewed growth beyond configured profile budgets.
- Replay and mutation tests fail CI on any acceptance of known-bad transcripts.
- Large-registry lookup benchmarks are recorded for scan path and hint path where implemented.
- CI distinguishes `not implemented`, `not measured`, `failed`, and `passed`; missing evidence is never reported as success.
- Metrics include both Rust and C where the feature exists in both lanes.

Suggested task:

```text
Create AUTH metrics CI that records wire bytes, lookup scaling, replay outcomes, transcript mutation outcomes, and constrained-memory evidence. Fail on unreviewed security regression or profile-budget regression.
```

## zk223 — Optional Reviewed Anonymous-Credential Migration Evaluation

Goal: evaluate replacing or augmenting the custom role-membership proof with a reviewed anonymous-credential construction without making heavy credential systems mandatory for constrained devices.

Research candidates:

- BBS-style selective-disclosure credentials for roles and attributes;
- Privacy Pass-style unlinkable authorization tokens for coarse role admission;
- Direct Anonymous Attestation-style hardware-backed authorization for deployments with attestation-capable devices;
- certificate-only authorization for non-anonymous operational modes.

IoT constraints:

- Anonymous-credential suites are `research-only` or `iot-edge` until proof size, verification time, RAM, code size, licensing, and external-review evidence exist.
- `iot-core` must retain a bounded classical AUTH path and must not require a large pairing-based dependency by default.
- Any credential migration must use suite/capability negotiation and deterministic test vectors.
- The custom proof must remain marked security-sensitive until reviewed or replaced.

Acceptance gates:

- A comparison table exists for proof size, verification cost, dependency footprint, review status, and privacy properties.
- Repeated-auth unlinkability is tested using at least 10k simulated sessions for candidate schemes before privacy claims are expanded.
- A downgrade path from anonymous-credential suite to classical suite is either forbidden or transcript-bound and policy-controlled.
- External cryptographic review is required before production claims.

Suggested task:

```text
Evaluate anonymous credential options for role authorization under IoT constraints. Do not make BBS, Privacy Pass-style, DAA, or certificate-only modes mandatory for iot-core. Produce proof-size, CPU/RAM, dependency, privacy, and review-status evidence before migration.
```

## zk224 — Optional Post-Quantum Hybrid Suite Research

Goal: explore post-quantum hybrid key establishment as a future optional suite while preserving the current low-footprint classical AUTH profile for constrained IoT.

Design principle:

```text
Post-quantum support is a negotiated future suite, not a hidden dependency of baseline AUTH.
Constrained devices must retain a classical iot-core profile until PQ byte/RAM/CPU evidence is acceptable.
```

Candidate direction:

- derive session secrets from both the existing classical ECDH path and a post-quantum KEM shared secret;
- keep hybrid behavior behind a new suite identifier and capability bit;
- prefer Linux-edge/gateway experiments first;
- evaluate MCU feasibility only after exact ciphertext, public-key, RAM, and CPU costs are measured;
- require explicit MTU, fragmentation, or TCP profile decisions before any UDP deployment claim.

Known design pressure:

```text
Hybrid KEM material can exceed the current small AUTH_1 datagram budget.
A PQ suite may require TCP, fragmentation, gateway-assisted enrollment/auth, or a different packet layout.
```

IoT constraints:

- Post-quantum is not mandatory for STM32/ESP32-S3-class `iot-core` until measured evidence proves feasibility.
- Any PQ implementation must be constant-time-audited for the selected backend and target class before security claims.
- PQ test vectors must not change the meaning of existing classical test vectors.
- Classical and PQ suites must be explicitly negotiated and transcript-bound to prevent downgrade confusion.

Acceptance gates:

- Exact public-key, ciphertext, signature if any, RAM, CPU, and packet-size costs are recorded for the selected candidate.
- Hybrid KDF construction has a design note explaining combiners, domain separation, and failure handling.
- Downgrade and cross-suite transcript mutation tests exist.
- No PQ production claim is made without external review and target evidence.

Suggested task:

```text
Research an optional post-quantum hybrid AUTH suite. Keep it suite-negotiated and non-mandatory for iot-core, measure exact byte/RAM/CPU impact, and require downgrade tests, hybrid-KDF design review, and external cryptographic review before implementation claims.
```

## zk225 — Python Lane Integration and Vector Alignment

Goal: integrate the latest Python implementation as a third reference lane while keeping Rust as the canonical vector source and C as the constrained-device implementation anchor.

Role of the Python lane:

- readable protocol reference;
- CLI/demo harness for UDP and TCP behavior;
- fast state-machine and negative-test prototyping;
- mirrored Rust-vector validation;
- specification examples for the future RFC-style package.

Non-goals:

- Python is not the baseline target for STM32/ESP32-S3-class constrained devices.
- Python test success does not prove side-channel resistance, memory-safety, or field readiness.
- Python vector fixtures must not become a competing source of truth against `rust/test-vectors/0x0001/`.

Required artifacts:

| Artifact | Path |
|---|---|
| Python implementation lane | `python/` |
| Python CI wrapper | `scripts/ci-python.sh` |
| Python evidence log | `evidence/python-ci.log` |
| Parent CI includes Python | `scripts/ci-all.sh` |
| Cross-language notes mention Python | `docs/cross-language-validation.md` |

Acceptance gates:

- `./scripts/ci-python.sh` installs the Python lane and runs `pytest`.
- `./scripts/ci-all.sh` invokes Rust, C, Python, and the C-against-Rust vector harness.
- Python vectors pass against the mirrored `0x0001` fixture set.
- A future vector-sync check is planned before any vector semantics change.
- README and roadmap identify Python as a reference/interoperability lane.

Suggested task:

```text
Keep the Python implementation integrated as a third reference lane. Add CI and evidence logging, preserve Rust as the canonical vector source, and do not make Python a constrained-device target.
```

## zk226 — RFC-Style Specification Package

Goal: evolve ZK-ARCHE toward an RFC-like protocol package with normative message grammar, registries, state machines, security considerations, privacy considerations, and conformance tests.

Reference discipline to emulate:

- EDHOC-style compact authenticated key exchange for constrained IoT;
- TLS/mTLS-style transcript binding, downgrade prevention, endpoint authentication, and alert/error discipline;
- DTLS-style datagram replay, anti-amplification, retry, and lossy-link behavior;
- OSCORE-style security-context/export thinking for constrained CoAP-like deployments.

Required artifacts:

| Artifact | Suggested path |
|---|---|
| RFC-style evolution plan | `docs/rfc-evolution-plan.md` |
| Protocol specification skeleton | `spec/zk-arche-protocol.md` |
| Registry document | `spec/registries.md` |
| IoT profile document | `spec/iot-profiles.md` |
| Security considerations | `spec/security-considerations.md` |
| Privacy considerations | `spec/privacy-considerations.md` |
| Test-vector specification | `spec/test-vectors.md` |
| Interop matrix | `evidence/interop-matrix.md` |

IoT constraints:

- RFC-style work must preserve `iot-core`, `iot-edge`, and `research-only` feature separation.
- Normative requirements must be testable on at least one relevant implementation lane before conformance claims.
- TLS, DTLS, CBOR, COSE, anonymous credentials, and post-quantum suites remain optional unless target evidence proves they fit the profile.

Acceptance gates:

- Every wire/security behavior claimed as normative has a test-vector, negative test, or explicit evidence gap.
- Suite, extension, alert, transport-binding, and profile identifiers are registry-controlled.
- State machines are specified for SETUP, AUTH, late enrollment, rekey, and resumption.
- Security and privacy considerations are explicit and do not overclaim production readiness.

Suggested task:

```text
Create an RFC-style specification skeleton for ZK-ARCHE. Define registries, state machines, security considerations, privacy considerations, IoT profiles, and test-vector requirements without changing wire semantics.
```

## zk227 — EDHOC/CoAP/OSCORE-Inspired Constrained Profile

Goal: evaluate a constrained profile that borrows EDHOC/OSCORE design discipline while keeping current ZK-ARCHE semantics and avoiding mandatory heavyweight dependencies for `iot-core`.

Candidate work:

- compact canonical encoding review;
- optional CBOR/CDDL description of messages for specification clarity;
- optional CoAP transport mapping;
- explicit security-context export function;
- measured packet sizes for SETUP/AUTH/RESUME under `R=1`, `R=4`, and `R=8`.

IoT constraints:

- CBOR/COSE dependencies must be optional until code-size, RAM, CPU, and fuzz evidence exists for STM32/ESP32-S3-class targets.
- Current native binary encoding remains valid unless a checkpoint-approved migration adds versioned vectors.
- CoAP/OSCORE binding must not hide replay or retransmission state-machine behavior.

Acceptance gates:

- Compact profile byte costs are reported.
- Any new parser has fuzz coverage and malformed-input tests.
- Cross-encoding equivalence tests exist if both native and CBOR-like encodings are supported.
- No constrained-profile claim is made without target evidence.

Suggested task:

```text
Evaluate an EDHOC/OSCORE-inspired constrained profile for ZK-ARCHE. Keep CBOR/COSE and CoAP bindings optional, measure byte/RAM/CPU costs, and preserve current vectors unless a versioned migration is approved.
```

## zk228 — TLS/mTLS-Style Channel-Binding Profile

Goal: define an optional profile for deployments that already run TLS or mTLS, binding ZK-ARCHE authentication to the TLS channel without assuming client certificates are privacy preserving.

Candidate binding:

```text
zk_arche_tls_channel_binding = TLS-Exporter("EXPORTER-ZK-ARCHE-v1", transcript_context, 32)
AUTH transcript includes zk_arche_tls_channel_binding
```

Policy:

- use TLS/mTLS as an optional deployment binding, not a replacement for ZK-ARCHE privacy semantics;
- document client-certificate metadata leakage in mTLS mode;
- define whether ZK-ARCHE authenticates the TLS endpoint or an application/device identity behind it;
- bind server identity, ALPN/application profile, suite id, deployment id, and exporter value into the ZK-ARCHE transcript.

Acceptance gates:

- wrong exporter, wrong server identity, wrong ALPN/profile, wrong deployment id, and wrong suite are rejected.
- TLS termination/proxy assumptions are documented.
- mTLS mode does not claim unlinkability when stable client certificates are visible.
- TLS is not mandatory for `iot-core` unless the deployment profile requires it and provides measurements.

Suggested task:

```text
Design an optional TLS/mTLS channel-binding profile for ZK-ARCHE using exporter-bound transcripts. Preserve privacy claims truthfully and reject wrong-channel mutations.
```

## zk229 — DTLS-Style Datagram Robustness Profile

Goal: make native UDP behavior specification-grade by adopting DTLS-style discipline for anti-amplification, retry, replay, retransmission, and lossy-link operation.

Required design points:

- address-validation and retry-cookie policy;
- amplification limit before peer validation;
- replay window and duplicate suppression rules;
- retransmission behavior for SETUP, AUTH, AUTH_RETRY, AUTH_ACK, and future RESUME;
- response-cache limits under attack;
- alert/error mapping with bounded privacy leakage.

IoT constraints:

- invalid unauthenticated datagrams must be cheap to reject.
- response-cache memory must be bounded and measurable.
- retransmission timers must tolerate lossy low-power links without requiring a large scheduler or heap.
- fallback to TCP or gateway-only profile is required if packet sizes exceed the configured UDP budget.

Acceptance gates:

- duplicate, reordered, stale, cross-session, wrong-sequence, wrong-address, and oversized datagrams are tested.
- unauthenticated packets do not trigger registry scans or proof verification when retry-cookie mode is required.
- amplification behavior is measured and policy-bounded.
- lossy-link tests or simulations are checked in as evidence, not only described.

Suggested task:

```text
Specify and test DTLS-style UDP robustness for ZK-ARCHE. Add retry-cookie, replay-window, retransmission, amplification, and response-cache rules with constrained-device memory bounds.
```


## zk230 — Protocol-Suite Decomposition for RFC-Like Maturity

Goal: evolve ZK-ARCHE from a repository into a protocol suite with the same engineering layers that make EDHOC, TLS/mTLS, and DTLS credible, while preserving ZK-ARCHE's distinct role as a privacy-preserving device authorization and sovereignty protocol.

Target suite structure:

```text
ZK-ARCHE-CORE
  Wire format, canonical encoding, registries, transcript construction,
  profile identifiers, extension negotiation, error handling, and test-vector rules.

ZK-ARCHE-AUTH
  Native privacy-preserving role authentication and constrained session establishment.

ZK-ARCHE-BIND
  Channel-bound mode for EDHOC/OSCORE, TLS/mTLS, and DTLS deployments.

ZK-ARCHE-ENROLL
  Setup, late enrollment, delegated enrollment, rekey, revocation, and registry epochs.

ZK-ARCHE-DATA
  Device-controlled encrypted data records, policy-bound release, auditability,
  and revocable per-device data-sovereignty semantics.
```

Required specification artifacts:

| Artifact | Suggested path |
|---|---|
| Architecture document | `spec/zk-arche-architecture.md` |
| AUTH protocol document | `spec/zk-arche-auth.md` |
| Enrollment document | `spec/zk-arche-setup-enrollment.md` |
| Wire-format document | `spec/zk-arche-wire-format.md` |
| Crypto-suite registry | `spec/zk-arche-crypto-suites.md` |
| IoT profile document | `spec/zk-arche-iot-profiles.md` |
| EDHOC/OSCORE binding | `spec/zk-arche-edhoc-oscore-binding.md` |
| TLS/DTLS binding | `spec/zk-arche-tls-dtls-binding.md` |
| Data-sovereignty document | `spec/zk-arche-data-sovereignty.md` |
| Security considerations | `spec/zk-arche-security-considerations.md` |
| Privacy considerations | `spec/zk-arche-privacy-considerations.md` |
| Test-vector document | `spec/zk-arche-test-vectors.md` |
| Registry document | `spec/zk-arche-iana-registries.md` |

Protocol layering target:

```text
Layer 0: Wire framing and canonical encoding
Layer 1: Crypto-suite and profile registries
Layer 2: Enrollment, late enrollment, rekey, and revocation
Layer 3: AUTH handshake and role-membership proof
Layer 4: Session export and channel binding
Layer 5: Application policy binding and data release
Layer 6: IoT profile constraints and deployment guidance
```

Formal security goals to state explicitly:

- server authentication;
- device enrollment authentication;
- role-membership authentication;
- optional mutual authentication;
- session-key secrecy and forward secrecy;
- downgrade resistance;
- unknown-key-share resistance;
- replay resistance;
- retry-cookie anti-DoS behavior;
- registry epoch binding;
- exporter/channel binding;
- passive unlinkability for protocol identity fields where claimed;
- bounded active-linkability limitations;
- role privacy limitations;
- explicit registry privacy limitations.

Non-goals and limitations that must remain visible:

- ZK-ARCHE does not hide identity from the authenticating server after successful lookup unless a future profile proves a stronger property.
- ZK-ARCHE does not protect a device after its root/private key is compromised.
- Custom role proofs are not production-grade anonymous credentials without external review.
- ZK-ARCHE does not replace transport security in binding modes.
- `iot-core` does not make post-quantum or general-purpose zkSNARK claims.
- Traffic-analysis correlation by timing, radio metadata, and packet size is not solved by default.

Acceptance gates:

- A third-party implementer can build each normative message and transcript from specification text and vectors.
- Suite, extension, profile, alert, proof-type, release-token, and data-record identifiers are registry-controlled.
- Every normative security behavior has a positive vector, negative vector, test, formal-model claim, or explicit evidence gap.
- Specification documents separate normative requirements from deployment guidance and research notes.
- No RFC-like maturity claim is made until wire grammar, state machines, security/privacy considerations, registries, and vectors are checked in.

Suggested task:

```text
Split ZK-ARCHE into CORE, AUTH, BIND, ENROLL, and DATA specification tracks. Add RFC-style normative registries, layered protocol descriptions, security goals, privacy limitations, and implementer-facing message grammar without changing current wire semantics.
```

## zk231 — Per-Device Data Sovereignty Architecture

Goal: define per-device data sovereignty as a first-class ZK-ARCHE objective without forcing heavy zero-knowledge systems onto constrained devices.

Definition:

```text
Per-device data sovereignty means that each device cryptographically controls
what protected data it releases, to whom, under which policy, during which epoch,
and with what proof, while revealing the minimum identity, role, and state metadata
required by the selected deployment profile.
```

Core constraint:

```text
ESP32-S3/STM32-class devices run only small, bounded crypto.
Gateways, issuers, and servers handle heavy policy indexing, anonymous credentials,
large Merkle structures, formal audit verification, or zkSNARK research modes.
```

Device-local sovereignty root:

```text
device_root_sk

auth_sk    = HKDF(device_root_sk, "zk-arche auth")
data_sk    = HKDF(device_root_sk, "zk-arche data sealing")
audit_sk   = HKDF(device_root_sk, "zk-arche audit")
consent_sk = HKDF(device_root_sk, "zk-arche consent")
```

Rules:

- `device_root_sk` is never exported.
- Derived keys use domain-separated labels and suite/profile identifiers.
- Key derivation binds `device_epoch` where rotation/revocation semantics require it.
- Hardware secure elements or TEEs may protect `device_root_sk`, but correctness must not require vendor-specific hardware.
- Rekey and revocation must advance `device_epoch` and invalidate stale release material.

Sovereignty properties:

| Property | Required behavior | Metric/evidence |
|---|---|---|
| Device-controlled encryption | Protected data is encrypted before release or storage by untrusted infrastructure | 100% protected telemetry records are AEAD encrypted in sovereignty profiles |
| Minimal identity disclosure | AUTH and data release avoid stable cleartext device identifiers | PID changes per session; no stable `device_id` on wire where privacy is claimed |
| Policy-bound release | Release binds recipient, purpose, time window, policy hash, and device epoch | Mutating any bound field invalidates the release token/proof |
| Local auditability | Device records release events in an append-only commitment chain | Deletion, reordering, or modification is detected by verifier tests |
| Revocable authorization | Device, registry, policy, and credential epochs invalidate stale proofs/tickets | Old release tokens fail after epoch revocation |

IoT constraints:

- `iot-core` must not require a general-purpose ZK circuit prover.
- `iot-core` must not require dynamic allocation in C hot paths after initialization.
- Protected-data upload must support fixed-size or bounded chunks.
- Audit entries must be small and append-only; target 64-96 bytes before optional metadata.
- Flash-write frequency must be documented to avoid hidden wear risks.

Acceptance gates:

- Protected-data plaintext paths are explicitly marked public/test-only or rejected by CI.
- Device epoch revocation invalidates old AUTH, release, and wrapped-key material according to profile policy.
- Sovereignty key derivation vectors exist across Rust, C, and Python before compatibility is claimed.
- No data-sovereignty claim is made without byte/RAM/CPU evidence for the selected profile.

Suggested task:

```text
Add a per-device data-sovereignty architecture document. Define device_root_sk derivation, encrypted-by-default protected data, release policy binding, audit-chain requirements, revocation epochs, and iot-core constraints without introducing heavyweight ZK dependencies.
```

## zk232 — ZK-Minimal Proof-Carrying Data Profile

Goal: use small zero-knowledge or proof-like primitives for constrained devices while reserving heavyweight anonymous credentials and zkSNARKs for gateway/research profiles.

Allowed `iot-core` proof primitives:

- Schnorr proof of possession;
- small fixed-role OR/set-membership proof with bounded branch count;
- MAC-based authorization ticket;
- blinded or unlinkable token redemption only if measured and bounded;
- hash commitment;
- small Merkle inclusion proof only if tree depth and stack use are bounded;
- channel-bound proof using EDHOC/OSCORE, TLS, or DTLS exporter material.

Forbidden as mandatory `iot-core` requirements:

- Groth16, Plonk, STARK, or other general-purpose ZK proving;
- large BBS proof generation;
- large local Merkle tree construction;
- large certificate-chain parsing;
- post-quantum hybrid AUTH;
- dynamic memory-heavy proof systems;
- network-visible stable identifiers used as proof shortcuts.

Proof hierarchy:

| Sovereignty claim | Low-end proof | Higher-end proof |
|---|---|---|
| Enrolled device | Schnorr proof | anonymous credential |
| Allowed role | small OR/set-membership proof | BBS-style selective disclosure |
| Release authorized | MAC ticket or signature | Privacy Pass-style unlinkable token |
| Data belongs to policy | hash commitment | Merkle proof or zkSNARK |
| Audit not tampered | hash chain | Merkle transparency log |
| Recipient allowed | policy token | ZK policy proof |

Target `iot-core` metrics:

| Metric | Target |
|---|---|
| `AUTH_1` | <= 1200 bytes for constrained default profile |
| Full `AUTH` | <= 1600 bytes for constrained default profile |
| RAM during AUTH | <= 16 KiB target or documented evidence gap |
| Role branches | <= 4 default for constrained devices |
| Proof verification | <= 100 ms target on ESP32-S3-class benchmark or explicit evidence gap |
| Data release proof | <= 512 bytes target |
| C hot-path allocation | no heap allocation after init for `iot-core` critical path |

Acceptance gates:

- Every proof type has a registry identifier, transcript binding rule, domain separator, and maximum encoded length.
- Heavy proof systems are marked `gateway` or `research-only` and cannot be negotiated by `iot-core` unless explicitly enabled by profile policy.
- Proof-carrying data vectors include positive and negative cases across Rust, C, and Python where implemented.
- Mutation tests show that changing policy hash, recipient, purpose, epoch, session binding, or data commitment invalidates the proof/token.

Suggested task:

```text
Define a ZK-minimal proof-carrying data profile. Restrict iot-core to Schnorr, bounded role membership, MAC tickets, hash commitments, and small optional Merkle proofs. Keep BBS, Privacy Pass, zkSNARK, and post-quantum work gateway/research-only unless measured and reviewed.
```

## zk233 — ZK-ARCHE-DATA Minimal Sovereignty Protocol

Goal: add a minimal data-sovereignty protocol on top of AUTH without changing the baseline AUTH semantics.

Smallest viable flow:

```text
1. AUTH
   Device proves enrollment/role and derives a session or channel-bound key.

2. DATA_COMMIT
   Device uploads encrypted protected data and policy/audit commitments.

3. RELEASE_REQUEST
   Verifier requests access for a specific data type, purpose, recipient, and time range.

4. RELEASE_PROOF
   Device or authorized gateway proves the request satisfies policy.

5. RELEASE_KEY
   Device releases or authorizes a wrapped data key scoped to recipient/purpose/time.

6. AUDIT_APPEND
   Device appends the release event to its local audit hash chain.
```

Minimal wire objects:

```text
EncryptedDataRecord {
  version
  suite_id
  profile_id
  record_id
  device_epoch
  data_type_commitment
  timestamp_bucket
  ciphertext
  policy_hash
  audit_head
  tag
}

DataProof {
  proof_type
  device_epoch
  policy_hash
  data_type_commitment
  role_or_capability_commitment
  freshness_nonce
  channel_binding_hash optional
  proof
}

DataCommit {
  record_id
  device_epoch
  data_type_commitment
  timestamp_bucket
  ciphertext
  policy_hash
  audit_head
  data_proof
  tag
}

ReleaseRequest {
  requester_commitment
  purpose_hash
  data_type
  time_range
  nonce
  channel_binding_hash optional
}

ReleaseProof {
  proof_type
  policy_hash
  request_hash
  device_epoch
  proof
}

WrappedDataKey {
  recipient_context
  key_wrap_alg
  wrapped_key
  expires_at
  device_epoch
  policy_hash
  tag
}
```

Design rules:

- `AUTH` proves enrollment; `ZK-ARCHE-DATA` controls protected-data release.
- Data encryption and policy commitments must not depend on the server being trusted to enforce policy honestly.
- Release keys must be scoped and bounded; no profile may define an unbounded “release everything forever” token as the default.
- `DataCommit` must be safe to store on untrusted infrastructure.
- `ReleaseProof` must bind to the request hash and channel/session context when available.

Acceptance gates:

- Replayed release requests and replayed release proofs are rejected or idempotently bounded according to profile rules.
- A `WrappedDataKey` for one recipient, purpose, time range, or policy cannot decrypt another scope.
- `DataCommit` parser fuzzing and malformed-input tests exist before compatibility claims.
- Cross-language vectors cover at least one protected-data commit and one release-token rejection path before feature-complete claims.

Suggested task:

```text
Design ZK-ARCHE-DATA as a minimal protocol with DATA_COMMIT, RELEASE_REQUEST, RELEASE_PROOF, RELEASE_KEY, and AUDIT_APPEND semantics. Keep protected data encrypted by default and bind release keys to recipient, purpose, policy, time range, device epoch, and channel context.
```

## zk234 — Policy-Bound Release Tokens and Revocable Epochs

Goal: make protected-data access explicitly scoped, replay-resistant, and revocable.

Candidate release token:

```text
ReleaseToken {
  version
  suite_id
  profile_id
  device_epoch
  registry_epoch
  policy_epoch
  credential_epoch optional
  recipient_id_hash
  data_type
  time_range
  purpose_hash
  policy_hash
  not_before
  expires_at
  nonce
  issuer_id optional
  issuer_mac_or_signature
}
```

Epoch model:

```text
device_epoch     controls device rekey/re-registration freshness
registry_epoch   controls server-side enrollment/revocation freshness
policy_epoch     controls policy changes
credential_epoch controls anonymous credential/token freshness where used
```

Required binding:

- recipient identity or recipient commitment;
- purpose hash;
- data type or data-type commitment;
- time range;
- policy hash;
- device epoch;
- registry/policy epoch where available;
- channel/session binding where used;
- expiry and replay nonce/counter.

IoT constraints:

- `iot-core` tokens should be MAC- or signature-based with fixed maximum size.
- Epoch state must fit bounded storage and support restart behavior.
- Token validation must be possible without a large policy engine on MCU clients.
- Clockless or weak-clock deployments must define counter/epoch alternatives to wall-clock expiry.

Acceptance gates:

- Mutating recipient, purpose, time range, policy hash, data type, epoch, suite, profile, or channel binding invalidates the token.
- Old tokens fail after epoch revocation according to profile policy.
- Replay behavior is deterministic across reboot for profiles that claim replay resistance.
- Token format and validation behavior have positive and negative vectors.

Suggested task:

```text
Define ReleaseToken and epoch semantics for per-device data sovereignty. Bind every release to recipient, purpose, data type, time range, policy hash, device/registry/policy epochs, expiry, replay nonce, and channel context where available.
```

## zk235 — Local Audit Hash Chain and Gateway Transparency Bridge

Goal: give constrained devices a small local audit mechanism while allowing gateways to provide heavier transparency and verification services.

Device-local audit chain:

```text
audit_0 = H("zk-arche audit init" || suite_id || profile_id || device_epoch || device_audit_salt)

audit_n = H(
  audit_{n-1} ||
  event_type ||
  timestamp_or_counter_bucket ||
  policy_hash ||
  recipient_hash ||
  data_commitment ||
  release_token_hash ||
  device_epoch
)
```

Required event types:

- protected data committed;
- release requested;
- release approved;
- release rejected;
- release key wrapped;
- policy changed;
- device epoch advanced;
- audit checkpoint exported.

IoT constraints:

- Audit entries must be bounded and append-only.
- Devices may keep only the current audit head plus a small ring buffer if storage is constrained.
- Gateways may mirror audit checkpoints into a Merkle transparency log, but this is not mandatory for `iot-core`.
- Audit export must avoid leaking stable identifiers unless the deployment profile accepts that tradeoff.

Acceptance gates:

- Verification detects deletion, reordering, and modification in checked-in tests.
- Audit head is bound into protected-data commits or release proofs where the profile claims auditability.
- Power-loss and partial-write behavior is documented for constrained targets.
- Gateway transparency bridge is optional and cannot be required by baseline `iot-core`.

Suggested task:

```text
Add a bounded device-local audit hash chain for data commit/release events and an optional gateway transparency bridge. Test deletion, reordering, modification, power-loss, and privacy-leakage behavior.
```

## zk236 — Sovereignty CI Gates and Footprint Budgets

Goal: make per-device data-sovereignty claims testable rather than aspirational.

Required CI gates:

```text
1. No plaintext protected-data path in sovereignty profiles.
2. All release tokens bind recipient, purpose, expiry, policy hash, and epoch.
3. AUTH proof and data-release proof cannot be replayed across sessions.
4. Device epoch revocation invalidates old release keys and release tokens.
5. Policy mutation invalidates old proof transcripts.
6. Parser fuzzing covers AUTH, DATA_COMMIT, RELEASE_REQUEST, RELEASE_PROOF, and RELEASE_KEY.
7. Cross-language vectors exist for Rust, C, and Python before compatibility claims.
8. Low-end profiles enforce maximum packet size, maximum proof size, and memory budget.
9. Heavy ZK features are forbidden in iot-core by profile negotiation tests.
10. Failure behavior covers malformed input, storage exhaustion, RNG failure, clock skew, restart, and replay cache loss.
```

Suggested vector layout:

```text
test-vectors/
  data-sovereignty/
    key-derivation/
    encrypted-data-record/
    data-commit/
    release-token/
    release-proof/
    wrapped-key/
    audit-chain/
    epoch-revocation/
    plaintext-negative/
    replay-negative/
```

Suggested fuzz layout:

```text
fuzz/
  wire-parser
  auth-state-machine
  tlv-parser
  role-proof-parser
  enrollment-grant-parser
  data-commit-parser
  release-request-parser
  release-proof-parser
  wrapped-key-parser
```

Footprint budget evidence:

| Profile | Required evidence |
|---|---|
| `iot-core` | packet sizes, fixed-buffer limits, stack/heap estimate, proof maximum size, flash-write behavior |
| `iot-edge` | persistent replay cache, registry size, encrypted lookup hint, local encrypted store, resumption ticket storage |
| `gateway` | policy engine, anonymous credential service, audit verifier, interop matrix, fuzz/formal artifacts |
| `research-only` | explicit warning that results are experimental and not baseline IoT requirements |

Acceptance gates:

- CI fails if a sovereignty profile emits protected plaintext in testable paths.
- CI fails if `iot-core` negotiates heavy proof suites by default.
- CI reports wire size and proof size for every sovereignty message.
- Missing hardware measurements are recorded as evidence gaps, not passes.

Suggested task:

```text
Add sovereignty CI gates and footprint budgets. Enforce encrypted-by-default protected data, release-token binding, replay rejection, epoch revocation, parser fuzzing, vector parity, and profile negotiation rules that keep heavy ZK out of iot-core.
```

## zk237 — Channel-Bound Sovereignty over EDHOC/OSCORE, TLS/mTLS, and DTLS

Goal: allow ZK-ARCHE sovereignty proofs to bind to mature secure-channel protocols where they already exist, without making ZK-ARCHE a replacement for EDHOC, TLS, or DTLS.

Deployment modes:

```text
ZK-ARCHE-Standalone
  Native AUTH derives session keys and DATA binds to the native transcript.

ZK-ARCHE-over-EDHOC/OSCORE
  EDHOC derives an OSCORE security context.
  ZK-ARCHE role/data-release proofs bind to the EDHOC transcript and OSCORE context.

ZK-ARCHE-over-TLS/DTLS
  TLS/mTLS or DTLS establishes the secure channel.
  ZK-ARCHE role/data-release proofs bind to exporter material and application context.
```

Binding requirements:

- suite ID;
- profile ID;
- deployment/domain ID;
- server identity or channel endpoint identity;
- ALPN/application profile where available;
- exporter or EDHOC/OSCORE context hash;
- role policy hash;
- release policy hash;
- device/policy/registry epochs;
- transcript hash of the ZK-ARCHE proof object.

Privacy caveats:

- mTLS with stable client certificates may reveal device identity and must not claim unlinkability.
- TLS termination and proxies must be documented because exporter binding may terminate at the proxy rather than the origin verifier.
- EDHOC/OSCORE bindings must specify whether the proof binds to endpoint identity, application identity, or gateway identity.
- DTLS replay windows and ZK-ARCHE replay windows must not contradict each other.

Acceptance gates:

- Wrong exporter/context, wrong server identity, wrong ALPN/profile, wrong deployment ID, wrong suite/profile, and wrong policy hash are rejected.
- Binding modes have positive and negative vectors before interoperability claims.
- Channel binding does not become mandatory for `iot-core` standalone deployments.
- Privacy claims are adjusted per binding mode and do not overstate anonymity.

Suggested task:

```text
Design channel-bound sovereignty modes for EDHOC/OSCORE, TLS/mTLS, and DTLS. Bind ZK-ARCHE AUTH and DATA proofs to exporter/context material, server/application identity, suite/profile, policy hash, and epoch state, with negative tests for wrong-channel mutations.
```

## zk238 — Advanced Sovereignty Research Outside iot-core

Goal: evaluate stronger anonymous-credential and zero-knowledge policy systems without increasing the baseline code footprint or compute requirement of constrained devices.

Research candidates:

- BBS-style selective disclosure for higher-end gateway/issuer profiles;
- Privacy Pass-style unlinkable authorization tokens;
- Direct Anonymous Attestation-style hardware-backed authorization;
- Merkle transparency logs for gateway audit aggregation;
- zkSNARK policy proofs for complex release policies;
- post-quantum hybrid suites only for gateway/research profiles.

Strict separation policy:

```text
Research features MAY inform future suites.
Research features MUST NOT become baseline `iot-core` requirements.
Research features MUST be suite-negotiated, profile-gated, measured, and externally reviewed
before any production or constrained-device claim is made.
```

Evaluation criteria:

| Criterion | Required question |
|---|---|
| Privacy gain | What metadata or linkage is reduced versus the current proof? |
| Footprint | What are code size, RAM, stack, heap, flash-write, and wire costs? |
| Compute | What are proving and verification costs on gateway and constrained targets? |
| Interop | Can Rust, C, and Python share vectors and reject malformed proofs? |
| Review | Has the construction received external cryptographic review? |
| Migration | Can it be isolated behind a new suite/profile without breaking existing vectors? |

Acceptance gates:

- Research suites cannot be enabled by default in `iot-core`.
- All research claims include measured or explicitly missing byte/RAM/CPU evidence.
- External review is required before production-grade privacy or cryptographic claims.
- Existing classical vectors and constrained profiles remain valid.

Suggested task:

```text
Evaluate advanced anonymous credentials, Privacy Pass-style tokens, DAA-style device authorization, Merkle transparency logs, zkSNARK policy proofs, and post-quantum hybrids as gateway/research-only sovereignty extensions. Preserve iot-core minimalism and require measurement, suite isolation, and external review.
```

## Agent Editing Contract

Future automated edits may improve implementation quality, validation, documentation, and traceability, but must preserve:

- Rust/C byte-level compatibility;
- the Rust vector corpus as the primary checked-in interop anchor unless a checkpoint-approved migration occurs;
- strict separation between evidence and claims;
- truthful assurance-status wording;
- no production/security/certification claims without evidence;
- checkpoint-style review for protocol, crypto, parsing, replay, RNG, memory-safety, formal-model, side-channel, and interop changes.
