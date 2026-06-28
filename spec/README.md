# ZK-ARCHE Specification Package

This directory is the staging area for turning ZK-ARCHE into an RFC-style protocol package.

Status: draft skeleton only. These files are not an RFC, not an Internet-Draft, and not production-readiness evidence. Normative requirements must be backed by implementation tests, vectors, negative tests, and security review before conformance claims.

Initial documents:

- `zk-arche-protocol.md` — protocol overview, message flows, state machines, cryptographic computations.
- `registries.md` — version, suite, profile, extension, alert, and transport-binding registries.
- `iot-profiles.md` — constrained and edge profile requirements.
- `security-considerations.md` — security threat analysis and assurance gates.
- `privacy-considerations.md` — identity, role, metadata, and credential privacy.
- `test-vectors.md` — canonical vectors, negative vectors, and regeneration procedure.
- `implementation-requirements.md` — parser, RNG, storage, side-channel, and failure behavior requirements.
