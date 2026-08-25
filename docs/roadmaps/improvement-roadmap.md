# ZK-ARCHE Unified Rust/C Improvement Roadmap

This roadmap defines the evidence-gated improvement path for the unified ZK-ARCHE Rust and C repository. It is the canonical long-horizon engineering plan for protocol maturity, constrained-device viability, interoperability, privacy, lifecycle security, data sovereignty, and P2P zero-trust operation.

It is deliberately **evidence-gated rather than calendar-gated**. A phase exists to define what must be implemented and what evidence must exist before the associated capability or maturity claim can advance. Research findings, standards drafts, papers, or prototypes do not become roadmap requirements automatically. Promotion occurs only through explicit human-reviewed roadmap updates.

## 1. Roadmap authority and evidence flow

Use the documentation layers as follows:

```text
external source / hypothesis
        ↓
docs/research/daily/
        ↓
docs/research/backlog.md
        ↓ human-reviewed synthesis
docs/findings/week-of-*-findings.md
        ↓ explicit human promotion
docs/roadmaps/                         ← this document
        ↓ bounded implementation intent
docs/requests/week-of-*-request.md
        ↓ consequential decision?
docs/adr/
        ↓ normative behavior
spec/
        ↓
rust/ + c/ + tests/vectors
        ↓
docs/assurance/ + retained evidence
        ↓
docs/release/
```

The arrows are gates, not automatic transitions. Weekly findings can strengthen or weaken a roadmap hypothesis without changing implementation scope. Weekly requests define bounded execution work; they must not silently rewrite protocol policy.

## 2. Current evidence baseline — 2026-08-25

The repository currently has two active implementation lanes:

| Lane | Path | Role |
|---|---|---|
| Rust reference | `rust/` | canonical deterministic vectors, protocol implementation, fuzz/formal tooling, higher-level validation |
| C implementation | `c/` | independent C11/libsodium implementation, constrained-device anchor, cross-language validation |

Canonical byte-level interoperability remains anchored by the Rust vectors under `rust/test-vectors/0x0001/`, with C expected to reproduce the same wire, transcript, proof, KDF, MAC/key-confirmation, and state-machine semantics wherever both lanes implement the same feature.

The active debt ceiling remains:

| Debt | Blocking gap | Roadmap ownership |
|---|---|---|
| TD-001 | independent cryptographic review of the custom role-membership proof | zk207 / zk209 / zk217 / zk225 / zk226 |
| TD-002 | reproducible STM32/ESP32-S3-class target measurements | zk208 / zk216 / zk222 / zk236 / zk241 |
| TD-003 | formal models remain skeletons without full model-to-code traceability | zk207 / zk209 / zk217 / zk218 |
| TD-004 | RFC-style normative specification remains incomplete | zk217 / zk218 / zk225 / zk226 / zk230 |

The current research backlog contains R-001 through R-015. The strongest human-reviewed findings now affecting roadmap acceptance criteria are:

- custom proof review must be property-driven rather than a generic “get review” task;
- MCU evidence must pin cryptographic execution context, entropy/key-storage posture, accelerator path, restart/rollback assumptions, and raw resource measurements;
- authentication, authorization, and trust mutation must be distinct semantics;
- profile, registry, and extension behavior need compatibility rules and anti-ossification tests;
- replay, key usage, rekey, resumption, revocation, and channel binding must be treated as one lifecycle problem;
- formal assurance needs one canonical or mechanically synchronized state model plus explicit privacy properties;
- resumption must revalidate authorization context and bound ticket/PSK reuse;
- revocation must converge across disconnected peers with an explicit stale-authorization bound;
- protocol conformance and whole-product/field readiness must remain separate claims;
- BBS and PQ/hybrid work remain optional research profiles until measured and reviewed.

These findings sharpen existing phases; they do not claim those phases are implemented.

## 3. Non-negotiable boundaries

