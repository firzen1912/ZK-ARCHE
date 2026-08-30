# AUTH-v3 Formal Traceability Supersession — 2026-08-29

Status: **scoped assurance reconciliation**.

This note updates the evidence interpretation of `docs/assurance/auth-v3-formal-traceability.md` and `docs/assurance/formal-model-contract.md` where later exact-head repository evidence has superseded dated statements. It does not change protocol behavior, the wire format, parser acceptance semantics, cryptographic computations, formal models, or any roadmap/debt completion state.

## 1. Superseded statement — parser hostile-count path

Section 4.1 of `docs/assurance/auth-v3-formal-traceability.md` retains an August 28 statement that the generic Rust canonical-context parser has a hostile-count resource asymmetry when called without a selected-profile bound.

That statement is no longer the correct active evidence boundary for the specifically tested malformed-count path.

The dated August 28 research report remains valid historical provenance. It must not be rewritten merely because later repository evidence superseded one hypothesis.

## 2. Current parser repository evidence

Current repository evidence establishes the following bounded facts.

### 2.1 Rust structural rejection occurs before heap materialization for the tested case

The current Rust parser checks whether the declared entry count can structurally fit in the remaining canonical-context bytes before calling `Vec::with_capacity(entry_count)`.

The dedicated regression:

```text
rust/crates/proto/tests/auth_v3_context_parser_allocation.rs
```

submits a nine-byte canonical-context header declaring `entry_count = 65535` with no encoded entries and requires:

```text
parse result = ContextParseError::Truncated
tracked allocation calls during parse = 0
tracked reallocation calls during parse = 0
```

This is direct hosted-test evidence for that malformed input. It is not a proof that every malformed count causes zero allocation.

### 2.2 Rust/C decision parity exists for the hostile-count fixture

The shared malformed canonical-context corpus includes the `entry_count = 65535` truncated-header case and fixes the expected decision as `TRUNCATED` for both implementation lanes.

This establishes Rust/C accept/reject-class compatibility for that fixture. It does not establish parser implementation equivalence.

### 2.3 Selected `iot-core` admission remains more tightly bounded

The selected `iot-core` authorization receive path continues to require the fixed profile contract before semantic use:

```text
canonical byte length = 148
entry count           = 7
profile_id            = 0x0001
context kind          = AUTHORIZATION
```

The Rust and C selected-profile paths therefore retain a stronger explicit bound than the generic parser surface.

### 2.4 C remains caller-bounded

The C canonical-context parser continues to operate over bounded caller-owned storage rather than allocating from the attacker-controlled entry count.

## 3. Corrected parser traceability interpretation

For AUTH-v3 formal traceability, the active boundary is now:

```text
attacker-controlled canonical-context bytes
        ↓
structural parser bounds before Rust heap materialization
        ↓
shared Rust/C malformed-input decision evidence
        ↓
selected-profile bounds where a profile is known
        ↓
concrete schema and semantic validation
        ↓
exact accepted-byte hashing / runtime handoff
        ↓
symbolic AuthorizationContextAdmittedV3 abstraction
```

The earlier hostile-count hypothesis is therefore **superseded for the tested `entry_count = 65535` truncated-header path**.

The stronger statements below remain unsupported and MUST NOT be inferred:

- generic Rust canonical-context parsing is allocation-free;
- all malformed entry-count values reject without allocation;
- Rust and C parsers are mechanically or formally equivalent;
- parser behavior and the symbolic ProVerif model are equivalent;
- bounded hosted behavior proves MCU RAM/heap behavior;
- physical STM32/ESP32-S3 resource limits are measured;
- parser resource exhaustion or denial-of-service resistance is comprehensively established;
- TD-002 or TD-003 is cleared;
- `COMMON-CONFORMANT`, `RFC-CLASS DOCUMENTED`, or `DEPLOYMENT-QUALIFIED` follows from this evidence.

## 4. Relationship to current research

`docs/research/daily/2026-08-29.md` explicitly records that the August 28 hostile-count hypothesis is superseded for the tested path and moves current formal research priority toward:

1. FM-06 peer-identity / credential-reference misbinding with an explicit attribution resolver relation; and
2. FM-22 dynamic-corruption timing and composition boundaries before any forward-secrecy or post-compromise claim expansion.

Those findings remain research/evidence inputs. This supersession note does not promote compromise, recovery, or lifecycle semantics into protocol requirements.

The FM-06 resolver dependency identified by that research has since been promoted through reviewed repository-owned specification, Rust/C implementation, shared negative decision evidence, synchronized formal modeling, and retained exact-head ProVerif qualification. Sections 7–9 below reconcile the dated formal-contract language with that newer evidence.

## 5. Parser evidence-state effect

This reconciliation advances only claim accuracy and model/runtime boundary traceability:

```text
hostile-count truncated-header decision parity     TESTED
Rust pre-allocation rejection for tested case      TESTED
selected iot-core parser/profile bounds             TESTED
parser ↔ symbolic-model equivalence                 NOT ESTABLISHED
physical constrained-target resource evidence       NOT ESTABLISHED
TD-002                                               OPEN
TD-003                                               OPEN
TD-004                                               OPEN
```

The retained AUTH-v3 ProVerif results remain scoped symbolic results.

## 6. Historical follow-on dependency

At the time of the original parser reconciliation, the next non-redundant dependency was a normative, implementation-independent identity-attribution resolver relation before FM-06 could be modeled without inventing trust semantics.

That dependency is now satisfied within a bounded scope by:

```text
spec/implementation-requirements.md §4.1
rust/crates/proto/src/auth_v3_iot_core_authz.rs
c/include/auth/auth_v3_iot_core_authz.h
c/src/proto/auth_v3_iot_core_authz.c
rust/test-vectors/auth-v3/iot-core-attribution-decisions-v1.txt
rust/crates/proto/tests/auth_v3_iot_core_attribution_corpus.rs
c/tests/test_auth_v3_iot_core_authz.c
```

