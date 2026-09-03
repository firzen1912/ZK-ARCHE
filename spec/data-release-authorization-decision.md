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

## Conformance evidence

The current canonical corpus is `rust/test-vectors/state/data-release-authorization-v4.txt`. Rust and C implementations claiming the current contract MUST reproduce its action/reason outputs. Version 3 remains historical evidence for the earlier generation-currentness surface that did not independently represent authenticated generation provenance.

## Evidence boundary

This demonstrates wire-neutral decision semantics and deterministic negative evidence. It is **not** evidence that DATA wire messages, durable release-operation storage, cryptographic release-token verification, key wrapping, encrypted-storage implementation, audit chaining, target budgets, physical rollback resistance, formal analysis, independent review, or deployment qualification are complete.
