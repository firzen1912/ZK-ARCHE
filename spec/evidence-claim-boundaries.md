# ZK-ARCHE Evidence and Claim Boundaries

Status: **normative claim-governance draft**.

This document defines when repository and release language may use ZK-ARCHE maturity and evidence claims. It does not allocate wire behavior, change protocol semantics, promote any draft profile, satisfy TD-001 through TD-004, or imply IETF/RFC status.

The purpose is to prevent a structurally valid artifact, successful local check, or partial implementation from being promoted into a stronger assurance claim than the evidence supports.

## 1. General rule

Claim classes are independent evidence states. Evidence for one class MUST NOT be treated as evidence for another class unless the stronger class has its own required evidence.

The canonical claim classes are:

```text
IMPLEMENTED
TESTED
INTEROPERABLE
COMMON-CONFORMANT
MEASURED
FORMALLY ANALYZED
EXTERNALLY REVIEWED
RFC-CLASS DOCUMENTED
DEPLOYMENT-QUALIFIED
```

A repository document, release note, dashboard, qualification manifest, or automated report MUST NOT infer a stronger claim merely from commit presence, line count, elapsed effort, a passing subset of tests, schema validity, successful parsing, or the existence of an evidence file.

## 2. IMPLEMENTED

`IMPLEMENTED` means the claimed behavior materially exists in the identified implementation and revision.

Minimum claim evidence:

- exact implementation lane and revision are identifiable;
- the behavior is reachable under the claimed profile/configuration;
- normative semantics are not contradicted by the implementation;
- known missing counterpart implementations or unsupported profiles are stated.

`IMPLEMENTED` does not imply `TESTED`, `INTEROPERABLE`, `MEASURED`, `FORMALLY ANALYZED`, `EXTERNALLY REVIEWED`, `COMMON-CONFORMANT`, `RFC-CLASS DOCUMENTED`, or `DEPLOYMENT-QUALIFIED`.

## 3. TESTED

`TESTED` means the identified behavior was actually exercised by the stated test or qualification lane against the stated revision.

A TESTED claim MUST identify, directly or through retained evidence:

- what command/lane executed;
- what implementation/revision it exercised;
- what test surface was in scope;
- whether the result was pass, fail, or unavailable.

A test that was not executed in the current environment MUST NOT be reported as passed. Tooling or cloud-runner unavailability is `UNAVAILABLE`/`UNKNOWN`, not failure and not success.

A narrow passing test MUST NOT be generalized into a whole-repository TESTED claim.

## 4. INTEROPERABLE

`INTEROPERABLE` means at least two independently maintained implementation lanes actually exchanged or reproduced the same required protocol behavior for the claimed surface.

Minimum claim evidence includes the applicable combination of:

- byte-compatible canonical vectors or wire traces;
- matching accept/reject decisions;
- successful cross-language/session execution where the claimed behavior requires live interaction;
- negative interoperability cases for malformed, stale, replayed, downgraded, or misbound inputs when those cases are part of the claimed surface.

Source-level similarity, shared fixtures, or two implementations compiling independently is not interoperability evidence by itself.

## 5. COMMON-CONFORMANT

`COMMON-CONFORMANT` means the implementation satisfies the currently declared mandatory ZK-ARCHE Common Contract for the claimed profile and target class.

The claim requires evidence that the mandatory floor remains intact across the relevant constrained/higher-capability peer combinations. At minimum, the claimed surface must preserve:

- local verification of mandatory authentication;
- authentication/authorization/trust-mutation separation;
- NO-LEARNING normal AUTH;
- local, non-transitive trust absent explicit accepted delegation;
- mandatory replay/freshness and binding semantics;
- fail-closed negotiation of mandatory security;
- infrastructure independence where the profile claims it;
- transport identity separation;
- bounded resource behavior appropriate to the claimed conformant target.

Passing one Common Contract property checker, one profile test, or one host-side simulation MUST NOT be promoted to full COMMON-CONFORMANT status.

## 6. MEASURED

`MEASURED` means the claimed quantitative result was observed in the identified execution context and retained with sufficient provenance to reproduce or audit the observation.

A MEASURED constrained-target claim MUST identify the applicable evidence context, including:

- physical target and board revision when the claim is physical-target specific;
- build/toolchain profile;
- implementation/library versions;
- crypto execution path and accelerator/software fallback;
- entropy/RNG and key-storage posture relevant to the measurement;
- wire, memory, flash, CPU/latency, or other claimed quantities;
- restart/rollback/persistence context where lifecycle behavior affects the claim.

A syntactically valid benchmark or qualification manifest is not a measurement. Host-side observations MUST NOT be reported as STM32/ESP32-S3-class measurements unless they were actually obtained on the claimed target.

If a measured qualification record declares `PASS`, every qualification predicate required by that profile's measurement contract MUST itself have been executed and satisfied. Missing physical evidence remains missing evidence; it must not be inferred from schema completeness.

## 7. FORMALLY ANALYZED

`FORMALLY ANALYZED` means the exact retained model, exact query/property set, exact tool/version, and stated attacker assumptions were actually analyzed.