This supersedes the older `BLOCKED-NORMATIVE` FM-06 readiness statement for the exact local-attribution correspondence now owned by the repository. It does not unblock broader UKS, delegation, lifecycle, or authority/provenance semantics.

## 7. Superseded statement — FM-06 and AUTH-v3 query count

`docs/assurance/formal-model-contract.md` currently retains two dated statements that are no longer the active evidence state:

1. the AUTH-v3 formal set is described as **9 retained queries**; and
2. FM-06 is described as `BLOCKED-NORMATIVE` pending an authoritative resolver relation and executable mapping-substitution semantics.

Later repository evidence supersedes both statements for the specifically modeled local identity-attribution property.

The active retained AUTH-v3 evidence is now:

```text
run record         = docs/assurance/formal-runs/2026-08-29-ae1eeb4-proverif-auth-v3-fm06.md
repository commit  = ae1eeb47b830996470beb489fe3875e5fc2635a2
CI run             = #86 / 33281604442
tool               = ProVerif 2.05
model blob         = 2f3817b5fb847ef948e4effab4b7d9871adc2e14
retained queries   = 10
result             = all 10 AUTH-v3 query results true; fail-closed gate passed
```

The tenth query is the scoped FM-06 correspondence:

```text
ServerAttributedCompleteV3(key, identity, role, sid, secctx, kcctx)
    ==>
IdentityAttributionResolvedV3(key, identity, role, sid, secctx)
```

Therefore, wherever the primary formal contract says **9 retained AUTH-v3 queries**, read **10 retained AUTH-v3 queries** for the current synchronized model and retained exact-model evidence.

## 8. Current FM-06 evidence interpretation

The active FM-06 status is:

> **FORMALLY ANALYZED, scoped** — modeled attributed server completion is downstream of exact local identity attribution for the same authentication key, peer identity, role, session, and security context under the A0 active-network abstraction.

This state is supported by four independent evidence layers:

```text
normative resolver contract
        ↓
matching Rust + C local resolver implementations
        ↓
shared Rust/C attribution accept/reject corpus
        ↓
synchronized ProVerif event/correspondence + retained exact-model run
```

The detailed traceability and claim boundary are retained in:

```text
docs/assurance/fm-06-identity-attribution-formal-traceability-2026-08-29.md
docs/assurance/formal-runs/2026-08-29-ae1eeb4-proverif-auth-v3-fm06.md
```

The older `BLOCKED-NORMATIVE` statement remains historically useful because it records the dependency that had to be satisfied before formalization. It is no longer the active status for this narrow property.

The following stronger claims remain unsupported:

- all unknown-key-share constructions are prevented;
- resolver alias uniqueness or ambiguity detection is symbolically proven;
- credential/reference wire encoding is formally verified;
- computational credential/key/identity binding is proven;
- Rust or C implementation behavior is formally verified;
- parser/model equivalence is established;
- authorization policy correctness beyond exact modeled context equality is established;
- delegation, revocation convergence, resumption, trust mutation, or lifecycle correctness follows;
- constant-time behavior, RNG quality, memory safety, or side-channel resistance follows;
- TD-001 independent cryptographic review or TD-002 physical measurements are satisfied;
- TD-003 is complete;
- TD-004 is complete;
- `COMMON-CONFORMANT`, `RFC-CLASS DOCUMENTED`, or `DEPLOYMENT-QUALIFIED` follows.

## 9. Current TD-003 follow-on boundary

With FM-06 no longer blocked on resolver ownership, the next TD-003 work must not add another identity-attribution theorem merely to increase formal query count.

The remaining high-value formal gaps include:

```text
FM-08 downgrade semantics                    BLOCKED-NORMATIVE
FM-10 authorization/policy separation        BLOCKED-NORMATIVE beyond current admission seam
FM-11..FM-18 lifecycle/trust/binding         BLOCKED-NORMATIVE or runtime-incomplete outside scoped FM-06
FM-19..FM-21 privacy                         NOT FORMALLY ANALYZED
FM-22 compromise/recovery                    BLOCKED pending explicit A3 timing/state/recovery semantics
fresh authenticated replay epoch             UNRESOLVED
parser ↔ symbolic equivalence                NOT ESTABLISHED
canonical/single-source formal generation    INCOMPLETE
complete model→spec→Rust/C→test traceability INCOMPLETE
```

Formal work must continue to follow the repository rule that normative/runtime ownership precedes theorem promotion. In particular, forward-secrecy, post-compromise, downgrade, authorization, revocation, resumption, delegation, and privacy theorems must not be invented ahead of the corresponding specification and executable semantics.

## 10. Combined evidence-state effect

This supersession note changes claim interpretation only; it does not change the roadmap score by itself:

```text
AUTH-v3 retained query inventory                     10
FM-06 local identity-attribution correspondence      FORMALLY ANALYZED, scoped
FM-06 Rust/C resolver behavior                        IMPLEMENTED + TESTED
FM-06 shared mapping-substitution decisions           TESTED across Rust/C
full UKS resistance                                   NOT ESTABLISHED
complete TD-003 formal coverage                       OPEN
TD-001 external crypto review                         OPEN
TD-002 constrained physical measurements              OPEN
TD-004 RFC-class normative package                    OPEN
FORMALLY VERIFIED implementation                      NOT CLAIMED
RFC-CLASS DOCUMENTED                                  NOT CLAIMED
COMMON-CONFORMANT                                      NOT CLAIMED
DEPLOYMENT-QUALIFIED                                   NOT CLAIMED
```
