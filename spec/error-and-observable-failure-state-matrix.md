# ERROR and Observable-Failure State Matrix

Status: **draft conformance inventory**. This document records behavior that is already jointly owned by the specification and/or the Rust and C implementations. It is not an IETF document and does not by itself make ZK-ARCHE RFC-class.

## 1. Purpose

ZK-ARCHE needs one explicit surface for three questions that are security-, interoperability-, and privacy-relevant:

1. does a rejected input produce a protocol `ERROR` packet or no protocol response;
2. what protocol/session state remains after the rejection; and
3. whether retry or retransmission is permitted without starting a new flight.

This document does **not** invent behavior where current repository evidence is incomplete or divergent. Such rows are marked `UNRESOLVED` or `DIVERGENT` and remain TD-004 / interoperability work.

## 2. Evidence vocabulary

- **OWNED** — normative repository text and both implementations agree on the relevant behavior.
- **IMPLEMENTATION-OBSERVED** — Rust and C currently agree, but the normative owner is incomplete.
- **DIVERGENT** — Rust and C currently differ in a security-relevant state transition.
- **UNRESOLVED** — the repository does not yet own a sufficient normative rule. No conformant behavior may be inferred from this document.

`ERROR` codes themselves remain governed by `spec/registries.md` and the canonical normalization corpus. Unknown received ERROR codes normalize to `UNSPECIFIED`; C-local API status values are not wire allocations.

## 3. General invariants

The following invariants apply independently of a particular row:

1. An `ERROR` response is diagnostic/protocol control; it MUST NOT by itself authenticate a peer, authorize an operation, mutate durable trust, or create a trusted credential lineage.
2. Authentication failure MUST NOT be converted into authorization or trust-learning success.
3. Transport addresses are not protocol identity and MUST NOT become identity merely because an error was received from that address.
4. Failure handling MUST preserve transcript/context/replay checks already required by the corresponding protocol flight.
5. Unknown wire ERROR values are normalized according to the canonical wire registry/corpus; implementations MUST NOT reinterpret local-only status values as protocol allocations.
6. An implementation MUST NOT claim retry/retransmission interoperability for a row marked `UNRESOLVED` or `DIVERGENT`.
7. Observable distinctions between silence and an `ERROR` packet are privacy-relevant. This matrix documents those distinctions; it does not claim observational equivalence.

## 4. Current failure/state matrix

| Surface | Trigger | Current response behavior | State consequence | Retry / retransmission | Evidence state |
|---|---|---|---|---|---|
| outer wire/header parse | malformed datagram before a trustworthy protocol header/session can be established | Rust: no protocol response. C: no protocol response. | no authenticated/session transition is established by the rejected datagram | not normatively owned; caller may start a fresh valid exchange | IMPLEMENTATION-OBSERVED |
| valid header, unsupported packet type | packet type is not supported in the receiving state/dispatcher | protocol `ERROR` using the registered unknown/invalid-type class where the dispatcher can form a response | no successful authentication/authorization/trust transition | generalized retry policy remains unowned | IMPLEMENTATION-OBSERVED |
| `SETUP_3` / `AUTH_3` for unknown session | validly framed terminal-flight message references no matching pending session | protocol `ERROR` (`UNKNOWN_SESSION`) | no new authenticated state is created | sender cannot rely on retrying the same missing state; general restart rule remains incompletely specified | OWNED for rejection; UNRESOLVED for generalized restart semantics |
| `AUTH_1` replay | duplicate authenticated AUTH_1 replay identity is detected | protocol `ERROR` (`REPLAY_DETECTED`) on the production C path; replay is rejected by the Rust replay owner as well | duplicate must not create a second accepted AUTH session; temporary reservation is released on the C production path | accepted/rejected duplicate handling is replay-governed, not permission to bypass AUTH | OWNED for fail-closed rejection; broader retransmission equivalence remains incomplete |
| session-capacity exhaustion | no bounded session slot/capacity is available | protocol error on implementations that can form the response | must not evict or overwrite an existing authenticated session merely to admit the rejected attempt | caller may only make progress after capacity is legitimately available; no weaker-security fallback | IMPLEMENTATION-OBSERVED |
| terminal-flight cryptographic/context failure | `AUTH_3` fails completion / Finished / bound-context verification for an existing pending AUTH session | both implementations return an error response where the header/session is usable | **Rust retains the pending AUTH session; C releases it after processing AUTH_3 even when processing fails** | **not owned**. Same-flight retry versus mandatory fresh AUTH is therefore not interoperably specified | **DIVERGENT** |
| terminal-flight success | valid `AUTH_3` completes the pending AUTH exchange | success response/completion path | both implementations remove/consume the pending AUTH-session owner after successful completion; authenticated result is produced only after verification | subsequent protocol use is governed by completed-session semantics, not by resending AUTH_3 to a pending slot | IMPLEMENTATION-OBSERVED / partially normative |
| received unknown `ERROR` code | peer sends an unallocated wire ERROR value | decoder normalizes code to `UNSPECIFIED` | receiving an unknown diagnostic code does not weaken protocol state/security requirements | retry decision cannot be inferred from an unknown code alone | OWNED |

