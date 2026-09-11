# ZK-ARCHE Authorization-Aware Resumption Lifecycle

Status: **draft normative logical contract / storage-neutral / wire-unassigned**.

This document defines the minimum fail-closed decision semantics that a future ZK-ARCHE resumption mechanism must satisfy before a stored resumption credential may preserve an authenticated association. It intentionally does **not** define a ticket encoding, PSK identity, on-wire resumption handshake, cipher-suite construction, storage backend, or production-selectable protocol mode.

Normative keywords **MUST**, **MUST NOT**, **SHOULD**, and **MAY** are used in the BCP 14 sense when the stated behavior is testable.

The purpose of this contract is to make authorization-preserving resumption and credential-reuse bounds implementable without allowing possession of a still-valid resumption secret to become an independent source of trust or privilege.

## 1. Security boundary

Resumption is a distinct lifecycle mode, not an abbreviated authorization bypass.

A resumption credential proves only that a peer possesses state derived from a prior authenticated association. Possession of that credential MUST NOT by itself:

- create or expand trust;
- repair missing local trust or authorization state;
- restore revoked authority;
- preserve privilege after an authorization, policy, revocation, lineage, profile, suite, or binding change;
- establish a fresh replay epoch after continuity loss;
- reset key-usage accounting;
- turn transport identity or address continuity into protocol identity.

Normal AUTH remains NO-LEARNING. Resumption inherits that rule.

If the verifier cannot establish every mandatory condition in this document from locally authoritative state, it MUST reject resumption. It MAY fall back to the applicable full AUTH flow only when that flow independently satisfies its own trust, authorization, replay, and lifecycle requirements.

## 2. Logical retained record

A future implementation MAY encode or store these facts differently, but its authoritative retained resumption state MUST be logically equivalent to at least:

```text
resumption_id                    opaque local identifier; not protocol identity
peer_binding                     exact locally attributed peer/security-object binding
holder_binding                   authenticated holder binding
 audience_id                     locally authorized audience/deployment binding
role_policy_id                   selected role/policy identity
scope_bits                       exact authorized scope
 authorization_generation        authorization generation at issuance
policy_epoch                     policy epoch at issuance
revocation_epoch                 revocation epoch at issuance
lineage_generation               credential/trust lineage generation at issuance
profile_id                       selected profile
suite_id                         selected suite/method
channel_binding_context          required binding identity/hash, or explicit NONE where allowed
replay_epoch                     authenticated replay-continuity epoch
key_epoch                        key/rekey generation from which the credential was derived
issued_at                        issuance time or equivalent monotonic issuance point
expires_at                       hard expiry or equivalent bounded lifetime
use_count                        durable successful-use count
max_uses                         finite configured reuse bound
credential_generation            durable record generation for rollback/freshness checks
```

Whitespace above is descriptive only and does not define a wire grammar.

An implementation MAY maintain additional fields. Optional fields MUST NOT weaken or replace any mandatory comparison or continuity decision below.

## 3. Current authority input

Before accepting resumption, the verifier MUST resolve current locally authoritative state for the same peer/holder and obtain the active values corresponding to:

```text
peer_binding
holder_binding
audience_id
role_policy_id
scope_bits
authorization_generation
policy_epoch
revocation_epoch
lineage_generation
profile_id
suite_id
channel_binding_context
replay_epoch
key_epoch
current time / monotonic lifetime point
current durable resumption-record generation
```

The lookup itself is not authority merely because it is cached. Cached state MAY be used only when its freshness is established under the applicable invalidation and persistence rules.

A CA, cloud service, central registry, DNS service, manufacturer service, gateway, or online policy service is not required for this decision when the peer already possesses sufficient locally authoritative state. Optional infrastructure MAY provide fresher synchronization information, but absence of infrastructure MUST NOT cause the verifier to synthesize newer authority facts.

## 4. Authorization-preservation rule