- Do not change cryptographic primitives, domain separators, packet formats, suite identifiers, test-vector meanings, replay semantics, credential/trust semantics, or wire compatibility without checkpoint-style review and retained evidence.
- Rust and C must remain byte- and decision-compatible wherever both lanes claim the same protocol behavior.
- Normal `AUTH` is **NO-LEARNING**: it proves possession/authorization relative to already trusted state and must not create or expand trust as a side effect.
- Trust-store mutation belongs only to explicit ENROLL, commissioner, grant, rekey/re-registration, or equivalent reviewed state-transition flows.
- Authentication does not by itself imply authorization. Authorization must be scoped, time/epoch bounded, revocable, audience/deployment bound, and tied to the authenticated holder or session context.
- `iot-core` and `p2p-iot-core` remain the constrained interoperability floors. Heavy anonymous credentials, post-quantum hybrids, large certificate-chain parsers, full trust-graph engines, and general-purpose SNARK/STARK provers cannot become mandatory MCU dependencies without explicit future charter change and target evidence.
- Hardware acceleration may optimize implementation behavior but must never define protocol correctness.
- Fuzzing, formal models, benchmarks, and simulation are evidence producers; none alone establishes complete security.
- Protocol conformance is not field readiness. Product/platform lifecycle, provisioning authority, secure storage, update posture, debug/boot posture, external controls, and residual assumptions must be identified separately before deployment claims.

## 4. IoT capability and evidence contract

| Class | Representative targets | Expected role |
|---|---|---|
| MCU-core | STM32-class bare-metal/RTOS | constrained peer/client, tiny trust store, bounded/fixed-buffer authentication |
| MCU-plus | ESP32/ESP32-S3-class RTOS | constrained peer, optional commissioner-lite or small registry when measured |
| Linux-edge | Raspberry Pi-class Linux | peer/server, commissioner, local gateway, benchmark/interop harness |
| Accelerated-edge | Jetson Orin-class Linux | peer/gateway/server, large-registry work, formal/fuzz/review artifact generation |

Before any target/profile maturity claim, retain a versioned benchmark manifest containing at minimum:

```text
target + board revision
build/toolchain profile
implementation/library versions
crypto boundary
accelerator family/microarchitecture + software fallback
entropy source + health-test posture
DRBG/reseed behavior
key-generation mode
root-seed vs private-key storage representation
seed/key self-test posture where applicable
secure-storage location
secure-boot/debug state
zeroization assumptions
rollback/clone/reprovision assumptions
wire bytes
stack / heap / static RAM / flash
CPU/latency measurements
registry/trust scaling where affected
replay/restart behavior
dependency inventory
```

Measurements without this context are implementation observations, not portable profile evidence.

## 5. Review policy

| Work type | Minimum review posture |
|---|---|
| Documentation cleanup, roadmap alignment, wrappers | lightweight review |
| CI repair without protocol semantics | normal review + final verification |
| Wire parsing, negotiation, transcript, proof, KDF/MAC, replay, resumption, authorization, trust mutation, RNG/DRBG | checkpoint-style review |
| C memory safety, unsafe Rust, fuzz/sanitizer crash corrections | checkpoint-style review |
| Cross-language semantic/vector changes | checkpoint-style review |
| Release-candidate or field-readiness claim | independent evidence review |

## 6. Phase map