## 5. AUTH_3 failure divergence is a conformance blocker

The current Rust and C server dispatchers disagree on the lifetime of a pending AUTH session after a failed `AUTH_3`:

- Rust removes the pending session only when the AUTH_3 handler succeeds, so a failed terminal flight leaves the pending session present.
- C releases the pending AUTH slot immediately after invoking the AUTH_3 handler, before checking whether the handler succeeded, so a failed terminal flight consumes the pending session.

The current protocol specification requires invalid completion/authenticator/context input to fail closed, but it does not yet define whether that failure is terminal for the pending AUTH session or whether an authenticated retransmission/retry of AUTH_3 is permitted.

Therefore this document intentionally does **not** choose either implementation as normative. Until a dedicated rule is owned and both implementations plus deterministic negative/retry tests agree:

- ZK-ARCHE MUST NOT claim Rust/C state-machine interoperability for failed AUTH_3 retry semantics;
- the RFC-class state-machine/error/retransmission gate remains open;
- privacy analysis MUST treat the implementations as potentially distinguishable after an invalid AUTH_3; and
- formal models MUST NOT silently assume one lifetime rule as if it were implemented by both languages.

## 6. Error versus silence and privacy

A protocol-visible `ERROR` leaks at least that the receiver parsed enough context to classify and respond. Silence may instead represent malformed framing, local transport loss, deliberate suppression, or another unobservable local condition. These outcomes are not observationally equivalent.

Accordingly:

- `spec/privacy-considerations.md` remains the privacy claim owner;
- this matrix is evidence for observable failure classes, not proof of privacy equivalence;
- future normalization/padding/timing policies require explicit specification and tests rather than being inferred here; and
- diagnostic text is not an authorization or trust signal.

## 7. Conformance requirements for closing the divergent row

The failed-AUTH_3 row may move from `DIVERGENT` only after all of the following exist:

1. a normative rule stating whether failed terminal-flight verification consumes or retains pending AUTH state;
2. an explicit replay/retransmission rule for duplicate or corrected terminal-flight messages;
3. a resource-exhaustion rationale so retention cannot become an unbounded DoS primitive;
4. a privacy analysis of externally distinguishable retry behavior;
5. deterministic Rust/C tests that exercise failure, retry/replay, timeout/expiry, and success after the chosen transition where permitted;
6. any affected formal property/model updated with the same lifecycle rule; and
7. repository-owned Rust/C qualification showing the same accept/reject and state-transition result.

## 8. Claim boundary

This matrix improves specification traceability but does not establish:

- complete RFC-class documentation;
- complete Rust/C interoperability;
- formal proof of the listed runtime behavior;
- constant-time behavior, memory safety, RNG quality, or computational proof soundness;
- external cryptographic review; or
- deployment qualification.

TD-001 through TD-004 remain governed by their existing clearing evidence.