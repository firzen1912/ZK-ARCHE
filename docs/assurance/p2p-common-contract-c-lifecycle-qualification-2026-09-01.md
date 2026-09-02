# P2P Common Contract C lifecycle qualification checkpoint — 2026-09-01

## Scope

This checkpoint advances Slot 9 with executable composition against the actual C secure-association admission implementation. It does not replace the existing exhaustive abstract property checker or the static roadmap qualification matrix.

The new corpus exercises MCU↔MCU, MCU↔Linux-edge, and Linux-edge↔MCU contexts with optional infrastructure both unavailable and available. Peer class and optional infrastructure are context only and never become protocol authority.

## Executed decision surface

`c/tests/test_p2p_common_contract_lifecycle.c` consumes `rust/test-vectors/p2p/common-contract-c-lifecycle-v1.txt` and invokes the production C `association_admission_classify` decision.

The corpus independently covers:

- constrained↔constrained and both constrained↔higher-capability directions;
- offline success with sufficient local state;
- identical success when optional infrastructure is present;
- failed AUTH;
- missing pre-existing trust;
- stale authorization;
- stale revocation state and explicit revocation;
- stale authorization lineage;
- stale replay/restart continuity;
- incompatible mandatory security floor;
- required binding success/failure;
- attempted trust mutation during normal AUTH.

The harness applies the mandatory-floor compatibility guard outside `association_admission_classify` because profile negotiation remains a separate owner; this is an explicit composition boundary, not a duplicate trust engine.

## Claim boundary

A passing run is executable C decision-composition evidence. It is not physical MCU evidence, network/wire interoperability, Rust/C executable parity for this composition, real CA/cloud outage testing, transport loss testing, formal proof, external review, or proof that `p2p-iot-core` is production-selectable. The profile remains draft.

TD-001 through TD-004 remain open, and zk240/zk241 remain below completion until their declared cross-language, constrained-target, RFC-class, and external evidence exists.
