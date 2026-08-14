# ZK-ARCHE Release and Claim Governance

This document defines maturity vocabulary and evidence gates for ZK-ARCHE. It exists to keep implementation progress, research results, and release claims distinct.

## Maturity states

| State | Meaning |
|---|---|
| `research-only` | Idea or experiment under evaluation; not a baseline requirement. |
| `planned` | Explicitly accepted into a roadmap, but not implemented or qualified. |
| `implemented-unqualified` | Code exists, but required interoperability/security/target evidence is incomplete. |
| `validated` | Defined project-level tests/evidence pass for the stated scope. This is not external certification. |
| `externally-reviewed` | Relevant design/cryptographic work has documented independent review; remaining findings must be stated. |
| `release-candidate` | All repository-defined gates for the target profile/release are satisfied or explicitly recorded as blockers. |

Do not infer a stronger state from a weaker one. In particular, code completion does not imply cryptographic security, field readiness, formal verification, or external review.

## Claim gates

Before making a material maturity claim, identify the evidence that supports it.

### Interoperability

Requires, for the claimed behavior:

- canonical/versioned Rust vectors;
- passing C validation against those semantics where C implements the feature;
- no unexplained vector drift;
- positive and negative cases appropriate to the feature.

### Security behavior

Depending on the feature, require replay, mutation, downgrade, malformed-input, state-machine, fuzz, RNG/failure, and storage/recovery evidence. Custom cryptographic constructions require review appropriate to the claim.

### Constrained-device readiness

Requires target-specific byte, RAM/stack/heap, CPU/latency, persistent-storage, packet-size, and dependency evidence. Workstation success is not MCU evidence.

### Formal-verification claims

Must be scoped to the properties, model, assumptions, and implementation correspondence actually established. A ProVerif/Tamarin skeleton is not full formal verification.

### External-review claims

Require a review artifact identifying reviewer scope, findings, disposition, and unresolved limitations. “Prepared for review” is not “reviewed.”

## Release qualification

The repository release-qualification workflow should remain reproducible from a clean checkout:

```bash
./scripts/ci-release-qualification.sh
```

At minimum, release qualification should preserve current Rust and C CI, deterministic-vector governance, C-against-Rust interoperability, and any additional gates explicitly required by the roadmap/spec for the claimed profile.

A green CI run demonstrates only the checks it actually executes.

## Research and roadmap boundary

- `docs/research/` may recommend or reject ideas.
- `docs/roadmaps/` may schedule engineering work.
- `docs/adr/` records consequential decisions.
- `spec/` defines normative protocol behavior.
- `rust/` and `c/` implement behavior.
- `docs/assurance/` and `evidence/` establish validation status.
- this document governs what may be claimed from that evidence.

No single research report or roadmap entry can award a maturity state by itself.

## Blocked claims by default

Unless specific evidence establishes otherwise, ZK-ARCHE should not be described as:

- production-ready cryptographic infrastructure;
- completely replay resistant;
- side-channel certified/resistant across supported hardware;
- formally verified as a full implementation;
- externally audited/reviewed in full;
- certified;
- field-ready across STM32/ESP32/Raspberry Pi/Jetson target classes.

The purpose of these boundaries is not to minimize progress. It is to make every stronger claim traceable to evidence that another engineer or reviewer can reproduce.