A resumption attempt MUST be rejected unless the retained record and current authority are equal for every privilege-bearing field whose semantics require exact continuity.

For the initial logical contract, the following values MUST match exactly:

```text
peer_binding
holder_binding
audience_id
role_policy_id
scope_bits
authorization_generation
policy_epoch
revocation_epoch
lineage_generation
profile_id
suite_id
channel_binding_context
replay_epoch
key_epoch
```

A newer authorization generation, policy epoch, revocation epoch, lineage generation, replay epoch, or key epoch MUST NOT silently upgrade an older retained resumption record. The older record is stale and MUST be rejected or invalidated.

A scope change is not a compatible narrowing/expansion operation for an existing record. Any scope change requires a newly authorized lifecycle transition and a newly issued resumption credential under that resulting authority state.

If channel binding is mandatory for the selected profile/transport mapping, a changed or unavailable binding MUST reject resumption. If the applicable profile explicitly permits no external channel binding, both issuance and resumption MUST represent that choice unambiguously rather than treating missing binding data as a wildcard.

## 5. Reuse and lifetime bounds

A resumption credential MUST have a bounded lifetime and a bounded reuse policy.

Acceptance requires all of the following:

1. the credential is not expired under the implementation's authoritative lifetime source;
2. `max_uses` is finite and non-zero;
3. `use_count < max_uses` before the attempted use;
4. successful use advances the durable use state before the implementation reports resumed authenticated completion when rollback could permit reuse beyond the bound;
5. failure to durably advance required reuse state fails closed;
6. restart MUST NOT reset the use counter, expiry, or credential generation;
7. rollback suspicion or an unverifiable retained generation MUST reject the credential.

An implementation MUST NOT reinterpret clock rollback, process restart, transport reconnection, address change, cache eviction, or storage loss as a fresh credential lifetime.

A future profile MAY set `max_uses = 1`; another profile MAY permit a larger bounded value. An unbounded sentinel is not conformant to this contract.

## 6. Replay and restart continuity

Resumption MUST compose with `spec/replay-continuity.md` and any authenticated replay-epoch recovery contract.

A valid resumption secret or ticket MUST NOT repair `CONTINUITY_BROKEN` replay state. If replay continuity is lost, stale, rolled back, unverifiable, or bound to a different authenticated replay epoch, resumption MUST fail closed.

Process restart MAY reload a retained credential only when its persistent generation, replay epoch, use state, and all authorization/lifecycle bindings remain current and verifiable. A structurally valid record is insufficient evidence of freshness.

A future authenticated fresh replay-epoch transition may invalidate or replace old resumption state, but resumption MUST NOT itself invent that transition.

## 7. Rekey and dependent-state invalidation

A resumption credential is dependent state. The implementation MUST invalidate or reject it when an upstream authority transition changes any mandatory binding in Section 4.

At minimum, the following transitions require rejection of a record issued under the predecessor state unless a future reviewed specification explicitly defines a safe replacement transition:

- credential or trust-lineage replacement;
- authorization generation change;
- policy epoch change;
- revocation epoch change;
- explicit revocation of the peer/credential/authorization lineage;
- profile or suite change;
- required channel-binding change;
- replay-epoch recovery/replacement;
- key epoch/rekey replacement;
- holder, audience, role/policy, or scope change.

Rekey MUST NOT merely re-encrypt or copy an old resumption credential while retaining predecessor authority facts.

## 8. Fail-closed decision classes

Rust and C implementations claiming this logical behavior SHOULD expose decision-equivalent outcomes for shared qualification. Local API names may differ, but the following semantic classes are required:

```text
ACCEPT
REJECT_INVALID_RECORD
REJECT_PEER_BINDING
REJECT_HOLDER_BINDING
REJECT_AUDIENCE
REJECT_ROLE_POLICY
REJECT_SCOPE
REJECT_AUTHORIZATION_GENERATION
REJECT_POLICY_EPOCH
REJECT_REVOCATION_EPOCH
REJECT_LINEAGE
REJECT_PROFILE
REJECT_SUITE
REJECT_CHANNEL_BINDING
REJECT_REPLAY_CONTINUITY
REJECT_KEY_EPOCH
REJECT_EXPIRED
REJECT_USE_LIMIT
REJECT_ROLLBACK_OR_GENERATION
REJECT_REVOKED
```

