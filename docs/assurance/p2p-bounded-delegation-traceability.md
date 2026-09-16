# P2P Bounded Delegation Common-Contract Traceability

Status: implementation-backed assurance record. This record does not promote `p2p-iot-core`, claim physical-target qualification, or establish `COMMON-CONFORMANT` status.

## Purpose

This record binds the current bounded P2P delegation decision across normative contract, Rust, C, and the shared deterministic corpus. It exists to prevent the Common Contract from drifting into a documentation-only or one-language behavior claim.

The authoritative behavior remains `spec/p2p-bounded-delegation-decision.md`. The Rust and C classifiers consume already-verified facts; grant parsing and cryptographic proof/signature verification are outside this classifier boundary.

## Traceability matrix

| Common-contract property | Normative contract | Rust | C | Shared deterministic evidence | Remaining qualification gap |
|---|---|---|---|---|---|
| Delegation is authorization, not authentication or trust mutation | §§1,4 | `P2pDelegationFacts.holder_authenticated`; classifier never mutates trust | `p2p_delegation_facts_t.holder_authenticated`; classifier is decision-only | `DEL3-003`, `DEL3-004`, `DEL3-021` | End-to-end ENROLL/AUTH/delegation integration evidence |
| Issuer trust is local and non-transitive | §§2,4 | `issuer_trusted` + `issuer_trust_local` checked independently | same decision facts and precedence | `DEL3-002`, `DEL3-003`, `DEL3-021` | Cross-peer A→B→C executable non-transitivity scenario |
| Holder must already be authenticated | §§1–3 | `holder_authenticated` fail-closed guard | matching guard | `DEL3-004` | Bind to concrete AUTH session identity in end-to-end qualification |
| Scope/audience/deployment are mandatory authorization bounds | §§2–3 | independent mismatch guards | matching guards | `DEL3-007`–`DEL3-009` | Concrete grant encoding/verification integration |
| Authorization generation must be bound and current | §§2–3,5 | bound/current guards | matching guards | `DEL3-011`, `DEL3-012` | Durable generation transition/restart evidence |
| Epoch/revocation/lineage changes invalidate retained authority | §§2–3,5 | explicit current/revoked/lineage guards plus retained-delegation revalidation test | matching classifier and retained revalidation test | `DEL3-013`–`DEL3-016` | Disconnected revocation convergence and stale-authorization bound |
| Delegation/redelegation is explicit and bounded | §§2,5 | depth + requested/permitted guards | matching guards | `DEL3-017`–`DEL3-019` | Multi-hop chain qualification under constrained resource bounds |
| Rollback suspicion fails closed | §§2–3,5 | rollback guard precedes ordinary authority checks | matching precedence | `DEL3-020` | Rollback-resistant persistent-state evidence on real targets |
| Compound failures have deterministic cross-language precedence | §3 | canonical v3 corpus consumer compares action + reason | same corpus consumed directly from Rust-owned vector path | all 21 `DEL3-*` cases | Exact-head executed Rust/C qualification retained with provenance |
| Already-authorized evaluation has no online infrastructure dependency | §6 | classifier accepts only supplied local facts and performs no network lookup | same | classifier/corpus architecture only | Executable offline constrained↔constrained and constrained↔higher-capability qualification |

## Evidence ownership

The current synchronized evidence chain is:

```text
spec/p2p-bounded-delegation-decision.md
        ↕
rust/crates/proto/src/p2p_delegation.rs
        ↕
c/include/auth/p2p_delegation.h
c/src/proto/p2p_delegation.c
        ↕
rust/test-vectors/p2p/bounded-delegation-v3.txt
        ↕
Rust canonical-v3 consumer + C canonical-v3 consumer
```

A semantic change to fields, precedence, or action/reason vocabulary requires synchronized Rust/C behavior and a new corpus version. Existing v3 vectors must not be silently reinterpreted.

## Claim boundary

This chain supports an implementation-backed, cross-language-testable bounded-delegation decision surface. It does **not** by itself establish:

- cryptographic correctness of a grant parser/proof/signature verifier;
- independent cryptographic review;
- complete ENROLL→AUTH→delegation integration;
- disconnected revocation convergence or a bounded stale-authorization interval;
- persistent rollback resistance;
- constrained-device RAM/flash/latency/energy evidence;
- constrained↔constrained or constrained↔higher-capability runtime interoperability;
- formal proof of delegation properties;
- deployment qualification; or
- complete `p2p-iot-core` Common Contract conformance.

Those remain separate exit evidence under the canonical roadmap and qualification matrix.

## Promotion rule

Do not promote this surface to `COMMON-CONFORMANT` from source inspection or corpus existence alone. Promotion requires retained exact-head executable evidence for the relevant cross-language and cross-class lanes, plus the physical/resource evidence required by the selected constrained profile. Cloud-runner inability to execute a lane is `UNAVAILABLE`, not `PASS` and not `RED`.