| Phase | Purpose | Exit evidence focus |
|---|---|---|
| zk201 | Unified repository baseline and validation inventory | reproducible repository map and validation entry points |
| zk202 | Parent-level CI wrappers and evidence normalization | clean-checkout Rust/C validation |
| zk203 | Replay-test automation and negative-case coverage | deterministic replay/reorder/restart fixtures |
| zk204 | Fuzzing automation, corpus layout, crash triage | retained corpus + reproducible crashes/fixes |
| zk205 | Rust/C deterministic-vector parity | byte- and decision-compatible vectors |
| zk206 | Provisioning/TOFU hardening | explicit trust-establishment semantics and negative tests |
| zk207 | Formal-model expansion and implementation traceability | canonical state model + property/attacker matrix + traceability |
| zk208 | Side-channel, RNG, key-storage, and execution-context evidence | target-specific crypto execution manifests and review boundaries |
| zk209 | External review package and reproducibility bundle | proof-review package + reproducible assurance bundle |
| zk210 | Release-candidate evidence gate | claim matrix bounded by actual evidence |
| zk211 | Signed one-time late-enrollment grants | scoped/replay-safe grant semantics + vectors |
| zk212 | Delegated commissioner enrollment | authority-limited commissioner flow + audit evidence |
| zk213 | Authenticated rekey and re-registration | atomic lineage replacement + rollback/replay tests |
| zk214 | Enrollment replay, abuse, authorization, and revocation controls | authn/authz separation + convergence/freshness tests |
| zk215 | Optional privacy-preserving credential research | benchmark/review only; no baseline promotion |
| zk216 | IoT profile matrix and benchmark harness | target manifests + exact profile resource budgets |
| zk217 | AUTH transcript v3 and complete context binding | selected profile + security context + transcript mutation coverage |
| zk218 | Strict AUTH state machine and observable-failure behavior | sequence/retry/error/privacy negative tests |
| zk219 | Stateless `AUTH_RETRY` and unauthenticated-work throttling | bounded pre-auth cost + privacy/DoS evidence |
| zk220 | Optional encrypted lookup hints | bounded prototype + privacy/rotation/DoS analysis |
| zk221 | Replay-safe 1-RTT session resumption | authorization-aware resumption record + invalidation/reuse tests |
| zk222 | AUTH metrics CI and assurance dashboard | reproducible metrics tied to manifests and claim boundaries |
| zk223 | Optional anonymous-credential migration evaluation | exact footprint/privacy/review comparison |
| zk224 | Optional PQ hybrid suite research | exact packet/resource/fragmentation/downgrade evidence |
| zk225 | Rust/C interop hardening and vector governance | registry/profile/extension compatibility corpus |
| zk226 | RFC-style specification package and registry discipline | normative grammar/state/registry/error text backed by evidence |
| zk227 | EDHOC/CoAP/OSCORE-inspired constrained-profile research | measured comparator, not dependency by analogy |
| zk228 | TLS/mTLS exporter-bound channel binding | unique AUTH-instance exporter context + negative fixtures |
| zk229 | DTLS-style datagram robustness | retry/replay/retransmit/CID-like behavior and privacy tests |
| zk230 | CORE/AUTH/LINK/TRUST/BIND/ENROLL/DATA decomposition | explicit ownership and cross-module lifecycle contracts |
| zk231 | Per-device data sovereignty architecture | threat/policy model and constrained ownership boundaries |
| zk232 | ZK-minimal proof-carrying data profile | bounded primitives and resource evidence |
| zk233 | Minimal `ZK-ARCHE-DATA` commit/release flow | deterministic positive/negative flows |
| zk234 | Policy-bound release tokens and revocable epochs | holder/audience/purpose/epoch lineage and revocation tests |
| zk235 | Local audit hash chain and transparency bridge | continuity/recovery/tamper evidence |
| zk236 | Sovereignty CI gates and footprint budgets | profile-specific measured budgets + deployment context |
| zk237 | Channel-bound sovereignty over secure transports | exporter/channel binding to release context |
| zk238 | Advanced sovereignty research | research-only isolation from constrained baseline |
| zk239 | P2P zero-trust trust graph and mutual authentication | non-transitive scoped trust + revocation/lineage evidence |
| zk240 | P2P IoT profile contract | constrained floor + explicit selected-profile semantics |
| zk241 | Conservative mandatory crypto baseline | reviewed portable floor with measured target evidence |

## 7. Dependency and priority sequence

Phase numbers identify ownership; they do not imply that every phase must execute strictly serially. The near-term dependency graph is:

```text
zk201–zk205 reproducible baseline
        ↓
zk207 formal traceability ───────┐
zk208 target execution evidence ├─→ zk209 external-review bundle → zk210 claim gate
zk206 trust semantics ───────────┘

zk211–zk214 enrollment / authorization / revocation lifecycle
        ↓
zk216 profile evidence
        ↓
zk217 transcript context
        ↓
zk218 state machine / failure privacy
        ↓
zk219 retry + zk220 lookup + zk221 resumption
        ↓
zk225 vector governance
        ↓
zk226 normative specification
        ↓
zk227–zk230 transport/profile/decomposition work
```

Data-sovereignty and P2P work may prototype in parallel, but neither may claim specification-grade maturity while the shared AUTH/TRUST/LINK lifecycle remains underspecified.

## 8. Baseline and assurance work — zk201–zk210

### zk201–zk205: reproducible implementation truth

The repository must have one clean-checkout path that reproduces Rust validation, C validation, canonical Rust vectors, C consumption of those vectors, parser/state negative tests, fuzz entry points, and evidence generation. Vector changes must be versioned and reviewed as protocol-impacting changes.

### zk207: formal assurance contract

Formal work must begin from a property/attacker matrix, not from tool choice alone. The target evidence includes:

