# ZK-ARCHE Specification Package

This directory is the normative staging area for evolving ZK-ARCHE into an RFC-style protocol package.

Status: draft skeleton only. These files are not an RFC, not an Internet-Draft, and not production-readiness evidence. Normative requirements must be backed by implementation tests, vectors, negative tests, and security review before conformance claims.

## Ownership boundary

`spec/` defines what conforming ZK-ARCHE protocol behavior means. Non-normative architecture, research, planning, and assurance material lives under [`docs/`](../docs/README.md).

The management flow is:

```text
docs/research/ → docs/roadmaps/ → docs/adr/ → spec/ → rust/ + c/ → docs/assurance/
```

Research or roadmap text does not override this package. If current implementation behavior is not yet captured here, the gap should be tracked explicitly rather than silently treating planning prose as normative.

## Initial documents

- `zk-arche-protocol.md` — protocol overview, message flows, state machines, cryptographic computations.
- `registries.md` — version, suite, profile, extension, alert, and transport-binding registries.
- `iot-profiles.md` — constrained and edge profile requirements.
- `replay-continuity.md` — fail-closed replay state across restart/state loss.
- `replay-epoch-recovery.md` — authenticated lineage-replacement requirements for any future fresh replay epoch; implementation remains blocked.
- `security-considerations.md` — security threat analysis and assurance gates.
- `privacy-considerations.md` — identity, role, metadata, and credential privacy.
- `test-vectors.md` — canonical vectors, negative vectors, and regeneration procedure.
- `implementation-requirements.md` — parser, RNG, storage, side-channel, and failure behavior requirements.

See [`docs/roadmaps/rfc-evolution-plan.md`](../docs/roadmaps/rfc-evolution-plan.md) for the non-normative specification-maturity plan.
