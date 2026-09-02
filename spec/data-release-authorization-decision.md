# ZK-ARCHE DATA Release Authorization Decision

Status: **normative implementation contract; not a wire-format specification**

This contract defines the minimum constrained-floor decision that gates release of protected DATA key material. It does not define `DATA_COMMIT`, release-token encoding, ciphertext format, key wrapping, audit-chain encoding, or a transport.

## Sovereignty boundary

Per-device data sovereignty means the device remains the final local authority for release. Successful AUTH, a remote policy engine, transport identity, or possession of a previously issued token MUST NOT substitute for a current device-local release authority.

The classifier consumes already-verified local facts. It does not define how the device stores that authority or cryptographically verifies a future release token.

## Invariants

1. Authentication, device-local release authority, release authorization, and trust mutation are distinct.
2. Protected data is encrypted by default. A key-release decision MUST fail closed if the caller cannot establish that the protected object remains in the encrypted protected state expected by policy.
3. A release key MUST be scoped to the protected object/data class represented by the decision. A broader or mismatched key scope MUST fail closed.
4. Successful AUTH does not itself authorize release and MUST NOT mint or refresh device-local release authority as a side effect.
5. Release authorization MUST bind holder, audience, purpose, data type, policy, authorization epoch/lineage, and revocation view.
6. A profile requiring channel binding MUST reject missing, stale, or invalid binding. Transport address equality MUST NOT substitute for cryptographic binding.
7. Stale local release authority, stale authorization, stale revocation, explicit revocation, stale lineage, epoch mismatch, or rollback suspicion MUST fail closed.
8. Optional proof-carrying mechanisms may supply verified facts to this classifier; they MUST NOT redefine mandatory decision semantics.
9. The constrained floor MUST NOT require a cloud policy engine, CA, gateway, DNS, Internet, blockchain, general-purpose ZK circuit, or large trust graph to evaluate a locally supportable release.

## Decision

- `RELEASE`: all mandatory local sovereignty, encryption, scope, authorization, freshness, and binding facts are current.
- `FRESH_AUTH_REQUIRED`: the requester is not currently authenticated; normal AUTH may be attempted.
- `DENY`: local release authority, protected-state evidence, scope, authorization, revocation, lineage, policy, binding, or rollback state is unsafe.

Fail-closed precedence is: rollback; authentication; device-local release authority; protected encrypted state; release-key scope; authorization presence/freshness; revocation/lineage; holder/audience/purpose/data-type/policy/epoch; required channel binding.

`RELEASE` authorizes only the bounded release represented by the inputs. It is not persistent trust, enrollment, arbitrary application authorization, permission to release other data, or permission to persist plaintext.

## Conformance evidence

The current canonical corpus is `rust/test-vectors/state/data-release-authorization-v2.txt`. Rust and C implementations claiming the current contract MUST reproduce its action/reason outputs. Version 1 remains historical evidence for the earlier policy/revocation/binding surface.

## Evidence boundary

This demonstrates wire-neutral decision semantics and deterministic negative evidence. It is **not** evidence that DATA wire messages, cryptographic release-token verification, key wrapping, encrypted-storage implementation, audit chaining, target budgets, physical rollback resistance, formal analysis, independent review, or deployment qualification are complete.
