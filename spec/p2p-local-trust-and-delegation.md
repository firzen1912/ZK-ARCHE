# P2P Local Trust and Bounded Delegation Contract

Status: pre-RFC engineering contract; not an IETF or RFC document.

## 1. Scope

This document owns the mandatory Common Contract semantics for local trust, non-transitivity, and the bounded-delegation trust boundary. It does not promote `p2p-iot-core` and does not define a delegation wire object or credential signature format.

Executable bounded-delegation **decision semantics** now exist in both Rust and C and are governed by `spec/p2p-bounded-delegation.md` plus the shared canonical decision corpus. That implementation evidence is narrower than a deployable delegation protocol: wire encoding, cryptographic credential verification, constrained-target persistence, on-wire interoperability, and deployment qualification remain separate requirements.

## 2. Local trust is the root decision

Trust is local and non-transitive by default.

A trust decision MUST NOT become transitive merely because an accepted peer trusts another peer. If A locally trusts B and B locally trusts C, A MUST NOT accept C solely from B's trust relationship.

Normal AUTH remains NO-LEARNING. Successful authentication MUST NOT create, expand, repair, or persist a trust relationship or delegation as a side effect.

Already-authorized peers with sufficient local state MUST be able to make the mandatory root authentication decision without requiring a CA, cloud identity provider, central registry, DNS, Internet connectivity, blockchain, manufacturer cloud, or gateway/controller approval.

## 3. Delegation boundary

Delegation MUST be explicit, scoped, audience-bound, depth-bounded, validity-bounded, issuer-bound, epoch-bound, authorization-generation-bound, and revocable.

A valid delegation MAY make a subject eligible for local authorization evaluation; it MUST NOT create automatic persistent trust.

A peer evaluating delegation MUST reject the delegation if any mandatory bound is absent, ambiguous, outside local policy, expired, stale beyond the applicable epoch/freshness rule, unbound to the current authorization generation, bound to a stale authorization generation, revoked, rooted only in transitive trust, or exceeds supported depth.

A constrained peer MAY support a smaller delegation depth or no delegation feature while remaining conformant to the mandatory authentication floor. Unsupported delegation MUST fail closed rather than weaken authentication or fall back to transitive trust.

The implementation-linked decision contract is `spec/p2p-bounded-delegation.md`. Its Rust/C classifier consumes already-verified local facts and distinguishes, among other failures, non-local issuer trust, authorization-generation provenance failure, stale authorization generation, stale policy/revocation/lineage state, depth overflow, and forbidden redelegation. This decision surface MUST NOT be interpreted as a complete delegation credential protocol.

## 4. Authentication and authorization separation

Direct local trust is not blanket application authorization. Authentication establishes the authenticated peer/session subject under the selected profile; authorization evaluates role, scope, audience, authorization generation, policy epoch, revocation state, lineage, and other owned policy constraints separately.

Delegation does not change that separation. Even a fully valid delegation makes the subject only eligible for the authorization decision permitted by its bounds.

A successful AUTH exchange, transport identity, peer address, gateway assertion, CA assertion, or cloud/registry lookup MUST NOT manufacture authorization-generation provenance or convert non-local trust into local trust.

## 5. Common Contract lifecycle composition

Bounded delegation composes with the same fail-closed lifecycle doctrine as direct authorization and association admission:

- authorization evidence MUST be bound to a specific authorization generation and that generation MUST be current;
- revocation and lineage state MUST be current;
- replay/restart/key-usage continuity remain mandatory where the owning lifecycle surface requires them;
- loss of infrastructure MUST NOT repair stale, missing, or unbound local state;
- resource asymmetry MAY remove optional features but MUST NOT weaken the mandatory authentication floor.

The current P2P Common Contract lifecycle qualification corpus separately exercises cross-class mandatory-floor behavior, including constrained↔higher-capability decisions and key-usage continuity. That lifecycle corpus does not replace the bounded-delegation corpus; each owns a distinct decision surface.

## 6. Qualification requirements

Common Contract qualification MUST retain negative evidence for at least:

- transitive-only issuer trust rejection;
- scope mismatch;
- audience mismatch;
- deployment/domain mismatch;
- expiry or stale validity;
- authorization-generation provenance missing;
- stale authorization generation;
- stale policy epoch;
- stale revocation view or explicit revocation;
- stale lineage;
- depth overflow;
- forbidden or unsupported redelegation;
- unsupported delegation;
- and the rule that a valid delegation does not itself mutate persistent trust.

Shared-corpus Rust/C decision parity is **TESTED decision-contract evidence**, not on-wire interoperability evidence. Complete delegation qualification additionally requires normative credential/wire encoding where such a credential is promoted, cryptographic verification semantics, parser/negative-vector coverage, constrained-target storage and restart behavior, cross-peer interoperability evidence, formal assurance at the claimed abstraction level, and exact-head qualification.

## 7. Claim boundary

Current repository evidence supports an implementation-linked bounded-delegation **decision classifier** and shared Rust/C conformance corpus. It does not by itself establish:

- a standardized delegation credential or wire format;
- physical MCU persistence or rollback resistance;
- MCU↔MCU or MCU↔edge wire interoperability;
- formal proof of the full Common Contract;
- independent cryptographic review;
- RFC/IETF status;
- production-selectable `p2p-iot-core` status;
- or deployment qualification.
