# P2P peer-context property correction — 2026-09-01

## Scope

The Slot 11 integration audit found that the first version of `scripts/check-p2p-common-contract-properties.py` contained a tautological peer-class invariance assertion: it compared `classify(*security_state)` with the same expression, so that specific assertion could never detect peer-context drift.

This checkpoint corrects the verifier without changing the Common Contract decision semantics or the canonical `ZKP2PDECISION/1` corpus.

## Correction

The checker now routes canonical and exhaustive decisions through an explicit `classify_context(peer_a, peer_b, infrastructure_available, security_state)` boundary. Peer class and infrastructure availability are validated context, but are intentionally excluded from protocol authority. Exhaustive qualification compares the same local security-evidence tuple across every `mcu-core` / `linux-edge` ordering and both infrastructure states.

The decision distribution is unchanged: 2,048 context-expanded states, 24 successful local mutual-authentication decisions, and 2,024 fail-closed decisions.

## Evidence boundary

This correction restores the intended regression check. It does not add physical MCU interoperability, runtime infrastructure-loss evidence, cryptographic execution evidence, formal proof, external review, hardware measurements, or deployment qualification. Roadmap scores therefore do not advance from this repair alone.

The original exhaustive-property checkpoint remains useful for its other guarded properties, but its peer-class-invariance claim should be read together with this correction for exact-current `dev`.