The claim MUST identify the scope of the formal result. A model edit requires a fresh result before the edited model inherits the claim.

Formal analysis MUST NOT be described as proof of properties outside the model, including unless explicitly modeled and justified:

- constant-time behavior;
- RNG/entropy quality;
- Rust/C memory safety;
- computational soundness of the custom proof;
- persistence atomicity;
- physical rollback resistance;
- secure erasure;
- target resource limits;
- field readiness.

A symbolic PASS does not imply `MEASURED`, `EXTERNALLY REVIEWED`, or `DEPLOYMENT-QUALIFIED`.

## 8. EXTERNALLY REVIEWED

`EXTERNALLY REVIEWED` requires actual independent review by a reviewer or review body outside the implementation authorship boundary for the claimed scope.

The claim requires retained review scope and disposition of findings. Internal review, automated analysis, model checking, fuzzing, unit tests, or self-authored assurance documents do not satisfy this state.

TD-001 remains open until the required independent cryptographic review of the custom role-membership proof exists and findings are dispositioned.

## 9. RFC-CLASS DOCUMENTED

`RFC-CLASS DOCUMENTED` means a complete standards-style specification package exists for the claimed protocol surface and is independently implementable from normative text rather than source code.

The claim requires, as applicable:

- precise terminology and applicability;
- normative wire grammar/canonical encoding;
- complete state machines and error behavior;
- cryptographic computations and key schedule;
- replay/retry/rekey/resumption lifecycle;
- negotiation, downgrade, and extension rules;
- registries and change-control policy;
- enrollment, authorization, revocation, and trust semantics;
- Security and Privacy Considerations;
- implementation and constrained-resource requirements;
- positive and negative vectors/traces;
- conformance language tied to executable evidence;
- known limitations and unresolved gaps.

RFC-like formatting, BCP 14 keywords, or a complete-looking prose document does not establish this claim if required normative behavior or independent-implementation evidence is still missing.

`RFC-CLASS DOCUMENTED` never means `RFC`, `Internet Standard`, or IETF adoption.

## 10. DEPLOYMENT-QUALIFIED

`DEPLOYMENT-QUALIFIED` is a platform/product claim, not merely a protocol claim.

It requires the protocol evidence needed by the selected profile plus the applicable deployment context, including where relevant:

- provisioning and enrollment authority;
- secure storage and rollback assumptions;
- entropy and key-generation posture;
- secure boot/debug/update posture;
- power-loss/restart behavior;
- target resource margins;
- operational revocation and recovery;
- platform integration and transport assumptions;
- residual risks and external controls;
- field or product qualification evidence required by the deployment.

Protocol conformance, a successful handshake, host-side tests, formal results, or physical benchmark measurements alone MUST NOT be reported as DEPLOYMENT-QUALIFIED.

## 11. Evidence provenance and exact-head language

Where a claim depends on execution, the retained evidence SHOULD identify the exact tested revision. If exact-current execution is unavailable, reports MUST preserve the distinction between:

```text
project/local baseline previously confirmed green
exact-current execution passed
exact-current execution failed
exact-current execution unavailable
```

A previously green local baseline may remain valid as project context, but it MUST NOT be rewritten as a fresh exact-head PASS when the current environment did not execute the lane.

The intentional absence of hosted GitHub Actions on `dev` is not a failure condition. It also does not create evidence. Exact-head claims on `dev` depend on repository-owned/local evidence that actually ran for that head.

## 12. Claim composition

Claims may be combined only when every named state independently satisfies its predicate.

For example:

```text
IMPLEMENTED + TESTED
```

is valid only when the behavior exists and the claimed test actually executed against the relevant revision.

Likewise:

```text
INTEROPERABLE + MEASURED
```

requires both interoperability evidence and actual measurement evidence. Neither state supplies the other.

If an artifact is partial, the claim MUST be scoped to that partial surface rather than elevated to the entire protocol, repository, profile, target class, or deployment.

## 13. Fail-closed reporting rule

When evidence is missing, stale, unexecuted, contradictory, or outside the claimed scope, reporting MUST reduce the claim to the strongest state actually supported.

Automation MUST NOT manufacture green by:

- suppressing or deleting failing tests;
- narrowing gates solely to avoid executing required evidence;
- treating unavailable tools as a PASS;
- converting a schema-valid manifest into a measurement claim;
- inheriting a formal result after changing the model;
- calling Rust/C source parity interoperability without executable evidence;
- calling a host test physical-target evidence;
- calling protocol conformance deployment readiness.

This rule governs claim language only. It does not define new protocol wire behavior.

## 14. Current unresolved evidence gates

This document does not close the current roadmap blockers. In particular:

- TD-001 independent cryptographic review remains required;
- TD-002 physical constrained-target evidence remains incomplete;
- TD-003 complete formal coverage and model-to-code traceability remain incomplete;
- TD-004 RFC-class normative package remains incomplete;
- complete Common Contract executable qualification remains incomplete;
- complete deployment qualification is not claimed.

The roadmap and retained assurance evidence remain authoritative for whether a particular phase has enough evidence to advance its score.
