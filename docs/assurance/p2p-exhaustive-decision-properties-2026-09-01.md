# P2P exhaustive decision-property checkpoint — 2026-09-01

## Scope

This checkpoint strengthens the executable qualification around the `ZKP2PDECISION/1` Common Contract decision surface. The existing canonical matrix is intentionally small and reviewable; this packet adds exhaustive property checking across every Boolean combination of the currently modeled mandatory decision inputs, both infrastructure states, and all `mcu-core` / `linux-edge` peer pairings.

The checker is repository-owned at `scripts/check-p2p-common-contract-properties.py` and is included in `scripts/ci-release-qualification.sh`.

## Properties enforced

The exhaustive checker retains the canonical matrix as the named regression corpus and then verifies the complete decision state space for the current classifier:

1. **Infrastructure is not protocol authority.** Identical local security evidence produces the same authentication decision whether optional infrastructure is available or unavailable.
2. **Peer class is not a weaker security model.** MCU-core and Linux-edge pairing changes do not alter the mandatory decision semantics when the same security evidence is presented.
3. **Mandatory guards fail closed.** Invalid AUTH, stale authorization, stale revocation state, explicit revocation, stale lineage, incompatible mandatory floor, or an invalid required binding cannot produce a successful local mutual-authentication decision.
4. **Optional binding metadata is not implicit authority.** When binding is not required, changing its validity bit cannot change the authentication decision.
5. **Decision-distribution drift is detected.** The current eight Boolean security inputs admit exactly three successful combinations out of 256; across four peer pairings and both infrastructure states this is 24 successful decisions and 2024 fail-closed decisions out of 2048 states.

The exact distribution is intentionally checked so adding or removing a decision dependency requires an explicit update rather than silently changing the Common Contract.

## Evidence boundary

This is **executable decision-model qualification**, not physical or cryptographic interoperability evidence.

It does not establish:

- that Rust and C execute an AUTH handshake for all 2048 modeled states;
- physical STM32/ESP32-S3-class interoperability;
- runtime behavior under actual infrastructure loss;
- transport, RNG, storage, timing, or memory-safety properties;
- formal proof of the Common Contract;
- independent cryptographic review;
- deployment qualification.

Those remain separate evidence requirements. In particular, TD-001 and TD-002 cannot be closed by this checker, and symbolic/model-level or combinational qualification must not be promoted into hardware or independent-review claims.

## Qualification intent

The value of this checkpoint is regression resistance: a future change that accidentally makes infrastructure availability, peer class, optional binding metadata, or one of the current mandatory fail-closed guards influence the decision incorrectly is caught before the broader Rust/C qualification lanes run.
