# AUTH-v3 Formal Traceability Supersession — 2026-08-29

Status: **scoped assurance reconciliation**.

This note updates the evidence interpretation of `docs/assurance/auth-v3-formal-traceability.md` at repository state `35b88317462abefebb814cffd08acdf392af0572`. It does not change protocol behavior, the wire format, parser acceptance semantics, cryptographic computations, formal models, or any roadmap/debt completion state.

## 1. Superseded statement

Section 4.1 of `docs/assurance/auth-v3-formal-traceability.md` retains an August 28 statement that the generic Rust canonical-context parser has a hostile-count resource asymmetry when called without a selected-profile bound.

That statement is no longer the correct active evidence boundary for the specifically tested malformed-count path.

The dated August 28 research report remains valid historical provenance. It must not be rewritten merely because later repository evidence superseded one hypothesis.

## 2. Current repository evidence

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

## 3. Corrected traceability interpretation

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

Those findings remain research/evidence inputs. This supersession note does not promote their proposed resolver, compromise, recovery, or lifecycle semantics into protocol requirements.

## 5. Evidence-state effect

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

The retained AUTH-v3 ProVerif results remain scoped symbolic results. This note neither adds nor changes a formal theorem.

## 6. Follow-on dependency

After this evidence reconciliation, the next non-redundant TD-003/TD-004 dependency is not another parser theorem. The repository still needs a normative, implementation-independent identity-attribution resolver relation before FM-06 can be modeled without inventing trust semantics. Until that relation exists, FM-06 remains `BLOCKED-NORMATIVE` as recorded in `docs/assurance/formal-model-contract.md`.
