# ERROR and Observable-Failure State Matrix

Status: **draft conformance inventory**. This document records behavior that is jointly owned by specification and/or implementation evidence. It is not an IETF document and does not by itself make ZK-ARCHE RFC-class.

## 1. Purpose

This matrix tracks three security-, interoperability-, and privacy-relevant questions for rejected inputs:

1. whether a rejection produces protocol `ERROR` or silence;
2. what protocol/session state remains; and
3. whether retry or retransmission is permitted without starting a new flight.

Behavior without sufficient repository ownership remains `UNRESOLVED`; implementations must not infer stronger semantics from this table.

## 2. Evidence vocabulary

- **OWNED** — normative repository text and both implementations agree on the relevant behavior.
- **IMPLEMENTATION-OBSERVED** — Rust and C currently agree, but the normative owner remains incomplete.
- **DIVERGENT** — Rust and C differ in a security-relevant state transition.
- **UNRESOLVED** — no sufficient normative rule exists.

`ERROR` codes are governed by `spec/registries.md` and the canonical normalization corpus. Unknown received ERROR codes normalize to `UNSPECIFIED`; C-local API status values are not wire allocations.

## 3. General invariants

1. An `ERROR` response MUST NOT itself authenticate a peer, authorize an operation, mutate durable trust, or create a trusted credential lineage.
2. Authentication failure MUST NOT be converted into authorization or trust-learning success.
3. Transport addresses MUST NOT become protocol identity merely because an error was received from that address.
4. Failure handling MUST preserve transcript/context/replay requirements of the corresponding flight.
5. Unknown wire ERROR values MUST follow the canonical normalization rule.
6. Retry/retransmission interoperability MUST NOT be claimed for `UNRESOLVED` or `DIVERGENT` rows.
7. Silence versus `ERROR` is privacy-relevant; no observational equivalence is implied.

## 4. Current failure/state matrix

| Surface | Trigger | Response behavior | State consequence | Retry / retransmission | Evidence state |
|---|---|---|---|---|---|
| outer wire/header parse | malformed datagram before trustworthy protocol/session context exists | Rust: silence. C: silence. | rejected input creates no authenticated/session transition | fresh valid exchange may be attempted; generalized retry policy remains incomplete | IMPLEMENTATION-OBSERVED |
| valid header, unsupported packet type | unsupported packet type reaches dispatcher | protocol `ERROR` using registered invalid/unknown-type class where a response can be formed | no authentication/authorization/trust success | generalized retry policy unresolved | IMPLEMENTATION-OBSERVED |
| `SETUP_3` / `AUTH_3` for unknown session | terminal-flight message references no pending session | `ERROR` (`UNKNOWN_SESSION`) | no authenticated state is created | sender cannot rely on continuation of missing state | OWNED for rejection; generalized restart semantics incomplete |
| `AUTH_1` replay | duplicate authenticated AUTH_1 replay identity is detected | replay rejection; C production path returns `REPLAY_DETECTED` | duplicate cannot create a second accepted AUTH session | no bypass of fresh AUTH/replay checks | OWNED for fail-closed rejection; broader retransmission equivalence incomplete |
| session-capacity exhaustion | no bounded slot/capacity is available | protocol error where a response can be formed | existing authenticated/pending state must not be overwritten merely to admit attacker input | progress only after legitimate capacity becomes available | IMPLEMENTATION-OBSERVED |
| terminal-flight cryptographic/context failure | `AUTH_3` fails completion, Finished, transcript, or bound-context verification for an existing pending AUTH session | protocol `ERROR` where response formation is possible | pending AUTH state is consumed before terminal verification and is not restored on failure | corrected/modified `AUTH_3` MUST NOT continue the consumed session; retry begins at fresh `AUTH_1` with fresh `session_id` | OWNED by `spec/auth-terminal-flight-disposition.md`; Rust/C implementation parity after this packet |
| terminal-flight success | valid `AUTH_3` completes pending AUTH | success/completion response | pending AUTH state is consumed; authenticated result exists only after verification | subsequent use follows completed-session semantics | OWNED for pending-state disposition; broader completed-session lifecycle remains partial |
| received unknown `ERROR` code | peer sends unallocated wire ERROR value | decoder normalizes to `UNSPECIFIED` | diagnostic does not weaken protocol-state requirements | retry decision cannot be inferred from unknown code alone | OWNED |

## 5. AUTH_3 terminal-session disposition

`spec/auth-terminal-flight-disposition.md` is the normative owner for AUTH terminal pending-state disposition.

Once a receiver locates the pending AUTH session for an incoming `AUTH_3`, that pending state is consumed before terminal verification. Success and terminal verification failure therefore both leave no pending AUTH session for that `session_id`.

A failed `AUTH_3` does not authorize same-session correction. A peer that retries AUTH starts from `AUTH_1` using a fresh `session_id`, preserving ordinary replay and authorization checks.

Exact duplicate response-cache replay is distinct from protocol-state retry: a cached response may be replayed without recreating pending AUTH state or re-running terminal verification.

## 6. Error versus silence and privacy

A protocol-visible `ERROR` reveals that the receiver parsed enough state to classify and respond. Silence may represent malformed framing, transport loss, deliberate suppression, or another local condition. These outcomes are not observationally equivalent.

A failed terminal `AUTH_3` followed by a later non-cached terminal message may expose a transition from a terminal verification error to `UNKNOWN_SESSION`. This is a documented observable distinction, not a privacy-equivalence claim.

`spec/privacy-considerations.md` remains the privacy claim owner. Diagnostic text is not an authorization or trust signal.

## 7. Remaining conformance work

The terminal-session divergence is closed at the specification/implementation level by the dedicated contract and Rust alignment, but full cross-language qualification still requires repository-owned executable evidence for:

1. failed terminal verification consumes pending state in both implementations;
2. a subsequent non-cached `AUTH_3` for the same session is rejected as unknown session;
3. exact duplicate cached-response behavior does not recreate pending state;
4. fresh retry starts from `AUTH_1` with a fresh session identifier;
5. formal-model assumptions, if any, match the same lifecycle rule; and
6. resource/privacy evidence remains bounded to the claims actually tested.

## 8. Claim boundary

This matrix does not establish complete RFC-class documentation, complete Rust/C interoperability, formal proof, constant-time behavior, memory safety, RNG quality, external cryptographic review, Common Contract conformance, or deployment qualification.

TD-001 through TD-004 remain governed by their existing clearing evidence.