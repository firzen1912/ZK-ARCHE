# ZK-ARCHE Negotiation and Selection State Machine

Status: **draft normative work**. This document defines the fail-closed selection boundary between peer advertisements and an authenticated ZK-ARCHE security context. It preserves deployed v2 compatibility and constrains draft AUTH-v3 work; it does not promote AUTH v3, `iot-core`, `iot-edge`, or `p2p-iot-core` to production-selectable status.

The registries in `spec/registries.md` remain authoritative for allocation status. `spec/zk-arche-protocol.md` remains authoritative for AUTH-v3 cryptographic context and completion semantics.

## 1. Security objective

Negotiation MUST select one internally compatible protocol version, suite, profile, capability set, critical-extension set, and channel-binding policy before those values can authorize AUTH processing. A peer MUST NOT derive stronger security semantics from raw advertisement intersection alone.

Negotiation MUST NOT:

- turn an unknown, reserved, deprecated, experimental, private-use, or draft-only value into production semantics merely because both peers advertise it;
- remove a mandatory security behavior required by the selected version/profile;
- treat a transport address as protocol identity;
- create enrollment, authorization, delegation, or trusted-peer state;
- make CA, DNS, Internet, manufacturer-cloud, gateway, blockchain, or central-registry access a hidden prerequisite for core AUTH between already-authorized peers with sufficiently fresh local state;
- reinterpret a selected version using another version's transcript, key schedule, completion, or error semantics.

Normal negotiation and AUTH are **NO-LEARNING**. Trust mutation remains an explicit lifecycle operation.

## 2. Inputs

A selection attempt consumes local policy plus peer/local advertisements. At minimum the selection function has these conceptual inputs:

```text
local_supported_versions
peer_supported_versions
local_supported_suites
peer_supported_suites
local_profile_policy
peer_profile_advertisement          (when defined by the selected version)
local_capability_advertisement
peer_capability_advertisement
local_extension_policy
peer_extension_advertisement        (when defined by the selected version)
local_channel_binding_policy
transport_context
production_or_test_mode
```

Raw advertisements are untrusted negotiation input until the selected security context is authenticated by the applicable AUTH transcript.

`transport_context` MAY supply a registered channel-binding input. A source/destination address, connection handle, interface identifier, or other locator MUST NOT become protocol identity merely because a transport adapter exposes it.

## 3. Selection states

The minimum semantic states are:

```text
NEGOTIATION_START
  -> VERSION_SELECTED
  -> SUITE_SELECTED
  -> PROFILE_SELECTED_OR_V2_COMPAT
  -> SECURITY_CAPABILITIES_SELECTED
  -> CRITICAL_EXTENSIONS_SELECTED
  -> CHANNEL_BINDING_SELECTED
  -> CONTEXT_READY
  -> AUTH_IN_PROGRESS
```

Any incompatible or unsupported security-relevant input transitions to `NEGOTIATION_REJECTED`. There is no transition from `NEGOTIATION_REJECTED` to `AUTH_IN_PROGRESS` for the same selection attempt.

`CONTEXT_READY` means only that a syntactically and semantically compatible context has been selected. It is **not** authentication success, authorization success, enrollment, trust mutation, or secure-association admission.

## 4. Version selection

A production peer MUST select only a protocol version that:

1. is supported by both peers;
2. is permitted by local policy;
3. is registered as production-selectable for the relevant implementation/profile; and
4. has complete semantics for every mandatory subsequent selection step.

Current production compatibility is version `0x02`.

Version `0x03` (`ZK-ARCHE-AUTH-v3`) remains `draft`. Production negotiation MUST NOT select it until its promotion gate is explicitly satisfied. Test/reference code MAY select it only in an explicitly isolated draft-conformance path that cannot be confused with production capability advertisement.

An unknown, reserved, unsupported, or policy-forbidden version MUST fail closed. A peer MUST NOT silently fall back after it has committed to a selected higher version. Any future fallback mechanism MUST be transcript-bound and must prove downgrade resistance before promotion.

## 5. Suite selection

The selected suite MUST be supported by both peers, allowed by the selected version/profile, and permitted by local policy.

A suite being registry-known does not make it compatible with every version/profile. Unknown, reserved, deprecated-for-selection, or incompatible suites MUST be rejected.

The selected suite is security-significant. For AUTH v3 it MUST equal the `suite_id` authenticated in the canonical security context. A mismatch between negotiated and authenticated suite is fatal.

## 6. Profile selection

For v2 compatibility, legacy runtime capability markers remain governed by existing v2 behavior and MUST NOT be reinterpreted as protocol/security `profile_id` values.

For draft AUTH v3, a profile MUST be explicitly registered and must define its mandatory version/suite, mandatory and forbidden security capabilities, critical-extension rules, channel-binding policy, replay/lifecycle requirements, error/privacy behavior, and evidence gates.

All currently named protocol/security profiles (`iot-core`, `iot-edge`, and `p2p-iot-core`) remain draft and therefore non-selectable in production.

