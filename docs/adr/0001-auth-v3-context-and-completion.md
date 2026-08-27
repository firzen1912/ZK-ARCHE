# ADR 0001 — Versioned AUTH v3 context binding and authenticated completion

- **Status:** proposed
- **Date:** 2026-08-26
- **Owners / reviewers:** repository maintainers; independent cryptographic reviewer required before stronger production claims
- **Related research:** `docs/research/daily/2026-08-26.md`, R-004, R-009
- **Related assurance:** `docs/assurance/formal-runs/2026-08-26-a73faa3-proverif-auth.md`, FM-02, FM-03, FM-05
- **Related roadmap:** zk207, zk217, zk218, zk225, zk226, zk240, TD-003, TD-004
- **Related spec / vectors:** `spec/zk-arche-protocol.md`, `spec/registries.md`, canonical Rust vectors under `rust/test-vectors/`

## Context

The first retained fail-closed ProVerif run of the synchronized AUTH model produced two genuine protocol-design counterexamples:

1. the current v2 key-confirmation transcript does not authenticate the outer 16-byte `session_id`, so an active attacker can relabel an otherwise valid AUTH exchange onto a different public session identifier without invalidating the current possession proofs or key-confirmation tags;
2. the current `AUTH_ACK` is a public one-byte constant, so an active attacker can block `AUTH_3` and synthesize the ACK seen by the client. The attacker does not learn the session key, but the client can report completion without cryptographic evidence that the server accepted the client-finished tag.

The current Rust implementation confirms both boundaries. `KcTranscriptParts` covers PID, client proof, nonces, ephemeral keys, server public key, and server proof, but not the packet `session_id`, negotiated version/suite/profile/capabilities, deployment/audience context, or channel-binding context. `handle_auth_3()` returns the public `encode_ack()` payload after checking the client-finished tag.

This is narrower than the zk217 target, which requires complete authenticated context and explicit downgrade-resistant version/profile/capability semantics.

The repair must not silently redefine existing v2 transcript bytes. ZK-ARCHE requires Rust/C byte compatibility, retained vectors, downgrade tests, and checkpoint review for transcript/domain-separation or wire changes.

### Standards/reference lessons

This ADR uses external protocols only as design references:

- TLS 1.3 (current RFC 9846, superseding RFC 8446) treats Finished as a MAC over the authenticated handshake transcript under a purpose-derived key and requires successful verification before the handshake is considered mutually complete.
- EDHOC (RFC 9528) evolves transcript hashes across authenticated handshake messages and binds selected cryptographic/authentication context into subsequent MAC/signature calculations.
- DTLS 1.3 (RFC 9147) illustrates that not every transport/record-layer sequence field belongs in the cryptographic handshake transcript; transport ordering state and authenticated handshake context can remain separate when the state machine defines their responsibilities precisely.

ZK-ARCHE therefore should authenticate the security-significant AUTH instance and negotiated security context without blindly MACing every transport-adapter field.

## Decision

This ADR proposes a **versioned AUTH v3** design. It is not accepted normative behavior until reviewed and promoted into `spec/` with vectors/tests.

### 1. Preserve v2 exactly

- Protocol version `0x02`, current transcript domain labels, current AUTH payload bytes, and current one-byte ACK semantics remain unchanged for v2 compatibility.
- No implementation may reinterpret a v2 packet using v3 transcript or completion rules.
- Strong same-session/mutual-completion claims remain unavailable for v2 unless separate evidence justifies a narrower claim.

### 2. Use protocol version `0x03` for the new AUTH semantics

AUTH v3 is selected only when HELLO/version negotiation selects protocol version `0x03` and both peers support the corresponding mandatory AUTH-v3 capability/profile requirements.

The existing packet type values `AUTH_1=0x21`, `AUTH_2=0x22`, `AUTH_3=0x23`, and `AUTH_ACK=0x24` may be retained because the packet header version disambiguates v2 and v3 payload semantics. No packet type is reassigned.

A v3 peer must fail closed if the negotiated version/security context used by AUTH differs from the context authenticated in the v3 transcript.

### 3. Make the existing `session_id` the authenticated AUTH-instance identifier

