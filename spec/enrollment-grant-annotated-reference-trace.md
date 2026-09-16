# Enrollment Grant Annotated Reference Trace

Status: RFC-class trace material for the implementation-backed enrollment-grant decision contract. This document is informative trace material over the normative behavior in `spec/enrollment-grant-issuance.md`; it does not define a new ENROLL wire format.

## Purpose

This trace makes the existing `enrollment-grant-v4` decision surface independently followable without treating Rust or C source code as normative. It binds the decision sequence to the canonical corpus and records which facts are security-authoritative at each step.

A conformant implementation still MUST reproduce the canonical corpus result and reason. This trace is explanatory evidence, not a substitute for executable Rust/C qualification.

## Trace A — current explicit enrollment

Canonical case: `ENR4-001`.

Initial facts:

```text
explicit_enroll_operation = true
normal_auth_path = false
commissioner_authenticated = true
commissioner_authorized = true
commissioner_authorization_fresh = true
commissioner_authorization_generation_bound = true
commissioner_authorization_generation_current = true
commissioner_not_revoked = true
enrollment_nonce_unused = true
subject_possession_verified = true
requested_authority_within_commissioner_scope = true
scope_bounded = true
audience_bound = true
deployment_bound = true
validity_bounded = true
epoch_current = true
revocation_current = true
lineage_current = true
delegation_depth_within_limit = true
rollback_suspected = false
```

Decision walk:

1. No rollback suspicion is present.
2. The request is not the normal AUTH path and is explicitly an ENROLL operation.
3. Commissioner authentication and authorization are established locally.
4. Commissioner authorization is fresh, has authenticated generation provenance, and belongs to the current local authorization generation.
5. The commissioner is not revoked.
6. The enrollment operation identifier is unused in the locally enforced replay window.
7. Subject possession is verified.
8. Requested authority is within commissioner scope and scope/audience/deployment/validity are bounded.
9. Policy epoch, revocation view, and lineage are current.
10. Delegation depth is within the configured bound.

Result:

```text
ISSUE / CURRENT
```

`ISSUE` authorizes only the local issuance decision. It does not itself persist trust, define grant bytes, create a signature, advance a replay store, or authorize later AUTH.

## Trace B — normal AUTH cannot learn trust

Canonical case: `ENR4-031`.

The request presents otherwise-current commissioner and subject facts, but `normal_auth_path = true` and delegation depth is also excessive. The classifier stops at the normal-AUTH prohibition before evaluating delegation depth.

Result:

```text
DENY / NORMAL_AUTH_FORBIDDEN
```

This ordering is security-significant. A successful AUTH exchange, possession proof, or delegation artifact cannot become an implicit enrollment operation. Normal AUTH remains NO-LEARNING.

## Trace C — stale authorization generation

Canonical case: `ENR4-030`.

The commissioner is authenticated and authorized and the authorization carries authenticated generation provenance, but that generation is not the current local authorization generation. Other later facts cannot repair the stale authority.

Result:

```text
DENY / COMMISSIONER_AUTHORIZATION_GENERATION_STALE
```

The generation value is supplied by existing authorization/lifecycle authority. Transport identity, caller metadata, successful AUTH, or subject possession cannot synthesize currentness.

## Trace D — revoked commissioner dominates replay and lineage faults

Canonical case: `ENR4-025`.

The commissioner is authenticated, authorized, fresh, and generation-current, but `commissioner_not_revoked = false`. The same vector also carries an already-used enrollment nonce and stale lineage. Required precedence selects revocation first.

Result:

```text
DENY / COMMISSIONER_REVOKED
```

This prevents a later replay or lineage diagnostic from obscuring the authoritative fact that the issuer no longer possesses enrollment authority.

## Trace E — authority escalation dominates excessive delegation depth

Canonical case: `ENR4-026`.

The commissioner is otherwise current, but the requested authority exceeds commissioner scope and delegation depth is also excessive. Authority containment is evaluated before delegation depth.

Result:

```text
DENY / AUTHORITY_ESCALATION
```

Delegation depth is an additional bound; it is never an alternate trust root and cannot widen issuer authority.

## Trace F — rollback dominates otherwise valid state

Canonical case: `ENR4-019`.

All positive issuance facts are present, including bounded delegation depth, but rollback suspicion is true.

Result:

```text
DENY / ROLLBACK_SUSPECTED
```

Rollback suspicion has highest precedence because apparently fresh authorization, revocation, lineage, and replay facts cannot be trusted to represent the newest durable state while rollback remains unresolved.

## Independent implementation checklist

An implementation consuming this contract MUST:

- distinguish AUTH from explicit ENROLL before trust mutation;
- obtain commissioner authorization generation provenance from authenticated/local lifecycle authority rather than caller or transport metadata;
- evaluate commissioner revocation before enrollment replay and subject possession;
- reject requested authority broader than commissioner authority;
- enforce bounded scope, audience, deployment, validity, and delegation depth;
- evaluate current epoch, revocation view, and lineage;
- fail closed on rollback suspicion;
- reproduce both action and reason for canonical compound-fault vectors.

An implementation MUST NOT infer that a successful decision makes trust transitive. A receiving peer still applies its own local trust policy to any subsequently presented grant.

## Evidence and claim boundary

The authoritative deterministic source for these examples is `rust/test-vectors/state/enrollment-grant-v4.txt`. The Rust and C classifiers consume the same fact model and precedence contract. This trace does not claim that the current exact HEAD was executed in this environment.

This document does not establish cryptographic grant encoding or verification, durable one-time replay storage, crash/power-loss atomicity, rollback-resistant storage, revocation propagation latency, physical MCU measurements, formal proof, independent cryptographic review, IETF/RFC status, or deployment qualification. Those remain separate roadmap evidence requirements.
