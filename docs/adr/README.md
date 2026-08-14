# Architecture Decision Records

This directory stores consequential ZK-ARCHE architecture and protocol decisions as immutable point-in-time records, following the ADR pattern used in HIVEAS.

## When an ADR is required

Create an ADR before or alongside changes that deliberately alter a stable boundary, including:

- cryptographic primitive or suite selection;
- transcript/domain-separation rules;
- wire format, canonical encoding, packet/TLV structure, or versioning policy;
- deterministic-vector ownership or compatibility policy;
- enrollment, trust, revocation, delegation, or identity model;
- authentication roles or P2P trust semantics;
- constrained-profile requirements;
- transport/channel-binding architecture;
- storage/replay model when it changes security semantics;
- a deliberate Rust/C compatibility break;
- replacement of a custom proof or credential mechanism;
- a major security/privacy tradeoff that future maintainers must understand.

Routine refactoring, documentation cleanup, tests that preserve semantics, and normal implementation fixes generally do not require an ADR.

## Naming

```text
docs/adr/0001-short-decision-name.md
docs/adr/0002-next-decision.md
```

Never renumber an accepted ADR. If a decision changes, create a new ADR that supersedes the old one and link both directions.

## Template

```markdown
# ADR NNNN — Decision title

- **Status:** proposed | accepted | superseded | rejected
- **Date:** YYYY-MM-DD
- **Owners / reviewers:**
- **Related research:**
- **Related roadmap:**
- **Related spec / vectors:**

## Context

What problem or incompatibility requires a decision? What evidence and constraints matter?

## Decision

State the chosen architecture/protocol rule precisely.

## Alternatives considered

Describe serious alternatives and why they were not selected.

## Security and privacy consequences

Document threat-model, metadata, replay, downgrade, side-channel, and trust implications as applicable.

## IoT / implementation consequences

Record expected wire, RAM, CPU, storage, dependency, Rust/C, and migration effects.

## Compatibility and migration

State whether the change is compatible, extension-only, or versioned/breaking. Identify vector/spec migration work.

## Evidence required

List tests, vectors, benchmarks, formal work, or external review needed before stronger claims.
```

An accepted ADR explains **why** a decision exists. Normative protocol behavior still belongs in `spec/`, and implementation evidence belongs in tests/assurance artifacts.