The existing 16-byte random `session_id` becomes security-significant for AUTH v3 and is included in the authenticated transcript.

This avoids adding another per-handshake identifier to constrained packets while directly closing the retained FM-02 session-rebinding counterexample.

For v3:

- the initiator generates the 16-byte `session_id` once for the AUTH instance;
- all AUTH flights for that instance use the same value;
- both peers include exactly those 16 bytes in the v3 key-confirmation transcript;
- relabeling a packet to a different `session_id` must cause key-confirmation/completion verification to fail.

The outer `seq` remains state-machine/retransmission metadata rather than part of the cryptographic transcript unless later evidence shows a security need to promote it. Implementations still must enforce legal message order, duplicate/retransmission rules, and expected sequence transitions. This separation follows the general TLS/DTLS discipline of distinguishing handshake-authenticated context from transport/record sequencing metadata.

### 4. Define a canonical AUTH security-context block

Before AUTH v3 is enabled, negotiation/spec work must provide one canonical serialization of the security context. The v3 transcript binds the following fields in this order:

```text
protocol_version          u8          (= 0x03)
suite_id                  u16 LE
profile_id                u16 LE
selected_capabilities     u64 LE
session_id                16 bytes
authz_context_hash        32 bytes
critical_extensions_hash  32 bytes
channel_binding_hash      32 bytes
```

Definitions:

- `selected_capabilities` is the mutually selected/active security-relevant capability bitset, not merely either peer's raw advertisement;
- `authz_context_hash` is a SHA-256 digest of the canonical authorization context that applies to this AUTH instance, including role/policy scope and, when present, deployment/domain/audience and relevant policy/authorization epoch values;
- `critical_extensions_hash` is SHA-256 over the canonical ordered set of negotiated critical extensions and their security-relevant values; use the hash of the empty canonical set when none are active;
- `channel_binding_hash` is SHA-256 over the normative channel-binding context for a bound profile; use the profile-defined empty value only when the selected profile explicitly has no channel binding.

The exact canonical encoding of the three hashed subcontexts belongs in the normative spec/registries and must have positive/negative vectors before AUTH v3 is advertised.

### 5. Define `KC-TRANSCRIPT-v3`

`KC-TRANSCRIPT-v3` extends, rather than mutates, the current v2 key-confirmation transcript.

The transcript uses a new domain separator (conceptually `zk-arche/kc/v3`) and appends:

```text
protocol_version
suite_id
profile_id
selected_capabilities
session_id
authz_context_hash
critical_extensions_hash
channel_binding_hash
pid
a_c
s_c
nonce_c
eph_c
server_pub
a_s
s_s
nonce_s
eph_s
```

Field labels, integer endianness, point/scalar canonical encoding, and omission rules must be fixed by the specification. A field listed above is never omitted; an empty optional subcontext is represented by its specified canonical empty hash.

This transcript is the input to all v3 key-confirmation/completion derivations.

### 6. Version and separate key-confirmation keys

AUTH v3 derives three purpose-separated 32-byte keys from the established session key and `TH_v3 = Hash(KC-TRANSCRIPT-v3)`:

```text
k_s2c_v3        HKDF-Expand(..., "kc s2c v3", 32)
k_c2s_v3        HKDF-Expand(..., "kc c2s v3", 32)
k_complete_v3   HKDF-Expand(..., "kc complete s2c v3", 32)
```

The existing v2 labels remain unchanged and are never reused as v3 labels.

### 7. Keep directional finished tags, but version their domains

For v3:

```text
tag_s = HMAC(k_s2c_v3, "server finished v3" || TH_v3)
tag_c = HMAC(k_c2s_v3, "client finished v3" || TH_v3)
```

The server sends `tag_s` in `AUTH_2`. The client verifies it before sending `AUTH_3`. The client sends `tag_c` in `AUTH_3`. The server verifies it before producing completion.

### 8. Replace public `AUTH_ACK` with authenticated server completion

Under protocol version `0x03`, the `AUTH_ACK` payload is no longer the v2 one-byte public constant. It carries a 32-byte server-completion authenticator.

Define:

