# LINEAGE_REPLACE lifecycle-possession contract

Status: wire-neutral lifecycle qualification surface; not an allocated protocol message.

A replacement operation MUST NOT translate an unauthenticated caller assertion into current-credential or successor-key control. The lifecycle layer accepts only proof results produced by an upstream cryptographic verifier and binds each accepted proof result to the exact authenticated session and exact subject reference used by the replacement request.

The classifier applies fail-closed precedence:

1. current-credential control is rejected when its proof result is absent, unverified, bound to a different session, or bound to a credential reference other than the current predecessor;
2. successor-key control is rejected when its proof result is absent, unverified, bound to a different session, or bound to a key reference other than the requested successor;
3. only when both proof results satisfy those bindings is the normalized lifecycle possession decision `VERIFIED`.

This surface does not define how a cryptographic possession proof is encoded or verified and does not allocate a wire field, algorithm, registry value, or retransmission behavior. A future protocol-integrated handler must derive these typed proof results from actual verified lifecycle messages; tests that construct them directly are qualification fixtures, not evidence that cryptographic verification is integrated.

Normal AUTH remains NO-LEARNING. A `VERIFIED` possession decision is necessary but not sufficient for trust mutation: session binding, scoped authorization, predecessor freshness, privilege preservation, atomic durable replacement, reconciliation, and revocation rules remain independent gates.
