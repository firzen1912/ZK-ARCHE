# TD-003 Closure Audit — 2026-08-29

Status: **scoped assurance/qualification audit** for TD-003. This document reconciles the current formal-property, model-ownership, traceability, runtime, and research evidence at repository head `29824af8d62159b5f5efc5dd326a71a93c85b15b`. It does not clear TD-003, does not claim formal implementation verification, and does not promote blocked normative behavior.

## 1. Starting integration state

The audited starting head is:

```text
branch = dev
commit = 29824af8d62159b5f5efc5dd326a71a93c85b15b
CI     = #73 / 33250897513 / success
```

All currently applicable CI jobs completed successfully on that exact commit:

```text
Rust lane — fmt, check, test, clippy, audit                 success
C lane — build, tests, static analysis, sanitizers          success
Formal lane — legacy v2 + AUTH v3 + replay continuity       success
Release qualification — Rust/C interop & security gate      success
CI complete — required-lane gate                            success
```

The formal lane checked synchronized model copies before running the retained fail-closed ProVerif gates.

## 2. Current TD-003 evidence already present

The repository now has material evidence for all of the following TD-003 sub-obligations:

- an explicit property/attacker matrix (`FM-01` through `FM-22`);
- named attacker profiles A0 through A5;
- mechanically checked Rust/C formal-model synchronization;
- a designated canonical side in `scripts/sync-formal-models.sh` (the left/Rust path is copied to the right/C mirror by `--write`);
- exact retained ProVerif tool/version/model-blob/run provenance;
- nine retained green AUTH-v3 queries;
- nine retained green replay-continuity queries;
- retained historical AUTH-v2 counterexamples rather than deletion of negative evidence;
- scoped FM-01 accepted-session-key secrecy under A0/no endpoint compromise;
- scoped FM-02/FM-03 agreement;
- scoped FM-04 replay-record ordering plus a separate replay-continuity state model;
- scoped FM-05 modeled security-context/admission traceability;
- scoped FM-07 Finished-direction reflection resistance;
- scoped FM-09 NO-LEARNING relative to pre-existing modeled trust;
- Rust/C executable reflection, replay, parser, and context fixtures linked to modeled boundaries;
- explicit limitations separating symbolic results from computational proof, memory safety, RNG quality, side-channel behavior, key storage, and field readiness.

This is substantial TD-003 progress, but it is not full model/runtime equivalence and not a formally verified implementation.

## 3. Parser-resource research reconciliation

The August 28 research report recorded D20260828-F01 against an older repository state: the Rust generic `ZKCTX` parser could derive allocation capacity directly from attacker-controlled `entry_count` before proving structural plausibility.

Current evidence supersedes that **specific active hypothesis** without rewriting the dated research history:

- current Rust parsing enforces selected-profile and structural count bounds before `Vec::with_capacity(entry_count)`;
- current C parsing rejects structurally impossible counts before entry capture and uses caller-owned bounded storage;
- the shared negative corpus contains the `entry_count = 65535` truncated-header case and both lanes classify it as `TRUNCATED`;
- `rust/crates/proto/tests/auth_v3_context_parser_allocation.rs` tracks allocation/reallocation and requires zero calls for that exact malformed header;
- exact-head CI #72 retained that Rust test plus the C/shared-corpus qualification;
- `docs/assurance/auth-v3-context-parser-resource-evidence.md` records the bounded claim and remaining limitations.

Therefore the old statement "the generic Rust canonical-context parser still has the August 28 hostile-count pre-allocation evidence gap" is no longer an accurate current blocker for the tested `65,535` truncated-header case.

The following remain open and MUST NOT be inferred from that correction:

- parser ↔ symbolic-model equivalence;
- zero allocation for all parser inputs;
- fixed-RAM behavior for every generic Rust context;
- production profile-specific resource limits beyond currently defined receive paths;
- physical MCU stack/heap/flash/CPU evidence under TD-002;
- general pre-authentication DoS resistance.

## 4. Canonical/synchronized formal-model ownership

Current repository behavior is stronger than an informal "keep two copies in sync" convention:

```text
scripts/sync-formal-models.sh --write
    canonical = left/Rust model path
    mirror    = right/C model path
    cp canonical -> mirror

scripts/sync-formal-models.sh --check
    cmp canonical mirror
    fail CI on drift
```

This establishes a mechanically enforced canonical-to-mirror relationship for the current ProVerif sources. It satisfies the narrow ownership requirement that the two paths are not independent protocol authorities.

It does **not** establish:

- one language-neutral protocol state-machine source from which ProVerif/Tamarin/executable scenarios are generated;
- automatic generation of the traceability table from source annotations;
- semantic equivalence between the symbolic source and Rust/C runtime implementations.

Those distinctions remain relevant when deciding whether TD-003 can eventually be cleared.

## 5. Property closure audit

