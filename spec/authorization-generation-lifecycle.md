# Authorization Generation Lifecycle Contract

Status: **draft normative work**. This document defines ownership, advancement, recovery, and use of the local authorization generation consumed by CORE association admission, ENROLL commissioner authorization, resumption, transport continuation, DATA release, and P2P Common Contract decisions. It does not define a wire field, global counter, storage format, physical durability mechanism, or online authorization service.

Normative keywords **MUST**, **MUST NOT**, **SHOULD**, and **MAY** are used in the BCP 14 sense when the stated behavior is testable.

## 1. Ownership

Each local authorization domain that uses generation freshness MUST have exactly one lifecycle authority responsible for the current local `authorization_generation` value. Protocol consumers MUST read that authority; they MUST NOT create independent counters whose values can diverge while still being treated as equivalent authorization state.

The authorization generation is local security state. It is not protocol identity, a transport property, a certificate serial number, a replay epoch, a revocation epoch, or a lineage generation. Implementations MAY physically co-locate these values, but MUST preserve their distinct security meanings and transition rules.

## 2. Generation semantics

A generation identifies the locally accepted authorization-policy state against which generation-bound authorization evidence was issued or last validated. Generation comparison is exact within the local authority domain: evidence bound to generation `g` is current only while the local authority reports `g` as current for that domain.

An implementation MUST NOT silently treat a missing, corrupt, restoring, incomparable, or rollback-suspected generation as current. Such state MUST fail closed for operations that depend on generation freshness.

## 3. Advancement

The lifecycle authority MUST advance the authorization generation whenever a committed local authorization change would make previously issued generation-bound authorization evidence unsafe to reuse without revalidation. Examples include removal or narrowing of granted authority, commissioner-authority replacement, policy replacement that invalidates prior grants, or another explicitly specified local invalidation event.

Pure authentication success, transport reconnect, address change, process restart, possession of a resumption secret, successful role-proof verification, or availability of optional infrastructure MUST NOT advance or reset the authorization generation.

A generation advance MUST be committed before newly issued generation-bound authorization evidence is treated as current. Where the same transition also invalidates predecessor authorization, revocation, lineage, replay, or resumption state, the implementation MUST use an atomic or recoverable commit protocol consistent with `lifecycle-persistence-freshness.md`.

## 4. Evidence binding

Generation-bound authorization evidence MUST identify, directly or through authenticated local metadata, the generation under which it is valid. A consumer MUST compare that binding with the current generation supplied by the lifecycle authority before granting the affected operation.

A cryptographically valid credential, grant, session, ticket, cached resolver result, channel binding, or transport identity MUST NOT repair a generation mismatch.

## 5. Consumer behavior

Consumers MUST preserve their own terminal-action semantics while enforcing the same freshness invariant:

- CORE association admission MUST fail closed when authorization generation is stale.
- ENROLL MUST deny issuance when commissioner authorization generation is stale.
- resumption MUST require safe full AUTH or reject according to its lifecycle contract when generation freshness cannot be established; full AUTH MUST NOT itself mutate trust or synthesize current authorization.
- transport continuation MUST NOT continue an association whose authorization generation is stale; route or connection migration cannot repair the mismatch.
- DATA release MUST deny release when generation-bound authorization is stale.
- P2P admission and bounded delegation MUST NOT use infrastructure availability, delegation validity, or successful AUTH to repair stale local authorization generation.

## 6. Restart and recovery

On restart, the implementation MUST restore the current authorization generation together with sufficient integrity/freshness context to determine whether it is usable. Until recovery establishes the required state as `FRESH`, generation-dependent authorization MUST NOT be admitted.

If power loss or partial commit leaves ambiguity over whether an advancement became authoritative, the affected domain MUST enter `STALE_OR_UNKNOWN` or `ROLLBACK_SUSPECTED`; it MUST NOT select the lower generation merely because that restores availability.

Recovery MUST NOT derive a replacement authorization generation from transport state, peer input, an unauthenticated clock, normal AUTH, or optional online infrastructure. A deployment MAY synchronize authorization information through infrastructure, but the accepted local generation remains the authority used by the local decision.

## 7. Revocation, lineage, replay, and epoch separation

Authorization-generation freshness is necessary where a profile uses it but is not sufficient authorization by itself. A current generation MUST NOT override explicit revocation, stale revocation state, stale lineage, broken replay continuity, invalid channel binding, expired scope/validity, or rollback suspicion.

Conversely, fresh revocation or replay state MUST NOT make stale authorization-generation evidence current. Implementations MUST NOT collapse these independent domains into a single permissive boolean.

## 8. Conformance scenarios

Shared Rust/C decision evidence for generation-consuming surfaces SHOULD retain at least these scenarios:

```text
AGL-01 current generation + otherwise valid authorization -> consumer may proceed to remaining checks
AGL-02 predecessor authorization generation -> fail closed or safe full-AUTH-required according to consumer contract
AGL-03 valid cryptographic AUTH with predecessor generation -> generation remains stale
AGL-04 restart with generation recovery incomplete -> generation-dependent operation not admitted
AGL-05 rollback-suspected generation storage -> fail closed
AGL-06 transport reconnect/address change -> generation unchanged
AGL-07 optional infrastructure unavailable with current local generation -> no new authorization failure solely from infrastructure absence
AGL-08 optional infrastructure available with stale local generation -> infrastructure presence does not repair the stale decision
AGL-09 bounded delegation valid but local authorization generation stale -> fail closed
AGL-10 generation advance composed with predecessor invalidation -> predecessor cannot reappear as current after recovery
```

The current repository has direct negative decision evidence for stale authorization generation in association admission (`ASC2-006`) and ENROLL commissioner authorization (`ENR3-020`). Other generation-consuming surfaces MUST retain equivalent tests wherever they claim support.

## 9. Implementation requirements

An implementation claiming authorization-generation freshness MUST document:

- the local authority domain and owner;
- what events advance the generation;
- how generation-bound evidence records its binding;
- comparison semantics, including wrap/exhaustion behavior if a numeric representation is used;
- restart and rollback behavior;
- atomic/recoverable composition with related lifecycle transitions;
- bounded storage/resource behavior for constrained profiles;
- positive and negative decision evidence for every consuming surface it claims.

A numeric counter is not required. If a counter is used, wraparound MUST NOT make predecessor evidence current. Exhaustion or inability to establish an unambiguous successor MUST fail closed or require an explicit reinitialization/re-enrollment procedure specified by the profile.

## 10. Evidence and claim boundary

Host decision tests can establish ownership/comparison and fail-closed semantics. They do not establish flash atomicity, monotonic-storage correctness, physical rollback resistance, power-loss safety, endurance, update latency, or MCU resource budgets. Those require physical TD-002 evidence.

Symbolic models may analyze the generation-freshness abstraction but do not establish persistence correctness or computational soundness. Independent cryptographic review, RFC-class completeness, physical target qualification, and deployment qualification remain separate evidence states.

This document advances TD-004 by making the authorization-generation lifecycle independently implementable at the semantic level. It does not close TD-004, make `iot-core` or `p2p-iot-core` selectable, or imply IETF/RFC status.
