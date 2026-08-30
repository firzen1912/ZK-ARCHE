# ZK-ARCHE Replay-Epoch Lineage Replacement State Owner

Status: **draft normative state-owner contract / wire-unassigned / implementation-blocked**.

This document defines the semantic state owner and canonical transition inputs for authenticated predecessor→successor replay-epoch replacement. It intentionally does **not** allocate a wire message, registry value, packet encoding, or production API. Rust and C do not yet implement this transition.

Normative keywords **MUST**, **MUST NOT**, **SHOULD**, and **MAY** are used in the BCP 14 sense where behavior is precise and testable.

## 1. Owner and module boundary

The lifecycle operation is named `LINEAGE_REPLACE` for specification purposes. The name denotes a semantic transition, not a wire message.

`LINEAGE_REPLACE` belongs to the trust-mutating enrollment/lifecycle surface (`ZK-ARCHE-ENROLL` / zk213) and consumes replay-state consequences owned by `ZK-ARCHE-LINK`. Normal `ZK-ARCHE-AUTH` remains NO-LEARNING and MUST NOT invoke or imply this transition merely because AUTH succeeds.

The transition MUST be explicit. An implementation MUST NOT synthesize it from restart, reconnect, empty local replay state, new transport metadata, ordinary capability negotiation, or an ordinary AUTH completion.

## 2. Abstract state

For one locally trusted peer lineage, the owner reasons over this minimum logical state:

```text
LineageState {
    predecessor_identity
    predecessor_credential_generation
    predecessor_authentication_binding
    predecessor_replay_epoch
    authorization_generation
    policy_epoch
    revocation_freshness
    continuity_state          // CONTINUITY_OK | CONTINUITY_BROKEN
    lineage_status            // ACTIVE | REPLACEMENT_PENDING | RETIRED
}
```

The exact storage representation is implementation-specific. A constrained implementation MAY compress or derive fields, but it MUST preserve the same accept/reject semantics and MUST be able to fail closed when required state cannot be established.

## 3. Canonical transition request

Before a future wire encoding is designed, every implementation and test corpus MUST express a replacement request in terms of the same semantic input tuple:

```text
LineageReplaceRequest {
    operation                 // REKEY | REREGISTER | REPROVISION
    predecessor_identity
    predecessor_credential_generation
    predecessor_authentication_binding
    predecessor_replay_epoch
    successor_identity
    successor_credential_generation
    successor_authentication_binding
    successor_replay_epoch
    protocol_version
    suite
    profile
    audience
    deployment_domain
    authorization_generation
    policy_epoch
    revocation_freshness
    recovery_challenge
    authority_evidence
}
```

The operation labels are semantic classes only. They do not allocate packet types or imply that all three classes will use one future message format.

`authority_evidence` MUST prove that the locally accepted recovery authority is permitted to replace this exact predecessor lineage and MUST authenticate the transition inputs on which that decision depends. Transport reachability, address ownership, an outer session identifier, or successful ordinary AUTH alone is not sufficient authority.

## 4. Preconditions

`LINEAGE_REPLACE` MUST fail closed unless all applicable conditions below hold:

1. the predecessor lineage is uniquely identified by locally trusted state;
2. the predecessor credential/generation and authentication binding match that lineage;
3. the recovery authority is locally authorized to replace that exact lineage;
4. the successor identity and authentication binding are unambiguous and policy-acceptable;
5. successor credential/replay-epoch state is fresh relative to locally retained lineage state;
6. version, suite, and profile are policy-compatible and do not negotiate below the mandatory security floor;
7. audience and deployment/domain match the locally authorized scope;
8. authorization generation, policy epoch, and revocation freshness are acceptable for the replacement decision;
9. the recovery challenge is fresh for this replacement attempt;
10. no competing replacement has already committed for the same predecessor generation/epoch;
11. the implementation can perform the replacement as one recoverable logical commit or remain fail closed.

If any condition cannot be evaluated safely, the result is rejection. Availability pressure MUST NOT convert unknown state into acceptance.

## 5. Decision classes

Rust, C, vectors, and future formal/runtime traceability MUST converge on these semantic result classes even if local APIs use different names:

```text
ACCEPT_SUCCESSOR
REJECT_AUTHORITY
REJECT_PREDECESSOR
REJECT_SUCCESSOR
REJECT_CONTEXT
REJECT_FRESHNESS
REJECT_REPLAY
REJECT_CONCURRENT
REJECT_ROLLBACK
REJECT_STORAGE
```

