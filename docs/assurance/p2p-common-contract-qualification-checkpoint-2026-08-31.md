# P2P Common Contract qualification checkpoint — 2026-08-31

Starting exact `dev` HEAD: `e36f720f8a38dc48b770c151b511418bd67d96ce`.

## Scope

This checkpoint advances zk239–zk240 qualification scaffolding without promoting `p2p-iot-core` or claiming runtime interoperability. The canonical corpus `rust/test-vectors/p2p/common-contract-qualification-v1.txt` records ten cross-class cases covering constrained↔constrained, constrained↔edge, reverse-direction edge↔constrained, unauthorized peers, non-transitive trust, infrastructure loss, optional-capability asymmetry, transport-address changes, stale authorization, and incompatible mandatory floors.

The corpus deliberately uses evidence states `required-unexecuted` and `blocked-normative`. It MUST NOT be interpreted as retained runtime PASS evidence.

## Qualification gate

`scripts/check-p2p-common-contract-qualification.py` fails closed if:

- the canonical ten-case matrix drifts;
- any static corpus case self-declares runtime `passed`;
- the roadmap loses local/non-transitive trust, infrastructure-independence, symmetric-assurance, cross-class interoperability, or downgrade-resistance ownership;
- registry allocation `0x0003 p2p-iot-core` stops being explicitly `draft`; or
- draft profile allocations are made production-selectable without the required promotion evidence.

The checker is required by repository-owned release qualification.

## Evidence boundaries

This packet does **not** establish:

- executable MCU↔MCU or MCU↔edge interoperability;
- a stable/selectable `p2p-iot-core` profile;
- physical MCU resource measurements;
- revocation-freshness convergence;
- an explicit bounded delegation implementation;
- complete formal traceability;
- independent cryptographic review; or
- deployment qualification.

`P2P-009` remains `blocked-normative` because the permitted stale-authorization/freshness bound is not yet fully owned for the P2P profile. TD-001, TD-002, TD-003, and TD-004 remain open.

## Validation in this run

The available shell environment could not clone GitHub (`Could not resolve host: github.com`), so an exact clean checkout and the full repository-owned qualification suite were unavailable. The new checker was syntax-checked and executed against a reconstructed fixture carrying the exact current roadmap/registry ownership strings and the canonical ten-case corpus; it returned:

`p2p-common-contract-qualification: PASS cases=10 required_unexecuted=9 blocked_normative=1`

That result validates checker mechanics only. It is not runtime P2P interoperability evidence.
