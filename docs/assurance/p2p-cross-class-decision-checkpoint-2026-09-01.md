# P2P cross-class decision qualification checkpoint — 2026-09-01

Status: executable matrix-level qualification evidence for the Common Contract. This checkpoint does not claim physical MCU interoperability, cryptographic execution on constrained targets, or deployment qualification.

## Scope

This packet reconciles the P2P Common Contract matrix with the now-existing revocation/freshness contract and adds an executable composition check for the minimum local decision inputs shared across constrained↔constrained and constrained↔higher-capability peers.

The executable matrix intentionally models decision composition rather than cryptographic execution. It checks that infrastructure availability is not an authorization input and that successful local admission requires current authentication, authorization, revocation, lineage, mandatory-floor, and required-binding evidence.

## Evidence produced

`rust/test-vectors/p2p/common-contract-decision-v1.txt` covers both directions of MCU↔edge operation, MCU↔MCU operation, offline/no-infrastructure acceptance with sufficient local state, invalid AUTH, stale authorization, stale revocation state, explicit revocation, stale lineage, mandatory-floor incompatibility, required-binding failure, and successful required binding.

`scripts/check-p2p-common-contract-decision.py` recomputes every expected decision. It also fails closed if the normative revocation, transport-binding, roadmap, or canonical P2P matrix ownership text drifts.

`P2P-009` is no longer `blocked-normative`: `spec/revocation-convergence-and-stale-authorization.md` now defines the stale-authorization rule. It remains `required-unexecuted` in the original runtime-oriented matrix because physical/runtime cross-class evidence is still absent.

## Evidence boundary

This checkpoint establishes:

- executable matrix-level Common Contract decision composition;
- explicit bidirectional MCU↔edge and MCU↔MCU case ownership;
- executable no-infrastructure decision semantics;
- fail-closed freshness, revocation, lineage, mandatory-floor, and binding composition.

It does not establish:

- actual Rust↔C AUTH execution for these cross-class cases;
- physical STM32/ESP32-S3↔Linux interoperability;
- physical no-cloud/no-CA/no-gateway network execution;
- target resource budgets or TD-002 closure;
- cryptographic soundness or TD-001 closure;
- complete bounded delegation runtime support;
- selectable `p2p-iot-core`;
- deployment qualification.

Accordingly `COMMON-CONFORMANT` remains incomplete and the phase score does not advance solely from this checkpoint.
