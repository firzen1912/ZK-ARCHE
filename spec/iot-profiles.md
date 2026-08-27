# ZK-ARCHE IoT Profile Specification

Status: **draft normative work**. This document defines how protocol/security profile identifiers map to immutable prescriptive contracts and records the current draft `iot-core` contract. It does not make AUTH v3 selectable, stabilize `profile_id = 0x0001`, or clear replay/resource evidence blockers.

## 1. Profile identity rule

A stable `profile_id` identifies an immutable protocol/security contract. Per-session values such as the authenticated `session_id`, authorization context, selected critical extensions, and channel-binding context remain separate authenticated inputs, but they MUST be validated against the selected profile's prescriptive definition.

Once a profile becomes `stable`, a material change to any prescriptive item requires a new profile ID or an explicit replacement/deprecation allocation. Implementations MUST NOT silently reinterpret a stable profile ID across releases.

Draft profiles may evolve before promotion, but every draft definition revision MUST have a deterministic machine-readable manifest and fingerprint so Rust, C, reviews, vectors, and formal evidence can state exactly which semantics they exercised.

This follows the engineering lesson recorded in `docs/research/daily/2026-08-27.md`: profile identity and circumstance-specific selected context are different objects, and known profile identifiers are not sufficient unless peers agree on the same prescriptive semantics.

## 2. Machine-readable contract format — `ZKPROFILE/1`

The canonical manifest is UTF-8 text with exactly one `key=value` field per line, LF line endings, no blank lines, no comments, and the field order shown by the profile definition. The final field is:

```text
contract_sha256=<64 lowercase hex characters>
```

`contract_sha256` is SHA-256 over every byte preceding the `contract_sha256=` line, including the LF after the last prescriptive field. The fingerprint is evidence metadata; it is not currently a wire field.

A parser MUST reject duplicate keys, unknown keys in a conformance manifest, missing required keys, malformed numeric forms, or a fingerprint mismatch. A future format change requires a new `format` value rather than silently changing `ZKPROFILE/1` parsing.

## 3. Draft `iot-core` definition — `profile_id = 0x0001`

The canonical machine-readable definition is:

```text
rust/test-vectors/profiles/iot-core-v1.profile
```

Current fingerprint:

```text
31b53234616189ce470c8c7f2d3d446432bb20953a2f4e5a191fd356a1f54ad4
```

The current prescriptive values are:

| Field | Draft value | Meaning |
|---|---|---|
| protocol version | `0x03` | AUTH-v3 semantics only |
| suite | `0x0001` | current Ristretto255/SHA-256 suite |
| required selected capabilities | `0x0000000000000006` | `ROLE_RERAND` + `ROLE_SET_MEMBERSHIP` |
| allowed selected capabilities | same as required | no extra selected security capability in draft v1 |
| forbidden selected capabilities | `0xfffffffffffffff9` | every bit outside the exact allowed set |
| authorization schema | `iot-core-authz-v1` | schema in `spec/iot-core-authorization-context.md` |
| authorization context size | `148` bytes | exact canonical ZKCTX AUTHORIZATION size |
| critical extensions | `none` | no critical extension is selectable in draft v1 |
| channel binding | `none` | native baseline does not require an external transport/channel binding |
| maximum datagram | `2048` bytes | current transport-neutral implementation ceiling |
| replay policy | `unresolved` | blocks promotion/selectability |
| replay minimum entries | `unresolved` | blocks promotion/selectability |
| replay epoch rule | `unresolved` | blocks promotion/selectability |
| restart replay rule | `unresolved` | blocks promotion/selectability |
| resource evidence | `required-before-stable` | TD-002 measurements still required |
| selectable | `0` | implementations MUST NOT advertise/select this profile yet |

The capability set intentionally excludes `AUTH_V2`, pairing/TOFU enrollment features, legacy runtime-profile marker bits, vendor/private bits, and `CBOR_FRAMING`. Those values either describe v2/setup behavior, local runtime posture, private behavior, or framing rather than active AUTH-v3 security semantics.

## 4. Prescriptive versus unresolved state

A machine-readable contract is not automatically a complete profile. `iot-core` remains non-selectable because replay lifetime/epoch/restart semantics and constrained-target resource qualification are unresolved.

A conformant implementation or test harness MUST fail closed if `selectable=0`. It MUST NOT infer a replay policy from the local cache implementation, infer MCU readiness from host tests, or replace an unresolved field with an implementation default.

Before promotion to `stable`, every `unresolved` value MUST be replaced by reviewed normative semantics and the manifest/fingerprint regenerated. The resulting definition must then receive Rust/C conformance evidence and the review required by the roadmap.

## 5. Replacement and deprecation rules

The manifest records:

```text
prescriptive_change_rule=new-profile-id
deprecated_selectable=0
stable_semantics_mutable=0
```

Therefore:

- after stabilization, changing mandatory version, suite, capability set, authorization schema, extension policy, channel-binding policy, replay semantics, or normative resource/security behavior requires a new profile allocation;
- a deprecated profile remains parseable only where compatibility policy permits, but MUST NOT be selected for a new session;
- a stable profile's prescriptive semantics MUST NOT be mutated in place;
- a replacement profile MUST identify its predecessor/successor relationship in the registry and carry its own vectors/evidence.

## 6. Rust/C semantic-parity corpus

The shared `iot-core-v1.profile` manifest is the initial cross-language profile-definition fixture.

Rust and C conformance tests MUST independently verify at minimum:

1. the SHA-256 fingerprint;
2. profile ID, version, suite, and exact capability masks;
3. that required capabilities are a subset of allowed capabilities;
4. that allowed and forbidden capability masks do not overlap and cover the full 64-bit selected-capability namespace;
5. authorization schema identity and 148-byte context size;
6. `none` critical-extension and channel-binding policies;
7. the 2048-byte datagram ceiling;
8. the unresolved replay fields;
9. `selectable=0` while unresolved blockers remain;
10. immutable replacement/deprecation policy fields.

These fixtures establish **same-ID semantic parity**, not production interoperability. They do not enable HELLO advertisement or AUTH-v3 dispatch.

## 7. Promotion gate

`profile_id = 0x0001` MUST remain `draft` and non-selectable until at least:

- replay lifetime/epoch, capacity floor, eviction, restart, and rollback semantics are normative;
- TD-002 constrained target measurements satisfy the declared profile resource/evidence contract;
- Rust and C production AUTH-v3 state machines validate the same profile semantics;
- unknown/incompatible profile and capability negative tests pass;
- security/privacy considerations are updated for the final profile;
- the registry marks the final definition stable through checkpoint review.

Until then, the machine-readable contract is an evidence and conformance anchor only.
