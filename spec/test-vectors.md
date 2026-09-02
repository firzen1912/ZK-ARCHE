# ZK-ARCHE Test Vector and Conformance Corpus Contract

Status: **draft normative vector-governance contract**. This document specifies how deterministic and negative evidence is identified, changed, and consumed. It does not claim that every required vector category already exists, clear TD-001 through TD-004, or establish RFC-class/Common-Contract completion.

Normative keywords **MUST**, **MUST NOT**, **SHOULD**, and **MAY** are used in the BCP 14 sense where behavior is testable.

## 1. Canonical source and authority

The checked-in canonical byte-level corpus for the current protocol generation is:

```text
rust/test-vectors/0x0001/
```

Rust is the current vector producer. This is an evidence-governance role, not permission for Rust source code to override normative `spec/` behavior. Where Rust and C claim the same behavior, C MUST consume or reproduce the canonical corpus and MUST reach the same accept/reject decision.

A vector is authoritative only for the protocol/version/suite/profile and semantic surface explicitly identified by its corpus location or metadata. Evidence for one profile, transport binding, lifecycle mode, or protocol generation MUST NOT be generalized to another without an explicit compatibility rule.

## 2. Required provenance

A retained conformance corpus SHOULD identify, directly or through its containing manifest:

- protocol generation/version;
- suite/method identifier where applicable;
- selected profile where applicable;
- vector schema revision;
- producer implementation revision;
- normative specification section(s) exercised;
- expected outcome (`accept` or a stable reject class);
- whether the artifact is positive, negative, mutation-derived, or trace-derived;
- canonical byte representation for every wire-visible input/output covered by the vector.

Exact implementation commit provenance is required for retained generated evidence used in release qualification. A corpus without sufficient provenance MUST NOT be presented as exact-head evidence.

## 3. Canonical encoding

Canonical vectors MUST preserve exact octets. Hexadecimal text representations MUST use an unambiguous byte order and MUST decode to exactly one byte string. Parsers MUST reject malformed fixture encodings rather than silently repairing odd-length, non-hex, truncated, duplicate, or structurally impossible values.

When a specification requires canonical field ordering, minimal integer representation, unique TLVs, bounded counts, or reserved-value rejection, the corpus SHOULD contain negative fixtures that falsify each relevant rule.

A successful semantic decode is insufficient when byte-level interoperability is claimed: implementations MUST reproduce the normative canonical bytes.

## 4. Positive vector classes

As applicable to implemented behavior, the canonical corpus SHOULD cover:

- wire/header/TLV encodings;
- transcript and context bytes;
- PID or equivalent identifier derivation;
- authentication/Schnorr proof inputs and outputs;
- role-proof inputs, outputs, and rerandomization behavior;
- KDF/domain-separation inputs and derived outputs;
- key-confirmation/MAC inputs and outputs;
- HELLO/version/suite/profile negotiation;
- SETUP/AUTH state transitions;
- authorization-context binding;
- replay/retry/rekey/resumption state where implemented;
- channel/exporter binding where implemented;
- enrollment/revocation/lineage transitions where implemented;
- Common-Contract/P2P decisions where implemented.

The existence of a category in this contract does not imply that the corresponding protocol surface is currently implemented or selectable.

## 5. Negative vector classes

Security-relevant behavior MUST have negative evidence appropriate to the implemented surface. The corpus SHOULD include, where applicable:

- single-byte and structured mutation;
- truncation and trailing data;
- duplicate, reordered, forbidden, or non-canonical fields;
- unknown/reserved critical values;
- wrong message type, direction, state, or sequence;
- wrong version, suite, profile, or mandatory-floor selection;
- downgrade and capability-stripping attempts;
- transcript/context/domain-separation mismatch;
- reflection and cross-session substitution;
- replay, stale epoch, rollback, and restart-continuity violations;
- wrong audience/deployment/role/scope/authorization generation;
- revoked or superseded credential/lineage state;
- transport-identity substitution and channel-binding mismatch;
- identity-attribution/mapping substitution;
- malformed lengths/counts and resource-bound violations.

A negative vector MUST state the expected stable rejection class when the specification defines one. Tests MUST NOT treat “any crash/nonzero exit” as sufficient protocol conformance.

## 6. Accept/reject parity

For every shared vector consumed by both implementations:

1. Rust and C MUST agree on whether the input is accepted.
2. If accepted, they MUST agree on all normative bytes and security-relevant derived values exposed by the vector.
3. If rejected, they MUST agree on the protocol-level rejection class wherever that class is normative.
4. One implementation MUST NOT accept a fixture merely because the other implementation lacks the corresponding parser or state check.

Implementation-specific diagnostic strings are not normative unless explicitly promoted into the wire/error contract.

## 7. Change control

A semantic vector change MUST be classified before publication as one of:

```text
editorial/metadata-only
bug fix preserving normative semantics
new coverage for existing semantics
normative semantic change
new protocol/profile generation
```

Editorial/metadata changes MAY retain the same protocol corpus identity when decoded bytes and expected decisions are unchanged.

A bug fix that changes canonical bytes or expected accept/reject behavior MUST identify the normative defect being corrected and MUST update affected Rust/C tests together where both claim support.

A normative semantic change MUST NOT silently overwrite prior vector meaning under the same protocol/profile identity. It requires the applicable specification, registry/change-control, migration note, and version/profile decision before the new corpus becomes canonical.

Previously released vectors needed to verify backward compatibility SHOULD remain retained or reproducibly recoverable.

## 8. Regeneration and drift

Generated vectors MUST be reproducible from repository-owned tooling where such tooling exists. Release qualification SHOULD fail closed when regeneration changes checked-in canonical bytes unexpectedly.

A generator change that produces byte drift MUST be reviewed as a semantic change until proven otherwise. “The generator changed” is not sufficient justification for accepting new cryptographic or wire bytes.

Manual fixtures MAY exist for malformed or adversarial cases that cannot be emitted by a conformant generator. Such fixtures MUST clearly identify that they are intentionally non-canonical/adversarial.

## 9. Trace and transport evidence

Annotated traces MAY reference canonical vectors, but packet captures or transport logs MUST NOT become the normative source of protocol identity, ordering, or security semantics.

Transport adapters may alter framing outside the ZK-ARCHE protocol boundary only where the transport contract permits it. The inner canonical ZK-ARCHE bytes and accept/reject semantics MUST remain transport-independent unless an explicit binding specification says otherwise.

## 10. Evidence boundaries

Passing the canonical corpus establishes only the behavior actually represented by that corpus. It does not by itself establish:

- cryptographic soundness or independent review;
- formal proof of implementation correctness;
- constant-time behavior, RNG quality, or memory safety;
- physical MCU resource viability;
- complete interoperability for unrepresented profiles/transports/lifecycle modes;
- RFC-class independent implementability;
- Common-Contract or deployment qualification.

Those claims require their separately declared evidence.

## 11. Current gap register

The repository still requires broader canonical coverage before TD-004/RFC-class qualification can close, including complete normative grammar/state-machine vectors, production negotiation/downgrade coverage, lifecycle/resumption/revocation traces, and complete Common-Contract cross-class evidence. Advanced/research-only mechanisms remain outside the mandatory corpus until explicitly promoted.

This contract governs how those vectors are added without allowing evidence drift or silent semantic replacement.