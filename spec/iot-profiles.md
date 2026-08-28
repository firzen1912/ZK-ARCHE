# ZK-ARCHE IoT Profile Specification

Status: **draft normative work**. This document defines how protocol/security profile identifiers map to immutable prescriptive contracts and records the current draft `iot-core` contract. It does not make AUTH v3 selectable, stabilize `profile_id = 0x0001`, or clear replay-epoch/resource evidence blockers.

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

Current definition revision and fingerprint:

```text
definition_revision=3
contract_sha256=d5ed5c886bf7f7b480975bebecbc995a2370c846373821817ac50c7cd0b41d62
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
| replay policy | `accepted-auth1-fifo-window` | bounded duplicate-suppression window over successfully accepted AUTH_1 replay identifiers |
| replay minimum entries | `64` | every conformant `iot-core` implementation must retain at least 64 accepted replay identifiers |
| replay epoch rule | `unresolved` | authenticated fresh-epoch recovery still blocks promotion/selectability |
| restart replay rule | `restore-trusted-state-or-continuity-broken` | after restart, restore sufficiently recent trusted replay state or fail closed in `CONTINUITY_BROKEN` |
| resource evidence | `required-before-stable` | TD-002 measurements still required |
| selectable | `0` | implementations MUST NOT advertise/select this profile yet |

The capability set intentionally excludes `AUTH_V2`, pairing/TOFU enrollment features, legacy runtime-profile marker bits, vendor/private bits, and `CBOR_FRAMING`. Those values either describe v2/setup behavior, local runtime posture, private behavior, or framing rather than active AUTH-v3 security semantics.

## 4. Accepted-AUTH1 replay-window contract

The replay window is a bounded admission-state contract. It is not a substitute for authenticated AUTH-v3 completion, and it does not by itself establish rollback- or epoch-safe replay continuity.

For draft `iot-core` revision 3:

1. an implementation MUST retain at least the 64 most recently inserted, distinct replay identifiers for successfully accepted AUTH_1 inputs;
2. retention order MUST be FIFO: when the configured capacity is full, inserting a new distinct identifier evicts the oldest retained identifier;
3. a replay identifier already present in the retained window MUST be rejected as replay, including when the same authenticated AUTH_1 material is presented under a different outer `session_id` or sequence value;
4. a failed or not-yet-accepted AUTH_1 MUST NOT consume a replay entry;
5. replay state MUST be committed only after the implementation has completed the AUTH_1 checks required for acceptance;
6. implementations with capacity greater than 64 MAY retain a larger window, but they MUST preserve the same FIFO decision semantics and MUST NOT provide less than the 64-entry floor;
7. the exact AUTH-v3 replay-identifier derivation remains owned by the AUTH-v3 normative state-machine/transcript specification and MUST be byte/decision compatible across Rust and C before production v3 selection.

The shared decision corpus:

```text
rust/test-vectors/replay-cache/fifo-capacity-64.txt
```

is the canonical minimum-capacity fixture. Rust and C tests consume the same corpus. FT-022 additionally exercises same accepted AUTH material under a fresh outer session/sequence, and FT-023 exercises concurrent duplicate submission.

The current ProVerif replay table remains persistent and unbounded, so its accepted-message replay result is stronger than this runtime retention contract after eviction or restart. Formal claims MUST retain that assumption boundary.

## 5. Replay continuity and fail-closed profile status

`restart_replay_rule` is resolved for draft revision 3 as `restore-trusted-state-or-continuity-broken`, matching `spec/replay-continuity.md` and the shared Rust/C replay-continuity decision corpus. Before the first post-restart AUTH acceptance in an existing replay domain, an implementation MUST either establish a trusted restored replay window satisfying the profile floor or enter `CONTINUITY_BROKEN`. It MUST NOT initialize an empty replay cache, accept a fresh outer session as recovery, or infer continuity from transport reconnection.

`replay_epoch_rule` remains `unresolved` because the authenticated fresh-epoch transition, predecessor-epoch handling, interrupted-transition behavior, and target rollback evidence are not yet defined and retained.

The profile therefore still makes **no** normative claim that:

- an AUTH_1 replay evicted from the bounded window is rejected indefinitely;
- replay state is rollback-resistant on a concrete target;
- loss of trusted replay state is recoverable without an authenticated fresh-epoch transition;
- the persistent/unbounded formal replay abstraction is runtime-equivalent.

A conformant implementation or test harness MUST fail closed if `selectable=0`. It MUST NOT infer unresolved replay-epoch semantics from the local cache implementation, infer MCU readiness from host tests, or replace an unresolved field with an implementation default.

Before promotion to `stable`, every remaining `unresolved` value MUST be replaced by reviewed normative semantics and the manifest/fingerprint regenerated. The resulting definition must then receive Rust/C conformance evidence and the review required by the roadmap.

## 6. Replacement and deprecation rules

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

## 7. Rust/C semantic-parity corpus

The shared `iot-core-v1.profile` manifest is the cross-language profile-definition fixture.

Rust and C conformance tests MUST independently verify at minimum:

1. the SHA-256 fingerprint and draft definition revision;
2. profile ID, version, suite, and exact capability masks;
3. that required capabilities are a subset of allowed capabilities;
4. that allowed and forbidden capability masks do not overlap and cover the full 64-bit selected-capability namespace;
5. authorization schema identity and 148-byte context size;
6. `none` critical-extension and channel-binding policies;
7. the 2048-byte datagram ceiling;
8. `accepted-auth1-fifo-window` and the 64-entry minimum;
9. `restart_replay_rule=restore-trusted-state-or-continuity-broken`;
10. unresolved `replay_epoch_rule` and `selectable=0` while remaining blockers exist;
11. immutable replacement/deprecation policy fields.

The replay-window semantic fixture additionally requires the shared `fifo-capacity-64.txt` decisions to remain equal across Rust and C. The replay-continuity corpus independently exercises the fail-closed restart/state-loss decisions represented by the manifest restart rule.

These fixtures establish **same-ID semantic parity**, not production interoperability. They do not enable HELLO advertisement or AUTH-v3 dispatch.

## 8. Promotion gate

`profile_id = 0x0001` MUST remain `draft` and non-selectable until at least:

- authenticated replay-epoch recovery, rollback, and interrupted-transition semantics are normative and tested;
- TD-002 constrained target measurements satisfy the declared profile resource/evidence contract, including the cost of the required replay floor and persistence assumptions;
- Rust and C production AUTH-v3 state machines validate the same profile semantics;
- unknown/incompatible profile and capability negative tests pass;
- security/privacy considerations are updated for the final profile;
- the registry marks the final definition stable through checkpoint review.

Until then, the machine-readable contract is an evidence and conformance anchor only.
