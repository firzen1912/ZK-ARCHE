# ProVerif AUTH run — `a73faa3dd842e577133ff3abb46cb1431df95010`

This artifact retains the first fail-closed ProVerif run of the synchronized ZK-ARCHE AUTH model after the CI parser/tooling defects were corrected. It is evidence for TD-003 and the FM-02/FM-03/FM-04/FM-05/FM-09 property set. It is **not** a claim of formal verification.

## Run identity

| Field | Value |
|---|---|
| Repository commit | `a73faa3dd842e577133ff3abb46cb1431df95010` |
| Branch | `dev` |
| GitHub Actions run | `33037366231` / run 15 |
| Formal job | `98402922430` |
| Tool | ProVerif `2.05` |
| Canonical model | `rust/models/proverif/zk_arche_auth_skeleton.pv` |
| Synchronized C mirror | `c/models/proverif/zk_arche_auth_skeleton.pv` |
| Model SHA-256 | `260ae30ae8452bf9e45813e47ac08869dabbbb823838bdc31cc6a8c6200031fe` |
| Retained workflow artifact | `formal-proverif-evidence`, GitHub Actions artifact id `9632538690` |
| Rust lane | PASS |
| C lane | PASS |
| Release qualification lane | PASS |
| Formal lane | FAIL — genuine false correspondence results retained |

The formal lane parsed and executed the model successfully. The failure is therefore a protocol/model result, not a parser, installation, synchronization, or evidence-pipeline failure.

## Query-result summary

The retained run produced the following result classes.

| Property/query family | Result | Interpretation boundary |
|---|---|---|
| FM-02 server AUTH_1 acceptance -> matching client AUTH_1 with same `session_id` and authenticated AUTH_1 context | **FALSE** | Active attacker can rewrite the public outer session identifier without changing the authenticated AUTH_1 material. |
| FM-04 `ServerAuth1Accepted` -> `ReplayRecorded` | **TRUE** | Accepted AUTH_1 is ordered after the modeled replay-record transition. This does not establish bounded-runtime persistence, restart, rollback, or eviction guarantees. |
| FM-02/FM-03 server completion -> client AUTH_3 with same `session_id` and KC context | **FALSE** | Current KC context does not authenticate the outer session identifier; an attacker can splice/relabel header session identifiers while preserving valid KC material. |
| FM-03 client completion -> matching server AUTH_2 with same `session_id` and KC context | **FALSE** | The modeled `AUTH_ACK` is a public unauthenticated value; a network attacker can synthesize the completion signal after observing/blocking protocol traffic. |
| FM-09 server completion -> pre-existing trusted client record | **TRUE** | Within this abstraction, successful server completion remains relative to pre-existing trust. This is not a proof of the custom role-membership primitive. |

No global `FORMALLY ANALYZED`, `FORMALLY VERIFIED`, Common Contract, or RFC-class claim is justified by this run.

## Counterexample A — outer `session_id` rebinding

The model and current implementations carry `session_id` in the transport-neutral packet header, but the current v2 client/server proof and KC transcript inputs do not cryptographically bind that value.

An active network attacker can therefore transform, conceptually:

```text
client sends:
  session_id = A
  AUTH_1 payload = P

attacker forwards to server:
  session_id = B
  AUTH_1 payload = P
```

The server can validate `P` because PID, nonce, ephemeral key, possession proof, role proof, and later KC material are unchanged. The attacker can similarly relabel the unauthenticated outer identifier on subsequent packets. The retained ProVerif trace therefore defeats correspondence queries that require both peers to agree on the same `session_id`.

This does **not** mean AUTH cryptographic possession is forged. It means the current v2 authenticated context is weaker than the roadmap's zk217 transcript-v3 target, which requires session/sequence identifiers and other security-relevant negotiation/context inputs to be unambiguously bound.

### Concrete implementation anchors

Rust:

- `rust/crates/proto/src/proto/auth.rs` creates a random outer `session_id` and passes it to packet framing.
- `rust/crates/proto/src/crypto.rs` / `kc_transcript_hash` derive KC context from AUTH proof/key material.
- `rust/crates/proto/src/transcript.rs` defines the current stable v2 transcript domains.
- The current `KcTranscriptParts` call in AUTH does not include the packet `session_id` or sequence.

C:

- `c/src/proto/proto.c` keeps `ctx->session_id` in packet framing and checks returned header equality at the client API boundary.
- `auth_kc_transcript_hash` is called with PID/proof/nonces/ephemeral/server material, not the outer `session_id`/sequence.

