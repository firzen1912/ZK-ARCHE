# LINEAGE_REPLACE durable freshness contract

Status: bounded zk213 storage-adapter contract. This document defines normalized restart freshness semantics only. It does not establish physical anti-rollback, crash-safe storage, secure erasure, or target qualification.

## 1. Purpose

`LINEAGE_REPLACE` already separates logical transition state, restart structural consistency, and deterministic write-cut behavior. A further distinction is required for rollback: a stored lineage record can be authentic and structurally valid while still being an older authorized snapshot.

Accordingly, a storage adapter MUST treat record integrity and record freshness as separate inputs.

## 2. Normalized freshness facts

A storage adapter exposes these normalized facts to the common classifier:

```text
anchor_available
anchor_integrity_valid
anchor_binding_valid
record_generation
trusted_high_water_generation
```

`record_generation` is the generation carried by the authenticated lineage record being considered for restart.

`trusted_high_water_generation` is the highest generation that the adapter can justify as current under its declared storage threat model.

`anchor_binding_valid` states that the high-water evidence is bound to the same local device/security domain and lineage namespace as the candidate record. Transport address, filesystem path, or discovery address MUST NOT be treated as this binding.

## 3. Decision contract

The classifier MUST return exactly one of:

```text
CURRENT
ANCHOR_UNAVAILABLE
ANCHOR_INVALID
BINDING_MISMATCH
ROLLBACK_DETECTED
GENERATION_AHEAD
```

The rules are:

1. Invalid anchor integrity yields `ANCHOR_INVALID`.
2. Otherwise unavailable high-water evidence yields `ANCHOR_UNAVAILABLE`.
3. Otherwise invalid local/security-domain binding yields `BINDING_MISMATCH`.
4. `record_generation < trusted_high_water_generation` yields `ROLLBACK_DETECTED`.
5. `record_generation > trusted_high_water_generation` yields `GENERATION_AHEAD` because the high-water evidence is stale, incomplete, or inconsistent with the candidate record.
6. Only exact generation equality yields `CURRENT`.

A non-`CURRENT` decision MUST map restart to `CONTINUITY_BROKEN`. Freshness evidence MUST NOT repair a structurally ambiguous recovery observation. Conversely, structurally valid recovery facts MUST NOT bypass missing or contradictory freshness evidence when this contract is selected.

## 4. Adapter ordering requirement

A target adapter that claims this contract MUST arrange durable updates so that after interruption it can establish either:

- the predecessor or successor record at the same trusted high-water generation; or
- a non-current/ambiguous freshness result that fails closed.

Advancing the high-water anchor before the corresponding record is durably represented can produce `ROLLBACK_DETECTED`. Persisting a newer record before the anchor is durably advanced can produce `GENERATION_AHEAD`. Both outcomes are deliberately fail closed.

This specification does not prescribe one physical write order, journal format, secure-element API, monotonic-counter primitive, or wear-leveling strategy. Those are target-adapter responsibilities and require target evidence.

## 5. Security boundaries

This common classifier establishes only deterministic interpretation of normalized facts. It does NOT establish that an adapter's high-water source is itself rollback resistant.

A target/profile MUST NOT claim malicious-rollback resistance unless retained evidence shows that its selected freshness mechanism resists the declared attacker. A MAC or signature over a lineage record is insufficient if an attacker can restore an older valid record together with equally old freshness metadata.

If a target cannot establish trusted freshness under its declared attacker model, it MUST expose an unavailable/invalid freshness result and restart MUST fail closed for profiles requiring this contract.

Normal AUTH remains NO-LEARNING. Restart classification does not enroll, authorize, rekey, replace trust, or activate a successor that was not already durably and explicitly committed.

## 6. Conformance corpus

`rust/test-vectors/replay/lineage-replace-freshness-v1.txt` is the canonical cross-language decision corpus for this contract. Rust and C MUST consume the same cases and agree on both freshness decision and resulting restart state.

The corpus includes current predecessor/successor states, old authenticated snapshots, record-ahead/high-water-behind inconsistency, unavailable or invalid anchors, binding mismatch, structurally partial state despite current freshness, and the maximum `u64` generation without arithmetic wraparound.

## 7. Evidence still required

This contract does not close zk213. Remaining evidence includes at least:

- an authenticated production rekey/re-registration flow proving control of current and successor credentials;
- real trust-store mutation using a target adapter;
- target-specific crash/power-loss testing at every durable write boundary;
- old-valid-snapshot restoration tests against the actual freshness mechanism;
- clone/reprovision/reset semantics for the freshness anchor;
- endurance/wear and counter-exhaustion behavior where relevant;
- distributed peer convergence and key/lineage confirmation for concurrent or asymmetric replacement attempts;
- appropriate formal and external-review coverage.