```text
completion_hash = SHA-256(
    "zk-arche/auth-complete/v3" ||
    TH_v3 ||
    tag_c
)

tag_ack = HMAC(
    k_complete_v3,
    "server complete v3" || completion_hash
)
```

`AUTH_ACK-v3.payload = tag_ack` (32 bytes).

Rules:

- the server computes/sends `tag_ack` only after successfully receiving and verifying the matching `AUTH_3.tag_c` for the same authenticated `session_id`/context;
- the client reports AUTH completion only after verifying `tag_ack` in constant time;
- a v2 one-byte ACK is invalid in a v3 session;
- an ACK from another session/context/version/profile must fail verification;
- completion authentication is domain-separated from both `server finished` and `client finished`.

This closes the retained FM-03 forgeable-completion counterexample without introducing a new cryptographic primitive.

### 9. Completion state is stronger than key derivation state

AUTH v3 state machines distinguish:

```text
keys_derived
server_finished_verified
client_finished_sent
server_client_finished_verified
mutually_complete
```

The client may possess a derived session key before `mutually_complete`, but application logic claiming mutual AUTH completion must not use that state until the authenticated `AUTH_ACK-v3` is verified.

The server enters its completed state only after validating `AUTH_3` and creating the authenticated completion response.

### 10. Downgrade behavior is fail-closed

A peer that negotiated v3 must not silently execute v2 AUTH semantics.

Negative conformance must cover at least:

- version 3 HELLO context followed by version 2 AUTH packets;
- changed suite/profile/capability selection after negotiation;
- v3 transcript evaluated with v2 domain labels;
- v2 public ACK presented in v3;
- v3 completion tag presented under a different `session_id`;
- stripping/changing a critical extension or channel-binding context;
- changed authorization/deployment/audience context;
- unknown critical context that cannot be canonicalized.

## Alternatives considered

### A. Weaken the formal queries and keep v2 unchanged

Rejected as the target architecture. This would preserve compatibility but knowingly retain weaker same-session and mutual-completion semantics than zk217 requires. v2 remains available only as a legacy compatibility behavior with appropriately narrow claims.

### B. Add a second AUTH-instance nonce inside AUTH_1

Not selected initially. The existing 16-byte random `session_id` already provides an instance identifier and costs no additional constrained-wire bytes. A separate authenticated instance nonce should be introduced only if future transport-adapter requirements show that `session_id` cannot safely serve both routing and authenticated-instance roles.

### C. MAC every outer header field, including `seq` and flags

Not selected. Sequence/retransmission metadata has transport/state-machine responsibilities and may legitimately differ during retransmission handling. Binding the security-significant session identifier and canonical negotiated context is sufficient for the retained counterexample while avoiding unnecessary coupling between the Common Contract and transport adapter mechanics. Any later promotion of `seq`/flags into authenticated context requires its own evidence and versioned decision.

### D. Reuse `k_s2c` for completion with a different label

Possible, but not selected. Deriving a dedicated `k_complete_v3` keeps server-finished and post-`AUTH_3` completion purposes structurally separate at minimal implementation cost and makes review/formal reasoning simpler.

### E. Encrypt the ACK instead of MACing it

Not required for the current problem. The ACK carries no confidential application data; authenticity and context binding are the required properties. A fixed-size MAC minimizes wire/parser complexity for constrained peers.

## Security and privacy consequences

Expected security improvements after implementation/evidence:

- prevents active relabeling of a valid AUTH instance onto another `session_id` without detection;
- binds negotiated version/suite/profile/capability and authorization/channel context into key confirmation;
- prevents a network attacker from synthesizing successful server completion without the derived completion key;
- strengthens downgrade/cross-profile/cross-context separation;
- gives formal correspondence queries a concrete authenticated AUTH-instance identifier.

Residual/explicit boundaries:

- this ADR does not prove the custom role-membership proof;
- this ADR does not solve replay-state persistence/eviction/restart semantics from R-009;
- `session_id` remains visible metadata and does not itself provide unlinkability;
- any profile/deployment/audience identifiers included in canonical context must be reviewed for metadata/fingerprinting consequences even though only their hashes enter the KC transcript;
- formal analysis of the proposed construction is still required; the proposal must not be called secure merely because it resembles TLS/EDHOC patterns.