| Property group | Current state | Agent-closable now? | Blocking reason / next evidence |
|---|---|---:|---|
| FM-01 | scoped FORMALLY ANALYZED | no immediate theorem needed | A3 compromise semantics are not yet modeled; do not broaden the A0 result |
| FM-02/FM-03 | scoped FORMALLY ANALYZED | no immediate theorem needed | preserve regression coverage and exact traceability |
| FM-04 | split formal/runtime evidence | partially | fresh authenticated replay epoch remains normatively unresolved |
| FM-05 | scoped for modeled fields | partially | critical-extension and channel-binding canonical boundaries remain less complete than `iot-core` authorization |
| FM-06 | DEFINED / partial agreement coverage | only if a non-redundant scenario exists | do not manufacture a UKS theorem already implied by bound identity/context agreement |
| FM-07 | scoped FORMALLY ANALYZED + Rust/C TESTED | no immediate theorem needed | generic all-message reflection is not established |
| FM-08 | BLOCKED-NORMATIVE | no | production negotiation/mandatory-floor downgrade semantics incomplete |
| FM-09 | scoped FORMALLY ANALYZED | no immediate theorem needed | applies only to pre-existing modeled trust |
| FM-10 | BLOCKED-NORMATIVE | no | authorization authority/provenance and policy semantics unresolved |
| FM-11/FM-12 | BLOCKED-NORMATIVE | no | local non-transitive trust/delegation semantics incomplete |
| FM-13 | BLOCKED-NORMATIVE | no | revocation convergence/freshness and authority namespace incomplete |
| FM-14/FM-15 | BLOCKED-NORMATIVE | no | authorization-aware resumption lifecycle incomplete |
| FM-16 | BLOCKED-NORMATIVE/runtime | no | reverse-role P2P path/common-contract semantics incomplete |
| FM-17 | architecture property | later | offline dependency test belongs with concrete P2P runtime/profile behavior |
| FM-18 | BLOCKED-NORMATIVE | no | credential/reference binding semantics incomplete |
| FM-19..FM-21 | privacy blocked | no | observability/equivalence policy and realistic runtime surface incomplete |
| FM-22 | BLOCKED-NORMATIVE/model expansion | no | compromise/recovery transition semantics incomplete |

The audit therefore finds **no justified new formal theorem that should be added before downstream normative ownership advances**. Continuing to add symbolic queries now would risk modeling invented policy rather than repository-owned behavior.

## 6. Remaining agent-closable TD-003 work

The useful agent-closable work that remains under TD-003 is traceability and evidence hygiene rather than new security semantics:

1. reconcile long-lived assurance documents that still repeat the superseded hostile-count allocation gap;
2. keep exact model/spec/Rust/C/test mappings synchronized as implementation symbols change;
3. make canonical/mirror ownership explicit wherever contributors could otherwise edit the C mirror as an independent authority;
4. retain new exact-model formal outputs whenever model text changes;
5. retain counterexamples and convert representable attacks into Rust/C executable negatives;
6. improve traceability for critical-extension and channel-binding canonical inputs once their normative/runtime schemas are concrete enough;
7. keep replay-runtime assumptions (capacity, persistence, restart, eviction, epoch) distinct from the persistent/unbounded AUTH-v3 symbolic table.

None of these permits clearing TD-003 while large property families remain blocked on missing protocol semantics.

## 7. Dependency transfer to TD-004/lifecycle work

The audit identifies the principal reason TD-003 cannot progress efficiently through additional theorems: several remaining properties are intentionally blocked by incomplete normative behavior.

The highest-value downstream specification owners are:

```text
TD-004 / zk226
    production negotiation + downgrade semantics
    critical extension rules
    Security/Privacy Considerations
    normative state/error behavior

zk211–zk214 / zk230 / zk239
    authorization authority/provenance
    revocation convergence and stale-state bounds
    non-transitive trust / bounded delegation
    credential/reference binding

zk221
    authorization-aware resumption lifecycle

replay lifecycle
    authenticated fresh-epoch transition
    predecessor/new-context binding
    crash/reboot semantics

zk240–zk241
    P2P role symmetry / mandatory-floor qualification
    constrained cross-class evidence
```

Formal work should resume on those properties only after the corresponding behavior is specified and executable enough that the model cannot silently choose policy.

## 8. Evidence-state effect

```text
TD-003 property/attacker matrix             PRESENT
TD-003 synchronized formal sources          CI-ENFORCED
TD-003 canonical-to-mirror relation         PRESENT in sync script
TD-003 retained exact-model results         PRESENT for current scoped AUTH/replay properties
TD-003 model→spec→Rust/C traceability        PARTIAL / MATERIAL
TD-003 parser hostile-count active gap       SUPERSEDED for tested 65,535 truncated case
TD-003 complete property coverage            NO
TD-003 full runtime/model equivalence         NO
TD-003                                       OPEN

TD-001 independent crypto review             OPEN
TD-002 physical constrained-target evidence  OPEN
TD-004 RFC-class specification               OPEN
FORMALLY VERIFIED                            NOT CLAIMED
RFC-CLASS DOCUMENTED                         NOT CLAIMED
COMMON-CONFORMANT                            NOT CLAIMED
DEPLOYMENT-QUALIFIED                         NOT CLAIMED
```

## 9. Next dependency-ready packet

After this audit, the smallest dependency-ready assurance packet is to reconcile the stale parser-resource wording in the long-lived TD-003 authorities (`formal-model-contract.md` and `auth-v3-formal-traceability.md`) to the retained parser resource evidence.

After that reconciliation, additional TD-003 theorem work should pause unless a genuinely non-redundant FM-06 scenario emerges from existing normative identity/context semantics. The next substantive roadmap frontier is then TD-004: select the highest-priority independently-specifiable normative gap that unblocks later formalization without changing the mandatory security floor by implication.