- one canonical protocol state model or a mechanically synchronized model source;
- secrecy, authentication/agreement, replay/lifecycle, credential/reference misbinding, anonymity, and unlinkability properties where in scope;
- explicit compromise models;
- observable failure/abort behavior where privacy depends on it;
- retained successful proofs and counterexamples;
- mapping from model states/events to concrete Rust/C functions, fields, vectors, and persistent-state transitions.

Symbolic results must not be reported as proof of constant-time behavior, RNG quality, memory safety, or security of the custom role proof itself.

### zk208: execution-context and side-channel evidence

Target evidence must distinguish platform properties from measured ZK-ARCHE properties. Record crypto boundaries, accelerators, seed/private-key representation, entropy/DRBG posture, secure storage, zeroization, restart/rollback assumptions, and implementation versions. Define which operations are expected constant time and which dependencies provide those guarantees.

### zk209: independent review package

TD-001 must be converted into a concrete cryptographic review target covering at minimum:

- statement and witness relation;
- completeness and soundness/extractability assumptions;
- simulator behavior for OR composition;
- Fiat-Shamir challenge construction and domain separation;
- canonical serialization and role ordering;
- protocol/instance identifiers;
- scalar sampling and invalid-point/identity handling;
- witness-dependent control flow / constant-time boundaries;
- Rust/C deterministic and negative-vector parity.

Review findings must be retained, dispositioned, and converted into regression evidence when representable.

### zk210: claim gate

The release gate must separate at least:

```text
implementation tests pass
protocol/vector conformance
cryptographic review status
formal-model status
constrained-target evidence
platform/product deployment context
field evidence
certification status
```

No lower-level success may be summarized as a stronger claim.

## 9. Enrollment, authorization, and lifecycle — zk211–zk215

### Core semantic split

ZK-ARCHE must model three different operations:

1. **Authentication** — prove possession/credential/role membership relative to existing trusted state.
2. **Authorization** — decide what the authenticated peer may do now, under explicit audience, scope, role/policy, validity, epoch, and revocation state.
3. **Trust mutation** — create, replace, delegate, revoke, or otherwise change trusted state through explicit authorized workflows.

Normal AUTH is NO-LEARNING and cannot perform (3).

### zk211–zk212: enrollment grants and commissioners

Late enrollment remains separate from normal AUTH. Grants should be one-time/scoped and bind the intended peer/holder where possible, audience/deployment/domain, suite/profile compatibility, role/policy scope, issuer/authority, validity, nonce/replay state, and authorization lineage. Commissioner authority must be bounded, auditable, and non-transitive unless separately authorized.

### zk213: rekey and lineage replacement

Rekey/re-registration must prove control of the current credential and new key, bind the operation to the current authorized session/context, and atomically replace or tombstone the predecessor. Negative evidence must cover replay, rollback, lost predecessor state, concurrent replacement, partial write, privilege expansion, and recovery.

### zk214: revocation as convergence

Revocation is not a local delete event. Offline-capable profiles need a versioned issuer/epoch-scoped revocation view with enough state to:

- reconcile full and differential updates;
- survive missed notifications and reconnect;
- detect rollback or stale snapshots;
- define a maximum stale-authorization window or freshness objective;
- invalidate dependent sessions, resumption credentials, derived keys, cached authorization, and DATA release state where applicable.

Always-online infrastructure must not become mandatory for the constrained baseline unless a future profile explicitly chooses that tradeoff.

### zk215: advanced credentials remain isolated

BBS/selective-disclosure or other advanced credentials remain optional research until exact proof/signature bytes, pairing/CPU/RAM/flash cost, dependency maturity, revocation/issuance model, privacy gain, and external-review status justify a versioned optional suite.

## 10. AUTH hardening — zk216–zk224

### zk216: profile and benchmark contract

Distinguish:

- local runtime/resource configuration;
- negotiated protocol/security profile;
- optional capability/extension bits.

A profile claim must include exact/bounded wire bytes, memory, CPU, persistent-state behavior, dependency cost, registry scaling, and the `crypto_execution_context` manifest defined above.

### zk217: transcript v3

Every security-relevant semantic input should be unambiguously bound, including as applicable:

```text
protocol version
suite / method / selected profile
capabilities and critical extensions
session / sequence identifiers
ephemeral keys
peer/server identity or commitment
role/policy and authorization context
deployment/domain and audience
transport/channel-binding label
fresh AUTH-instance identifier where required
canonical payload bytes
```