A high-capability peer negotiating with a constrained conformant peer MUST adapt to the mandatory constrained security floor rather than weakening authentication assurance. Optional high-end features MUST remain isolated from the mandatory floor.

## 7. Security-capability selection

Raw HELLO capability advertisements are not the selected security-capability set.

For v2, deployed compatibility behavior remains unchanged.

For draft AUTH v3 and future Common Contract profiles:

- the selected set MUST contain only registry-known protocol-managed capabilities allowed by the selected profile;
- every capability mandatory for that profile MUST be present;
- a forbidden capability MUST NOT be selected;
- legacy runtime-profile markers MUST NOT appear as v3 selected security capabilities;
- vendor/private bits MUST NOT enter a Common Contract selected set;
- an unknown bit MUST NOT gain semantics merely because both peers advertised it;
- a security-critical behavior not representable by a registered selected capability MUST use a registered critical extension or cause rejection.

Negotiation MUST fail rather than clear a mandatory capability to manufacture compatibility.

## 8. Critical-extension selection

Unknown critical extensions MUST cause rejection before AUTH completion. Unknown non-critical extensions MAY be ignored only where the selected version/profile explicitly permits ignorance at that location.

An extension affecting authentication, authorization, trust, transcript construction, key schedule, replay, downgrade behavior, privacy, or channel binding MUST be treated as critical unless its specification explicitly establishes safe ignorance.

The selected critical-extension set and security-relevant values MUST have one canonical encoding. For AUTH v3 its hash is authenticated as `critical_extensions_hash`.

## 9. Channel-binding selection

Channel binding is profile/version policy, not an implicit property of the transport adapter.

If the selected profile requires a channel binding, negotiation MUST select a registered binding type compatible with the actual channel context. If the selected profile permits no binding, it MUST define the canonical empty representation. Implementations MUST NOT substitute an ad-hoc all-zero hash.

Changing a transport locator without changing the authenticated channel-binding context MUST NOT create a new protocol identity or new authorization. Conversely, when the selected binding semantics require a new binding context, retained AUTH/authorization/resumption state MUST NOT be silently reused under the old binding.

For AUTH v3 the resulting binding is authenticated as `channel_binding_hash`.

## 10. Context commitment and AUTH handoff

Only after all required selections succeed may the implementation enter `CONTEXT_READY`.

For AUTH v3, the handoff MUST provide exactly one immutable semantic selection tuple:

```text
(protocol_version,
 suite_id,
 profile_id,
 selected_capabilities,
 critical_extensions_hash,
 channel_binding_hash)
```

The AUTH instance then adds its `session_id` and `authz_context_hash` to the canonical authenticated security context defined in `spec/zk-arche-protocol.md`.

An implementation MUST NOT recompute the tuple from mutable advertisements midway through AUTH. A change to any security-significant selected value requires rejection or a new explicitly versioned negotiation/AUTH attempt.

Successful negotiation MUST NOT itself grant authorization. Secure-association admission remains separately conditioned on completed AUTH, pre-existing local trust, fresh authorization/revocation/lineage state, replay/restart/rollback continuity, and binding requirements.

## 11. Failure and privacy behavior

Negotiation failures MUST follow the registered error/observable-failure policy for the selected protocol stage. Before peer authentication, implementations SHOULD minimize distinctions that would expose local role, trust, authorization, profile, or policy state beyond what interoperability requires.

Implementations MUST NOT use a more specific wire-visible error than the selected version/profile permits merely because an internal classifier has a more specific reason.

Rate limiting, retry cookies, source validation, and other pre-authentication DoS controls MAY be applied where specified, but they MUST NOT become identity or authorization evidence.

## 12. Conformance requirements and evidence boundary

A production-selectable negotiation path requires evidence for at least:

- positive selection of every mandatory production version/suite/profile combination;
- rejection of unknown/reserved/unsupported versions and suites;
- rejection of draft profiles in production mode;
- rejection when a mandatory capability is absent;
- rejection of unknown critical extensions;
- proof that unknown/private capability bits cannot alter Common Contract security semantics;
- downgrade negatives showing a committed selection cannot be silently reinterpreted;
- Rust/C agreement on selected tuple and accept/reject result where both claim support;
- channel-binding mismatch and transport-locator-independence negatives;
- deterministic vectors or annotated traces for the selected context;
- exact-head qualification evidence.

This document does not claim that those evidence classes are complete. Existing draft AUTH-v3 vectors and classifiers remain development evidence only. Production promotion requires the registry, implementation, vector, interoperability, formal/review, and constrained-profile gates declared elsewhere in `spec/` and the canonical roadmaps.

## 13. Change control

Any change that alters selection precedence, mandatory capabilities, profile compatibility, critical-extension behavior, channel-binding policy, downgrade behavior, or the authenticated selection tuple is security-relevant and requires:

```text
normative specification update
+ registry update when allocation semantics change
+ Rust/C behavior update where both claim support
+ positive and negative vectors/tests
+ downgrade/compatibility analysis
+ versioned change record
```

A documentation-only edit MUST NOT promote an allocation or capability that implementation/evidence does not support.
