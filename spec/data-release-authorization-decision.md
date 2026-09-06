# ZK-ARCHE DATA Release Authorization Decision

Status: **normative implementation contract; not a wire-format specification**

This contract defines the minimum constrained-floor decision that gates release of protected DATA key material. It does not define `DATA_COMMIT`, release-token encoding, ciphertext format, key wrapping, audit-chain encoding, or a transport.

## Sovereignty boundary

Per-device data sovereignty means the device remains the final local authority for release. Successful AUTH, a remote policy engine, transport identity, or possession of a previously issued token MUST NOT substitute for a current device-local release authority.

The classifier consumes already-verified local facts. It does not define how the device stores that authority, persists one-time release-operation state, or cryptographically verifies a future release token.

## Invariants

1. Authentication, device-local release authority, release authorization, and trust mutation are distinct.
2. Protected data is encrypted by default. A key-release decision MUST fail closed if the caller cannot establish that the protected object remains in the encrypted protected state expected by policy.
3. A release key MUST be scoped to the protected object/data class represented by the decision. A broader or mismatched key scope MUST fail closed.
4. Successful AUTH does not itself authorize release and MUST NOT mint or refresh device-local release authority as a side effect.
5. Release authorization MUST bind holder, audience, purpose, data type, policy, authorization epoch/lineage, revocation view, and one authenticated local authorization generation.
6. The implementation MUST establish the release authorization's generation binding from authenticated credential material or authenticated local metadata before testing whether that generation is current. A caller assertion, transport label, socket/address, cached policy result, or unauthenticated token field MUST NOT establish this binding.
7. Missing generation provenance MUST fail closed as `AUTHORIZATION_GENERATION_UNBOUND`; a proven binding to an older local generation MUST fail closed as `AUTHORIZATION_GENERATION_STALE`.
8. Advancing the local authorization generation MUST invalidate release authority bound to an older generation even when the requester remains authenticated.
9. A profile requiring channel binding MUST reject missing, stale, or invalid binding. Transport address equality MUST NOT substitute for cryptographic binding.
10. A bounded release operation MUST be one-time at the decision boundary. A consumed release-operation identifier MUST NOT authorize another release, and successful AUTH MUST NOT reset its consumed state.
11. Stale local release authority, stale authorization, unbound or stale authorization generation, stale revocation, explicit revocation, stale lineage, epoch mismatch, release replay, or rollback suspicion MUST fail closed.
12. Optional proof-carrying mechanisms may supply verified facts to this classifier; they MUST NOT redefine mandatory decision semantics.
13. The constrained floor MUST NOT require a cloud policy engine, CA, gateway, DNS, Internet, blockchain, general-purpose ZK circuit, or large trust graph to evaluate a locally supportable release.

## Decision

- `RELEASE`: all mandatory local sovereignty, encryption, scope, authorization, authenticated generation binding/currentness, freshness, binding, and one-time release facts are current.
- `FRESH_AUTH_REQUIRED`: the requester is not currently authenticated; normal AUTH may be attempted.
- `DENY`: local release authority, protected-state evidence, scope, authorization, generation provenance/currentness, revocation, lineage, policy, binding, replay, or rollback state is unsafe.

Fail-closed precedence is: rollback; authentication; device-local release authority; protected encrypted state; release-key scope; authorization presence/freshness; authorization-generation binding/currentness; revocation/lineage; holder/audience/purpose/data-type/policy/epoch; required channel binding; one-time release replay.

`RELEASE` authorizes only the bounded release represented by the inputs. It is not persistent trust, enrollment, arbitrary application authorization, permission to release other data, or permission to persist plaintext. The surrounding DATA lifecycle must atomically bind release-key use and release-operation consumption so a crash or rollback cannot recover a consumed operation as reusable; that persistence mechanism remains outside this wire-neutral classifier.

## Composition with retained association authority

The classifier's `authenticated` fact means **currently security-authoritative authentication context**, not merely evidence that an AUTH exchange succeeded at some earlier time or that transport/session keys still exist.

When a DATA release request is carried by a retained ZK-ARCHE secure association, the caller MUST re-use the current CORE/LINK association-admission result as part of establishing this fact. If `core-association-admission.md` returns `FAIL_CLOSED` for the association, the DATA caller MUST NOT treat the association as authenticated authority for a new release merely because:

- the peer completed AUTH earlier;
- traffic keys remain resident;
- the underlying transport connection is still open;
- a cached release authorization was previously valid; or
- a previous DATA release under the association succeeded.

A retained association that has lost authority therefore cannot carry a new protected-data release. The implementation MUST first restore every owning lifecycle fact required by the common contract and obtain a fresh successful association-admission decision where an association is required by the selected DATA profile.

Fresh AUTH alone is insufficient to repair stale authorization generation, revocation, lineage, replay continuity, restart continuity, key-usage continuity, rollback suspicion, or required channel binding. Those facts remain owned by their respective CORE/TRUST/LINK/BIND lifecycle authorities.

This composition rule deliberately does **not** add a second revocation, replay, restart, key-usage, or association classifier inside ZK-ARCHE-DATA. DATA consumes the authoritative result of those layers and then applies its additional device-local sovereignty checks. This preserves one lifecycle authority per fact while ensuring that DATA cannot continue using an association after CORE has removed its authority.

For profiles that permit a local/offline DATA operation without a retained secure association, `authenticated` MUST still be established by the profile's explicitly defined local authenticated context; transport presence or cached remote identity cannot synthesize it. Such a profile does not bypass the remaining release-authority, authorization-generation, revocation, lineage, replay, rollback, policy, or one-time-release checks.

### Required temporal qualification

Executable qualification should include at least the following cross-module sequence in both Rust and C harnesses:

```text
AUTH succeeds
→ association admission = ESTABLISH
→ DATA release decision = RELEASE for operation N
→ authoritative lifecycle fact becomes unsafe
→ association re-evaluation = FAIL_CLOSED
→ new DATA release attempt N+1 under retained keys/transport is not RELEASE
```

The unsafe lifecycle mutations should cover, at minimum, authorization-generation advance, explicit revocation, stale lineage, restart-continuity loss, key-usage-continuity loss, rollback suspicion, and required-binding invalidation where applicable. Reusing operation N must remain independently rejected by the DATA one-time-release rule.

This section is normative composition semantics. Until that temporal sequence is represented in executable Rust/C qualification, it MUST NOT be reported as new cross-module TESTED evidence.

## Conformance evidence

The current canonical corpus is `rust/test-vectors/state/data-release-authorization-v4.txt`. Rust and C implementations claiming the current contract MUST reproduce its action/reason outputs. Version 3 remains historical evidence for the earlier generation-currentness surface that did not independently represent authenticated generation provenance.

The v4 corpus validates the DATA-local classifier inputs. It does not by itself establish the retained-association temporal composition above; that remains an explicit qualification gap until a cross-module executable sequence exists.

## Evidence boundary

This demonstrates wire-neutral decision semantics and deterministic negative evidence. It is **not** evidence that DATA wire messages, durable release-operation storage, cryptographic release-token verification, key wrapping, encrypted-storage implementation, audit chaining, target budgets, physical rollback resistance, formal analysis, independent review, retained-association temporal qualification, or deployment qualification are complete.
