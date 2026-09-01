# ZK-ARCHE DATA Release Authorization Decision

Status: **normative implementation contract; not a wire-format specification**

This contract defines the minimum constrained-floor decision that gates release of protected DATA material. It does not define `DATA_COMMIT`, token encoding, ciphertext format, or a transport.

## Invariants

1. Authentication, release authorization, and trust mutation are distinct.
2. Protected data is encrypted by default; a successful AUTH does not itself authorize `RELEASE_KEY`.
3. A release authorization decision MUST be bound to the authenticated holder, audience, purpose, data type, policy, authorization epoch/lineage, and revocation view.
4. A profile requiring channel binding MUST reject a missing, stale, or invalid binding. Transport address equality MUST NOT substitute for cryptographic binding.
5. Stale authorization, stale revocation state, explicit revocation, stale lineage, epoch mismatch, or rollback suspicion MUST fail closed for release.
6. An unauthenticated request MAY require fresh AUTH, but normal AUTH MUST NOT learn or expand persistent trust or mint release authority as a side effect.
7. Optional proof-carrying mechanisms may supply verified facts to this classifier; they MUST NOT redefine the mandatory decision semantics.
8. The constrained floor MUST NOT require a cloud policy engine, CA, gateway, DNS, Internet access, blockchain, general-purpose ZK circuit, or large trust graph to evaluate a locally supportable release.

## Decision

The classifier consumes already-verified local facts and returns:

- `RELEASE`: all mandatory policy and freshness facts are current.
- `FRESH_AUTH_REQUIRED`: the requester is not currently authenticated; a normal AUTH may be attempted.
- `DENY`: local authority evidence is absent, stale, revoked, mismatched, rollback-suspect, or otherwise unsafe.

Precedence is fail-closed: rollback; authentication; authorization presence/freshness; revocation; lineage; holder; audience; purpose; data type; policy; epoch; required channel binding.

`RELEASE` authorizes only the bounded release represented by the input facts. It is not persistent trust, enrollment, arbitrary application authorization, or permission to release other data.

## Evidence boundary

Rust and C implement the same wire-neutral classifier and a canonical negative corpus. This is **not** evidence that DATA wire messages, proof/token verification, key wrapping, encrypted storage, audit chaining, target resource budgets, physical rollback resistance, or deployment qualification are complete.