A profile/capability distinction must be explicit enough that a high-end peer cannot negotiate broad capabilities and then execute semantics inconsistent with the selected constrained profile.

### zk218: state machine and privacy observability

The state machine must reject wrong type, wrong sequence, stale/cross-session messages, reflected messages, invalid retransmissions, unsupported critical selections, and wrong peer/address bindings. Error classes, response/no-response behavior, size buckets, retry behavior, and timing must be considered part of privacy assurance rather than purely operational detail.

### zk219: retry/source validation

`AUTH_RETRY` should provide a stateless or bounded MAC-based source-validation gate before expensive registry scans or proof verification in exposed datagram deployments. Evidence must include amplification bounds, CPU asymmetry, retry-token lifetime/replay behavior, malformed/unknown-device equivalence where intended, and interaction with privacy-normalized failures.

### zk220: optional encrypted lookup hint

The first prototype path may use a randomized standard-primitive encrypted opaque registry key as a **non-authoritative prefilter**. Full PID/proof/possession/transcript/state verification remains mandatory. Promotion requires exact wire/CPU/RAM cost, key-epoch rotation/revocation behavior, retry placement, passive-linkability analysis, and Rust/C deterministic/negative vectors.

### zk221: authorization-aware resumption

A resumption credential is not merely a secret. The future record should bind enough state to prove that privilege remains valid, such as:

```text
peer/holder
deployment/domain/audience
suite/profile
original security context
authorization lineage/generation
policy/registry/revocation epoch
issue + expiry
reuse/usage limits
privacy-relevant identifier state
```

On resumption, changed context that can affect authorization must be reevaluated. If safe reevaluation is impossible or required cached state is missing, fall back to full AUTH. Negative tests must cover revoked credentials, stale lineage, changed role/policy/audience/deployment, missing cache, rollback, excessive reuse, and repeated-identifier linkability.

General-purpose state-changing 0-RTT remains out of scope for the constrained baseline.

### zk222: metrics and evidence

Metrics must be tied to reproducible manifests and claim boundaries. Dashboards must not convert “test passed” into “secure,” “formally verified,” or “field ready.”

### zk223–zk224: optional advanced suites

Anonymous credentials and PQ hybrids remain research/edge profiles until target measurements and interoperability/review evidence justify promotion. PQ work must account for packet/MTU/fragmentation/loss pressure and downgrade/method-suite compatibility, not only primitive availability.

## 11. Rust/C vector and registry governance — zk225

Rust remains the canonical checked-in vector source unless a future reviewed migration changes that policy. C remains an independent implementation lane.

Required gates include:

- C passes current canonical Rust vectors;
- vector regeneration has no unexplained drift;
- wire/transcript/proof/KDF/MAC/state changes produce versioned vectors;
- unknown non-critical extension values remain ignorable where specified;
- unsupported critical selections fail closed;
- suite/method/profile compatibility is tested, not merely documented;
- deterministic GREASE-style reserved-value fixtures exercise extension points so they do not ossify;
- Rust/C produce the same accept/reject decision for the conformance corpus.

Live random GREASE traffic is optional and must respect constrained-link budgets; deterministic CI coverage is the required baseline.

## 12. Specification maturity — zk226–zk230

ZK-ARCHE should evolve toward an RFC-like package with normative grammar, registries, state machines, security/privacy considerations, implementation requirements, and deterministic positive/negative vectors.

Target suite ownership remains:

```text
ZK-ARCHE-CORE   wire format, canonical encoding, registries, transcript rules
ZK-ARCHE-AUTH   native device/role authentication and P2P mutual authentication
ZK-ARCHE-LINK   secure association, replay, key lifecycle, resumption, export
ZK-ARCHE-TRUST  trusted records, scoped authorization evidence, lineage, revocation
ZK-ARCHE-BIND   transport/channel bindings
ZK-ARCHE-ENROLL setup, late enrollment, commissioner grants, rekey/revocation
ZK-ARCHE-DATA   encrypted data records, policy-bound release, auditability
```

Every normative security behavior needs at least one of: positive vector, negative vector, executable test, scoped formal-model result, reviewed proof argument, or explicit evidence gap.

