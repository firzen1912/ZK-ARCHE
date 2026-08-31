# LINEAGE_REPLACE Storage Capability Contract

Status: draft, wire-neutral lifecycle qualification contract.

This contract defines the minimum adapter metadata required before ZK-ARCHE may treat a concrete storage target as eligible for the rollback-resistant LINEAGE_REPLACE qualification path. It does not create a new wire field, packet, cryptographic primitive, trust rule, or profile identifier.

## 1. Scope

`lineage_replace_execute_storage_transaction` defines logical ordering only. An adapter callback returning success means that the adapter asserts that one logical step is durable. That assertion is not sufficient to claim power-loss recovery or malicious-rollback resistance.

The storage capability classifier therefore separates these target properties:

1. `durable_commit_confirmed` — the adapter has a defined success boundary after which the logical record update is durable under the declared target model;
2. `power_loss_recovery_supported` — interrupted writes/restarts are conservatively recoverable under the declared target model;
3. `record_integrity_protected` — stored lineage state has integrity protection appropriate to the declared attacker model;
4. `replay_protection_supported` — an attacker covered by the declared model cannot silently restore an older valid storage object and have it accepted as current;
5. `freshness_anchor_available` — a trusted high-water/freshness source is available to the adapter;
6. `freshness_anchor_integrity_valid` — the freshness source itself has integrity under the declared model;
7. `freshness_anchor_lineage_bound` — the freshness source is bound to the same local security domain and lineage namespace as the record being classified.

These fields are capability declarations. A declaration MUST NOT be promoted to MEASURED, DEPLOYMENT-QUALIFIED, or malicious-rollback-resistant evidence without retained target-specific proof.

## 2. Deterministic decision order

The classifier fails closed using this precedence:

```text
missing/false durable_commit_confirmed
→ REJECT_DURABILITY
missing power-loss recovery
→ REJECT_POWER_LOSS_RECOVERY
missing record integrity
→ REJECT_RECORD_INTEGRITY
missing replay protection
→ REJECT_REPLAY_PROTECTION
missing freshness anchor
→ REJECT_FRESHNESS_ANCHOR
invalid freshness-anchor integrity
→ REJECT_FRESHNESS_INTEGRITY
freshness anchor not lineage-bound
→ REJECT_FRESHNESS_BINDING
otherwise
→ QUALIFIED
```

A missing capability object is equivalent to missing durability evidence and fails closed.

## 3. Security boundary

`QUALIFIED` means only that the adapter metadata is internally sufficient for the rollback-resistant qualification path. It does not prove the metadata is truthful, measured, or correctly implemented.

In particular:

- PSA/TF-M ITS, Protected Storage, vendor NVS, a filesystem, or a secure element MUST NOT be treated as equivalent merely because each is described as secure storage;
- a durable commit acknowledgment MUST NOT be equated with freshness;
- record integrity MUST NOT be equated with replay protection;
- firmware anti-rollback counters MUST NOT automatically be reused as per-lineage high-water counters;
- hardware features named EPOCH or anti-rollback MUST be mapped to ZK-ARCHE lineage semantics before `freshness_anchor_lineage_bound` is asserted;
- a software-only fixture may exercise the classifier but does not provide physical power-cut, rollback, wear/endurance, secure-erasure, or board-level evidence.

## 4. Common Contract implications

This contract does not permit weaker mandatory authentication semantics on constrained targets. If a target cannot satisfy the rollback-resistant storage qualification path, that is an explicit capability/evidence limitation. It MUST NOT silently reduce AUTH, transcript, possession, authorization, replay, or fail-closed requirements under the same profile name.

No CA, cloud service, gateway, DNS, Internet connection, blockchain, or manufacturer service is introduced as a root decision dependency.

## 5. Conformance evidence

Canonical decision vectors are `rust/test-vectors/replay/lineage-replace-storage-capability-v1.txt` and are consumed by Rust and C tests. Target qualification remains separately owned by TD-002 and must record the concrete storage service/API, implementation/version, configuration/flags, physical backend, durability boundary, power-loss behavior, integrity mechanism, replay/freshness mechanism, attacker model, and unsupported properties.