## IoT / implementation consequences

Expected constrained impact is deliberately small:

- no extra AUTH-instance field if the existing 16-byte `session_id` is reused;
- `AUTH_ACK` grows from 1 byte to 32 bytes in v3;
- one additional 32-byte derived completion key while AUTH is in progress;
- three additional fixed 32-byte context hashes plus small negotiated scalar fields in transcript input/state, with implementation freedom to stream transcript hashing rather than retain all bytes;
- no new asymmetric operation, curve, signature, or general-purpose ZK primitive;
- HMAC/HKDF/SHA-256 reuse the existing mandatory-suite primitives;
- Rust and C require byte-identical canonical context construction and tags.

Memory-sensitive implementations may erase `k_s2c_v3`, `k_c2s_v3`, temporary transcript state, and `k_complete_v3` as soon as their final dependent verification/response step completes.

## Compatibility and migration

Classification: **versioned / intentionally incompatible AUTH semantics**.

Migration sequence:

1. accept/review this ADR;
2. define protocol/profile/capability/context registries and canonical encodings;
3. add deterministic Rust canonical vectors for v3 transcript, three KC keys, finished tags, completion hash, and ACK tag;
4. make C consume/reproduce those vectors;
5. implement v3 as side-by-side code paths while preserving v2 bytes;
6. add HELLO/version/capability selection and fail-closed downgrade tests;
7. add active-attacker negative tests for session relabeling and forged/cross-session ACK;
8. update synchronized ProVerif model and retain results for FM-02/FM-03/FM-05;
9. only then consider making v3 part of a mandatory Common Contract profile.

No implementation should advertise v3 until the canonical context registry/encoding, deterministic vectors, and both Rust/C verification paths exist.

## Evidence required

Before this proposal can support stronger claims, retain at least:

### Specification / registry

- accepted ADR or superseding decision;
- normative v3 AUTH grammar/state machine;
- protocol-version, profile, capability, and extension registry entries;
- exact canonical serialization for all v3 context fields;
- Security and Privacy Considerations updates;
- explicit v2/v3 compatibility and downgrade rules.

### Deterministic interoperability

- Rust-generated v3 transcript bytes/hash;
- Rust-generated `k_s2c_v3`, `k_c2s_v3`, `k_complete_v3`;
- Rust-generated `tag_s`, `tag_c`, `completion_hash`, and `tag_ack`;
- exact C reproduction of every value;
- mutation vectors for every context field.

### Negative / state-machine tests

- session-id rewrite after AUTH_1 creation;
- suite/profile/capability downgrade/rewrite;
- authorization/deployment/audience context mismatch;
- forged public/zero/random ACK;
- valid ACK replayed from another session;
- changed `tag_c` with otherwise valid context;
- ACK delivered before server receipt/validation of AUTH_3;
- duplicate/reordered completion behavior;
- v2/v3 cross-protocol packet confusion.

### Formal assurance

- synchronized ProVerif model updated to v3 context/completion semantics;
- retained exact-tool output for FM-02 same-instance agreement;
- retained exact-tool output for FM-03 mutual completion;
- retained exact-tool output for FM-05 complete authenticated context;
- counterexamples retained and dispositioned rather than hidden by weaker queries;
- replay-table abstraction caveat retained separately until R-009 is resolved.

### External / review

- checkpoint review of transcript/domain-separation construction;
- independent cryptographic review remains required for TD-001 and for any strong production claim involving the custom role proof;
- target resource measurements before v3 is claimed suitable for constrained profiles.

## Promotion gate

This ADR is **proposed**, not accepted. It records the smallest versioned design that addresses the retained FM-02/FM-03 counterexamples while respecting the Common Contract, constrained-device, WireGuard-like simplicity, and RFC-class evidence disciplines.

Promotion to `accepted` requires explicit human/reviewer approval plus normative spec/registry work. Implementation may prototype the proposed design behind non-advertised/internal test paths, but production capability negotiation must not advertise AUTH v3 until the evidence listed above exists.