EDHOC/OSCORE, TLS/mTLS, DTLS, ACE, BRSKI/FDO/Matter, and related protocols are engineering comparators. They do not become dependencies merely because they motivate a requirement.

## 13. Transport and channel-binding work — zk227–zk230

### EDHOC/OSCORE-inspired constrained work

Use measured comparison for handshake bytes, flights, exporter/context semantics, retransmission behavior, parser/dependency footprint, and MCU resource cost. Do not optimize for “smaller than EDHOC”; explain what additional ZK-ARCHE bytes/cycles provide in privacy/authorization terms.

### TLS exporter binding

A future TLS binding must be unique to the **ZK-ARCHE AUTH instance**, not only to the underlying TLS connection. Define a normative exporter label/context including application/ALPN, deployment/domain, endpoint identity or commitment, suite/profile, fresh AUTH-instance identity, and relevant transcript state. Cross-instance, cross-application, wrong-endpoint, proxy/termination, and TLS-resumption cases need deterministic fixtures.

### Datagram robustness

Native UDP/DTLS-style behavior must define retry/source validation, replay windows, duplicate handling, retransmission, bounded response caches, session/epoch identity, reordering behavior, address changes where supported, amplification limits, and privacy-aware error behavior.

## 14. Data sovereignty — zk231–zk238

Per-device data sovereignty means a device cryptographically controls release of protected data by recipient/holder, audience, purpose, data type, policy, time/epoch, and revocation state. Protected data is encrypted by default and release authorization remains separate from authentication.

The constrained proof floor remains bounded primitives such as Schnorr possession proofs, small role-membership proofs, MAC/signature authorization tickets, hash commitments, and measured fixed-depth inclusion proofs. General-purpose circuits and heavyweight credentials remain optional research.

A minimal flow may include `DATA_COMMIT`, `RELEASE_REQUEST`, `RELEASE_PROOF`, `RELEASE_KEY`, and `AUDIT_APPEND`. Promotion requires parser/replay/policy mutation/epoch invalidation/protected-plaintext/fixed-buffer tests plus target resource evidence.

Field-readiness evidence for sovereignty must include deployment context: platform update posture, provisioning/revocation authority, storage protections, external controls, and residual trust assumptions.

## 15. P2P zero trust — zk239–zk241

P2P mode uses per-handshake `initiator` and `responder` roles rather than permanent client/server identity roles. Both sides authenticate, confirm the same context, and locally evaluate scoped trust evidence.

Trust is not implicitly transitive. Delegation must be scope-, role-, audience-, depth-, validity-, and epoch-limited and revocable. Credential/reference mappings belong in the threat model; a reference cannot be treated as security identity unless it is bound to the exact key/commitment, authorization context, deployment/audience, epoch, and allowed operations.

Every implementation claiming P2P interoperability must support `p2p-iot-core` without requiring gateway-class trust engines or heavyweight crypto on MCUs. High-end peers adapt downward to the constrained floor.

The mandatory crypto baseline remains conservative and reviewable. Optional alternative suites must not silently redefine the constrained floor.

## 16. Roadmap completion and claim rules

A roadmap phase is not “done” because code exists. Completion means the phase’s declared evidence exists and the repository claim language matches that evidence.

Use these distinctions consistently:

```text
DESIGNED       architecture/spec intent exists
IMPLEMENTED    code path exists
TESTED         deterministic tests pass
INTEROPERABLE  Rust/C or declared peer implementations agree
MEASURED       target/resource evidence exists
FORMALLY ANALYZED scoped model result exists
EXTERNALLY REVIEWED independent review exists
DEPLOYMENT-QUALIFIED platform/product context and required field evidence exist
```

Never infer a stronger state from a weaker one.

## 17. Agent editing contract

Future automated edits must preserve:

- Rust/C byte-level and decision compatibility;
- canonical vector governance;
- NO-LEARNING normal AUTH;
- separation of authentication, authorization, and trust mutation;
- `iot-core` and `p2p-iot-core` constrained interoperability floors;
- strict evidence/claim separation;
- no production/security/certification/field-readiness claims without corresponding evidence;
- no mandatory SNARK/STARK, advanced credential, or PQ work unless a future explicit charter reopens the constrained baseline;
- checkpoint-style review for protocol, crypto, parsing, negotiation, replay/resumption, authorization/revocation, RNG/key lifecycle, memory safety, formal-model, privacy, side-channel, and interoperability changes.
