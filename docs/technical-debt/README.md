# ZK-ARCHE Technical Debt Register

This directory makes unresolved work explicit instead of burying it inside roadmaps, source comments, or maturity claims. The register is inspired by HIVEAS's separate technical-debt area but uses categories appropriate to a cryptographic/protocol framework.

## Debt categories

| Category | Examples |
|---|---|
| Protocol debt | underspecified messages, incomplete state machines, unversioned extension behavior, ambiguous error/replay semantics |
| Cryptographic debt | custom proof requiring review, incomplete domain-separation analysis, missing primitive migration design |
| Interoperability debt | Rust/C mismatch, missing vectors, unsupported feature in one lane, non-reproducible interop test |
| Constrained-device debt | missing STM32/ESP32 measurements, heap use, packet-size excess, flash-write or RNG assumptions |
| Assurance debt | missing fuzz/replay/mutation/formal/side-channel/external-review evidence |
| Tooling debt | non-reproducible CI, missing benchmark harness, evidence collection gaps |
| Documentation/spec debt | implemented behavior not captured normatively, stale registry/profile/security text |

## Active register

| ID | Category | Gap | Impact / blocked claim | Clearing evidence | Owner | Status |
|---|---|---|---|---|---|---|
| TD-001 | Assurance | Custom role-membership proof still requires independent cryptographic review before strong production/privacy claims | Blocks externally reviewed / production-grade proof claims | External review memo plus resolved findings and regression vectors | unassigned | open |
| TD-002 | Constrained-device | Target-class byte/RAM/CPU evidence is incomplete for STM32/ESP32-S3-class deployments | Blocks IoT field-readiness claims | Reproducible target benchmark reports and profile limits | unassigned | open |
| TD-003 | Assurance | Formal models are skeletons and do not establish full implementation verification | Blocks formal-verification claims | Scoped ProVerif/Tamarin results, assumptions, and model-to-code traceability | unassigned | open |
| TD-004 | Protocol | RFC-style specification package is still a draft skeleton and does not fully encode implemented behavior | Blocks specification-grade/conformance claims | Normative message grammar, registries, state machines, vectors, and negative tests | unassigned | open |

## Status vocabulary

- `open` — unresolved and relevant.
- `in-progress` — clearing work is underway.
- `blocked` — requires external/hardware/human dependency.
- `accepted` — consciously tolerated with documented reason and scope.
- `cleared` — required evidence exists and links to it are recorded.
- `superseded` — replaced by another debt item or architectural decision.

## Adding debt

A debt entry should state what is missing, what claim or capability it blocks, and what evidence clears it. Avoid vague TODOs such as “improve security.”

Use:

```text
TD-NNN
category
gap
impact / blocked claim
clearing evidence
owner
status
links: research / roadmap / ADR / spec / issue / evidence
```

Roadmap work may clear debt, but a roadmap checkbox alone is not evidence that the debt is cleared.
