# P2P Local Trust and Bounded Delegation Contract

Status: pre-RFC engineering contract; not an IETF or RFC document.

## 1. Scope

This document owns the mandatory Common Contract semantics for local trust, non-transitivity, and the boundary around future bounded delegation. It does not promote `p2p-iot-core`, does not add a new wire object, and does not claim executable delegation support.

## 2. Local trust is the root decision

Trust is local and non-transitive by default.

A trust decision MUST NOT become transitive merely because an accepted peer trusts another peer. If A locally trusts B and B locally trusts C, A MUST NOT accept C solely from B's trust relationship.

Normal AUTH remains NO-LEARNING. Successful authentication MUST NOT create, expand, repair, or persist a trust relationship or delegation as a side effect.

Already-authorized peers with sufficient local state MUST be able to make the mandatory root authentication decision without requiring a CA, cloud identity provider, central registry, DNS, Internet connectivity, blockchain, manufacturer cloud, or gateway/controller approval.

## 3. Delegation boundary

Delegation MUST be explicit, scoped, audience-bound, depth-bounded, validity-bounded, issuer-bound, epoch-bound, and revocable.

A valid delegation MAY make a subject eligible for local authorization evaluation; it MUST NOT create automatic persistent trust.

A peer evaluating delegation MUST reject the delegation if any mandatory bound is absent, ambiguous, outside local policy, expired, stale beyond the applicable epoch/freshness rule, revoked, or exceeds supported depth.

A constrained peer MAY support a smaller delegation depth or no delegation feature while remaining conformant to the mandatory authentication floor. Unsupported delegation MUST fail closed rather than weaken authentication or fall back to transitive trust.

The current mandatory Common Contract does not yet claim executable delegation support. Until an explicit wire representation, verifier semantics, revocation/freshness rules, and Rust/C conformance tests are implemented and qualified, delegation cases that require those surfaces remain blocked rather than implicitly accepted.

## 4. Authentication and authorization separation

Direct local trust is not blanket application authorization. Authentication establishes the authenticated peer/session subject under the selected profile; authorization evaluates role, scope, audience, policy generation/epoch, revocation state, and other owned policy constraints separately.

Delegation does not change that separation. Even a fully valid delegation makes the subject only eligible for the authorization decision permitted by its bounds.

## 5. Qualification requirements

Common Contract qualification MUST retain negative evidence for at least:

- transitive-only trust rejection;
- scope mismatch;
- audience mismatch;
- expiry or stale epoch;
- depth overflow;
- revocation;
- unsupported delegation;
- and the rule that a valid delegation does not itself mutate persistent trust.

Static corpus conformance is not runtime interoperability evidence. Promotion of executable delegation requires aligned normative encoding/state semantics, Rust and C behavior where both claim support, negative tests, lifecycle/revocation evidence, and exact-head qualification.