A future implementation MAY expose less detail to an unauthenticated remote peer for privacy or oracle-resistance reasons. Observable error normalization does not change the internal conformance decision.

## 6. State transition

On successful replacement, the owner MUST perform one logical transition:

```text
ACTIVE(predecessor, old_epoch)
        -> REPLACEMENT_PENDING(predecessor, successor)
        -> ACTIVE(successor, new_epoch) + RETIRED(predecessor, old_epoch)
```

`REPLACEMENT_PENDING` is an internal durability concept, not a remotely selectable protocol state. The successor MUST NOT become AUTH-usable before the authenticated replacement has committed.

The commit MUST:

- activate exactly one successor replay epoch;
- retire or tombstone the predecessor replay epoch;
- prevent predecessor traffic from becoming newly admissible after activation;
- invalidate or explicitly revalidate predecessor-bound session/traffic keys, resumption state, authorization caches, identity-attribution caches, channel bindings, and replay state;
- preserve or narrow authorization scope, never expand it implicitly;
- record enough durable lineage information to detect replay or rollback of the replacement transition according to the eventual target storage contract.

If crash, power loss, partial write, storage failure, or rollback suspicion prevents the implementation from establishing which lineage is active, it MUST remain or return to `CONTINUITY_BROKEN` rather than accept both lineages.

## 7. Concurrency and idempotence boundary

Two different successor requests for the same predecessor generation/epoch MUST NOT both commit.

A byte-identical or semantically identical previously committed request MUST NOT create another fresh successor epoch when replayed. A future implementation MAY return a local idempotent status only if it can prove that the already-committed successor is exactly the same lineage transition; remote observability of that distinction remains a privacy/error-design question.

A stale request whose predecessor has already been retired MUST be rejected as replay/stale lineage rather than treated as authority to replace the current successor.

## 8. Infrastructure independence and trust semantics

For already-authorized peers with sufficient local state, evaluating `LINEAGE_REPLACE` MUST NOT require a CA, cloud identity provider, central-registry lookup, DNS, Internet access, blockchain, manufacturer cloud, or gateway/controller approval.

External systems MAY assist policy distribution, revocation synchronization, backup, audit, or fleet administration, but they are not the root decision maker for the constrained Common Contract.

Trust remains local and non-transitive by default. Authority to replace one lineage MUST NOT imply authority over another lineage, role, audience, deployment, or peer unless separately and explicitly authorized.

## 9. Required decision corpus

The future shared Rust/C corpus MUST instantiate the `RE-01` through `RE-12` cases from `replay-epoch-recovery.md` using the canonical request fields above. Before claiming implementation, it MUST additionally demonstrate at least:

```text
RE-13 two competing successors cannot both commit
RE-14 retired predecessor cannot authorize replacement of the active successor
RE-15 wrong predecessor generation is rejected
RE-16 wrong successor identity/authentication binding is rejected
RE-17 version/suite/profile downgrade below policy is rejected
RE-18 wrong audience/deployment-domain binding is rejected
RE-19 stale policy/revocation freshness is rejected
RE-20 crash/partial-write ambiguity remains CONTINUITY_BROKEN
```

The corpus MUST govern semantic inputs and expected internal decisions; it MUST NOT become a de facto wire specification before TD-004 allocates normative grammar.

## 10. Formal-analysis boundary

This state-owner contract supplies a concrete recovery event boundary for later FM-22 work, but it is not itself a formal result. A future symbolic model may represent successful `LINEAGE_REPLACE` only after mapping the model event to the same authority, predecessor, successor, context, freshness, and atomicity assumptions defined here.

No claim of forward secrecy, post-compromise security, KCI resistance, secure erasure, rollback-proof hardware, constant-time behavior, RNG quality, or implementation verification follows from this document.

## 11. Current qualification state

```text
replay continuity fail-closed behavior        IMPLEMENTED + TESTED in Rust/C
replay continuity symbolic model              SCOPED FORMALLY ANALYZED
recovery security requirements                SPECIFIED
LINEAGE_REPLACE semantic state owner          SPECIFIED in this document
canonical semantic transition inputs          SPECIFIED in this document
wire grammar / registry allocation            NOT PRESENT
Rust/C successor transition                   NOT IMPLEMENTED
shared RE-01..RE-20 executable corpus          NOT PRESENT
iot-core replay_epoch_rule                    remains unresolved
selectable                                    0
```

Therefore this document advances zk213/TD-004 specification precision and FM-22 readiness, but it does not close zk213, TD-003, TD-004, profile promotion, Common Contract conformance, or deployment qualification.