Existing FT-022 replay tests demonstrate that an already-accepted identical AUTH_1 payload is rejected under a fresh outer session while the replay key remains retained. That is useful replay evidence, but it does not cryptographically authenticate the header/session context of a first valid run.

## Counterexample B — unauthenticated `AUTH_ACK` completion

The current AUTH completion acknowledgement is not key-confirmed.

Rust v2 currently defines `SETUP_ACK` and `AUTH_ACK` as the same one-byte payload (`0x01`). After sending a valid client-finished tag, the client accepts that public ACK byte and returns the session key. The server sends the same public ACK only after validating the client-finished tag.

C mirrors the same semantic shape: `auth_client_handle_auth_ack` accepts `AUTH_PKT_AUTH_ACK` plus `auth_ack_decode(...)` and sets `auth_complete = 1`.

Under a full active network attacker, a public unauthenticated ACK can be synthesized. The attacker can block the client's `AUTH_3` from reaching the server and still send a syntactically valid `AUTH_ACK` to the client. The client can therefore report completion without evidence that the server accepted `AUTH_3`.

This is an **agreement/completion gap**, not a claim that the attacker learns the session key. It blocks a strong mutual-completion/key-confirmation claim.

## Required remediation contract

Do not "fix" these results by merely weakening correspondence queries. A protocol change or an explicitly narrowed security claim is required.

### R1 — transcript-v3 authenticated context

Owned by zk217 / FM-05 / TD-004.

A versioned future AUTH transcript must define which of the following are authenticated and why:

```text
protocol version
selected suite / method / profile
critical capability / extension selection
AUTH instance identity
session identifier where security-significant
sequence / flight identity where security-significant
client and server identities / commitments
nonces and ephemeral keys
role / authorization context
deployment / domain / audience
channel-binding label when used
canonical AUTH payload semantics
```

If `session_id` is intentionally only an unauthenticated routing hint, the normative specification must say so and formal agreement queries must instead bind a fresh authenticated AUTH-instance identifier. The current roadmap already anticipates such an identifier. Either design must prevent ambiguous cross-session/cross-instance security semantics.

Any wire/transcript change must be explicitly versioned and accompanied by deterministic Rust/C vectors and downgrade/negative tests. Do not silently redefine v2 transcript bytes.

### R2 — authenticated server completion

Owned by AUTH/LINK lifecycle and FM-03.

The client must not treat a forgeable public constant as proof that the server accepted `AUTH_3`. A versioned design should use an authenticated completion indication derived from the established KC/session secret and bound AUTH context, for example a distinct server-finished/ACK MAC with its own domain-separation label.

The exact construction requires reviewed specification and vectors; this artifact does not select a final primitive or packet encoding. Required properties are:

- only a peer with the established session/KC secret can produce the completion authenticator;
- the completion authenticator is domain-separated from `server finished` and `client finished`;
- it binds the same versioned authenticated context as the rest of AUTH;
- replay/cross-session/cross-protocol use fails closed;
- Rust and C accept/reject decisions are byte-compatible;
- negative vectors cover forged ACK, ACK from another session, changed authenticated context, duplicate/reordered completion, and missing server receipt of AUTH_3.

### R3 — formal and executable regression gate

Before FM-02/FM-03/FM-05 can advance:

1. update the synchronized model to the approved versioned context/completion design;
2. retain ProVerif output showing the targeted correspondences under the declared abstraction;
3. add Rust/C deterministic vectors for the new transcript/completion bytes;
4. add active-attacker negative tests for session/context rewriting and forged completion;
5. retain exact CI execution at one repository commit;
6. preserve the existing replay-state abstraction caveat: persistent ProVerif replay memory remains stronger than bounded volatile runtime state until replay epoch/restart semantics are separately resolved.

## Claim impact

After this run:

```text
FM-04 replay record ordering        scoped TRUE in model
FM-09 pre-existing-trust relation   scoped TRUE in model
FM-02 same-session agreement        FALSE
FM-03 mutual completion agreement   FALSE
FM-05 complete context integrity    NOT ESTABLISHED
TD-003                              OPEN
TD-004                              OPEN
COMMON-CONFORMANT                   NOT ESTABLISHED
RFC-CLASS DOCUMENTED                NOT ESTABLISHED
```

The false results are useful retained assurance evidence. They should remain visible until a versioned implementation/spec/model change removes the counterexamples or the protocol's normative security claim is deliberately narrowed with explicit justification.
