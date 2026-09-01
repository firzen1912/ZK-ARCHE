# AUTH Terminal-Flight Disposition Contract

Status: **draft normative implementation contract** for the currently implemented AUTH terminal flight. This document uses BCP 14 keywords for behavior that is testable in the Rust and C implementations. It does not claim IETF status, clear TD-001 through TD-004, or establish complete RFC-class documentation.

## 1. Scope

This contract owns the pending-session disposition for `AUTH_3` after a receiver has parsed a valid protocol header and located the corresponding pending AUTH session.

It does not change AUTH wire encoding, transcript inputs, proof construction, KDF/MAC behavior, replay identity, authorization semantics, or trust mutation.

## 2. Terminal-flight rule

`AUTH_3` is terminal for the pending AUTH state identified by its `session_id`.

Once a receiver locates a pending AUTH session for an incoming `AUTH_3`, it **MUST consume that pending AUTH session before terminal cryptographic/context verification is evaluated**.

The pending AUTH session **MUST NOT** be restored if completion verification, Finished verification, transcript/context verification, authorization-context verification, or another terminal-flight verification step fails.

A failed `AUTH_3` therefore **MUST NOT** permit a corrected or modified `AUTH_3` to continue against the consumed pending state.

A peer that wishes to retry AUTH after such a failure **MUST begin a fresh AUTH exchange from `AUTH_1` using a fresh `session_id`**.

## 3. Retransmission and response-cache interaction

A receiver MAY replay an already-cached response for an exact duplicate `(session_id, seq)` according to its transport/retransmission cache rules. Returning that cached response does not recreate pending AUTH state and does not re-run terminal verification.

A changed terminal-flight payload under a consumed AUTH session MUST NOT be treated as a continuation merely because the transport or peer address is unchanged.

Reusing the consumed `session_id` for a new AUTH attempt is not conformant to this contract because response caches and replay state may still retain entries keyed to that identifier. A retry therefore uses a fresh `session_id` and executes the normal `AUTH_1` replay and authorization checks.

## 4. Security rationale

Fail-closed terminal consumption prevents a peer from obtaining repeated cryptographic/context verification attempts against retained ephemeral server state after a terminal authenticator failure. It also bounds the lifetime of pending AUTH state under malformed or adversarial terminal flights.

This rule does not make an `ERROR` response an authentication or authorization signal. It does not weaken replay handling, and it does not authorize trust learning.

## 5. Privacy and observability

A receiver that can form a protocol response may return the applicable registered `ERROR` for the failed `AUTH_3`. A subsequent non-cached terminal-flight message for the consumed session is expected to fail as `UNKNOWN_SESSION` because no pending AUTH state remains.

Those two observable error classes are not claimed to be privacy-equivalent. `spec/privacy-considerations.md` remains the privacy claim owner.

## 6. Resource and availability boundary

Consuming pending state on terminal failure is a bounded-resource rule: invalid terminal flights cannot indefinitely retain the associated pending AUTH slot through repeated corrected attempts.

This rule does not define session timeout values, global capacity policy, transport retry timers, or a general-purpose rate-limit mechanism. Those remain owned by their applicable profile/runtime specifications.

## 7. Conformance evidence

Implementations claiming this behavior require evidence that:

1. successful `AUTH_3` consumes the pending AUTH session;
2. failed terminal verification consumes the pending AUTH session;
3. a subsequent non-cached `AUTH_3` for that consumed session is rejected as unknown session;
4. an exact cached duplicate may receive the cached prior response without recreating pending state; and
5. a fresh AUTH retry proceeds through `AUTH_1` with a fresh session identifier and normal replay/authorization checks.

Rust and C must produce equivalent state-transition decisions for these cases before this surface is reported as cross-language interoperable.

## 8. Claim boundary

This contract owns only terminal pending-session disposition. It does not establish complete replay/restart semantics, full observable-failure privacy equivalence, downgrade resistance, formal proof, external cryptographic review, Common Contract conformance, or deployment qualification.