# ZK-ARCHE Combined Rust/C Improvement Roadmap

This roadmap describes an evidence-gated improvement path for the combined ZK-ARCHE Rust and C repository. It is designed for Hermes Legion Commander runs that alternate between Codex CLI and Claude Code CLI, and for checkpoint competition when security-sensitive implementation changes are proposed.

The roadmap is evidence-gated, not calendar-gated. It does not claim production readiness, formal verification, side-channel resistance, replay-resistance completeness, IoT field readiness, external cryptographic review, or certification unless the required evidence exists as checked-in artifacts.

## Current Baseline

The repository contains two preserved implementation lanes:

| Lane | Path | Role |
|---|---|---|
| Rust reference | `rust/` | Cargo workspace, protocol library, binaries, deterministic test vectors, fuzz targets, ProVerif skeleton, security docs |
| C implementation | `c/` | libsodium-based C11 implementation, public headers, unit/e2e/vector tests, fuzz harnesses, ProVerif skeleton, security docs |

Rust test vectors under `rust/test-vectors/0x0001/` are the primary byte-level interop anchor for the C test-vector harness.

## Non-Negotiable Boundaries

- Do not change cryptographic primitives, transcript domain separators, packet formats, suite identifiers, test-vector meanings, replay semantics, or wire compatibility without explicit checkpoint competition and evidence.
- Do not claim production-ready cryptographic security, side-channel resistance, memory-safety completeness, formal verification, replay-resistance completeness, external review completion, IoT field readiness, or certification without checked-in evidence.
- Rust and C must remain byte-compatible at the vector, transcript, wire-header, TLV, proof, KDF, MAC, and protocol-state-machine boundaries.
- Normal `AUTH` must remain proof of prior enrollment. Unknown-device self-registration inside `AUTH` is not allowed unless a future checkpoint explicitly proves equivalent authorization, replay, privacy, and abuse resistance.
- The C implementation must preserve strict warnings and sanitizer-clean validation as release-gate evidence.
- Rust code must preserve `cargo fmt`, `cargo check`, `cargo test`, and `cargo clippy -D warnings` evidence as release-gate evidence.
- Fuzz targets and formal models are evidence producers, not proof of complete security by themselves.
- Hermes agents may improve clarity, tests, scripts, and validation, but must not weaken assurance-status truthfulness.



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

## Commander Mode Policy

| Work type | Required Commander mode |
|---|---|
| Documentation cleanup, repo graph, roadmap alignment, validation script wrappers | Council alternating mode is acceptable |
| CI repair without semantic protocol changes | Alternating mode, followed by final verification |
| Replay tests, wire parsing, transcript generation, proof verification, KDF/MAC behavior, RNG/DRBG handling | Checkpoint competition |
| C memory-safety changes, unsafe Rust changes, sanitizer findings, fuzz-crash fixes | Checkpoint competition |
| Cross-language compatibility changes | Checkpoint competition |
| Release-candidate gate review | Checkpoint final verification |

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

## zk201 — Baseline and Repo Graph

Goal: make the combined repository navigable and establish baseline evidence without modifying protocol semantics.

Required artifacts:

| Artifact | Path |
|---|---|
| Combined repo README | `README.md` |
| Combined roadmap | `docs/improvement-roadmap.md` |
| Rust lane preserved | `rust/` |
| C lane preserved | `c/` |
| Parent validation scripts | `scripts/` |
| Commander repo graph | `shared-context/repo-map/graph.json` |
| Commander graph report | `shared-context/repo-map/GRAPH_REPORT.md` |

Acceptance gates:

- Both implementation lanes are present and independently buildable.
- The Rust test-vector path is documented as the C interop anchor.
- Hermes repo graph can be generated for the parent repository.
- No production/security-certification claims are introduced.

Suggested Commander task:

```text
Build and inspect the repository graph for the combined ZK-ARCHE Rust/C workspace. Identify validation commands, security docs, test-vector anchors, fuzz targets, and formal-model files. Do not change protocol semantics.
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

Suggested Commander task:

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
- Any semantic change requires checkpoint competition and final verification.

Suggested Commander task:

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
- Any change to enrollment trust semantics requires checkpoint competition.

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
- C memory handling and Rust secret handling are reviewed in checkpoint competition.

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

Suggested Commander task:

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

Suggested Commander task:

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

Suggested Commander task:

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

Suggested Commander task:

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

Suggested Commander task:

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

Suggested Commander task:

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

Suggested Commander task:

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

Suggested Commander task:

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

Suggested Commander task:

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

Suggested Commander task:

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

Suggested Commander task:

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

Suggested Commander task:

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

Suggested Commander task:

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

Suggested Commander task:

```text
Research an optional post-quantum hybrid AUTH suite. Keep it suite-negotiated and non-mandatory for iot-core, measure exact byte/RAM/CPU impact, and require downgrade tests, hybrid-KDF design review, and external cryptographic review before implementation claims.
```

## Agent Editing Contract

Future automated edits may improve implementation quality, validation, documentation, and traceability, but must preserve:

- Rust/C byte-level compatibility;
- the Rust vector corpus as the primary checked-in interop anchor unless a checkpoint-approved migration occurs;
- strict separation between evidence and claims;
- truthful assurance-status wording;
- no production/security/certification claims without evidence;
- checkpoint competition for protocol, crypto, parsing, replay, RNG, memory-safety, formal-model, side-channel, and interop changes.