Implementations MAY collapse externally visible protocol errors to reduce privacy leakage. Internal qualification MUST nevertheless be able to distinguish enough causes to prove the required precedence and invalidation behavior without exposing those distinctions to an unauthenticated peer.

When multiple facts are invalid simultaneously, implementations MUST use one documented deterministic precedence for shared Rust/C qualification. The precedence itself MUST NOT permit a later check to repair or override an earlier fail-closed result.

## 9. Required shared qualification

Before ZK-ARCHE claims authorization-aware resumption as TESTED or INTEROPERABLE, shared Rust/C evidence MUST include at least:

- one accepted current record;
- stale authorization generation;
- stale policy epoch;
- stale revocation epoch;
- explicitly revoked authority;
- stale lineage generation;
- holder/audience/role/scope mismatch;
- profile and suite mismatch;
- required channel-binding mismatch;
- replay-continuity loss and replay-epoch mismatch;
- stale key epoch after rekey;
- expiry boundary;
- use-count boundary (`max_uses - 1` accepted if all other facts are current, `max_uses` rejected);
- restart with preserved current durable use state;
- restart with rolled-back use/generation state;
- infrastructure unavailable while sufficient current local authority exists;
- simultaneous-failure precedence cases;
- full-AUTH fallback being a separate fresh decision rather than mutation of the rejected resumption record.

The same canonical corpus SHOULD drive Rust and C decision tests wherever both implementations claim the behavior.

## 10. Privacy and observability

Resumption may create cross-session linkability through a ticket/PSK identity, local handle, cache key, reuse policy, authorization-generation handle, or stable error behavior.

A future wire-visible mechanism MUST define:

- what stable material is observable;
- whether a credential is single-use or multi-use;
- how identifiers are refreshed;
- externally visible failure/no-response behavior;
- whether resumption can be probed before peer authentication;
- how ticket storage or lookup avoids becoming an identity oracle.

This logical contract does not claim unlinkability or privacy-equivalence evidence.

## 11. Mandatory implementation boundary

This document owns logical resumption eligibility only.

It does **not** authorize implementation-defined choices for:

- ticket/PSK wire encoding;
- ticket encryption/authentication format;
- resumption KDF labels or secret derivation;
- a 0-RTT or 1-RTT message flow;
- anti-replay policy for application data;
- registry identifiers;
- transport-specific binding extraction;
- storage transaction format;
- wall-clock versus monotonic-time encoding;
- authenticated fresh replay-epoch creation.

Those mechanisms require their own normative specification, vectors, Rust/C behavior, and evidence before they can be selected in production.

Until then, code MUST NOT infer missing wire or cryptographic semantics from this document.

## 12. Evidence and claim boundary

The existence of this contract establishes only a **DEFINED normative logical prerequisite** for authorization-aware resumption and bounded credential reuse.

It does not establish that resumption is IMPLEMENTED, TESTED, INTEROPERABLE, COMMON-CONFORMANT, FORMALLY ANALYZED, EXTERNALLY REVIEWED, RFC-CLASS DOCUMENTED, or DEPLOYMENT-QUALIFIED.

Promotion requires, as applicable:

```text
normative wire/state-machine specification
Rust implementation
C implementation
shared positive/negative deterministic corpus
restart/rollback persistence evidence
replay and rekey composition evidence
transport/channel-binding evidence
formal model + retained exact-model result
privacy/observability analysis
constrained-target measurements
independent cryptographic review where the mechanism depends on reviewed cryptography
```

This contract is intentionally a prerequisite for those implementation and assurance lanes rather than a substitute for them.