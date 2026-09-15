# DATA Release and Audit Composition Contract

Status: implementation-bounded draft. This document specifies only behavior already represented by the DATA release classifier and local audit-chain primitive. It does not promote the wider DATA wire lifecycle, persistence model, transparency bridge, or constrained-target evidence to implemented status.

## 1. Scope

This contract separates two existing local responsibilities:

1. `data_release_authorization_classify` decides whether a protected-data release is currently permitted.
2. `data_audit_append` commits a local release outcome and its security-relevant lifecycle context into a hash-linked audit sequence.

The audit chain is evidence about a local decision. It is not an authorization oracle and MUST NOT make an otherwise denied release permissible.

## 2. Release decision ordering

A release decision is fail closed. Implementations claiming this contract MUST preserve the implemented precedence:

1. rollback suspicion;
2. authenticated release context;
3. local device release authority presence and currentness;
4. protected-data encryption and release-key scope;
5. authorization presence and freshness;
6. authorization-generation binding and currentness;
7. revocation freshness and explicit revocation;
8. lineage currentness;
9. holder, audience, purpose, data-type, policy, and epoch binding;
10. required channel binding;
11. one-time release-operation freshness.

A later-valid fact MUST NOT repair an earlier-invalid fact. Fresh AUTH, a new transport, or a fresh channel binding MUST NOT revive stale authorization generation, stale revocation state, explicit revocation, stale lineage, stale local release authority, stale authorization, policy mismatch, a consumed release operation, or rollback suspicion.

## 3. Audit commitment

For each locally retained audit entry, the existing primitive commits to:

- previous audit head;
- monotonically increasing local sequence;
- release-context hash;
- authorization generation;
- revocation epoch;
- policy epoch;
- release outcome kind (`RELEASE_GRANTED` or `RELEASE_DENIED`).

Changing any committed input changes the expected entry digest. Sequence zero is invalid and sequence exhaustion fails closed.

The audit entry does not currently encode a denial reason, durable-storage proof, remote transparency receipt, or rollback-resistant persistence anchor. Implementations MUST NOT claim those properties from this primitive.

## 4. Composition boundary

A conforming caller SHOULD classify the release before appending the corresponding local audit event. The event kind MUST reflect the classifier outcome used by that caller.

The repository does not yet implement an atomic transaction coupling release classification, key disclosure, protected-data transfer, and durable audit persistence. Therefore this draft does not claim that a crash or power loss cannot occur between those operations. Deployment code MUST NOT describe the local audit chain as complete proof that every granted release was delivered or every attempted release was durably recorded.

An audit-chain verification success MUST NOT authorize a release. Authorization is re-evaluated from current release facts, including authorization generation, revocation, lineage, policy/epoch, channel binding, replay state, and rollback posture.

## 5. Restart, rollback, and retained authority

Retained traffic keys, an open transport, a successor association, or a newly valid channel binding MUST NOT synthesize current DATA release authority after the relevant DATA-local state becomes stale or revoked.

A successor association after lineage replacement does not revive predecessor-bound DATA release authority. A consumed release operation remains consumed; successful authentication does not reset its one-time-use status.

Durable recovery of audit head/sequence and rollback-resistant persistence remain open qualification requirements. Host-side hash-chain continuity is not physical persistence evidence.

## 6. Security and privacy considerations

The audit primitive commits to hashes and lifecycle counters rather than protected payload bytes. Callers remain responsible for ensuring that the release-context hash does not itself encode unnecessary identifying or sensitive material.

Audit retention creates metadata. Retention duration, export, disclosure, transparency publication, and deletion policy are deployment concerns until separately specified and qualified.

Neither symbolic analysis nor deterministic audit vectors establish constant-time behavior, entropy quality, secure storage, power-loss safety, or physical rollback resistance.

## 7. Conformance evidence

Current repository evidence for this draft consists of the Rust/C DATA release classifier, the shared `data-release-authorization-v4.txt` decision corpus, Rust/C retained-association temporal tests, the Rust/C local audit-chain primitive, and deterministic audit-chain continuity/tamper tests.

Promotion beyond this implementation-bounded draft requires, at minimum:

- an explicit operation identifier and durable replay/recovery semantics for the wider DATA lifecycle;
- crash/power-loss qualification across release and audit persistence boundaries;
- rollback-resistant audit state or a documented external anchoring model;
- Rust/C parity for any new audit fields or wire-visible behavior;
- constrained-target measurements before MCU claims;
- retained exact-head executable qualification;
- explicit transparency/export semantics before transparency claims.

This document does not define or claim implementation of `DATA_COMMIT`, `RELEASE_REQUEST`, `RELEASE_PROOF`, `RELEASE_KEY`, or network-visible `AUDIT_APPEND` messages. Those remain roadmap work and must not be inferred from the local classifier/audit primitives.